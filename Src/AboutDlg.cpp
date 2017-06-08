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
#include "AboutDlg.h"
#include <winVersion.h>
#include <boost/format.hpp>

LRESULT CAboutDlg::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    CenterWindow( GetParent() );

    SYSTEMTIME      ttime;

    GetLocalTime( &ttime );

    CStatic         label;

    label.Attach( GetDlgItem( IDC_INFO_TEXT ) );
    label.SetWindowText( boost::str( boost::wformat( L"%1%\nVersion v%2%\n(c) Copyright 2016 - %3%" )
                                                     % mAppName % ccwin::GetFileVersionStr( ccwin::get_module_handle() ) % ttime.wYear ).c_str() );
    return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    EndDialog( wID );
    return 0;
}

CAboutDlg::CAboutDlg( const std::wstring & app_name )
    : mAppName( app_name )
{
}
