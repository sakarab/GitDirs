// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(GITDIRS_MAINDLG_H)
#define GITDIRS_MAINDLG_H

#include "resource.h"
#include <atlframe.h>
#include <wtl_msg_map.h>

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
                 public CDialogResize<CMainDlg>,
                 public CMessageFilter, public CIdleHandler
{
private:
    HACCEL              mHAccel;
    CListViewCtrl       mListView;
    CMenu               mMainMenu;

    void GlobalHandleException( const std::exception& ex );
    virtual BOOL PreTranslateMessage( MSG* pMsg );
    virtual BOOL OnIdle();
    void CloseDialog( int nVal );

    // Handler prototypes (uncomment arguments if needed):
    //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
    LRESULT OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnAppAbout( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );

    BEGIN_MSG_MAP_XX( CMainDlg )
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
        MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
        COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
        COMMAND_ID_HANDLER( ID_FILE_ABOUT, OnAppAbout )
        COMMAND_ID_HANDLER( ID_FILE_QUIT, OnCancel )
        CHAIN_MSG_MAP( CDialogResize<CMainDlg> )
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP( CMainDlg )
        DLGRESIZE_CONTROL( IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y )
    END_DLGRESIZE_MAP()

    BEGIN_UPDATE_UI_MAP( CMainDlg )
    END_UPDATE_UI_MAP()
public:
	enum { IDD = IDD_MAINDLG };
};

#endif
