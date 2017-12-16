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
#include "MainDlg.h"
#include "AboutDlg.h"
#include <atlmisc.h>
#include <atldlgs.h>
#include <smException.h>
#include <winOSUtils.h>
#include <winUtils.h>
#include <boost/format.hpp>
#include <boost/scope_exit.hpp>

//=======================================================================
//==============    CMainDlg
//=======================================================================
void CMainDlg::GlobalHandleException( const std::exception& ex )
{
    MessageBoxA( this->m_hWnd, ex.what(), "Error", MB_OK | MB_ICONHAND );
}

BOOL CMainDlg::PreTranslateMessage( MSG* pMsg )
{
    if ( !mInLabelEdit && mHAccel != NULL && ::TranslateAccelerator( m_hWnd, mHAccel, pMsg ) )
        return TRUE;
    if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == 0x1B )
        mEscapeExit = true;

    BOOL    ret = CWindow::IsDialogMessage( pMsg );

    if ( !ret && mEscapeExit )
        mEscapeExit = false;
    return ret;
}

void CMainDlg::CloseDialog( int nVal )
{
    DestroyWindow();
    ::PostQuitMessage( nVal );
}

bool CMainDlg::ListView_IsCheckBoxesVisible() const
{
    return (mListView.GetExtendedListViewStyle() & LVS_EX_CHECKBOXES) && (mListView.GetStyle() & LVS_REPORT);
}

void CMainDlg::AddFile( const std::wstring& fname )
{
    std::wstring        skey = ccwin::ExtractFileName( fname );
    std::wstring        svalue = fname;

    mDataView.AddItem( mDataBase, skey, svalue );
    mListView.SetItemCount( mDataView.Count() );
    mListView.Invalidate( FALSE );
}

void CMainDlg::ReloadIni( ccwin::TIniFile& ini )
{
    mDataBase.LoadFromIni( ini );
    mDataView.LoadFromDb( mDataBase );
    mListView.SetItemCount( mDataView.Count() );
}

void CMainDlg::SortList( int column )
{
    if ( column >= 0 )
    {
        mDataView.SortColumn( static_cast<ListColumn>(column) );
        mListView.Invalidate( FALSE );
    }
}

bool CMainDlg::UniqueName( int idx, const std::wstring& name )
{
    for ( int n = 0, eend = mDataView.Count() ; n < eend ; ++n )
    {
        if ( n == idx )
            continue;
        if ( ListView_GetText( n, ListColumn::name ) == name )
            return false;
    }
    return true;
}

void CMainDlg::RefreshRepoStateAndView( GitDirStateList& state_list )
{
    GitGetRepositoriesState( state_list );

    for ( const GitDirStateList::value_type& item : state_list )
    {
        ListDataView::list_size_type    idx = mDataView.FindItem( item.Name );

        if ( idx != ListDataView::npos )
        {
            spListDataItem&     data_item = mDataView.Item( idx );

            data_item->Remotes( item.Remotes );
            data_item->Branch( ccwin::WidenStringStrict( item.Branch ) );
            data_item->Uncommited( item.Uncommited );
            data_item->NeedsUpdate( item.NeedsUpdate );
            mListView.Update( idx );
        }
    }
}

void CMainDlg::MainMenu_Append( CMenuHandle menu )
{
    int     last_menu_idx = menu.GetMenuItemCount() - 1;

    for ( int n = last_menu_idx ; n >= GROUPS_MENU_HeaderCount - 1 ; --n )
        menu.DeleteMenu( n, MF_BYPOSITION );

    const WStringList&      groups = mDataBase.AllGroups();

    if ( !groups.empty() )
    {
        menu.AppendMenu( MF_SEPARATOR );

        int     count = 0;
        int     checked_menu_id = ID_GROUPS_ALL;

        for ( const std::wstring& sstr : groups )
        {
            menu.AppendMenu( MF_STRING, GROUPS_MENU_CommandID + count, sstr.c_str() );
            if ( checked_menu_id == ID_GROUPS_ALL && sstr == mDataView.Group() )
                checked_menu_id = GROUPS_MENU_CommandID + count;
            ++count;
        }
        SetMenuRadioRecursive( menu, checked_menu_id );
    }
}

void CMainDlg::PopupMenu_Append( CMenuHandle menu, const WStringList& groups )
{
    if ( mDataBase.AllGroups().empty() )
        return;

    CMenuHandle     submenu;
    int             count = 0;

    submenu.CreateMenu();
    for ( const std::wstring& sstr : mDataBase.AllGroups() )
    {
        bool    in_group = std::find( groups.begin(), groups.end(), sstr ) != groups.end();

        submenu.AppendMenu( MF_STRING, GROUPS_MENU_SubMenuCommandID + count, sstr.c_str() );
        SetMenuCheck( submenu, count, in_group );
        ++count;
    }
    menu.AppendMenu( MF_STRING | MF_POPUP, submenu.m_hMenu, L"Groups" );
}

void CMainDlg::SetFilter( const spFilter& filter )
{
    mDataView.Filter( mDataBase, filter );
    mListView.SetItemCount( mDataView.Count() );
    mListView.Invalidate( FALSE );
}

BOOL CMainDlg::OnIdle()
{
    if ( mListView_LastSelected >= 0 && mInfoDlg )
    {
        mInfoDlg->SetInfo( *mDataView.Item( mListView_LastSelected ) );
        mListView_LastSelected = -1;
    }
    UIUpdateChildWindows();
    return FALSE;
}

LRESULT CMainDlg::OnContextMenu( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ )
{
    if ( reinterpret_cast<HWND>(wParam) != mListView.m_hWnd )
        return 0;

    int     idx = mListView.GetSelectedIndex();

    if ( idx < 0 )
        return 0;

    WTL::CPoint     pt( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
    CMenu           menu;

    menu.LoadMenu( IDR_MENU_POPUP );

    CMenuHandle     menuPopup = menu.GetSubMenu( 0 );

    PopupMenu_Append( menuPopup, mDataView.Item( idx )->Groups() );
    menuPopup.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_VERTICAL, pt.x, pt.y, this->m_hWnd );
    return 0;
}

LRESULT CMainDlg::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    DlgResize_Init( true, true, WS_THICKFRAME | WS_CLIPCHILDREN );
    DragAcceptFiles();

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
    mListView.InsertColumn( static_cast<int>(ListColumn::name), TEXT( "Name" ), LVCFMT_LEFT, 200, 0 );
    mListView.InsertColumn( static_cast<int>(ListColumn::path), TEXT( "Directory" ), LVCFMT_LEFT, 260, 0 );
    mListView.InsertColumn( static_cast<int>(ListColumn::n_repos), TEXT( "NR" ), LVCFMT_CENTER, 40, 0 );
    mListView.InsertColumn( static_cast<int>(ListColumn::branch), TEXT( "Branch" ), LVCFMT_LEFT, 140, 0 );
    mListView.InsertColumn( static_cast<int>(ListColumn::uncommited), TEXT( "Uncommited" ), LVCFMT_CENTER, 80, 0 );
    mListView.InsertColumn( static_cast<int>(ListColumn::needs), TEXT( "Update" ), LVCFMT_CENTER, 80, 0 );

    ccwin::TIniFile     ini( GetIniFileName() );

    CFormSize::Load( ini );

    mDataView.LoadState( ini );
    ReloadIni( ini );
    MainMenu_Append( mMainMenu.GetSubMenu( GROUPS_MENU_Position ) );
    ListView_SetShowCheckBoxes( ini.ReadBool( IniSections::ViewState, IniKeys::ViewState_ShowCheckBoxes, false ) );
    mViewState.Load( ini );
    return TRUE;
}

LRESULT CMainDlg::OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    // unregister message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT( pLoop != NULL );
    pLoop->RemoveMessageFilter( this );
    pLoop->RemoveIdleHandler( this );

    ccwin::TIniFile     ini( GetIniFileName() );

    CFormSize::Save( ini );
    mDataView.SaveState( ini );
    ini.WriteInteger( IniSections::ViewState, IniKeys::ViewState_ShowCheckBoxes, ListView_GetShowCheckBoxes() );
    mViewState.Save( ini );

    return 0;
}

LRESULT CMainDlg::OnDropFiles( UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    CHDrop<WCHAR>       drop( wParam );

    for ( int n = 0, eend = drop.GetNumFiles() ; n < eend ; ++n )
        if ( drop.GetDropedFile( n ) )
            AddFile( drop.GetDropedFileName() );
    return 0;
}

LRESULT CMainDlg::OnListEditResult( UINT, WPARAM wParam, LPARAM lParam, BOOL & )
{
    ListEditResult      result = static_cast<ListEditResult>(wParam);

    switch ( result )
    {
        case ListEditResult::cancel :
            // do nothing
            break;
        case ListEditResult::error:
            Throw_NoUniqueName( *mOldEditName );
            break;
        case ListEditResult::success:
            mListView.Update( static_cast<int>(lParam) );
            break;
    }
    return 0;
}

LRESULT CMainDlg::OnAppAbout( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CAboutDlg       dlg( L"GitDirs" );

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
    if ( mEscapeExit )
        mEscapeExit = false;
    else
        CloseDialog( wID );
    return 0;
}

LRESULT CMainDlg::OnView_OpenInExplorer( WORD, WORD, HWND, BOOL & )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( boost::str( boost::wformat( L"explorer.exe %1%" ) % sstr ) );
    return LRESULT();
}

LRESULT CMainDlg::OnView_OpenIniDirectory( WORD, WORD, HWND, BOOL & )
{
    ccwin::TCommonDirectories   dirs;
    std::wstring                fname = ccwin::IncludeTrailingPathDelimiter( dirs.AppDataDirectory_UserLocal() ).append( L"GitDirs.ini" );

    ccwin::ExecuteProgram( boost::str( boost::wformat( L"explorer.exe /select,\"%1%\"" ) % fname ) );
    return LRESULT();
}

LRESULT CMainDlg::OnFile_ReloadIni( WORD, WORD, HWND, BOOL & )
{
    ccwin::TIniFile     ini( GetIniFileName() );

    ReloadIni( ini );
    return LRESULT();
}

LRESULT CMainDlg::OnFile_ImportWorkset( WORD, WORD, HWND, BOOL & )
{
    std::wstring    fname = OpenDlg( L"", ccwin::ExtractFileName( mViewState.Workset_Filename ),
#if defined (WINDOWS_XP_BUILD)
                                     OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,                  // DWORD dwFlags = 
#else
                                     FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST,
#endif
                                     { open_filter_spec( L"All Files (*.*)", L"*.*" ) },
                                     *this );

    if ( !fname.empty() )
    {
        mViewState.Workset_Filename = fname;

        ccwin::TIniFile     ini( mViewState.Workset_Filename );

        mDataBase.WorksetFromString( ini.ReadString( IniSections::Data, IniKeys::Data_Marks, L"" ) );
        mListView.Invalidate( FALSE );
    }
    return LRESULT();
}

LRESULT CMainDlg::OnFile_ExportWorkset( WORD, WORD, HWND, BOOL & )
{
    std::wstring    fname = SaveDlg( L"", ccwin::ExtractFileName( mViewState.Workset_Filename ),
#if defined (WINDOWS_XP_BUILD)
                                     OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR,  // DWORD dwFlags = 
#else
                                     FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT | FOS_NOTESTFILECREATE,
#endif
                                     { open_filter_spec( L"All Files (*.*)", L"*.*" ) },
                                     *this );


    if ( !fname.empty() )
    {
        mViewState.Workset_Filename = fname;

        ccwin::TIniFile     ini( mViewState.Workset_Filename );

        ini.WriteString( IniSections::Data, IniKeys::Data_Marks, mDataBase.WorksetAsString().c_str() );
    }
    return LRESULT();
}

LRESULT CMainDlg::OnFile_SaveData( WORD, WORD, HWND, BOOL & )
{
    ccwin::TIniFile     ini( GetIniFileName() );

    mDataBase.SaveToIni( ini );
    return LRESULT();
}

LRESULT CMainDlg::OnFile_FetchAllRepositories( WORD, WORD, HWND, BOOL & )
{
    return LRESULT();
}

LRESULT CMainDlg::OnFile_RefreshRepositoryState( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    GitDirStateList     state_list;

    for ( const spListDataItem& item : mDataView )
        state_list.push_back( GitDirStateItem( item->Name(), item->Directory() ) );
    RefreshRepoStateAndView( state_list );
    return LRESULT();
}

LRESULT CMainDlg::OnEdit_EditName( WORD, WORD, HWND, BOOL & )
{
    int         idx = mListView.GetSelectedIndex();

    if ( idx >= 0 )
        mListView.EditLabel( idx );
    return LRESULT();
}

LRESULT CMainDlg::OnEdit_Delete( WORD, WORD, HWND, BOOL & )
{
    int         idx = mListView.GetSelectedIndex();

    if ( idx >= 0 && MessageBox( L"Delete selected repository link?\nRepository will remain intact.", L"Confirm", MB_ICONQUESTION | MB_OKCANCEL ) == IDOK )
    {
        spListDataItem      item = mDataView.Item( idx );

        mDataView.DeleteItem( mDataBase, item->Name() );
        mListView.SetItemCount( mDataView.Count() );
        mListView.Invalidate( FALSE );
    }
    return LRESULT();
}

LRESULT CMainDlg::OnView_ShowCheckBoxes( WORD, WORD, HWND, BOOL & )
{
    ListView_SetShowCheckBoxes( !ListView_GetShowCheckBoxes() );
    return LRESULT();
}

LRESULT CMainDlg::OnView_ShowInfoDialog( WORD, WORD, HWND, BOOL & )
{
    if ( mInfoDlg )
    {
        mInfoDlg->CloseDialog();
    }
    else
    {
        Procedure                   on_close = [this]() {
                                        mInfoDlg.reset();
                                        UISetCheck( ID_VIEW_SHOWINFORMATIONDIALOG, false );
                                    };
        qpMonitorDlg::deleter_type  form_deleter = []( CInfoDlg *frm ) { frm->CloseDialog(); };
        qpMonitorDlg                dlg( new CInfoDlg( on_close ),form_deleter );

        dlg->Create( *this );
        dlg->ShowWindow( SW_NORMAL );
        mInfoDlg = std::move( dlg );
        UISetCheck( ID_VIEW_SHOWINFORMATIONDIALOG, true );
        mListView_LastSelected = mListView.GetSelectedIndex();
    }
    return 0;
}

LRESULT CMainDlg::OnEdit_ClearCheckBoxes( WORD, WORD, HWND, BOOL & )
{
    if ( ListView_IsCheckBoxesVisible() )
    {
        for ( spListDataItem& item : mDataView )
            item->Checked( false );
        mListView.Invalidate( FALSE );
    }
    return LRESULT();
}

LRESULT CMainDlg::OnEdit_Options( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    return 0;
}

LRESULT CMainDlg::OnGroup_All( WORD, WORD wID, HWND, BOOL & )
{
    spFilter    filter;

    if ( wID == ID_SPECIAL_CHECKED )
        filter = std::make_shared<FilterChecked>( true );
    else if ( wID == ID_SPECIAL_UNCHECKED )
        filter = std::make_shared<FilterChecked>( false );
    else if ( wID == ID_SPECIAL_TAGGED )
        filter = std::make_shared<FilterTagged>( true );
    else if ( wID == ID_SPECIAL_UNTAGGED )
        filter = std::make_shared<FilterTagged>( false );
    else if ( wID >= GROUPS_MENU_CommandID && wID <= GROUPS_MENU_CommandID + 999 )
        filter = std::make_shared<FilterGroup>( mDataBase.AllGroups().at( wID - GROUPS_MENU_CommandID ) );
    else
        filter = std::make_shared<FilterGroup>( std::wstring() );

    SetFilter( filter );

    CMenuHandle     menu = mMainMenu.GetSubMenu( GROUPS_MENU_Position );

    SetMenuRadioRecursive( menu, wID );
    return LRESULT();
}

LRESULT CMainDlg::OnGroup_SubMenuCommand( WORD, WORD wID, HWND, BOOL & )
{
    int                 idx = mListView.GetSelectedIndex();

    if ( idx >= 0 )
    {
        ListDataItem&           data_item = *mDataView.Item( idx );
        const std::wstring&     group = mDataBase.AllGroups().at( wID - GROUPS_MENU_SubMenuCommandID );

        if ( data_item.InGroup( group ) )
        {
            mDataView.Item( idx )->RemoveFromGroup( group );

            if ( group == mDataView.Group() )
            {
                mDataView.RemoveItem( data_item.Name() );
                mListView.SetItemCount( mDataView.Count() );
            }
        }
        else
            mDataView.Item( idx )->AddToGroup( group );
    }
    return LRESULT();
}

LRESULT CMainDlg::OnPopup_RefreshState( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    int                 idx = mListView.GetSelectedIndex();

    if ( idx >= 0 )
    {
        spListDataItem      item = mDataView.Item( idx );
        GitDirStateList     state_list;

        state_list.push_back( GitDirStateItem( item->Name(), item->Directory() ) );
        RefreshRepoStateAndView( state_list );
        mListView_LastSelected = idx;
    }
    return 0;
}

LRESULT CMainDlg::OnGit_CheckForModifications( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( MakeCommand( L"repostatus", sstr.c_str() ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_Fetch( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( MakeCommand( L"fetch", sstr.c_str() ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_Pull( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( MakeCommand( L"pull", sstr.c_str() ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_Push( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( MakeCommand( L"push", sstr.c_str() ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_Commit( WORD, WORD, HWND, BOOL & )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( MakeCommand( L"commit", sstr.c_str() ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_ViewLog( WORD, WORD, HWND, BOOL & )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( MakeCommand( L"log", sstr.c_str() ) );
    return LRESULT();
}

LRESULT CMainDlg::OnGit_RevisionGraph( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( MakeCommand( L"revisiongraph", sstr.c_str() ) );
    return LRESULT();
}

LRESULT CMainDlg::OnList_ItemChanged( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/ )
{
    NMLISTVIEW      *pNMListView = reinterpret_cast<NM_LISTVIEW *>(pnmh);

    if ( pNMListView->iItem != -1 && (pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED) )
        mListView_LastSelected = pNMListView->iItem;
    return LRESULT();
}

LRESULT CMainDlg::OnList_ColumnClick( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/ )
{
    SortList( reinterpret_cast<NMLISTVIEW *>(pnmh)->iSubItem );
    return LRESULT();
}

LRESULT CMainDlg::OnList_BeginLabelEdit( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/ )
{
    LVITEM      lvitem = reinterpret_cast<NMLVDISPINFO *>(pnmh)->item;

    mOldEditName = std::make_unique<std::wstring>( ListView_GetText( lvitem.iItem, ListColumn::name ) );
    mInLabelEdit = true;
    return FALSE;
}

LRESULT CMainDlg::OnList_EndLabelEdit( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/ )
{
    LVITEM      lvitem = reinterpret_cast<NMLVDISPINFO *>(pnmh)->item;
    HRESULT     result = TRUE;

    if ( lvitem.pszText == nullptr || std::wstring( lvitem.pszText ) == *mOldEditName )
    {
        PostMessage( WM_LIST_EDIT_RESULT, static_cast<WPARAM>(ListEditResult::cancel), 0 );
    }
    else if ( !mDataBase.IsUniqueKey( std::wstring( lvitem.pszText ) ) )
    {
        mOldEditName = std::make_unique<std::wstring>( std::wstring( lvitem.pszText ) );
        PostMessage( WM_LIST_EDIT_RESULT, static_cast<WPARAM>(ListEditResult::error), 0 );
        result = FALSE;
    }
    else
    {
        mDataView.Item( lvitem.iItem )->Name( std::wstring( lvitem.pszText ) );
        PostMessage( WM_LIST_EDIT_RESULT, static_cast<WPARAM>(ListEditResult::success), lvitem.iItem );
    }
    mInLabelEdit = false;
    return result;
}

void CopyItemText( LV_ITEM& lv_item, const wchar_t *text )
{
    lstrcpyn( lv_item.pszText, text, lv_item.cchTextMax );
}

LRESULT CMainDlg::OnList_GetDispInfo( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/ )
{
    NMLVDISPINFO            *pDispInfo = reinterpret_cast<NMLVDISPINFO *>(pNMHDR);
    LV_ITEM &               lv_item = pDispInfo->item;
    int                     itemid = lv_item.iItem;           //Which item number?
    const ListDataItem&     item = *mDataView.Item( itemid );

    //Do the list need text information?
    if ( lv_item.mask & LVIF_TEXT )
        CopyItemText( lv_item, item.GetText( static_cast<ListColumn>(lv_item.iSubItem) ).c_str() );

    //Do the list need image information?
    if ( lv_item.mask & LVIF_IMAGE )
    {
        //Set which image to use
        //pItem.iImage = m_database[itemid].m_image;

        //Show check box?
        if ( ListView_IsCheckBoxesVisible() )
        {
            //To enable check box, we have to enable state mask...
            lv_item.mask |= LVIF_STATE;
            lv_item.stateMask = LVIS_STATEIMAGEMASK;

            if ( item.Checked() )
                lv_item.state = INDEXTOSTATEIMAGEMASK( 2 );          //Turn check box on
            else
                lv_item.state = INDEXTOSTATEIMAGEMASK( 1 );          //Turn check box off
        }
    }
    return 0;
}

LRESULT CMainDlg::OnList_FindItem( int, LPNMHDR pNMHDR, BOOL & )
{
    // pNMHDR has information about the item we should find
    // In pResult we should save which item that should be selected
    NMLVFINDITEM    *pFindInfo = reinterpret_cast<NMLVFINDITEM *>(pNMHDR);

    /* pFindInfo->iStart is from which item we should search.
    We search to bottom, and then restart at top and will stop
    at pFindInfo->iStart, unless we find an item that match
    */

    // Set the default return value to -1
    // That means we didn't find any match.
    LRESULT     result = -1;

    //Is search NOT based on string?
    //This will probably never happend...
    if ( (pFindInfo->lvfi.flags & LVFI_STRING) != 0 )
    {
        //This is the string we search for
        std::wstring    searchstr( pFindInfo->lvfi.psz );
        size_t          startPos = pFindInfo->iStart;

        //Is startPos outside the list (happens if last item is selected)
        if ( startPos >= mDataView.Count() )
            startPos = 0;

        ListDataView::list_size_type    idx = mDataView.FindItemCI_fromPos( searchstr, searchstr.size(), startPos );

        if ( idx == ListDataView::npos && startPos != 0 )
            idx = mDataView.FindItemCI( searchstr, searchstr.size() );
        if ( idx != ListDataView::npos )
            result = idx;
    }
    return result;
}

LRESULT CMainDlg::OnList_KeyDown( int, LPNMHDR pNMHDR, BOOL & )
{
    LV_KEYDOWN      *pLVKeyDown = reinterpret_cast<LV_KEYDOWN *>(pNMHDR);

    if ( pLVKeyDown->wVKey == VK_SPACE && ListView_IsCheckBoxesVisible() )
    {
        int         idx = mListView.GetSelectedIndex();

        if ( idx >= 0 )
        {
            mDataView.Item( idx )->ToggleChecked();
            mListView.Update( idx );
        }
    }
    return LRESULT();
}

LRESULT CMainDlg::OnList_Click( int, LPNMHDR pNMHDR, BOOL & )
{
    NMLISTVIEW      *pNMListView = reinterpret_cast<NM_LISTVIEW *>(pNMHDR);
    LVHITTESTINFO   hitinfo;

    hitinfo.pt = pNMListView->ptAction;

    int             idx = mListView.HitTest( &hitinfo );

    // We hit one item... did we hit state image (check box)?
    // This test only works if we are in list or report mode.
    // hitinfo.flags must contain only LVHT_ONITEMSTATEICON flag
    if ( idx >= -1 && hitinfo.flags == LVHT_ONITEMSTATEICON )
    {
        mDataView.Item( idx )->ToggleChecked();
        mListView.Update( idx );
    }
    return LRESULT();
}

CMainDlg::CMainDlg()
    : CFormSize( std::wstring( L"FormSize" ), std::wstring( L"CMainDlg_" ) )
{
}

std::wstring CMainDlg::ListView_GetText( int idx, ListColumn col )
{
    if ( !mDataView.IndexInBounds( idx ) )
        return std::wstring();
    return mDataView.Item( idx )->GetText( col );
}

std::wstring CMainDlg::ListView_GetText_Checked( int idx, ListColumn col )
{
    std::wstring    result = ListView_GetText( idx, col );

    if ( result.empty() )
        throw cclib::BaseException( boost::str( boost::format( "Repository\n%1%\nhas no directory assigned." ) % ccwin::NarrowStringStrict( result ) ) );
    else if ( !ccwin::DirectoryExists( result ) )
        throw cclib::BaseException( boost::str( boost::format( "Repository\n%1%\nis not present." ) % ccwin::NarrowStringStrict( result ) ) );
    return result;
}

std::wstring CMainDlg::ListView_GetSelectedText( ListColumn col )
{
    return ListView_GetText( mListView.GetSelectedIndex(), col );
}

std::wstring CMainDlg::ListView_GetSelectedText_Checked( ListColumn col )
{
    return ListView_GetText_Checked( mListView.GetSelectedIndex(), col );
}

void CMainDlg::ListView_SetShowCheckBoxes( bool value )
{
    DWORD   style = mListView.GetExtendedListViewStyle();

    if ( value )
    {
        mListView.SetExtendedListViewStyle( style | LVS_EX_CHECKBOXES );
    }
    else
    {
        mListView.SetExtendedListViewStyle( style & ~LVS_EX_CHECKBOXES );
    }
    UISetCheck( ID_VIEW_SHOWCHECKBOXES, value );
}

bool CMainDlg::ListView_GetShowCheckBoxes()
{
    return (UIGetState( ID_VIEW_SHOWCHECKBOXES ) & UPDUI_CHECKED) != 0;
}
