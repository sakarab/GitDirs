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
#include "ListData.h"
#include <smException.h>
#include <boost/lexical_cast.hpp>
#include <win_str.h>

// https://www.codeproject.com/articles/7891/using-virtual-lists

//=======================================================================
//==============    FilterBase
//=======================================================================
FilterBase::~FilterBase()
{}

//=======================================================================
//==============    ListDataItem
//=======================================================================
const std::wstring ListDataItem::Yes = std::wstring( L"Yes" );
const std::wstring ListDataItem::No  = std::wstring( L"No" );
const std::wstring ListDataItem::Empty = std::wstring();

ListDataItem::ListDataItem( const GitDirItem & item )
    : mDataItem( item ), mBranch(), mRemotes(), mNRemotes_str( L"0" ), mUncommited(), mNeedsUpdate()
{}

ListDataItem::~ListDataItem()
{}

bool ListDataItem::ToggleChecked()
{
    mChecked = !mChecked;
    return mChecked;
}

void ListDataItem::Remotes( const git2::RemoteInfoList& value )
{
    mNRemotes_str = boost::lexical_cast<std::wstring>(value.size());
    mRemotes = value;
}

const std::wstring& ListDataItem::GetText( ListColumn col ) const
{
    switch ( col )
    {
        case ListColumn::name       : return Name();
        case ListColumn::path       : return Directory();
        case ListColumn::n_repos    : return IsInited() ? mNRemotes_str : Empty;
        case ListColumn::branch     : return Branch();
        case ListColumn::uncommited : return IsInited() ? mUncommited ? Yes : No : Empty;
        case ListColumn::needs      : return IsInited() ? mNeedsUpdate ? Yes : No : Empty;
    }
    return Empty;
}

//=======================================================================
//==============    ListData
//=======================================================================
ListData::ListData()
{}

ListData::~ListData()
{}

void ListData::LoadFromIni( ccwin::TIniFile& ini )
{
    ccwin::TStringList          slist;

    ini.ReadSectionKeys( IniSections::Repositories, slist );
    for ( int n = 0, eend = slist.Count() ; n < eend ; ++n )
    {
        const std::wstring&     key = slist[n];
        const std::wstring      value = ini.ReadString( IniSections::Repositories, key.c_str(), L"" );

        if ( !value.empty() )
        {
            const std::wstring      groups = ini.ReadString( IniSections::Repositories_Groups, key.c_str(), L"" );

            mData[key] = std::make_shared<ListDataItem>( GitDirItem( key, value, groups ) );
        }
    }

    WorksetFromString( ini.ReadString( IniSections::Data, IniKeys::Data_Marks, L"" ) );
    mAllGroups = DelimitedTextToList( ini.ReadString( IniSections::Data, IniKeys::Data_AllGroups, L"" ), L',' );

    ccwin::case_insensitive_string_compare_ptr<wchar_t>     cmp;

    std::sort( mAllGroups.begin(), mAllGroups.end(), [&cmp]( const std::wstring& item1, const std::wstring& item2 ) {
        return cmp( item1.c_str(), item2.c_str() ) < 0;
    } );
}

void ListData::SaveToIni( ccwin::TIniFile& ini )
{
    ini.EraseSection( IniSections::Repositories );
    ini.EraseSection( IniSections::Repositories_Groups );
    for ( Container::value_type& item : mData )
    {
        const ListDataItem&     item_ref = *item.second;

        ini.WriteString( IniSections::Repositories, item_ref.Name().c_str(), item_ref.Directory().c_str() );
        if ( ! item_ref.Groups().empty() )
            ini.WriteString( IniSections::Repositories_Groups, item_ref.Name().c_str(), ListToDelimitedText( item_ref.Groups(), L',' ).c_str() );
    }

    ini.WriteString( IniSections::Data, IniKeys::Data_Marks, WorksetAsString().c_str() );
    ini.WriteInteger( IniSections::Version, IniKeys::Version, LastDataVersion );
}

void ListData::Clear()
{
    mData.clear();
}

void ListData::AddItem( const spListDataItem& item )
{
    if ( !IsUniqueKey( item->Name() ) )
        Throw_NoUniqueName( item->Name() );
    mData[item->Name()] = item;
}

void ListData::DeleteItem( const std::wstring& key )
{
    Container::iterator     it = FindItem( key );

    if ( it != mData.end() )
        mData.erase( it );
}

std::wstring ListData::WorksetAsString()
{
    WStringList                 marks;

    for ( Container::value_type& item : mData )
    {
        const ListDataItem&     item_ref = *item.second;

        if ( item_ref.Checked() )
            marks.push_back( item_ref.Name() );
    }
    return ListToDelimitedText( marks, L',' );
}

void ListData::WorksetFromString( const std::wstring& sstr )
{
    WStringList                 marks = DelimitedTextToList( sstr, L',' );

    for ( Container::value_type& item : mData )
    {
        ListDataItem&           item_ref = *item.second;

        item_ref.Checked( std::find( marks.begin(), marks.end(), item_ref.Name() ) != marks.end() );
    }
}

//=======================================================================
//==============    ListDataView
//=======================================================================

ListDataView::ListDataView()
    : mFilter( std::make_shared<FilterGroup>() )
{}

ListDataView::~ListDataView()
{}

void ListDataView::LoadFromDb( const ListData& data )
{
    LoadFromDb( data, mSortColumn );
}

void ListDataView::LoadFromDb( const ListData& data, ListColumn col )
{
    mData.clear();
    for ( const ListData::Container::value_type& item : data )
        if ( (*mFilter)( item.second ) )
            mData.push_back( item.second );
    SortColumn( col );
}

void ListDataView::Filter( ListData& data, const spFilter& filter )
{
    mFilter = filter;
    LoadFromDb( data );
}

void ListDataView::LoadState( ccwin::TIniFile& ini )
{
    mFilter = std::make_shared<FilterGroup>( ini.ReadString( IniSections::ViewState, IniKeys::ViewState_Group, L"" ) );
    SortColumn( static_cast<ListColumn>(ini.ReadInteger( IniSections::ViewState, IniKeys::ViewState_SortColumn, static_cast<byte>(ListColumn::name) )) );
}

void ListDataView::SaveState( ccwin::TIniFile & ini )
{
    ini.WriteString( IniSections::ViewState, IniKeys::ViewState_Group, mFilter->Group().c_str() );
    ini.WriteInteger( IniSections::ViewState, IniKeys::ViewState_SortColumn, static_cast<int>(mSortColumn) );
}

void ListDataView::AddItem( ListData& data, const std::wstring& key, const std::wstring& value )
{
    spListDataItem      item = std::make_shared<ListDataItem>( GitDirItem( key, value, std::wstring() ) );

    mFilter->SetFilterField( item );
    data.AddItem( item );
    mData.push_back( item );
}

void ListDataView::DeleteItem( ListData& data, const std::wstring& key )
{
    Container::iterator     it = std::find_if( mData.begin(), mData.end(), [&key]( const Container::value_type& item ) {
        return item->Name() == key;
    } );

    if ( it != mData.end() )
    {
        mData.erase( it );
        data.DeleteItem( key );
    }
}

void ListDataView::RemoveItem( const std::wstring& key )
{
    Container::iterator     it = std::find_if( mData.begin(), mData.end(), [&key]( const Container::value_type& item ) {
        return item->Name() == key;
    } );

    if ( it != mData.end() )
        mData.erase( it );
}

ListDataView::list_size_type ListDataView::FindItem( const std::wstring & key ) const
{
    Container::const_iterator     it = std::find_if( mData.begin(), mData.end(), [&key]( const Container::value_type& item ) {
        return item->Name() == key;
    } );

    if ( it == mData.end() )
        return npos;
    return std::distance( mData.begin(), it );
}

ListDataView::list_size_type ListDataView::FindItemCI( const std::wstring& key ) const
{
    return FindItemCI_fromPos( key, 0 );
}

ListDataView::list_size_type ListDataView::FindItemCI( const std::wstring& key, std::wstring::size_type max_chars ) const
{
    return FindItemCI_fromPos( key, max_chars, 0 );
}

ListDataView::list_size_type ListDataView::FindItemCI_fromPos( const std::wstring & key, list_size_type pos ) const
{
    if ( !IndexInBounds( pos ) )
        return npos;

    ccwin::case_insensitive_string_compare<wchar_t>     cmp;
    Container::const_iterator                           it_start = mData.begin();
        
    std::advance( it_start, pos );

    Container::const_iterator     it = std::find_if( it_start, mData.end(), [&key, &cmp]( const Container::value_type& item ) {
        return cmp( item->Name(), key ) == 0;
    } );

    if ( it == mData.end() )
        return npos;
    return std::distance( mData.begin(), it );
}

ListDataView::list_size_type ListDataView::FindItemCI_fromPos( const std::wstring & key, std::wstring::size_type max_chars, list_size_type pos ) const
{
    if ( !IndexInBounds( pos ) )
        return npos;

    ccwin::case_insensitive_string_compare_n<wchar_t>   cmp( max_chars );
    Container::const_iterator                           it_start = mData.begin();

    std::advance( it_start, pos );

    Container::const_iterator     it = std::find_if( it_start, mData.end(), [&key, &cmp]( const Container::value_type& item ) {
        return cmp( item->Name(), key ) == 0;
    } );

    if ( it == mData.end() )
        return npos;
    return std::distance( mData.begin(), it );
}

const spListDataItem& ListDataView::Item( list_size_type idx ) const
{
    if ( !IndexInBounds( idx ) )
        throw std::runtime_error( "Index out of bounds." );
    return mData[idx];
}

spListDataItem& ListDataView::Item( list_size_type idx )
{
    if ( !IndexInBounds( idx ) )
        throw std::runtime_error( "Index out of bounds." );
    return mData[idx];
}

void ListDataView::SortColumn( ListColumn value )
{
    if ( static_cast<byte>(value) > ListColumn_Max )
        value = static_cast<ListColumn>(ListColumn_Min);
    mSortColumn = value;

    ccwin::case_insensitive_string_compare_ptr<wchar_t>     cmp;

    std::sort( mData.begin(), mData.end(), [&cmp, &value]( const spListDataItem& item1, const spListDataItem& item2 ) {
        return cmp( item1->GetText( value ).c_str(), item2->GetText( value ).c_str() ) < 0;
    } );
}
