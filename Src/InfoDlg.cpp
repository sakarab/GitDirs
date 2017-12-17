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
#include "InfoDlg.h"
#include "gd_Utils.h"

//=======================================================================
//==============    CInfoDlg
//=======================================================================
void CInfoDlg::GlobalHandleException( const std::exception & ex )
{
    MessageBoxA( this->m_hWnd, ex.what(), "Error", MB_OK | MB_ICONHAND );
}

void CInfoDlg::OnFinalMessage( HWND )
{
    mOnClose();
    delete this;
}

LRESULT CInfoDlg::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    DlgResize_Init( true, true, WS_THICKFRAME | WS_CLIPCHILDREN );

    mGroupInfo.Attach( GetDlgItem( IDC_GROUP_INFO ) );
    mGroupInfo.SetWindowText( L"" );
    mReferances.Attach( GetDlgItem( IDC_GROUP_INFO_REFEDIT ) );

    ccwin::TIniFile     ini( GetIniFileName() );

    CFormSize::Load( ini );
    return LRESULT();
}

LRESULT CInfoDlg::OnDestroy( UINT, WPARAM, LPARAM, BOOL & )
{
    ccwin::TIniFile     ini( GetIniFileName() );

    CFormSize::Save( ini );
    return LRESULT();
}

LRESULT CInfoDlg::OnCloseCmd( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CloseDialog();
    return LRESULT();
}

CInfoDlg::~CInfoDlg()
{
    mCloseDialogCalled = false;
}

CInfoDlg::CInfoDlg( Procedure on_close )
    : mOnClose( on_close ), CFormSize( std::wstring( L"FormSize" ), std::wstring( L"CInfoDlg_" ) )
{}

void CInfoDlg::CloseDialog()
{
    if ( !mCloseDialogCalled )
    {
        mCloseDialogCalled = true;
        if ( ::IsWindow( m_hWnd ) )
            DestroyWindow();
    }
}

void CInfoDlg::SetInfo( const ListDataItem& item )
{
    mGroupInfo.SetWindowText( ListToDelimitedText( item.Groups(), L',' ).c_str() );

    std::string     info;

    for ( const git2::RemoteInfo& remote : item.Remotes() )
        info.append( boost::str( boost::format( "%1% - %2%\r\n" ) % remote.Name() % remote.Url() ) );
    if ( info.size() > 0 )
        info.resize( info.size() - 2 );
    mReferances.SetWindowText( ccwin::WidenStringStrict( info ).c_str() );
}
