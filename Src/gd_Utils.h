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
        std::vector<BranchInfo> ListBranches();
        std::vector<std::string> ListRemotes();
        size_t RevisionCount( const std::string& src, const std::string& dst );
    };
}

#endif
