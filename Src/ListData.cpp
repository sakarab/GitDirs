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

// https://www.codeproject.com/articles/7891/using-virtual-lists

//=======================================================================
//==============    ListDataItem
//=======================================================================
ListDataItem::ListDataItem( const GitDirItem & item )
    : mDataItem( item ), mBranch(), mNRepos(), mUncommited(), mNeedsUpdate()
{}

ListDataItem::~ListDataItem()
{}

//enum class ListColumn { name, path, n_repos, branch, uncommited, needs };

std::wstring ListDataItem::GetText( ListColumn col ) const
{
    switch ( col )
    {
        case ListColumn::name       : return Name();
        case ListColumn::path       : return Directory();
        case ListColumn::n_repos    : return std::wstring();
        case ListColumn::branch     : return Branch();
        case ListColumn::uncommited : return std::wstring();
        case ListColumn::needs      : return std::wstring();
    }
    return std::wstring();
}

//=======================================================================
//==============    ListData
//=======================================================================
ListData::ListData()
{}

ListData::~ListData()
{}

void ListData::LoadFromIni( const std::wstring & ini_fname )
{
    ccwin::TIniFile             ini( ini_fname );
    ccwin::TStringList          slist;

    ini.ReadSectionKeys( IniSections::Repositories, slist );
    for ( int n = 0, eend = slist.Count() ; n < eend ; ++n )
    {
        const std::wstring&     key = slist[n];
        const std::wstring      value = ini.ReadString( IniSections::Repositories, key.c_str(), L"" );

        if ( !value.empty() )
        {
            const std::wstring      groups = ini.ReadString( IniSections::Repositories_Groups, key.c_str(), L"" );

            mData.push_back( ListDataItem( GitDirItem( key, value, groups ) ) );
        }
    }

    WStringList                 marks = LoadMarks();

    for ( ListDataItem& item : mData )
        item.Checked( std::find( marks.begin(), marks.end(), item.Name() ) != marks.end() );
}

void ListData::SaveToIni( const std::wstring & ini_fname )
{
    ccwin::TIniFile             ini( ini_fname );

    ini.EraseSection( IniSections::Repositories );
    for ( ListDataItem& item : mData )
    {
        ini.WriteString( IniSections::Repositories, item.Name().c_str(), item.Directory().c_str() );

        if ( item.Groups().empty() )
            ini.EraseKey( IniSections::Repositories_Groups, item.Name().c_str() );
        else
            ini.WriteString( IniSections::Repositories_Groups, item.Name().c_str(), ListToDelimitedText( item.Groups(), L',' ).c_str() );
    }
    ini.WriteInteger( L"Version", L"Version", LastDataVersion );
}

void ListData::Clear()
{
    mData.clear();
}

void ListData::Sort( ListColumn col )
{
    ccwin::case_insensitive_string_compare_ptr<wchar_t>     cmp;

    std::sort( mData.begin(), mData.end(), [&cmp, &col]( const ListDataItem& item1, const ListDataItem& item2 ) {
        return cmp.operator()( item1.GetText( col ).c_str(), item2.GetText( col ).c_str() ) < 0;
    } );
}

void ListData::AddItem( const std::wstring& key, const std::wstring& value )
{
    if ( FindItem( key ) != npos )
        Throw_NoUniqueName( key );
    mData.push_back( ListDataItem( GitDirItem( key, value, std::wstring() ) ) );
}

ListData::Container::size_type ListData::FindItem( const std::wstring& key ) const
{
    Container::const_iterator     it = std::find_if( mData.begin(), mData.end(), [&key]( const Container::value_type& item ) { return item.Name() == key; } );

    if ( it == mData.end() )
        return npos;
    return std::distance( mData.begin(), it );
}

const ListDataItem & ListData::Item( Container::size_type idx ) const
{
    return mData[idx];
}

ListDataItem & ListData::Item( Container::size_type idx )
{
    if ( !IndexInBounds( idx ) )
        throw std::runtime_error( "Index out of bounds." );
    return mData[idx];
}
