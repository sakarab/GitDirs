//***************************************************************************
// GitDirs - git repositories directory
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Please read the "LICENSE" file for more copyright and license
// information.
//***************************************************************************

#include "stdafx.h"
#include "gd_Utils.h"
#include <winUtils.h>
#include <winOSUtils.h>
#include <winClasses.h>
#include <win_str.h>
#include <boost/format.hpp>
#include <boost/scope_exit.hpp>
#include <smException.h>

namespace
{
    void GetDirectoryStateNeeds( git2::LibGit2& libgit, GitDirStateItem& state_item )
    {
        // for each local branch
        // for each remote
        // constract the remote-branch name and find it
        // if found git_revwalk() back and forth to see differances

        std::vector<git2::BranchInfo>   branch_list = libgit.ListBranches();
        std::vector<std::string>        remotes_list = libgit.ListRemotes();

        state_item.NRepos = remotes_list.size();
        for ( git2::BranchInfo& branch : branch_list )
        {
            if ( branch.Type() != GIT_BRANCH_LOCAL )
                continue;

            for ( std::string& name : remotes_list )
            {
                std::string     remote_name = boost::str( boost::format( "%1%/%2%" ) % name % branch.Name() );

                std::vector<git2::BranchInfo>::iterator     it =
                    std::find_if( branch_list.begin(), branch_list.end(), [&remote_name]( const git2::BranchInfo& branch )->bool {
                    return remote_name == branch.Name();
                } );

                if ( it != branch_list.end() )
                {
                    std::string     ref_local  = boost::str( boost::format( "refs/heads/%1%" ) % branch.Name() );
                    std::string     ref_remote = boost::str( boost::format( "refs/remotes/%1%" ) % it->Name() );

                    state_item.NeedsUpdate = libgit.RevisionCount( ref_local, ref_remote ) != 0;
                    if ( !state_item.NeedsUpdate )
                        state_item.NeedsUpdate = libgit.RevisionCount( ref_remote, ref_local ) != 0;
                    if ( state_item.NeedsUpdate )
                        return;
                }
            }
        }
    }

    void GetDirectoryStateUncommited( git2::LibGit2& libgit, GitDirStateItem& state_item )
    {
        git_status_options      statusopt;

        statusopt.pathspec.strings = nullptr;
        statusopt.pathspec.count = 0;
        statusopt.version = GIT_STATUS_OPTIONS_VERSION;
        statusopt.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        statusopt.flags = GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
            GIT_STATUS_OPT_SORT_CASE_SENSITIVELY |
            GIT_STATUS_OPT_INCLUDE_UNTRACKED;
        //GIT_STATUS_OPT_INCLUDE_IGNORED

        git_status_list         *status = libgit.GetStatusList( statusopt );
        BOOST_SCOPE_EXIT( &status ) { git_status_list_free( status ); }       BOOST_SCOPE_EXIT_END;

        if ( status )
        {
            for ( size_t n = 0, maxi = git_status_list_entrycount( status ) ; n < maxi ; ++n )
            {
                state_item.Uncommited = git_status_byindex( status, n )->status != GIT_STATUS_CURRENT;
                if ( state_item.Uncommited )
                    break;
            }
        }
    }

    void UpgradeDB( const std::wstring& ini_fname )
    {
        static bool         upgraded = false;

        if ( upgraded )
            return;

        ccwin::TIniFile     ini( ini_fname );
        int                 version = ini.ReadInteger( L"Version", L"Version", 0 );

        while ( version < LastDataVersion )
        {
            switch ( version )
            {
                case 0 :
                {
                    std::wstring    marks = ini.ReadString( IniSections::ViewState, L"Marks", L"" );

                    if ( !marks.empty() )
                    {
                        ini.WriteString( IniSections::Data, IniKeys::Data_Marks, marks.c_str() );
                        ini.EraseKey( IniSections::ViewState, L"Marks" );
                    }
                    break;
                }
            }
            ++version;
            ini.WriteInteger( L"Version", L"Version", version );
        }
        upgraded = true;
    }

}

//=======================================================================
//==============    IniStrings
//=======================================================================
const wchar_t * IniSections::Repositories = L"Repositories";
const wchar_t * IniSections::Repositories_Groups = L"Repositories_Groups";
const wchar_t * IniSections::ViewState = L"ViewState";
const wchar_t * IniSections::Data = L"Data";
const wchar_t * IniKeys::ViewState_SortColumn = L"SortColumn";
const wchar_t * IniKeys::ViewState_Group = L"Group";
const wchar_t * IniKeys::ViewState_ShowCheckBoxes = L"ShowCheckBoxes";
const wchar_t * IniKeys::Data_Marks = L"Marks";
const wchar_t * IniKeys::Data_AllGroups = L"AllGroups";

//=======================================================================
//==============    FREE FUNCTIONS
//=======================================================================
std::wstring GetIniFileName()
{
    ccwin::TCommonDirectories   dirs;
    std::wstring                result = ccwin::IncludeTrailingPathDelimiter( dirs.AppDataDirectory_UserLocal() ).append( L"GitDirs.ini" );

    UpgradeDB( result );
    return result;
}

WStringList DelimitedTextToList( const std::wstring& text, const wchar_t delimiter )
{
    ccwin::TStringList      slist;
    WStringList             result;

    slist.DelimitedText( text, delimiter );
    for ( int n = 0, eend = slist.Count(); n < eend; ++n )
        result.push_back( slist[n] );
    return result;
}

std::wstring ListToDelimitedText( const WStringList & list, const wchar_t delimiter )
{
    ccwin::TStringList          slist;

    for ( const std::wstring& sstr : list )
        slist.Add( sstr );
    return slist.DelimitedText( delimiter );
}

std::wstring MakeCommand( const wchar_t *command, const wchar_t *path )
{
    return boost::str( boost::wformat( L"TortoiseGitProc.exe /command:%1% /path:%2%" ) % command % path );
}

void Throw_NoUniqueName( const std::wstring& name )
{
    std::string     msg = boost::str( boost::format( "There already is an entry with the name '%1%'" ) % ccwin::NarrowStringStrict( name ) );

    throw cclib::BaseException( msg );
}

void GetDirectoryState( git2::LibGit2& libgit, GitDirStateItem& state_item )
{
    libgit.OpenRepository( ccwin::NarrowStringStrict( state_item.Directory ).c_str() );
    BOOST_SCOPE_EXIT( &libgit ) { libgit.CloseRepository(); }       BOOST_SCOPE_EXIT_END;

    state_item.Branch = libgit.GetCurrentBranch();
    GetDirectoryStateUncommited( libgit, state_item );
    GetDirectoryStateNeeds( libgit, state_item );
}

bool ToggleMenuCheck( CMenuHandle menu, int position )
{
    bool    result = !GetMenuCheck( menu, position );

    SetMenuCheck( menu, position, result );
    return result;
}

bool GetMenuCheck( CMenuHandle menu, int position )
{
    MENUITEMINFO            info;

    info.cbSize = sizeof( MENUITEMINFO );
    info.fMask = MIIM_STATE;
    menu.GetMenuItemInfo( position, TRUE, &info );
    return info.fState == MFS_CHECKED;
}

void SetMenuCheck( CMenuHandle menu, int position, bool value )
{
    MENUITEMINFO            info;

    info.cbSize = sizeof( MENUITEMINFO );
    info.fMask = MIIM_STATE;
    info.fState = value ? MFS_CHECKED : MFS_UNCHECKED;
    menu.SetMenuItemInfo( position, TRUE, &info );
}

void SetMenuRadio( CMenuHandle menu, int position )
{
    int     menu_count = menu.GetMenuItemCount();

    for ( int n = 0 ; n < menu_count ; ++n )
        SetMenuCheck( menu, n, position == n );
}

//=======================================================================
//==============    GitDirItem
//=======================================================================
GitDirItem::GitDirItem( const std::wstring& name, const std::wstring& dir, const std::wstring& groups )
    : mName( name ), mDirectory( dir ), mGroups( DelimitedTextToList( groups, L',' ) )
{
}

void GitDirItem::AddToGroup( const std::wstring& group )
{
    mGroups.push_back( group );
}

void GitDirItem::RemoveFromGroup( const std::wstring& group )
{
    WStringList::iterator   it = std::find( mGroups.begin(), mGroups.end(), group );

    if ( it != mGroups.end() )
        mGroups.erase( it );
}

bool GitDirItem::InGroup( const std::wstring& group )
{
    return std::find( mGroups.begin(), mGroups.end(), group ) != mGroups.end();
}

namespace git2
{
    //=======================================================================
    //==============    LibGit2
    //=======================================================================
    LibGit2::LibGit2()
        : mRepository()
    {
        git_libgit2_init();
    }

    LibGit2::~LibGit2()
    {
        CloseRepository();
        git_libgit2_shutdown();
    }

    void LibGit2::Check( int git_error_code )
    {
        if ( !git_error_code )
            return;

        const git_error     *lg2err;

        if ( (lg2err = giterr_last()) != nullptr && lg2err->message != nullptr )
            throw cclib::BaseException( boost::str( boost::format( "libgit2 error [%1%]\n%2%" ) % git_error_code % lg2err->message ) );
        else
            throw cclib::BaseException( boost::str( boost::format( "libgit2 unknown error [%1%]" ) % git_error_code ) );
    }

    void LibGit2::CheckOpen()
    {
        if ( !mRepository )
            throw cclib::BaseException( "Repository not opened." );
    }

    void LibGit2::OpenRepository( const char *path )
    {
        if ( mRepository )
            throw cclib::BaseException( "Repository already opened." );

        git_repository      *rep = nullptr;

        Check( git_repository_open_ext( &rep, path, 0, NULL ) );
        mRepository = std::move( RepositoryHolder( rep, []( git_repository *rep ) { git_repository_free( rep ); } ) );
    }

    void LibGit2::CloseRepository()
    {
        mRepository.reset();
    }

    std::string LibGit2::GetCurrentBranch()
    {
        CheckOpen();

        git_reference   *head = nullptr;
        int             error = git_repository_head( &head, mRepository.get() );
        std::string     result;

        if ( error && error != GIT_EUNBORNBRANCH && error != GIT_ENOTFOUND )
            Check( error );
        else
        {
            result = git_reference_shorthand( head );
            git_reference_free( head );
        }
        return result;
    }

    git_status_list * LibGit2::GetStatusList( git_status_options& options )
    {
        CheckOpen();

        git_status_list     *result = nullptr;

        Check( git_status_list_new( &result, mRepository.get(), &options ) );
        return result;
    }

    std::vector<BranchInfo> LibGit2::ListBranches()
    {
        CheckOpen();

        std::vector<BranchInfo>     result;
        git_branch_iterator         *it;

        Check( git_branch_iterator_new( &it, mRepository.get(), GIT_BRANCH_ALL ) );
        BOOST_SCOPE_EXIT( it ) { git_branch_iterator_free( it ); }         BOOST_SCOPE_EXIT_END;

        int     gret;

        do
        {
            git_reference   *ref;
            git_branch_t    branch_type;

            gret = git_branch_next( &ref, &branch_type, it );
            if ( gret == 0 )
            {
                const char      *name;

                Check( git_branch_name( &name, ref ) );
                result.push_back( BranchInfo( std::string( name ), branch_type ) );
                git_reference_free( ref );
            }
        }
        while ( gret == 0 );
        if ( gret != GIT_ITEROVER )
            Check( gret );
        return result;
    }

    std::vector<std::string> LibGit2::ListRemotes()
    {
        CheckOpen();

        std::vector<std::string>    result;
        git_strarray                list;

        Check( git_remote_list( &list, mRepository.get() ) );
        BOOST_SCOPE_EXIT( list ) { git_strarray_free( &list ); }      BOOST_SCOPE_EXIT_END;

        for ( size_t n = 0 ; n < list.count ; ++n )
            result.push_back( list.strings[n] );
        return result;
    }

    size_t LibGit2::RevisionCount( const std::string& src, const std::string& dst )
    {
        CheckOpen();

        git_revwalk         *walker = nullptr;

        Check( git_revwalk_new( &walker, mRepository.get() ) );
        BOOST_SCOPE_EXIT( walker ) { git_revwalk_free( walker ); }         BOOST_SCOPE_EXIT_END;

        Check( git_revwalk_push_ref( walker, dst.c_str() ) );
        Check( git_revwalk_hide_ref( walker, src.c_str() ) );

        git_oid     id;
        size_t      count = 0;

        while ( !git_revwalk_next( &id, walker ) )
            ++count;
        return count;
    }

}
// namespace git2
