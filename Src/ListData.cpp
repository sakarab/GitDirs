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

// https://www.codeproject.com/articles/7891/using-virtual-lists

//=======================================================================
//==============    ListDataItem
//=======================================================================
const std::wstring ListDataItem::Yes = std::wstring( L"Yes" );
const std::wstring ListDataItem::No  = std::wstring( L"No" );
const std::wstring ListDataItem::Empty = std::wstring();

ListDataItem::ListDataItem( const GitDirItem & item )
    : mDataItem( item ), mBranch(), mNRepos(), mNRepos_str( L"0" ), mUncommited(), mNeedsUpdate()
{}

ListDataItem::~ListDataItem()
{}

void ListDataItem::NRepos( int value )
{
    mNRepos_str = boost::lexical_cast<std::wstring>(value);
    mNRepos = value;
}

const std::wstring& ListDataItem::GetText( ListColumn col ) const
{
    switch ( col )
    {
        case ListColumn::name       : return Name();
        case ListColumn::path       : return Directory();
        case ListColumn::n_repos    : return mNRepos_str;
        case ListColumn::branch     : return Branch();
        case ListColumn::uncommited : return mUncommited ? Yes : No;
        case ListColumn::needs      : return mNeedsUpdate ? Yes : No;
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

    WStringList                 marks = DelimitedTextToList( ini.ReadString( IniSections::Data, IniKeys::Data_Marks, L"" ), L',' );

    for ( Container::value_type& item : mData )
    {
        ListDataItem&           item_ref = *item.second;

        item_ref.Checked( std::find( marks.begin(), marks.end(), item_ref.Name() ) != marks.end() );

    }
}

void ListData::SaveToIni( ccwin::TIniFile& ini )
{
    ini.EraseSection( IniSections::Repositories );
    for ( Container::value_type& item : mData )
    {
        const ListDataItem&     item_ref = *item.second;

        ini.WriteString( IniSections::Repositories, item_ref.Name().c_str(), item_ref.Directory().c_str() );

        if ( item_ref.Groups().empty() )
            ini.EraseKey( IniSections::Repositories_Groups, item_ref.Name().c_str() );
        else
            ini.WriteString( IniSections::Repositories_Groups, item_ref.Name().c_str(), ListToDelimitedText( item_ref.Groups(), L',' ).c_str() );
    }

    WStringList                 marks;

    for ( Container::value_type& item : mData )
    {
        const ListDataItem&     item_ref = *item.second;

        if ( item_ref.Checked() )
            marks.push_back( item_ref.Name() );
    }
    ini.WriteString( IniSections::Data, IniKeys::Data_Marks, ListToDelimitedText( marks, L',' ).c_str() );

    ini.WriteInteger( L"Version", L"Version", LastDataVersion );
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

//=======================================================================
//==============    ListDataView
//=======================================================================

ListDataView::ListDataView()
{}

ListDataView::~ListDataView()
{}

void ListDataView::LoadFromDb( const ListData& data, const std::wstring& group )
{
    if ( group.empty() )
    {
        for ( const ListData::Container::value_type& item : data )
            mData.push_back( item.second );
    }
    else
    {
        for ( const ListData::Container::value_type& item : data )
        {
            const WStringList&  slist = item.second->Groups();

            if ( std::find( slist.begin(), slist.end(), group ) != slist.end() )
                mData.push_back( item.second );
        }
    }
    mGroup = group;
    Sort( mSort );
}

void ListDataView::Sort( ListColumn col )
{
    ccwin::case_insensitive_string_compare_ptr<wchar_t>     cmp;

    std::sort( mData.begin(), mData.end(), [&cmp, &col]( const spListDataItem& item1, const spListDataItem& item2 ) {
        return cmp( item1->GetText( col ).c_str(), item2->GetText( col ).c_str() ) < 0;
    } );
}

void ListDataView::AddItem( ListData& data, const std::wstring& key, const std::wstring& value )
{
    spListDataItem      item = std::make_shared<ListDataItem>( GitDirItem( key, value, mGroup ) );

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

const spListDataItem& ListDataView::Item( Container::size_type idx ) const
{
    if ( !IndexInBounds( idx ) )
        throw std::runtime_error( "Index out of bounds." );
    return mData[idx];
}

spListDataItem& ListDataView::Item( Container::size_type idx )
{
    if ( !IndexInBounds( idx ) )
        throw std::runtime_error( "Index out of bounds." );
    return mData[idx];
}
