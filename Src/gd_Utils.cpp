#include "stdafx.h"
#include "gd_Utils.h"
#include <winUtils.h>
#include <winOSUtils.h>
#include <winClasses.h>
#include <boost/format.hpp>

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