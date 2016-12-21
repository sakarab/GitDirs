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
#include <predef_cc.h>

//=======================================================================
//==============    CHDrop
//=======================================================================
template <class CHAR> class CHDrop
{
    HDROP   m_hd;
    bool    bHandled;               // Checks if resources should be released.
    CHAR    m_buff[MAX_PATH + 5];   // DragQueryFile() wants LPTSTR.
public:
    CHDrop( WPARAM wParam )
        : m_hd( reinterpret_cast<HDROP>(wParam) ), bHandled(false)
    {
        m_buff[0] = '\0';
    }

    ~CHDrop()
    {
        if ( bHandled )
            ::DragFinish( m_hd );
    }

    // Helper function, detects if the message is meant for 'this' window.
    BOOL IsInClientRect( HWND hw )
    {
        ATLASSERT( ::IsWindow( hw ) );

        POINT       p;

        ::DragQueryPoint( m_hd, &p );

        RECT        rc;

        ::GetClientRect( hw, &rc );

        return ::PtInRect( &rc, p );
    }

    // This function returns the number of files dropped on the window by the current operation.
    UINT GetNumFiles( void )
    {
        return ::DragQueryFile( m_hd, 0xffffFFFF, NULL, 0 );
    }

    // This function gets the whole file path for a file, given its ordinal number.
    UINT GetDropedFile( UINT iFile )
    {
        bHandled = true;
        return ::DragQueryFile( m_hd, iFile, m_buff, MAX_PATH );
    }

    std_string GetDropedFileName()              { return std_string( m_buff ); }

#ifdef _WTL_USE_CSTRING
    // CString overload for DragQueryFile (not used here, might be used by a handler
    // which is implemented outside CDropFilesHandler<T>.
    UINT GetDropedFile( UINT iFile, CString &cs )
    {
        bHandled = true;

        UINT    ret = ::DragQueryFile( m_hd, iFile, m_buff, MAX_PATH );

        cs = m_buff;
        return ret;
    }

    //inline operator CString() const         { return CString( m_buff ); }
#endif
    // Other string overloads (such as std::string) might come handy...

    // This class can 'be' the currently held file's path.
    //inline operator const CHAR *() const    { return m_buff; }
}; // class CHDrop

//=======================================================================
//==============    CMainDlg
//=======================================================================
class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
                 public CDialogResize<CMainDlg>,
                 public CMessageFilter, public CIdleHandler
{
private:
    typedef std::pair<int, CMainDlg *>      ListCompare_lParamSort;
#if defined (CC_HAVE_ENUM_CLASS)
    enum class ListColumn                   { name, path, branch, uncommited };
#else
    class ListColumn { public: static const int name = 0; static const int path = 1; static const int branch = 2; static const int uncommited = 3; };
#endif
private:
    HACCEL              mHAccel;
    CListViewCtrl       mListView;
    CMenu               mMainMenu;

    void GlobalHandleException( const std::exception& ex );
    virtual BOOL PreTranslateMessage( MSG* pMsg );
    virtual BOOL OnIdle();
    void CloseDialog( int nVal );
    CString ListView_GetSelectedText( int col );
    void AddFile( const std::wstring& fname );

    static int CALLBACK List_Compare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );

    // Handler prototypes (uncomment arguments if needed):
    //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
    LRESULT OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnContextMenu( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnDropFiles( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnAppAbout( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_OpenInExplorer( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_OpenIniDirectory( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_RefreshRepositoryState( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnPopup_RefreshState( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_CheckForModifications( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Fetch( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Pull( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Push( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_RevisionGraph( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    HRESULT OnList_ColumnClick( int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/ );

    BEGIN_MSG_MAP_XX( CMainDlg )
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog );
        MESSAGE_HANDLER( WM_DESTROY, OnDestroy );
        MESSAGE_HANDLER( WM_CONTEXTMENU, OnContextMenu );
        MESSAGE_HANDLER( WM_DROPFILES, OnDropFiles );
        COMMAND_ID_HANDLER( IDCANCEL, OnCancel );
        COMMAND_ID_HANDLER( ID_FILE_ABOUT, OnAppAbout );
        COMMAND_ID_HANDLER( ID_FILE_QUIT, OnCancel );
        COMMAND_ID_HANDLER( ID_FILE_OPENINEXPLORER, OnFile_OpenInExplorer );
        COMMAND_ID_HANDLER( ID_FILE_OPENINIDIRECTORY, OnFile_OpenIniDirectory );
        COMMAND_ID_HANDLER( ID_FILE_REFRESHREPOSITORYSTATE, OnFile_RefreshRepositoryState );
        COMMAND_ID_HANDLER( ID_POPUP_REFRESHSTATE, OnPopup_RefreshState );
        COMMAND_ID_HANDLER( ID_GIT_CHECKFORMODIFICATIONS, OnGit_CheckForModifications );
        COMMAND_ID_HANDLER( ID_GIT_FETCH, OnGit_Fetch );
        COMMAND_ID_HANDLER( ID_GIT_PULL, OnGit_Pull );
        COMMAND_ID_HANDLER( ID_GIT_PUSH, OnGit_Push );
        COMMAND_ID_HANDLER( ID_GIT_REVISIONGRAPPH, OnGit_RevisionGraph );
        NOTIFY_HANDLER( IDC_LIST, LVN_COLUMNCLICK, OnList_ColumnClick );
        CHAIN_MSG_MAP( CDialogResize<CMainDlg> );
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
