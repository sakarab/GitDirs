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

//=======================================================================
//==============    IniStrings
//=======================================================================
struct IniStrings
{
    // sections
    static const wchar_t *Repositories;
    static const wchar_t *ViewState;
    // keys
    static const wchar_t *SortColumn;
};

//=======================================================================
//==============    ViewState
//=======================================================================
struct ViewState
{
    int     SortColumn = -1;

    void Save();
    void Load();
};

//=======================================================================
//==============    GitDirItem
//=======================================================================
struct GitDirItem
{
    std::wstring    Name;
    std::wstring    Directory;
    GitDirItem( const std::wstring& name, const std::wstring& dir )
        : Name( name ), Directory( dir )
    {
    }
};

//=======================================================================
//==============    GitDirStateItem
//=======================================================================
struct GitDirStateItem
{
    int             VisualIndex;
    std::wstring    Directory;
    std::string     Branch;
    bool            Uncommited;
    bool            NeedsUpdate;
    GitDirStateItem( int visual_index, const std::wstring& dir )
        : VisualIndex( visual_index ), Directory( dir ), Branch(), Uncommited(), NeedsUpdate()
    {}
};

typedef std::vector<GitDirItem>         GitDirList;
typedef std::vector<GitDirStateItem>    GitDirStateList;

GitDirList ReadFolderList();
std::wstring MakeCommand( const wchar_t *command, const wchar_t *path );
void GitGetRepositoriesState( GitDirStateList& state_list );
std::wstring GetIniFileName();

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

#endif
