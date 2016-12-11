#pragma once

#if !defined(GITDIRS_GD_UTILS_H)
#define GITDIRS_GD_UTILS_H

#include <predef_cc.h>
#include <string>
#include <vector>
#include <git2.h>

struct GitDirItem
{
    std::wstring    Name;
    std::wstring    Directory;
    GitDirItem( const std::wstring& name, const std::wstring& dir )
        : Name( name ), Directory( dir )
    {
    }
};

typedef std::vector<GitDirItem>     GitDirList;

GitDirList ReadFolderList();
std::wstring MakeCommand( const wchar_t *command, const wchar_t *path );

//=======================================================================
//==============    LibGit2
//=======================================================================
class LibGit2
{
private:
    git_repository      *mRepository;

    void Check( int git_error_code );
    // noncopyable
    LibGit2( const LibGit2& other ) CC_EQ_DELETE;
    LibGit2& operator=( const LibGit2& other ) CC_EQ_DELETE;
public:
    LibGit2();
    ~LibGit2();

    void OpenRepository( const char *path );
    void CloseRepository();
};

#endif
