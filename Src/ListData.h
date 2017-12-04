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

#if !defined(GITDIRS_LISTDATA_H)
#define GITDIRS_LISTDATA_H

#include "gd_Utils.h"
#include <vector>

enum class ListColumn { name, path, n_repos, branch, uncommited, needs };

//=======================================================================
//==============    ListDataItem
//=======================================================================
class ListDataItem
{
private:
    GitDirItem      mDataItem;
    std::wstring    mBranch;
    int             mNRepos;
    std::wstring    mNRepos_str;
    bool            mUncommited = false;
    bool            mNeedsUpdate = false;
    bool            mChecked = false;

    static const std::wstring Yes;
    static const std::wstring No;
    static const std::wstring Empty;
public:
    explicit ListDataItem( const GitDirItem& item );
    ~ListDataItem();

    const std::wstring& Name() const            { return mDataItem.Name(); }
    const std::wstring& Directory() const       { return mDataItem.Directory(); }
    const WStringList& Groups() const           { return mDataItem.Groups(); }
    const std::wstring& Branch() const          { return mBranch; }
    bool Checked() const                        { return mChecked; }
    
    void Checked( bool value )                  { mChecked = value; }
    void Name( const std::wstring& value )      { mDataItem.Name( value ); }
    void NRepos( int value );

    const std::wstring& GetText( ListColumn col ) const;
};

//=======================================================================
//==============    ListData
//=======================================================================
class ListData
{
private:
    typedef std::vector<ListDataItem>       Container;
private:
    Container       mData;
public:
    static const Container::size_type       npos = 0xFFFFFFFF;
public:
    ListData();
    ~ListData();

    void LoadFromIni( const std::wstring& ini_fname );
    void SaveToIni( const std::wstring& ini_fname );
    void Clear();
    void Sort( ListColumn col );
    void AddItem( const std::wstring& key, const std::wstring& value );
    void DeleteItem( const std::wstring& key );
    Container::size_type FindItem( const std::wstring& key ) const;

    const ListDataItem& Item( Container::size_type idx ) const;
    ListDataItem& Item( Container::size_type idx );
    Container::size_type Count()                                    { return mData.size(); }

    bool IndexInBounds( Container::size_type idx ) const            { return idx < mData.size(); }
    bool IsUniqueKey( const std::wstring& key )                     { return FindItem( key ) == npos; }
};

#endif
