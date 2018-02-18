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
#include <map>
#include <git2.h>

typedef std::function<void()>                   Procedure;
typedef std::vector<std::wstring>               WStringList;
typedef std::map<std::wstring, std::wstring>    ReposList;

const int       LastDataVersion = 1;
const int       GROUPS_MENU_Position = 3;
const int       GROUPS_MENU_HeaderCount = 3;
const int       GROUPS_MENU_CommandID = 40000;
const int       GROUPS_MENU_SubMenuCommandID = 41000;

//=======================================================================
//==============    IniStrings
//=======================================================================
struct IniSections
{
    static const wchar_t *Repositories;             // repository_name=directory_on_disk
    static const wchar_t *Repositories_Groups;      // group_name=list_of_repository_names
    static const wchar_t *ViewState;
    static const wchar_t *Data;
    static const wchar_t *Version;
};

struct IniKeys
{
    static const wchar_t *ViewState_SortColumn;
    static const wchar_t *ViewState_Group;
    static const wchar_t *ViewState_ShowCheckBoxes;
    static const wchar_t *ViewState_WorksetFilename;
    static const wchar_t *Data_Marks;
    static const wchar_t *Data_AllGroups;
    static const wchar_t *Version;
};

//=======================================================================
//==============    ViewState
//=======================================================================
struct ViewState
{
    std::wstring    Workset_Filename;

    void Save( ccwin::TIniFile& ini );
    void Load( ccwin::TIniFile& ini );
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
    
    void AddToGroup( const std::wstring& group );
    void RemoveFromGroup( const std::wstring& group );
    bool InGroup( const std::wstring& group );

    void Name( const std::wstring& value )      { mName = value; }
};

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

    typedef std::vector<BranchInfo>     BranchInfoList;

    //=======================================================================
    //==============    RemoteInfo
    //=======================================================================
    class RemoteInfo
    {
    private:
        std::string     mName;
        std::string     mUrl;
    public:
        RemoteInfo( const std::string& name, const std::string& url )
            : mName( name ), mUrl( url )
        {}

        const std::string& Name() const     { return mName; }
        const std::string& Url() const      { return mUrl; }
    };

    typedef std::vector<RemoteInfo>     RemoteInfoList;

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
        BranchInfoList ListBranches();
        RemoteInfoList ListRemotes();
        size_t RevisionCount( const std::string& src, const std::string& dst );
    };
}

//=======================================================================
//==============    GitDirStateItem
//=======================================================================
struct GitDirStateItem
{
    std::wstring            Name;
    std::wstring            Directory;
    std::string             Branch;
    git2::RemoteInfoList    Remotes;
    bool                    Uncommited;
    bool                    NeedsUpdate;
    explicit GitDirStateItem( const std::wstring& name, const std::wstring& dir )
        : Name( name ), Directory( dir ), Branch(), Remotes(), Uncommited(), NeedsUpdate()
    {}
};

typedef std::vector<GitDirStateItem>    GitDirStateList;

//=======================================================================
//==============    FREE FUNCTIONS
//=======================================================================

std::wstring MakeCommand( const wchar_t *command, const wchar_t *path );
std::wstring GetIniFileName();
void Throw_NoUniqueName( const std::wstring& name );

WStringList DelimitedTextToList( const std::wstring& text, const wchar_t delimiter );
std::wstring ListToDelimitedText( const WStringList& list, const wchar_t delimiter );

void GetDirectoryState( git2::LibGit2& libgit, GitDirStateItem& state_item );

void SetMenuCheck( CMenuHandle menu, int position, bool value );
void SetMenuRadioRecursive( CMenuHandle menu, UINT menu_id );

const wchar_t *RS_LoadPtr( UINT rc_id, int& len );
std::wstring RS_LoadString( UINT rc_id );

void RepositoryExists( const std::wstring& result );

typedef std::pair<const wchar_t *, const wchar_t *>     open_filter_spec;
typedef std::initializer_list<open_filter_spec>         open_filter_list;

#if defined (WINDOWS_XP_BUILD)

std::wstring OpenDlg( const std::wstring& def_ext, const std::wstring& filename, DWORD flags, const open_filter_list& filters, HWND wnd );
std::wstring SaveDlg( const std::wstring& def_ext, const std::wstring& filename, DWORD flags, const open_filter_list& filters, HWND wnd );

#else

std::wstring OpenDlg( const std::wstring& def_ext, const std::wstring& filename, DWORD flags, const open_filter_list& filters, HWND wnd );
std::wstring SaveDlg( const std::wstring& def_ext, const std::wstring& filename, DWORD flags, const open_filter_list& filters, HWND wnd );

#endif

#endif
