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

struct GitDirStateItem
{
    std::wstring    Directory;
    std::string     Branch;
    bool            Uncommited;
    GitDirStateItem( const std::wstring& dir )
        : Directory( dir ), Branch(), Uncommited()
    {}
};

typedef std::vector<GitDirItem>         GitDirList;
typedef std::vector<GitDirStateItem>    GitDirStateList;

GitDirList ReadFolderList();
std::wstring MakeCommand( const wchar_t *command, const wchar_t *path );
void GitGetRepositoriesState( GitDirStateList& state_list );
std::wstring GetIniFileName();

//=======================================================================
//==============    LibGit2
//=======================================================================
class LibGit2
{
private:
    git_repository      *mRepository;

    void Check( int git_error_code );
    void CheckOpen();
    // noncopyable
    LibGit2( const LibGit2& other ) CC_EQ_DELETE;
    LibGit2& operator=( const LibGit2& other ) CC_EQ_DELETE;
public:
    LibGit2();
    ~LibGit2();

    void OpenRepository( const char *path );
    void CloseRepository();

    std::string GetCurrentBranch();
    git_status_list * GetStatusList( git_status_options& options );
};

#endif
