This patch enables this abilities.

1. Removes exports when compiling as a static library.  
File:`"common.h"`  
Action: remove `git2_EXPORTS` #define
2. Enables the compilation of Windows XP(TM) compatible library.  
Files: `"path.c"` and `"utf-conv.h"`  
Action: #define `WINDOWS_XP_BUILD`
3. Enables the compilation with older than Visual Studio 2010 compilers.  
File:`"win32-compat.h"`  
Tested only with Visual Studio 2008

This patch is generated based on `v0.24.3` tag of libgit2 with TortoiseGit (Create Patch Serial...).
