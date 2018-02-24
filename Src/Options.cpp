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
#include "Options.h"

//=======================================================================
//==============    COptionsDlg
//=======================================================================
LRESULT COptionsDlg::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    CenterWindow( GetParent() );

    SYSTEMTIME      ttime;

    GetLocalTime( &ttime );

    return TRUE;
}

LRESULT COptionsDlg::OnCmd_Cancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    EndDialog( wID );
    return 0;
}

LRESULT COptionsDlg::OnCmd_OK( WORD, WORD wID, HWND, BOOL & )
{
    EndDialog( wID );
    return LRESULT();
}

COptionsDlg::COptionsDlg()
{}
