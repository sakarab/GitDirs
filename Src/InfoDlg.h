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

#if !defined(GITDIRS_INFODLG_H)
#define GITDIRS_INFODLG_H

#include <atlcrack.h>
#include <atlctrls.h>
#include <wtl_msg_map.h>
#include <wtlUtils.h>
#include "gd_Utils.h"
#include "ListData.h"
#include "resource.h"

//=======================================================================
//==============    CInfoDlg
//=======================================================================
class CInfoDlg;
typedef std::function<void( CInfoDlg * )>               InfoDlg_Deleter;
typedef std::unique_ptr<CInfoDlg, InfoDlg_Deleter>      qpMonitorDlg;

class CInfoDlg : public CDialogImpl<CInfoDlg>,
                 public CDialogResize<CInfoDlg>,
                 public ccwtl::CFormSize<CInfoDlg>
{
private:
    CStatic         mGroupInfo;
    CStatic         mReferances;
    Procedure       mOnClose;
    bool            mCloseDialogCalled = false;

    void GlobalHandleException( const std::exception& ex );
    void OnFinalMessage( HWND ) override;

    LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnCloseCmd( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );

    BEGIN_MSG_MAP_XX( CMonitorDlg )
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog );
        MESSAGE_HANDLER( WM_DESTROY, OnDestroy );
        COMMAND_ID_HANDLER( IDCANCEL, OnCloseCmd );
        CHAIN_MSG_MAP( ccwtl::CFormSize<CInfoDlg> );
        CHAIN_MSG_MAP( CDialogResize<CInfoDlg> );
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP( CMainDlg )
        DLGRESIZE_CONTROL( IDC_GROUP_INFO, DLSZ_SIZE_X )
        DLGRESIZE_CONTROL( IDC_GROUP_INFO_REFEDIT, DLSZ_SIZE_X | DLSZ_SIZE_Y )
    END_DLGRESIZE_MAP()

    ~CInfoDlg();
public:
    enum { IDD = IDD_REPO_INFO };

    explicit CInfoDlg( Procedure on_close );

    void CloseDialog();
    void SetInfo( const ListDataItem& item );
};

#endif
