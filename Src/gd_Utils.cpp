#include "stdafx.h"
#include "gd_Utils.h"
#include <winUtils.h>
#include <winOSUtils.h>
#include <winClasses.h>

ccwin::TStringList ReadFolderList()
{
    ccwin::TCommonDirectories   dirs;
    ccwin::TIniFile             ini( ccwin::IncludeTrailingPathDelimiter( dirs.AppDataDirectory_UserLocal() ).append( L"GitDirs.ini" ) );
    ccwin::TStringList          slist;

    ini.ReadSectionKeys( L"GitDirs", slist );
    return slist;
}
