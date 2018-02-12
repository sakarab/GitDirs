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
#include <future>
#include <map>
#include <boost/any.hpp>
#include "gd_Utils.h"

//=======================================================================
//==============    ThreadResult
//=======================================================================
class ThreadResult
{
private:
    boost::any      mPromise;
    boost::any      mFuture;
public:
    void Clear()
    {
        mPromise = boost::any();
        mFuture = boost::any();
    }

    template <class T> void SetPromise( const boost::any& promise )
    {
        mPromise = promise;

        std::shared_ptr<std::promise<T>>    promise_ptr = boost::any_cast<std::shared_ptr<std::promise<T>>>(promise);
        std::shared_ptr<std::future<T>>     future_ptr = std::make_shared<std::future<T>>( promise_ptr->get_future() );

        mFuture = boost::any( future_ptr );
    }

    template <class T> std::shared_ptr<std::promise<T>> GetPromise()
    {
        return boost::any_cast<std::shared_ptr<std::promise<T>>>(mPromise);
    }
        
    template <class T> std::shared_ptr<std::future<T>> GetFuture()
    {
        return boost::any_cast<std::shared_ptr<std::future<T>>>(mFuture);
    }

    template <class T> bool IsClass()
    {
        //return boost::any_cast<std::shared_ptr<std::promise<T>> *>(&mPromise) != nullptr;
        bool    result = true;

        try
        {
            boost::any_cast<std::shared_ptr<std::future<T>>>(mFuture);
        }
        catch ( const boost::bad_any_cast& )
        {
            result = false;
        }
        return result;
    }
};

//=======================================================================
//==============    Work
//=======================================================================
class Work
{
public:
    class Flags
    {
    private:
        std::wstring    mErrorMessage;
        bool            mAquireCancelRun = false;
        bool            mTerminated = false;
        bool            mError = false;
    public:
        Flags()
            : mErrorMessage()
        {}

        void CancelRun()                { mAquireCancelRun = true; }
        void MarkTerminated()           { mTerminated = true; }
        bool IsIerminated() const       { return mTerminated; }
        bool IsCancelAquired() const    { return mAquireCancelRun; }
        bool IsErroneous() const        { return mError; }

        void SetErrorMessage( const std::wstring& msg )
        {
            mErrorMessage = msg;
            mError = true;
        }
    };

    typedef std::shared_ptr<Flags>      spFlags;
private:
    typedef std::unique_ptr<std::thread, std::function<void ( std::thread * )> >        uqThread;

    spFlags     mFlags;
    uqThread    mThread;
    // non-copyable
    Work( const Work& ) CC_EQ_DELETE;
    Work& operator=( const Work& ) CC_EQ_DELETE;
public:
    Work();
    Work( Work&& other );

    template <class FUNC, class ... Args>
    void Run( FUNC&& func, Args&& ... args )
    {
        mFlags = std::make_shared<Flags>();
        func( mFlags, args... );
    }

    template <class FUNC, class ... Args>
    void RunThreaded( FUNC&& func, Args&& ... args )
    {
        mFlags = std::make_shared<Flags>();
        mThread = uqThread( new std::thread( func, mFlags, args... ), [this]( std::thread *ptr ) {
            mFlags->CancelRun();
            ptr->join();
            delete ptr;
        } );
    }

    void CancelRun()                { mFlags->CancelRun(); }
    void MarkTerminated()           { mFlags->MarkTerminated(); }
    bool IsIerminated() const       { return mFlags->IsIerminated(); }
    bool IsCancelAquired() const    { return mFlags->IsCancelAquired(); }
    bool IsErroneous() const        { return mFlags->IsErroneous(); }

    void SetErrorMessage( const std::wstring& msg )          { mFlags->SetErrorMessage( msg ); }
};

typedef std::shared_ptr<Work>   spWork;
typedef std::unique_ptr<Work>   uqWork;

////=======================================================================
////==============    ActionBase
////=======================================================================
//class ActionBase
//{
//private:
//    Work::spFlag        mAquireCancelRun;
//    Work::spFlag        mTerminated;
//    std::wstring        mErrorString;
//    bool                mError;
//
//    void MarkTerminated()           { *mTerminated = true; }
//protected:
//    virtual void Run() = 0;
//public:
//    ActionBase();
//    virtual ~ActionBase()           {}  // empty
//    void operator()();
//
//    void SetFlags( const Work::spFlag& cancel, const Work::spFlag& terminated );
//
//    bool IsCancelAquired() const    { return *mAquireCancelRun; }
//    bool ErronTermination() const   { return mError; }
//    bool IsIerminated() const       { return *mTerminated; }
//};
//
//typedef std::unique_ptr<ActionBase>     uqActionBase;
//
////=======================================================================
////==============    Action_FetchRepos
////=======================================================================
typedef std::map<std::wstring, std::wstring>        ReposList;
//
//class Action_FetchRepos : public ActionBase
//{
//private:
//    ReposList       mFetchList;
//protected:
//    virtual void Run() CC_OVERRIDE;
//public:
//    Action_FetchRepos( const ReposList& fetch_list );
//};
//
////=======================================================================
////==============    Action_RefreshRepos
////=======================================================================
//class Action_RefreshRepos : public ActionBase
//{
//private:
//    GitDirStateList     mStateList;
//    git2::LibGit2       mLibGit;
//protected:
//    virtual void Run() CC_OVERRIDE;
//public:
//    Action_RefreshRepos( const GitDirStateList& state_list );
//
//    const GitDirStateList& StateList() const;
//};

#endif
