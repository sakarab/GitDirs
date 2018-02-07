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
private:
    typedef std::unique_ptr<std::thread>    uqThread;

    std::atomic_bool    mAquireCancelRun;
    uqThread            mThread;
public:
    Work()
        : mAquireCancelRun(), mThread()
    {}

    template <class FUNC> void Run( FUNC func )
    {
        func();
    }

    template <class FUNC> void RunThreaded( FUNC func )
    {
        mAquireCancelRun = false;
        mThread = std::make_unique<uqThread::element_type>( func );
    }

    const std::atomic_bool& getAquireCancelRun()            { return mAquireCancelRun; }
};


#endif
