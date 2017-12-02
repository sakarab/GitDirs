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
    return CWindow::IsDialogMessage( pMsg );
}

void CMainDlg::CloseDialog( int nVal )
{
    try
    {
        SaveMarks();
    }
    catch ( const std::exception& ex )
    {
        GlobalHandleException( ex );
    }
    DestroyWindow();
    ::PostQuitMessage( nVal );
}

void CMainDlg::AddListLine( const std::wstring& name, const std::wstring& directory )
{
    int     list_count = mListView.GetItemCount();

    mListView.AddItem( list_count, static_cast<int>(ListColumn::name), name.c_str() );
    mListView.AddItem( list_count, static_cast<int>(ListColumn::path), directory.c_str() );
}

void CMainDlg::AddFile( const std::wstring& fname )
{
    std::wstring        skey = ccwin::ExtractFileName( fname );
    std::wstring        svalue = fname;

    if ( !UniqueName( -1, skey ) )
        Throw_NoUniqueName( skey );

    AddListLine( skey, svalue );

    ccwin::TIniFile     ini( GetIniFileName() );

    ini.WriteString( IniSections::Repositories, skey.c_str(), svalue.c_str() );
}

void CMainDlg::ReloadIni()
{
    GitDirList      slist = ReadFolderList();

    mViewState.SortColumn = -1;
    for ( GitDirList::iterator it = slist.begin(), eend = slist.end() ; it != eend ; ++it )
        AddListLine( it->Name(), it->Directory() );
}

void CMainDlg::SortList( int column )
{
    if ( column < 0 )
        ReloadIni();
    else
    {
        ListCompare_lParamSort      sort_data;

        sort_data.first = column;
        sort_data.second = this;
        mListView.SortItemsEx( List_Compare, reinterpret_cast<LPARAM>(&sort_data) );
    }
}

bool CMainDlg::UniqueName( int idx, const std::wstring& name )
{
    for ( int n = 0, eend = mListView.GetItemCount() ; n < eend ; ++n )
    {
        if ( n == idx )
            continue;
        if ( ListView_GetText( n, ListColumn::name ) == name )
            return false;
    }
    return true;
}

void CMainDlg::Throw_NoUniqueName( const std::wstring& name )
{
    std::string     msg = boost::str( boost::format( "There already is an entry with the name '%1%'" ) % ccwin::NarrowStringStrict( name ) );

    throw cclib::BaseException( msg );
}

void CMainDlg::RefreshRepoStateAndView( GitDirStateList& state_list )
{
    GitGetRepositoriesState( state_list );

    for ( const GitDirStateList::value_type& item : state_list )
    {
        mListView.SetItemText( item.VisualIndex, static_cast<int>(ListColumn::n_repos), boost::str( boost::wformat( L"%1%" ) % item.NRepos ).c_str() );
        mListView.SetItemText( item.VisualIndex, static_cast<int>(ListColumn::branch), ccwin::WidenStringStrict( item.Branch ).c_str() );
        mListView.SetItemText( item.VisualIndex, static_cast<int>(ListColumn::uncommited), item.Uncommited ? L"Yes" : L"No" );
        mListView.SetItemText( item.VisualIndex, static_cast<int>(ListColumn::needs), item.NeedsUpdate ? L"Yes" : L"No" );
    }
}

void CMainDlg::LoadMarks()
{
    if ( (mListView.GetExtendedListViewStyle() & LVS_EX_CHECKBOXES) == 0 )
        return;

    std::vector<std::wstring>   slist = ::LoadMarks();

    for ( int n = 0, eend = mListView.GetItemCount() ; n < eend ; ++n )
    {
        std::wstring    name = ListView_GetText( n, ListColumn::name );

        if ( std::find( slist.begin(), slist.end(), name ) != slist.end() )
            mListView.SetCheckState( n, true );
    }
}

void CMainDlg::SaveMarks()
{
    if ( (mListView.GetExtendedListViewStyle() & LVS_EX_CHECKBOXES) == 0 )
        return;

    std::vector<std::wstring>   slist;

    for ( int n = 0, eend = mListView.GetItemCount(); n < eend; ++n )
        if ( mListView.GetCheckState( n ) )
            slist.push_back( ListView_GetText( n, ListColumn::name ) );
    ::SaveMarks( slist );
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

    WTL::CPoint     pt( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
    CMenu           menu;

    menu.LoadMenu( IDR_MENU_POPUP );

    CMenuHandle     menuPopup = menu.GetSubMenu( 0 );

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

    ReloadIni();
    mViewState.Load();
    if ( mViewState.SortColumn >= 0 )
        SortList( mViewState.SortColumn );
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
    mViewState.Save();

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
        {   // delete old key, save new
            ccwin::TIniFile             ini( GetIniFileName() );

            ini.EraseKey( IniSections::Repositories, mOldEditName->c_str() );
            ini.WriteString( IniSections::Repositories,
                             ListView_GetText( lParam, ListColumn::name ).c_str(),
                             ListView_GetText( lParam, ListColumn::path ).c_str() );
            break;
        }
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

LRESULT CMainDlg::OnFile_OpenInExplorer( WORD, WORD, HWND, BOOL & )
{
    std::wstring    sstr = ListView_GetSelectedText_Checked( ListColumn::path );

    if ( !sstr.empty() )
        ccwin::ExecuteProgram( boost::str( boost::wformat( L"explorer.exe %1%" ) % sstr ) );
    return LRESULT();
}

LRESULT CMainDlg::OnFile_OpenIniDirectory( WORD, WORD, HWND, BOOL & )
{
    ccwin::TCommonDirectories   dirs;
    std::wstring                fname = ccwin::IncludeTrailingPathDelimiter( dirs.AppDataDirectory_UserLocal() ).append( L"GitDirs.ini" );

    ccwin::ExecuteProgram( boost::str( boost::wformat( L"explorer.exe /select,\"%1%\"" ) % fname ) );
    return LRESULT();
}

LRESULT CMainDlg::OnFile_ReloadIni( WORD, WORD, HWND, BOOL & )
{
    mListView.DeleteAllItems();
    ReloadIni();
    return LRESULT();
}

LRESULT CMainDlg::OnFile_FetchAllRepositories( WORD, WORD, HWND, BOOL & )
{
    return LRESULT();
}

LRESULT CMainDlg::OnFile_RefreshRepositoryState( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    CString             sstr;
    GitDirStateList     state_list;

    for ( int n = 0, eend = mListView.GetItemCount() ; n < eend ; ++n )
        state_list.push_back( GitDirStateItem( n, ListView_GetText_Checked( n, ListColumn::path ).c_str() ) );
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
        ccwin::TIniFile     ini( GetIniFileName() );

        ini.EraseKey( IniSections::Repositories, ListView_GetText( idx, ListColumn::name ).c_str() );
        mListView.DeleteItem( idx );
    }
    return LRESULT();
}

LRESULT CMainDlg::OnEdit_ShowCheckBoxes( WORD, WORD, HWND, BOOL & )
{
    DWORD   style = mListView.GetExtendedListViewStyle();
    bool    state = (UIGetState( ID_EDIT_SHOWCHECKBOXES ) & UPDUI_CHECKED) != 0;

    state = !state;
    if ( state )
    {
        mListView.SetExtendedListViewStyle( style | LVS_EX_CHECKBOXES );
        LoadMarks();
    }
    else
    {
        SaveMarks();
        mListView.SetExtendedListViewStyle( style & ~LVS_EX_CHECKBOXES );
    }
    UISetCheck( ID_EDIT_SHOWCHECKBOXES, state );
    return LRESULT();
}

LRESULT CMainDlg::OnEdit_ClearCheckBoxes( WORD, WORD, HWND, BOOL & )
{
    if ( (mListView.GetExtendedListViewStyle() & LVS_EX_CHECKBOXES) == 0 )
        ::SaveMarks( std::vector<std::wstring>() );
    else for ( int n = 0, eend = mListView.GetItemCount() ; n < eend ; ++n )
        mListView.SetCheckState( n, false );
    return LRESULT();
}

LRESULT CMainDlg::OnEdit_Options( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    return 0;
}

LRESULT CMainDlg::OnPopup_RefreshState( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
    int                 idx = mListView.GetSelectedIndex();

    if ( idx >= 0 )
    {
        GitDirStateList     state_list;

        state_list.push_back( GitDirStateItem( idx, ListView_GetText_Checked( idx, ListColumn::path ) ) );
        RefreshRepoStateAndView( state_list );
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

HRESULT CMainDlg::OnList_ColumnClick( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/ )
{
    mViewState.SortColumn = reinterpret_cast<NMLISTVIEW *>(pnmh)->iSubItem;
    SortList( mViewState.SortColumn );
    return LRESULT();
}

HRESULT CMainDlg::OnList_BeginLabelEdit( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/ )
{
    LVITEM      lvitem = reinterpret_cast<NMLVDISPINFO *>(pnmh)->item;

    mOldEditName = std::make_unique<std::wstring>( ListView_GetText( lvitem.iItem, ListColumn::name ) );
    mInLabelEdit = true;
    return FALSE;
}

HRESULT CMainDlg::OnList_EndLabelEdit( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/ )
{
    LVITEM      lvitem = reinterpret_cast<NMLVDISPINFO *>(pnmh)->item;
    HRESULT     result = TRUE;

    if ( lvitem.pszText == nullptr )
    {
        PostMessage( WM_LIST_EDIT_RESULT, static_cast<WPARAM>(ListEditResult::cancel), 0 );
    }
    else if ( !UniqueName( lvitem.iItem, lvitem.pszText ) )
    {
        mOldEditName = std::make_unique<std::wstring>( std::wstring( lvitem.pszText ) );
        PostMessage( WM_LIST_EDIT_RESULT, static_cast<WPARAM>(ListEditResult::error), 0 );
        result = FALSE;
    }
    else
    {
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
    const ListDataItem&     item = mData.Item( itemid );

    //Do the list need text information?
    if ( lv_item.mask & LVIF_TEXT )
    {

        //Which column?
        if ( lv_item.iSubItem == 0 )
            CopyItemText( lv_item, item.Name().c_str() );
        else if ( lv_item.iSubItem == 1 )
            CopyItemText( lv_item, item.Directory().c_str() );
        else if ( lv_item.iSubItem == 2 )
            CopyItemText( lv_item, item.Branch().c_str() );
    }

    //Do the list need image information?
    if ( lv_item.mask & LVIF_IMAGE )
    {
        //Set which image to use
        //pItem.iImage = m_database[itemid].m_image;

        //Show check box?
        if ( mListView.GetExtendedListViewStyle() & LVS_EX_CHECKBOXES )
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

CMainDlg::CMainDlg()
    : CFormSize( std::wstring( L"FormSize" ), std::wstring( L"CMainDlg_" ) )
{
}

std::wstring CMainDlg::ListView_GetText( int idx, ListColumn col )
{
    CString     sstr;

    if ( idx >= 0 )
        mListView.GetItemText( idx, static_cast<int>(col), sstr );
    return std::wstring( static_cast<const wchar_t *>(sstr) );
}

std::wstring CMainDlg::ListView_GetText_Checked( int idx, ListColumn col )
{
    std::wstring    result = ListView_GetText( idx, col );

    if ( !result.empty() && !ccwin::DirectoryExists( result ) )
        throw cclib::BaseException( boost::str( boost::format( "Repository\n%1%\nis not present." ) % ccwin::NarrowStringStrict( result ) ) );
    return result;
}

std::wstring CMainDlg::ListView_GetSelectedText( ListColumn col )
{
    CString     sstr;
    int         idx = mListView.GetSelectedIndex();

    if ( idx >= 0 )
        mListView.GetItemText( idx, static_cast<int>(col), sstr );
    return std::wstring( static_cast<const wchar_t *>(sstr) );
}

std::wstring CMainDlg::ListView_GetSelectedText_Checked( ListColumn col )
{
    std::wstring    result = ListView_GetSelectedText( col );

    if ( !result.empty() && !ccwin::DirectoryExists( result ) )
        throw std::runtime_error( "Repository is not present." );
    return result;
}
