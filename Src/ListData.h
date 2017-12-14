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

typedef std::vector<GitDirStateItem>    GitDirStateList;

void GitGetRepositoriesState( GitDirStateList& state_list );

enum class ListColumn : byte           { name, path, n_repos, branch, uncommited, needs };
const byte ListColumn_Min = static_cast<byte>(ListColumn::name);
const byte ListColumn_Max = static_cast<byte>(ListColumn::needs);

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

    bool IsInited() const                       { return !mBranch.empty(); }
public:
    explicit ListDataItem( const GitDirItem& item );
    ~ListDataItem();

    void AddToGroup( const std::wstring& group )        { mDataItem.AddToGroup( group ); }
    void RemoveFromGroup( const std::wstring& group )   { mDataItem.RemoveFromGroup( group ); }
    bool InGroup( const std::wstring& group )           { return mDataItem.InGroup( group ); }

    const std::wstring& Name() const            { return mDataItem.Name(); }
    const std::wstring& Directory() const       { return mDataItem.Directory(); }
    const WStringList& Groups() const           { return mDataItem.Groups(); }
    const std::wstring& Branch() const          { return mBranch; }
    bool Checked() const                        { return mChecked; }
    bool ToggleChecked();

    void Name( const std::wstring& value )      { mDataItem.Name( value ); }
    void NRepos( int value );
    void Branch( const std::wstring& value )    { mBranch = value; }
    void Uncommited( bool value )               { mUncommited = value; }
    void NeedsUpdate( bool value )              { mNeedsUpdate = value; }
    void Checked( bool value )                  { mChecked = value; }

    const std::wstring& GetText( ListColumn col ) const;

    static const std::wstring Empty;
};

typedef std::shared_ptr<ListDataItem>       spListDataItem;
typedef std::weak_ptr<ListDataItem>         wpListDataItem;

//=======================================================================
//==============    FilterBase
//=======================================================================
class FilterBase
{
public:
    virtual bool operator()( const spListDataItem& data_item ) = 0;
    virtual void SetFilterField( spListDataItem& data_item ) = 0;
    virtual const std::wstring& Group() const                           { return ListDataItem::Empty; }
    virtual ~FilterBase();
};

typedef std::shared_ptr<FilterBase>         spFilter;

//=======================================================================
//==============    FilterChecked
//=======================================================================
class FilterChecked : public FilterBase
{
private:
    bool    mChecked;
public:
    explicit FilterChecked( bool value ) : mChecked( value ) {}
    bool operator()( const spListDataItem& data_item ) override
    {
        return !(data_item->Checked() ^ mChecked);
    }
    void SetFilterField( spListDataItem& data_item ) override
    {
        data_item->Checked( mChecked );
    }
};

//=======================================================================
//==============    FilterTagged
//=======================================================================
class FilterTagged : public FilterBase
{
private:
    bool    mInGroup;
public:
    explicit FilterTagged( bool value ) : mInGroup( value ) {}
    bool operator()( const spListDataItem& data_item ) override
    {
        return (data_item->Groups().empty() ^ mInGroup);
    }
    void SetFilterField( spListDataItem& /*data_item*/ ) override
    {
        if ( mInGroup )
            throw std::runtime_error( "Not supported." );
    }
};

//=======================================================================
//==============    FilterGroup
//=======================================================================
class FilterGroup : public FilterBase
{
private:
    std::wstring    mGroup;
public:
    FilterGroup() : mGroup() {}
    explicit FilterGroup( const std::wstring value ) : mGroup( value ) {}
    bool operator()( const spListDataItem& data_item ) override
    {
        if ( mGroup.empty() )
            return true;
        else
        {
            const WStringList&  slist = data_item->Groups();

            return std::find( slist.begin(), slist.end(), mGroup ) != slist.end();
        }
    }
    void SetFilterField( spListDataItem& data_item ) override
    {
        data_item->AddToGroup( mGroup );
    }
    const std::wstring& Group() const override                  { return mGroup; }
};

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
    WStringList     mAllGroups;
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
    bool IsUniqueKey( const std::wstring& key ) const           { return FindItem( key ) == mData.end(); }

    const WStringList& AllGroups() const                        { return mAllGroups; }
};

//=======================================================================
//==============    ListDataView
//=======================================================================
class ListDataView
{
private:
    typedef std::vector<spListDataItem>         Container;
public:
    typedef Container::iterator                 iterator;
    typedef Container::const_iterator           const_iterator;
    typedef Container::size_type                list_size_type;
private:
    Container       mData;
    spFilter        mFilter;
    ListColumn      mSortColumn = ListColumn::name;
public:
    static const Container::size_type       npos = 0xFFFFFFFF;
private:
    // noncopyable
    ListDataView( const ListDataView& src ) = delete;
    ListDataView& operator = ( const ListDataView& src ) = delete;
public:
    explicit ListDataView();
    ~ListDataView();

    iterator begin()                { return mData.begin(); }
    const_iterator begin() const    { return mData.begin(); }
    iterator end()                  { return mData.end(); }
    const_iterator end() const      { return mData.end(); }

    void LoadFromDb( const ListData& data );
    void LoadFromDb( const ListData& data, ListColumn col );

    const std::wstring& Group() const                               { return mFilter->Group(); }
    void Filter( ListData& data, const spFilter& filter );

    void LoadState( ccwin::TIniFile& ini );
    void SaveState( ccwin::TIniFile& ini );

    void AddItem( ListData& data, const std::wstring& key, const std::wstring& value );
    void DeleteItem( ListData& data, const std::wstring& key );
    void RemoveItem( const std::wstring& key );

    list_size_type FindItem( const std::wstring& key ) const;
    list_size_type FindItemCI( const std::wstring& key ) const;
    list_size_type FindItemCI( const std::wstring& key, std::wstring::size_type max_chars ) const;
    list_size_type FindItemCI_fromPos( const std::wstring& key, list_size_type pos ) const;
    list_size_type FindItemCI_fromPos( const std::wstring& key, std::wstring::size_type max_chars, list_size_type pos ) const;

    const spListDataItem& Item( list_size_type idx ) const;
    spListDataItem& Item( list_size_type idx );
    list_size_type Count()                                          { return mData.size(); }

    bool IndexInBounds( list_size_type idx ) const                  { return idx < mData.size(); }

    ListColumn SortColumn() const                                   { return mSortColumn; }
    void SortColumn( ListColumn value );
};

#endif
