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
    void GetDirectoryState( LibGit2& libgit, GitDirStateItem& state_item )
    {
        libgit.OpenRepository( ccwin::NarrowStringStrict( state_item.Directory ).c_str() );
        BOOST_SCOPE_EXIT( &libgit )     { libgit.CloseRepository(); }       BOOST_SCOPE_EXIT_END;

        state_item.Branch = libgit.GetCurrentBranch();

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
        BOOST_SCOPE_EXIT( &status )     { git_status_list_free( status ); }       BOOST_SCOPE_EXIT_END;

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
}

GitDirList ReadFolderList()
{
    ccwin::TCommonDirectories   dirs;
    ccwin::TIniFile             ini( ccwin::IncludeTrailingPathDelimiter( dirs.AppDataDirectory_UserLocal() ).append( L"GitDirs.ini" ) );
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
    LibGit2     libgit;

#if defined (CC_HAVE_RANGE_FOR)
    for ( GitDirStateList::value_type& item : state_list )
        GetDirectoryState( libgit, item );
#else
    for ( GitDirStateList::iterator it = state_list.begin(), eend = state_list.end() ; it != eend ; ++it )
        GetDirectoryState( libgit, *it );
#endif
}

//=======================================================================
//==============    LibGit2
//=======================================================================
LibGit2::LibGit2()
    : mRepository(nullptr)
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
    Check( git_repository_open_ext( &mRepository, path, 0, NULL ) );
}

void LibGit2::CloseRepository()
{
    if ( mRepository )
    {
        git_repository_free( mRepository );
        mRepository = nullptr;
    }
}

std::string LibGit2::GetCurrentBranch()
{
    CheckOpen();

    git_reference   *head = nullptr;
    int             error = git_repository_head( &head, mRepository );
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

    Check( git_status_list_new( &result, mRepository, &options ) );
    return result;
}
