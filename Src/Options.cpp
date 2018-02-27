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
#include "wtlUtils.h"
#include "gd_Utils.h"
#include <atldlgs.h>
#include <winUtils.h>

//=======================================================================
//==============    Options
//=======================================================================
void Options::LoadOptions( ccwin::TIniFile & ini )
{
    RefreshAfterFetch = ini.ReadBool( IniSections::Options, IniKeys::Options_RefreshAfterFetch, false );
    UseStaticWorksetFilename = ini.ReadBool( IniSections::Options, IniKeys::Options_UseStaticWorksetFilename, false );
    SaveWorksetAfterDB = ini.ReadBool( IniSections::Options, IniKeys::Options_SaveWorksetAfterDB, false );
    WoksetFilename = ini.ReadString( IniSections::Options, IniKeys::Options_WorksetFilename, CCTEXT("") );
}

void Options::SaveOptions( ccwin::TIniFile & ini )
{
    ini. WriteBool( IniSections::Options, IniKeys::Options_RefreshAfterFetch, RefreshAfterFetch );
    ini.WriteBool( IniSections::Options, IniKeys::Options_UseStaticWorksetFilename, UseStaticWorksetFilename );
    ini.WriteBool( IniSections::Options, IniKeys::Options_SaveWorksetAfterDB, SaveWorksetAfterDB );
    ini.WriteString( IniSections::Options, IniKeys::Options_WorksetFilename, WoksetFilename.c_str() );
}

//=======================================================================
//==============    COptionsDlg
//=======================================================================
void COptionsDlg::LoadOptions()
{
    mRefreshAfterFetch.SetCheck( mOptions.RefreshAfterFetch ? BST_CHECKED : BST_UNCHECKED );
    mUseStaticWorksetFilename.SetCheck( mOptions.UseStaticWorksetFilename ? BST_CHECKED : BST_UNCHECKED );
    mSaveWorksetAfterDB.SetCheck( mOptions.SaveWorksetAfterDB ? BST_CHECKED : BST_UNCHECKED );
    mWorksetFilename.SetWindowText( mOptions.WoksetFilename.c_str() );
}

void COptionsDlg::SaveOptions()
{
    mOptions.RefreshAfterFetch = mRefreshAfterFetch.GetCheck() == BST_CHECKED;
    mOptions.UseStaticWorksetFilename = mUseStaticWorksetFilename.GetCheck() == BST_CHECKED;
    mOptions.SaveWorksetAfterDB = mSaveWorksetAfterDB.GetCheck() == BST_CHECKED;
    mOptions.WoksetFilename = ccwtl::getControlText( mWorksetFilename );
}

LRESULT COptionsDlg::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    CenterWindow( GetParent() );

    mRefreshAfterFetch.Attach( GetDlgItem( IDC_REFRESH_AFTER_FETCH ) );
    mUseStaticWorksetFilename.Attach( GetDlgItem( IDC_STATIC_FILENAME ) );
    mSaveWorksetAfterDB.Attach( GetDlgItem( IDC_SAVE_WORKSET ) );
    mWorksetFilename.Attach( GetDlgItem( IDC_EDIT_FILENAME ) );

    LoadOptions();

    return TRUE;
}

LRESULT COptionsDlg::OnCmd_Cancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    EndDialog( wID );
    return 0;
}

LRESULT COptionsDlg::OnCmd_OK( WORD, WORD wID, HWND, BOOL & )
{
    SaveOptions();
    EndDialog( wID );
    return LRESULT();
}

LRESULT COptionsDlg::OnCmd_Browse( WORD, WORD, HWND, BOOL & )
{
    std::wstring    fname = SaveDlg( L"", ccwtl::getControlText( mWorksetFilename ),
#if defined (WINDOWS_XP_BUILD)
                                     OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR,  // DWORD dwFlags = 
#else
                                     FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_NOTESTFILECREATE,
#endif
                                     { open_filter_spec( L"Text Files (*.txt)", L"*.txt" ), open_filter_spec( L"All Files (*.*)", L"*.*" ) },
                                     *this );

    if ( !fname.empty() )
        mWorksetFilename.SetWindowText( fname.c_str() );
    return LRESULT();
}

COptionsDlg::COptionsDlg( Options& options )
    : mOptions( options )
{
}
