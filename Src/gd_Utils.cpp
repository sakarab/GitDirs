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

        for ( git2::BranchInfo branch : branch_list )
        {
            if ( branch.Type() != GIT_BRANCH_LOCAL )
                continue;

            for ( std::string name : remotes_list )
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

    void GetDirectoryState( git2::LibGit2& libgit, GitDirStateItem& state_item )
    {
        libgit.OpenRepository( ccwin::NarrowStringStrict( state_item.Directory ).c_str() );
        BOOST_SCOPE_EXIT( &libgit )     { libgit.CloseRepository(); }       BOOST_SCOPE_EXIT_END;

        state_item.Branch = libgit.GetCurrentBranch();
        GetDirectoryStateUncommited( libgit, state_item );
        GetDirectoryStateNeeds( libgit, state_item );
    }
}

std::wstring GetIniFileName()
{
    ccwin::TCommonDirectories   dirs;

    return ccwin::IncludeTrailingPathDelimiter( dirs.AppDataDirectory_UserLocal() ).append( L"GitDirs.ini" );
}

GitDirList ReadFolderList()
{
    ccwin::TIniFile             ini( GetIniFileName() );
    ccwin::TStringList          slist;

    ini.ReadSectionKeys( L"GitDirs", slist );

    GitDirList                  result;

    for ( int n = 0, eend = slist.Count() ; n < eend ; ++n )
        result.push_back( GitDirItem( slist[n], ini.ReadString( L"GitDirs", slist[n].c_str(), L"" ) ) );
    return result;
}

std::wstring MakeCommand( const wchar_t *command, const wchar_t *path )
{
    return boost::str( boost::wformat( L"TortoiseGitProc.exe /command:%1% /path:%2%" ) % command % path );
}

void GitGetRepositoriesState( GitDirStateList& state_list )
{
    git2::LibGit2     libgit;

    for ( GitDirStateList::value_type& item : state_list )
        GetDirectoryState( libgit, item );
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
