#include "stdafx.h"
#include "gd_Utils.h"
#include <winUtils.h>
#include <winOSUtils.h>
#include <winClasses.h>
#include <boost/format.hpp>
#include <smException.h>

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

void LibGit2::OpenRepository( const char * path )
{
    git_repository_open_ext( &mRepository, path, 0, NULL );
}

void LibGit2::CloseRepository()
{
    if ( mRepository )
        git_repository_free( mRepository );
}
