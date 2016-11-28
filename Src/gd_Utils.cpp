#include "stdafx.h"
#include "gd_Utils.h"
#include <winUtils.h>
#include <winOSUtils.h>
#include <winClasses.h>

ccwin::TStringList ReadFolderList()
{
    ccwin::TIniFile     ini( ccwin::ChangeFileExt( ccwin::getAPPName(), L".ini" ) );
    ccwin::TStringList  slist;

    ini.ReadSectionKeys( L"GitPaths", slist );
    return slist;
}
