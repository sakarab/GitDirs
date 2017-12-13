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

#if !defined(GITDIRS_MAINDLG_H)
#define GITDIRS_MAINDLG_H

#include <atlframe.h>
#include <wtl_msg_map.h>
#include <utility>
#include <predef_cc.h>
#include <winClasses.h>
#include <wtlUtils.h>
#include "resource.h"
#include "gd_Utils.h"
#include "ListData.h"

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
#define WM_LIST_EDIT_RESULT     (WM_USER + 1)

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
                 public CDialogResize<CMainDlg>,
                 public ccwtl::CFormSize<CMainDlg>,
                 public CMessageFilter, public CIdleHandler
{
private:
    typedef std::unique_ptr<std::wstring>   unique_string;
    enum class ListEditResult               { error, success, cancel };
private:
    ListData            mDataBase;
    ListDataView        mDataView;
    HACCEL              mHAccel;
    CListViewCtrl       mListView;
    CMenu               mMainMenu;
    unique_string       mOldEditName;
    bool                mEscapeExit = false;
    bool                mInLabelEdit = false;

    void GlobalHandleException( const std::exception& ex );
    virtual BOOL PreTranslateMessage( MSG* pMsg );
    virtual BOOL OnIdle();
    void CloseDialog( int nVal );
    bool ListView_IsCheckBoxesVisible() const;
    std::wstring ListView_GetText( int idx, ListColumn col );
    std::wstring ListView_GetText_Checked( int idx, ListColumn col );
    std::wstring ListView_GetSelectedText( ListColumn col );
    std::wstring ListView_GetSelectedText_Checked( ListColumn col );
    void AddFile( const std::wstring& fname );
    void ReloadIni( ccwin::TIniFile& ini );
    void SortList( int column );
    bool UniqueName( int idx, const std::wstring& name );
    void RefreshRepoStateAndView( GitDirStateList& state_list );
    void MainMenu_Append( CMenuHandle menu );
    void PopupMenu_Append( CMenuHandle menu, const WStringList& groups );
    void SetFilter( const spFilter& filter );

    // Handler prototypes (uncomment arguments if needed):
    //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
    LRESULT OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnContextMenu( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnDropFiles( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnListEditResult( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );
    LRESULT OnAppAbout( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_OpenInExplorer( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_OpenIniDirectory( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_ReloadIni( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_SaveData( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_FetchAllRepositories( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnFile_RefreshRepositoryState( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnEdit_EditName( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnEdit_Delete( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnEdit_ShowCheckBoxes( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnEdit_ClearCheckBoxes( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnEdit_Options( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGroup_All( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGroup_MenuCommand( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGroup_SubMenuCommand( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnPopup_RefreshState( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_CheckForModifications( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Fetch( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Pull( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Push( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_Commit( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_ViewLog( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnGit_RevisionGraph( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ );
    LRESULT OnList_ColumnClick( int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/ );
    LRESULT OnList_BeginLabelEdit( int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/ );
    LRESULT OnList_EndLabelEdit( int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/ );
    LRESULT OnList_GetDispInfo( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/ );
    LRESULT OnList_FindItem( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/ );
    LRESULT OnList_KeyDown( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/ );
    LRESULT OnList_Click( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/ );

    BEGIN_MSG_MAP_XX( CMainDlg )
        MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog );
        MESSAGE_HANDLER( WM_DESTROY, OnDestroy );
        MESSAGE_HANDLER( WM_CONTEXTMENU, OnContextMenu );
        MESSAGE_HANDLER( WM_DROPFILES, OnDropFiles );
        MESSAGE_HANDLER( WM_LIST_EDIT_RESULT, OnListEditResult );
        COMMAND_ID_HANDLER( IDCANCEL, OnCancel );
        COMMAND_ID_HANDLER( ID_HELP_ABOUT, OnAppAbout );
        COMMAND_ID_HANDLER( ID_FILE_QUIT, OnCancel );
        COMMAND_ID_HANDLER( ID_FILE_OPENINEXPLORER, OnFile_OpenInExplorer );
        COMMAND_ID_HANDLER( ID_FILE_OPENINIDIRECTORY, OnFile_OpenIniDirectory );
        COMMAND_ID_HANDLER( ID_FILE_RELOADINI, OnFile_ReloadIni );
        COMMAND_ID_HANDLER( ID_FILE_SAVEDATA, OnFile_SaveData );
        COMMAND_ID_HANDLER( ID_FILE_FETCHALLREPOSITORIES, OnFile_FetchAllRepositories );
        COMMAND_ID_HANDLER( ID_FILE_REFRESHREPOSITORYSTATE, OnFile_RefreshRepositoryState );
        COMMAND_ID_HANDLER( ID_EDIT_EDITNAME, OnEdit_EditName );
        COMMAND_ID_HANDLER( ID_EDIT_DELETE_2, OnEdit_Delete );
        COMMAND_ID_HANDLER( ID_EDIT_SHOWCHECKBOXES, OnEdit_ShowCheckBoxes );
        COMMAND_ID_HANDLER( ID_EDIT_CLEARCHECKBOXES, OnEdit_ClearCheckBoxes );
        COMMAND_ID_HANDLER( ID_EDIT_OPTIONS, OnEdit_Options );
        COMMAND_ID_HANDLER( ID_GROUPS_ALL, OnGroup_All );
        COMMAND_RANGE_HANDLER( GROUPS_MENU_CommandID, GROUPS_MENU_CommandID + 999, OnGroup_MenuCommand );
        COMMAND_RANGE_HANDLER( GROUPS_MENU_SubMenuCommandID, GROUPS_MENU_SubMenuCommandID + 999, OnGroup_SubMenuCommand );
        COMMAND_ID_HANDLER( ID_POPUP_REFRESHSTATE, OnPopup_RefreshState );
        COMMAND_ID_HANDLER( ID_GIT_CHECKFORMODIFICATIONS, OnGit_CheckForModifications );
        COMMAND_ID_HANDLER( ID_GIT_FETCH, OnGit_Fetch );
        COMMAND_ID_HANDLER( ID_GIT_PULL, OnGit_Pull );
        COMMAND_ID_HANDLER( ID_GIT_PUSH, OnGit_Push );
        COMMAND_ID_HANDLER( ID_GIT_COMMIT, OnGit_Commit );
        COMMAND_ID_HANDLER( ID_GIT_VIEWLOG, OnGit_ViewLog );
        COMMAND_ID_HANDLER( ID_GIT_REVISIONGRAPPH, OnGit_RevisionGraph );
        NOTIFY_HANDLER( IDC_LIST, LVN_COLUMNCLICK, OnList_ColumnClick );
        NOTIFY_HANDLER( IDC_LIST, LVN_BEGINLABELEDIT, OnList_BeginLabelEdit );
        NOTIFY_HANDLER( IDC_LIST, LVN_ENDLABELEDIT, OnList_EndLabelEdit );
        NOTIFY_HANDLER( IDC_LIST, LVN_GETDISPINFO, OnList_GetDispInfo );
        NOTIFY_HANDLER( IDC_LIST, LVN_ODFINDITEM, OnList_FindItem );
        NOTIFY_HANDLER( IDC_LIST, LVN_KEYDOWN, OnList_KeyDown );
        NOTIFY_HANDLER( IDC_LIST, NM_CLICK, OnList_Click );
        CHAIN_MSG_MAP( ccwtl::CFormSize<CMainDlg> );
        CHAIN_MSG_MAP( CDialogResize<CMainDlg> );
        CHAIN_MSG_MAP( CUpdateUI<CMainDlg> );
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP( CMainDlg )
        DLGRESIZE_CONTROL( IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y )
    END_DLGRESIZE_MAP()

    BEGIN_UPDATE_UI_MAP( CMainDlg )
        UPDATE_ELEMENT( ID_EDIT_SHOWCHECKBOXES, UPDUI_MENUPOPUP )
    END_UPDATE_UI_MAP()
public:
	enum { IDD = IDD_MAINDLG };

    CMainDlg();
};

#endif
