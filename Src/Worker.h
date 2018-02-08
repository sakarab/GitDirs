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
    Work()
        : mAquireCancelRun( std::make_shared<bool>( false ) ), mTerminated( std::make_shared<bool>( true ) ), mThread()
    {}

    Work( Work&& other )
    {
        mAquireCancelRun = std::move( other.mAquireCancelRun );
        mTerminated = std::move( other.mTerminated );
        mThread = std::move( other.mThread );
    }

    void Run( std::function<void( const spFlag& cancel, const spFlag& terminated )> func )
    {
        *mAquireCancelRun = false;
        func( mAquireCancelRun, mTerminated );
    }

    void RunThreaded( std::function<void( const spFlag& cancel, const spFlag& terminated )> func )
    {
        *mAquireCancelRun = false;
        mThread = uqThread( new std::thread( func, mAquireCancelRun, mTerminated ), []( std::thread *ptr ) {
            ptr->join();
            delete ptr;
        } );
    }

    void CancelRun()            { *mAquireCancelRun = true; }
    bool IsIerminated() const   { return *mTerminated; }
};

typedef std::shared_ptr<Work>   spWork;
typedef std::unique_ptr<Work>   uqWork;

#endif
