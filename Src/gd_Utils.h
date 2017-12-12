//***************************************************************************
// GitDirs - git repositories directory
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Please read the "LICENSE" file for more copyright and license
// information.
//***************************************************************************

#if !defined(GITDIRS_GD_UTILS_H)
#define GITDIRS_GD_UTILS_H

#include <predef_cc.h>
#include <string>
#include <vector>
#include <git2.h>

typedef std::vector<std::wstring>   WStringList;

const int       LastDataVersion = 1;
const int       GroupMenuCommandID = 40000;

//=======================================================================
//==============    IniStrings
//=======================================================================
struct IniSections
{
    static const wchar_t *Repositories;             // repository_name=directory_on_disk
    static const wchar_t *Repositories_Groups;      // group_name=list_of_repository_names
    static const wchar_t *ViewState;
    static const wchar_t *Data;
};

struct IniKeys
{
    static const wchar_t *ViewState_SortColumn;
    static const wchar_t *ViewState_Group;
    static const wchar_t *Data_Marks;
    static const wchar_t *Data_AllGroups;
};

//=======================================================================
//==============    GitDirItem
//=======================================================================
class GitDirItem
{
private:
    std::wstring        mName;
    std::wstring        mDirectory;
    WStringList         mGroups;
public:
    GitDirItem( const std::wstring& name, const std::wstring& dir, const std::wstring& groups );

    const std::wstring& Name() const            { return mName; }
    const std::wstring& Directory() const       { return mDirectory; }
    const WStringList& Groups() const           { return mGroups; }

    void Name( const std::wstring& value )      { mName = value; }
};

//=======================================================================
//==============    GitDirStateItem
//=======================================================================
struct GitDirStateItem
{
    std::wstring    Name;
    std::wstring    Directory;
    std::string     Branch;
    int             NRepos;
    bool            Uncommited;
    bool            NeedsUpdate;
    explicit GitDirStateItem( const std::wstring& name, const std::wstring& dir )
        : Name( name ), Directory( dir ), Branch(), NRepos(), Uncommited(), NeedsUpdate()
    {}
};

std::wstring MakeCommand( const wchar_t *command, const wchar_t *path );
std::wstring GetIniFileName();
void Throw_NoUniqueName( const std::wstring& name );

WStringList DelimitedTextToList( const std::wstring& text, const wchar_t delimiter );
std::wstring ListToDelimitedText( const WStringList& list, const wchar_t delimiter );

namespace git2
{
    //=======================================================================
    //==============    BranchInfo
    //=======================================================================
    class BranchInfo
    {
    private:
        std::string     mName;
        git_branch_t    mType;
    public:
        BranchInfo( const std::string& name, git_branch_t type )
            : mName(name), mType(type)
        {
        }

        const std::string& Name() const         { return mName; }
        git_branch_t Type() const               { return mType; }
    };

    //=======================================================================
    //==============    LibGit2
    //=======================================================================
    class LibGit2
    {
    private:
        typedef std::function<void( git_repository *rep )>              RepositoryDeleter;
        typedef std::unique_ptr<git_repository, RepositoryDeleter>      RepositoryHolder;
    private:
        RepositoryHolder        mRepository;

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
        std::vector<BranchInfo> ListBranches();
        std::vector<std::string> ListRemotes();
        size_t RevisionCount( const std::string& src, const std::string& dst );
    };
}

void GetDirectoryState( git2::LibGit2& libgit, GitDirStateItem& state_item );

#endif
