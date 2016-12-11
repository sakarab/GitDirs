// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(GITDIRS_MAINDLG_H)
#define GITDIRS_MAINDLG_H

#include "resource.h"
#include <atlframe.h>
#include <wtl_msg_map.h>
#include <utility>

//=======================================================================
//==============    CMainDlg
//=======================================================================
class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
                 public CDialogResize<CMainDlg>,
                 public CMessageFilter, public CIdleHandler
{
private:
    typedef std::pair<int, CMainDlg *>      ListCompare_lParamSort;
private:
    HACCEL              mHAccel;
    CListViewCtrl       mListView;
    CMenu               mMainMenu;

    void GlobalHandleException( const std::exception& ex );
    virtual BOOL PreTranslateMessage( MSG* pMsg );
    virtual BOOL OnIdle();
    void CloseDialog( int nVal );
    CString ListView_GetSelectedText( int col );

    static int CALLBACK List_Compare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );

    // Handler prototypes (uncomment arguments if needed):
    //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
    LRESULT OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnContextMenu( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnAppAbout( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_OpenInExplorer( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_CheckForModifications( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Fetch( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Pull( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Push( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_RevisionGraph( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    HRESULT OnList_ColumnClick( int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/ );

    BEGIN_MSG_MAP_XX( CMainDlg )
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
        MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
        MESSAGE_HANDLER( WM_CONTEXTMENU, OnContextMenu )
        COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
        COMMAND_ID_HANDLER( ID_FILE_ABOUT, OnAppAbout )
        COMMAND_ID_HANDLER( ID_FILE_QUIT, OnCancel )
        COMMAND_ID_HANDLER( ID_FILE_OPENINEXPLORER, OnFile_OpenInExplorer )
        COMMAND_ID_HANDLER( ID_GIT_CHECKFORMODIFICATIONS, OnGit_CheckForModifications )
        COMMAND_ID_HANDLER( ID_GIT_FETCH, OnGit_Fetch )
        COMMAND_ID_HANDLER( ID_GIT_PULL, OnGit_Pull )
        COMMAND_ID_HANDLER( ID_GIT_PUSH, OnGit_Push )
        COMMAND_ID_HANDLER( ID_GIT_REVISIONGRAPPH, OnGit_RevisionGraph )
        NOTIFY_HANDLER( IDC_LIST, LVN_COLUMNCLICK, OnList_ColumnClick )
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
