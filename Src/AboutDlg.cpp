#include "stdafx.h"
#include "AboutDlg.h"
#include <boost/format.hpp>

LRESULT CAboutDlg::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    CenterWindow( GetParent() );

    SYSTEMTIME      ttime;

    GetLocalTime( &ttime );

    CStatic         label;

    label.Attach( GetDlgItem( IDC_INFO_TEXT ) );
    label.SetWindowText( boost::str( boost::wformat( L"%1%\nVersion v1.0\n(c) Copyright 2016 - %2%" ) % mAppName % ttime.wYear ).c_str() );
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
