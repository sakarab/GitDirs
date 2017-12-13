#ifndef RC_INVOKED
#pragma once
#endif

#define RC_TO_STRING_SUB(a)     #a
#define RC_TO_STRING(a)         RC_TO_STRING_SUB(a)

#define SReleaseDescr   "Release Build "
#define IMajor          1
#define IMinor          1
#define IRelease        0
#define IBuild          20

#define SMajor          RC_TO_STRING(IMajor)
#define SMinor          RC_TO_STRING(IMinor)
#define SRelease        RC_TO_STRING(IRelease)
#define SBuild          RC_TO_STRING(IBuild)

#define FileVer 	    IMajor,IMinor,IRelease,IBuild
#define ProductVer 	    IMajor,IMinor,IRelease,IBuild
#define QFileVer        SMajor "." SMinor "." SRelease "." SBuild
#define QProductVer     SMajor "." SMinor "." SRelease "." SBuild
#define QFileVerZ       SMajor "." SMinor "." SRelease "." SBuild "\0"
#define QProductVerZ    SMajor "." SMinor "." SRelease "." SBuild "\0"
#define Translation     0x408, 1253
#define QBuild 		    SReleaseDescr " " SRelease "\0"
