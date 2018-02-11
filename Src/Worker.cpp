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

#include "stdafx.h"
#include "Worker.h"
#include "gd_Utils.h"
#include <win_str.h>

//=======================================================================
//==============    Work
//=======================================================================
Work::Work()
    : mFlags(), mThread()
{}

Work::Work( Work&& other )
{
    mFlags = std::move( other.mFlags );
    mThread = std::move( other.mThread );
}

//void Work::Run( ActionBase& func )
//{
//    *mAquireCancelRun = false;
//    *mTerminated = false;
//    func();
//}
//
//void Work::RunThreaded( ActionBase& func )
//{
//    *mAquireCancelRun = false;
//    *mTerminated = false;
//    mThread = uqThread( new std::thread( func, mAquireCancelRun, mTerminated ), [this]( std::thread *ptr ) {
//        *mAquireCancelRun = true;
//        ptr->join();
//        delete ptr;
//    } );
//}

////=======================================================================
////==============    ActionBase
////=======================================================================
//ActionBase::ActionBase()
//    : mAquireCancelRun( std::make_shared<bool>( false ) ), mTerminated( std::make_shared<bool>( true ) )
//{
//}
//
//void ActionBase::SetFlags( const Work::spFlag & cancel, const Work::spFlag & terminated )
//{
//    mAquireCancelRun = cancel;
//    mTerminated = terminated;
//}
//
//void ActionBase::operator()()
//{
//    try
//    {
//        mError = false;
//        Run();
//    }
//    catch ( const std::exception& ex )
//    {
//        mError = true;
//        mErrorString = ccwin::WidenStringStrict( std::string( ex.what() ) );
//    }
//    MarkTerminated();
//}
//
////=======================================================================
////==============    Action_FetchRepos
////=======================================================================
//void Action_FetchRepos::Run()
//{
//    for ( const ReposList::value_type& item : mFetchList )
//    {
//        if ( IsCancelAquired() )
//            break;
//        RepositoryExists( item.second );
//        ccwin::ExecuteProgramWait( MakeCommand( L"fetch", item.second.c_str() ), INFINITE );
//    }
//}
//
//Action_FetchRepos::Action_FetchRepos( const ReposList& fetch_list )
//    : mFetchList( fetch_list )
//{
//}
//
////=======================================================================
////==============    Action_RefreshRepos
////=======================================================================
//void Action_RefreshRepos::Run()
//{
//    for ( GitDirStateList::value_type& item : mStateList )
//        if ( !IsCancelAquired() )
//            GetDirectoryState( mLibGit, item );
//}
//
//Action_RefreshRepos::Action_RefreshRepos( const GitDirStateList& state_list )
//    : mStateList( state_list )
//{
//}
//
//const GitDirStateList& Action_RefreshRepos::StateList() const
//{
//    if ( !IsIerminated() )
//        throw std::runtime_error( "Thread not terminated. Cannot access variable" );
//    return mStateList;
//}
