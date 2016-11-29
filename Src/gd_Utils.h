#pragma once

#if !defined(GITDIRS_GD_UTILS_H)
#define GITDIRS_GD_UTILS_H

#include <string>
#include <vector>

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

#endif
