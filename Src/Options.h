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

#if ! defined (GITDIRS_OPTIONS_DLG_H)
#define GITDIRS_OPTIONS_DLG_H

#include "resource.h"

//=======================================================================
//==============    Options
//=======================================================================
class Options
{
public:
    std::wstring    WoksetFilename;
    bool            RefreshAfterFetch = true;
    bool            UseStaticWorksetFilename = false;
    bool            SaveWorksetAfterDB = false;
public:
    void LoadOptions( ccwin::TIniFile& ini );
    void SaveOptions( ccwin::TIniFile& ini );
};

//=======================================================================
//==============    COptionsDlg
//=======================================================================
class COptionsDlg : public CDialogImpl<COptionsDlg>
{
private:
    CButton     mBrowse;
    CButton     mRefreshAfterFetch;
    CButton     mUseStaticWorksetFilename;
    CButton     mSaveWorksetAfterDB;
    CEdit       mWorksetFilename;

    Options&    mOptions;

    void LoadOptions();
    void SaveOptions();
    // Handler prototypes (uncomment arguments if needed):
    //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnCmd_Cancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnCmd_OK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );

    BEGIN_MSG_MAP( COptionsDlg )
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
        COMMAND_ID_HANDLER( IDOK, OnCmd_OK )
        COMMAND_ID_HANDLER( IDCANCEL, OnCmd_Cancel )
    END_MSG_MAP()
public:
    COptionsDlg( Options& options );

    enum { IDD = IDD_OPTIONS };
};



#endif
