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

#pragma once

#if !defined(GITDIRS_WORKER_H)
#define GITDIRS_WORKER_H

#include <atomic>
#include <memory>
#include <thread>
#include <map>
#include "gd_Utils.h"

class ActionBase;

//=======================================================================
//==============    Work
//=======================================================================
class Work
{
public:
    typedef std::shared_ptr<bool>       spFlag;
private:
    typedef std::unique_ptr<std::thread, std::function<void ( std::thread * )> >        uqThread;

    spFlag      mAquireCancelRun;
    spFlag      mTerminated;
    uqThread    mThread;
    // non-copyable
    Work( const Work& ) CC_EQ_DELETE;
    Work& operator=( const Work& ) CC_EQ_DELETE;
public:
    Work();
    Work( Work&& other );

    void Run( ActionBase& func );
    void RunThreaded( ActionBase& func );

    void CancelRun()            { *mAquireCancelRun = true; }
    bool IsIerminated() const   { return *mTerminated; }
};

typedef std::shared_ptr<Work>   spWork;
typedef std::unique_ptr<Work>   uqWork;

//=======================================================================
//==============    ActionBase
//=======================================================================
class ActionBase
{
private:
    Work::spFlag        mAquireCancelRun;
    Work::spFlag        mTerminated;
    std::wstring        mErrorString;
    bool                mError;

    void MarkTerminated()           { *mTerminated = true; }
protected:
    virtual void Run() = 0;
public:
    ActionBase();
    virtual ~ActionBase()           {}  // empty
    void operator()();

    void SetFlags( const Work::spFlag& cancel, const Work::spFlag& terminated );

    bool IsCancelAquired() const    { return *mAquireCancelRun; }
    bool ErronTermination() const   { return mError; }
    bool IsIerminated() const       { return *mTerminated; }
};

typedef std::unique_ptr<ActionBase>     uqActionBase;

//=======================================================================
//==============    Action_FetchRepos
//=======================================================================
typedef std::map<std::wstring, std::wstring>        ReposList;

class Action_FetchRepos : public ActionBase
{
private:
    ReposList       mFetchList;
protected:
    virtual void Run() CC_OVERRIDE;
public:
    Action_FetchRepos( const ReposList& fetch_list );
};

//=======================================================================
//==============    Action_RefreshRepos
//=======================================================================
class Action_RefreshRepos : public ActionBase
{
private:
    GitDirStateList     mStateList;
    git2::LibGit2       mLibGit;
protected:
    virtual void Run() CC_OVERRIDE;
public:
    Action_RefreshRepos( const GitDirStateList& state_list );

    const GitDirStateList& StateList() const;
};

#endif
