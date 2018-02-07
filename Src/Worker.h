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
    typedef std::shared_ptr<std::atomic_bool>   spFlag;
private:
    typedef std::unique_ptr<std::thread, std::function<void ( std::thread * )> >        uqThread;

    spFlag      mAquireCancelRun;
    uqThread    mThread;
    // non-copyable
    Work( const Work& ) CC_EQ_DELETE;
    Work& operator=( const Work& ) CC_EQ_DELETE;
public:
    Work()
        : mAquireCancelRun( std::make_shared<spFlag::element_type>() ), mThread()
    {}

    Work( Work&& other )
    {
        mAquireCancelRun = std::move( other.mAquireCancelRun );
        mThread = std::move( other.mThread );
    }

    void Run( std::function<void( const spFlag& )> func )
    {
        *mAquireCancelRun = false;
        func( mAquireCancelRun );
    }

    void RunThreaded( std::function<void( const spFlag& )> func )
    {
        *mAquireCancelRun = false;
        mThread = uqThread( new std::thread, []( std::thread *ptr ) {
            ptr->join();
            delete ptr;
        } );
    }
};

typedef std::shared_ptr<Work>   spWork;
typedef std::unique_ptr<Work>   uqWork;

#endif
