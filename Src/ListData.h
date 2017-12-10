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
#include <map>
#include <winClasses.h>

enum class ListColumn       { name, path, n_repos, branch, uncommited, needs };

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

typedef std::shared_ptr<ListDataItem>       spListDataItem;

//=======================================================================
//==============    ListData
//=======================================================================
class ListData
{
public:
    typedef std::map<std::wstring, spListDataItem>      Container;
    typedef Container::iterator                         iterator;
    typedef Container::const_iterator                   const_iterator;
private:
    Container       mData;
public:
    static const Container::size_type       npos = 0xFFFFFFFF;
private:
    // noncopyable
    ListData( const ListData& src ) = delete;
    ListData& operator = ( const ListData& src ) = delete;
public:
    ListData();
    ~ListData();

    iterator begin()                         { return mData.begin(); }
    const_iterator begin() const             { return mData.begin(); }
    iterator end()                           { return mData.end(); }
    const_iterator end() const               { return mData.end(); }

    void LoadFromIni( ccwin::TIniFile& ini );
    void SaveToIni( ccwin::TIniFile& ini );
    void Clear();
    void AddItem( const spListDataItem& item );
    void DeleteItem( const std::wstring& key );
    iterator FindItem( const std::wstring& key )                { return mData.find( key ); }
    const_iterator FindItem( const std::wstring& key ) const    { return mData.find( key ); }

    Container::size_type Count()                                { return mData.size(); }

    bool IsUniqueKey( const std::wstring& key ) const           { return FindItem( key ) != mData.end(); }
};

//=======================================================================
//==============    ListDataView
//=======================================================================
class ListDataView
{
private:
    class str_cmp
    {
    private:
        ccwin::case_insensitive_string_compare_ptr<wchar_t>     cmp;
    public:
        bool operator()( const spListDataItem& item_1, const spListDataItem& item_2 )
        {
            return cmp.operator()( item_1->Name().c_str(), item_2->Name().c_str() ) < 0;
        }
    };

    typedef std::vector<spListDataItem>         Container;
private:
    Container       mData;
    std::wstring    mGroup;
    ListColumn      mSort = ListColumn::name;
private:
    // noncopyable
    ListDataView( const ListDataView& src ) = delete;
    ListDataView& operator = ( const ListDataView& src ) = delete;
public:
    explicit ListDataView();
    ~ListDataView();

    void LoadFromDb( const ListData& data, const std::wstring& group );
    void Sort( ListColumn col );

    void AddItem( ListData& data, const std::wstring& key, const std::wstring& value );
    void DeleteItem( ListData& data, const std::wstring& key );

    const spListDataItem& Item( Container::size_type idx ) const;
    spListDataItem& Item( Container::size_type idx );
    Container::size_type Count()                                    { return mData.size(); }

    bool IndexInBounds( Container::size_type idx ) const            { return idx < mData.size(); }
};

#endif
