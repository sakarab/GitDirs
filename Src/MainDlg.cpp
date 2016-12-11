#include "stdafx.h"
#include "MainDlg.h"
#include "AboutDlg.h"
#include <winClasses.h>
#include <winOSUtils.h>
#include "gd_Utils.h"
#include <atlmisc.h>
#include <boost/format.hpp>

//=======================================================================
//==============    CMainDlg
//=======================================================================
void CMainDlg::GlobalHandleException( const std::exception& ex )
{
    MessageBoxA( this->m_hWnd, ex.what(), "Error", MB_OK | MB_ICONHAND );
}

BOOL CMainDlg::PreTranslateMessage( MSG* pMsg )
{
    if ( mHAccel != NULL && ::TranslateAccelerator( m_hWnd, mHAccel, pMsg ) )
        return TRUE;
    return CWindow::IsDialogMessage( pMsg );
}

void CMainDlg::CloseDialog( int nVal )
{
    DestroyWindow();
    ::PostQuitMessage( nVal );
}

//static
int CALLBACK CMainDlg::List_Compare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
    ListCompare_lParamSort      *param = reinterpret_cast<ListCompare_lParamSort *>(lParamSort);
    CString                     str1, str2;

    param->second->mListView.GetItemText( lParam1, param->first, str1 );
    param->second->mListView.GetItemText( lParam2, param->first, str2 );
    return ccwin::case_insensitive_string_compare_ptr<wchar_t>()( static_cast<const wchar_t *>(str1), static_cast<const wchar_t *>(str2) );
}

BOOL CMainDlg::OnIdle()
{
    UIUpdateChildWindows();
    return FALSE;
}

LRESULT CMainDlg::OnContextMenu( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ )
{
    if ( reinterpret_cast<HWND>(wParam) != mListView.m_hWnd )
        return 0;

    WTL::CPoint     point( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
    CMenu           menu;

    menu.LoadMenu( IDR_MENU_POPUP );

    CMenuHandle     menuPopup = menu.GetSubMenu( 0 );
    CPoint          pt;

    GetCursorPos( &pt );
    menuPopup.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_VERTICAL, pt.x, pt.y, this->m_hWnd );
    return 0;
}

LRESULT CMainDlg::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    DlgResize_Init( true, true, WS_THICKFRAME | WS_CLIPCHILDREN );

    // center the dialog on the screen
    CenterWindow();

    // set icons
    HICON hIcon = AtlLoadIconImage( IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics( SM_CXICON ), ::GetSystemMetrics( SM_CYICON ) );
    SetIcon( hIcon, TRUE );
    HICON hIconSmall = AtlLoadIconImage( IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics( SM_CXSMICON ), ::GetSystemMetrics( SM_CYSMICON ) );
    SetIcon( hIconSmall, FALSE );

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT( pLoop != NULL );
    pLoop->AddMessageFilter( this );
    pLoop->AddIdleHandler( this );

    UIAddChildWindowContainer( m_hWnd );

    mMainMenu.Attach( GetMenu() );
    if ( m_hWnd != NULL )
        mHAccel = ::LoadAccelerators( ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE( IDR_MAINFRAME ) );

    mListView.Attach( GetDlgItem( IDC_LIST ) );
    mListView.SetExtendedListViewStyle( LVS_EX_FULLROWSELECT );
    mListView.InsertColumn( 0, TEXT( "Name" ), LVCFMT_LEFT, 100, 0 );
    mListView.InsertColumn( 1, TEXT( "Directory" ), LVCFMT_LEFT, 320, 0 );

    GitDirList      slist = ReadFolderList();

    for ( GitDirList::iterator it = slist.begin(), eend = slist.end() ; it != eend ; ++it )
    {
        int     list_count = mListView.GetItemCount();

        mListView.AddItem( list_count, 0, it->Name.c_str() );
        mListView.AddItem( list_count, 1, it->Directory.c_str() );
    }
    return TRUE;
}

LRESULT CMainDlg::OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    // unregister message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT( pLoop != NULL );
    pLoop->RemoveMessageFilter( this );
    pLoop->RemoveIdleHandler( this );

    return 0;
}

LRESULT CMainDlg::OnAppAbout( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CAboutDlg dlg;
    dlg.DoModal();
    return 0;
}

LRESULT CMainDlg::OnOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    // TODO: Add validation code 
    CloseDialog( wID );
    return 0;
}

LRESULT CMainDlg::OnCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CloseDialog( wID );
    return 0;
}

LRESULT CMainDlg::OnFile_OpenInExplorer( WORD, WORD, HWND, BOOL & )
{
    CString     sstr = ListView_GetSelectedText( 1 );

    if ( !sstr.IsEmpty() )
        ccwin::ExecuteProgram( boost::str( boost::wformat( L"explorer.exe %1%" ) % static_cast<const wchar_t *>(sstr) ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_CheckForModifications( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CString     sstr = ListView_GetSelectedText( 1 );

    if ( !sstr.IsEmpty() )
        ccwin::ExecuteProgram( MakeCommand( L"repostatus", sstr ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_Fetch( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CString     sstr = ListView_GetSelectedText( 1 );

    if ( !sstr.IsEmpty() )
        ccwin::ExecuteProgram( MakeCommand( L"fetch", sstr ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_Pull( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CString     sstr = ListView_GetSelectedText( 1 );

    if ( !sstr.IsEmpty() )
        ccwin::ExecuteProgram( MakeCommand( L"pull", sstr ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_Push( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CString     sstr = ListView_GetSelectedText( 1 );

    if ( !sstr.IsEmpty() )
        ccwin::ExecuteProgram( MakeCommand( L"push", sstr ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_RevisionGraph( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CString     sstr = ListView_GetSelectedText( 1 );

    if ( !sstr.IsEmpty() )
        ccwin::ExecuteProgram( MakeCommand( L"revisiongraph", sstr ) );
    return LRESULT();
}

HRESULT CMainDlg::OnList_ColumnClick( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/ )
{
    ListCompare_lParamSort      sort_data;

    sort_data.first = reinterpret_cast<NMLISTVIEW *>(pnmh)->iSubItem;
    sort_data.second = this;
    mListView.SortItemsEx( List_Compare, reinterpret_cast<LPARAM>(&sort_data) );
    return LRESULT();
}

CString CMainDlg::ListView_GetSelectedText( int col )
{
    CString     sstr;
    int         idx = mListView.GetSelectedIndex();

    if ( idx >= 0 )
        mListView.GetItemText( idx, col, sstr );
    return sstr;
}
