#include "stdafx.h"
#include "MainFrm.h"
#include "FileView.h"
#include "Resource.h"
#include "UIDesigner.h"

#include "DialogProjectNew.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileViewTree

CFileViewTree::CFileViewTree()
{
}

CFileViewTree::~CFileViewTree()
{
}

BEGIN_MESSAGE_MAP(CFileViewTree, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CFileViewTree::OnNMDblclk)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, &CFileViewTree::OnTvnEndlabeledit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileViewTree 消息处理程序

BOOL CFileViewTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != NULL);

	if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL)
	{
		GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}

void CFileViewTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	g_pFileView->OpenUIFile(GetSelectedItem());
}

void CFileViewTree::OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	CString strNewName = pTVDispInfo->item.pszText;
	CString strOldName = GetItemText(pTVDispInfo->item.hItem);
	if(!strNewName.IsEmpty() && g_pFileView->RenameFile(strNewName, pTVDispInfo->item.hItem))
		SetItemText(pTVDispInfo->item.hItem, strNewName);
}

/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView()
{
	g_pFileView = this;
}

CFileView::~CFileView()
{
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDR_PROJECT_NEW, OnProjectNew)
	ON_COMMAND(IDR_FILE_NEW, OnFileNew)
	ON_UPDATE_COMMAND_UI(IDR_FILE_NEW, &CMainFrame::OnUpdateProjectExist)
	ON_COMMAND(IDR_FILE_DELETE, OnFileDelete)
	ON_UPDATE_COMMAND_UI(IDR_FILE_DELETE, &CMainFrame::OnUpdateProjectExist)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_DIRECTORY_NEW, &CFileView::OnDirectoryNew)
	ON_COMMAND(ID_CREATE_COPY, &CFileView::OnCreateCopy)
	ON_COMMAND(ID_FILE_RENAME, &CFileView::OnFileRename)
	ON_COMMAND(ID_TREE_COLLAPSE, &CFileView::OnTreeCollapse)
	ON_COMMAND(ID_TREE_EXPAND, &CFileView::OnTreeExpand)
	ON_COMMAND(ID_PROJECT_CLOSE, &CMainFrame::OnProjectClose)
	ON_UPDATE_COMMAND_UI(ID_PROJECT_CLOSE, &CMainFrame::OnUpdateProjectExist)
	ON_UPDATE_COMMAND_UI(ID_CREATE_COPY, &CMainFrame::OnUpdateProjectExist)
	ON_UPDATE_COMMAND_UI(ID_FILE_RENAME, &CMainFrame::OnUpdateProjectExist)
	ON_UPDATE_COMMAND_UI(ID_TREE_COLLAPSE, &CMainFrame::OnUpdateProjectExist)
	ON_UPDATE_COMMAND_UI(ID_TREE_EXPAND, &CMainFrame::OnUpdateProjectExist)
	ON_UPDATE_COMMAND_UI(ID_OPEN, &CMainFrame::OnUpdateProjectExist)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, &CMainFrame::OnUpdateProjectExist)
	ON_UPDATE_COMMAND_UI(ID_DIRECTORY_NEW, &CMainFrame::OnUpdateProjectExist)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 消息处理程序

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建视图:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS;

	if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("未能创建文件视图\n");
		return -1;      // 未能创建
	}

	// 加载视图图像:
	m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROJECT_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_PROJECT_EXPLORER, 0, 0, TRUE /* 已锁定*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
	AdjustLayout();

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFileView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndFileView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFileView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnProjectNew()
{
	CDialogProjectNew dlg;
	if(dlg.DoModal() == IDOK)
	{
		CGlobalVariable::m_strProjectName = dlg.m_strName;
		CGlobalVariable::m_strProjectPath = dlg.m_strPath;
		if(CGlobalVariable::m_strProjectPath[CGlobalVariable::m_strProjectPath.GetLength()] != '\\')
			CGlobalVariable::m_strProjectPath += '\\';

		CreateDirectory(CGlobalVariable::m_strProjectPath + CGlobalVariable::m_strProjectName, NULL);
		CGlobalVariable::m_bIsProjectExist = true;

		HTREEITEM hRoot = m_wndFileView.InsertItem(CGlobalVariable::m_strProjectName, 0, 0, TVI_ROOT);
		m_wndFileView.SetItemData(hRoot, INFO_PROJECT);
		m_wndFileView.SetItemState(hRoot, TVIS_BOLD,TVIS_BOLD);
	}
}

void CFileView::OnFileNew()
{
	if(theApp.m_pDocManager != NULL)
		theApp.m_pDocManager->OnFileNew();

	CDocument* pDoc = g_pMainFrame->MDIGetActive()->GetActiveDocument();
	HTREEITEM hSelectedItem = m_wndFileView.GetSelectedItem();
	DWORD dwInfo = m_wndFileView.GetItemData(hSelectedItem);
	HTREEITEM hParent = (dwInfo != INFO_FILE)?hSelectedItem:m_wndFileView.GetParentItem(hSelectedItem);
	HTREEITEM hNewItem = m_wndFileView.InsertItem(pDoc->GetTitle(), 1, 1, hParent);
	m_wndFileView.SetItemData(hNewItem, INFO_FILE);
	m_wndFileView.Expand(hParent, TVE_EXPAND);
	CString strFilePath = GetFileRelativePath(hNewItem) + _T(".xml");
	pDoc->DoSave(CGlobalVariable::m_strProjectPath + strFilePath);
}

void CFileView::OnFileDelete()
{
	HTREEITEM hSelectedItem = m_wndFileView.GetSelectedItem();
	DWORD dwInfo = m_wndFileView.GetItemData(hSelectedItem);
	if(dwInfo == INFO_FILE)
	{
		if(MessageBox(_T("您确定删除此文件？\n点击“是”，确认删除，“否”取消删除。"),
			_T("警告"), MB_YESNO | MB_ICONWARNING) == IDNO)
			return;

		DeleteUIFile(hSelectedItem);
	}
	else if(dwInfo == INFO_DIRECTORY)
	{
		if(MessageBox(_T("您确定删除此目录以及该目录中的所有内容，删除后将无法恢复？\n点击“是”，确认删除，“否”取消删除。"),
			_T("警告"), MB_YESNO | MB_ICONWARNING) == IDNO)
			return;

		DeleteDirectory(hSelectedItem);
	}
}

BOOL CFileView::DeleteUIFile(HTREEITEM hItem)
{
	CString strFilePath;
	strFilePath = GetFileRelativePath(hItem) + _T(".xml");
	strFilePath = CGlobalVariable::m_strProjectPath + strFilePath;
	CloseUIFile(strFilePath);
	if(DeleteFile(strFilePath))
	{
		m_wndFileView.DeleteItem(hItem);
		return TRUE;
	}

	return FALSE;
}

BOOL CFileView::DeleteDirectory(HTREEITEM hItem)
{
	HTREEITEM hChild = m_wndFileView.GetChildItem(hItem);
	while(hChild != NULL)
	{
		DWORD dwInfo = m_wndFileView.GetItemData(hChild);
		HTREEITEM hNext = m_wndFileView.GetNextItem(hChild, TVGN_NEXT);
		if(dwInfo == INFO_FILE)
			DeleteUIFile(hChild);
		else if(dwInfo == INFO_DIRECTORY)
			DeleteDirectory(hChild);

		hChild = hNext;
	}
	CString strDirPath;
	strDirPath = GetFileRelativePath(hItem);
	if(RemoveDirectory(CGlobalVariable::m_strProjectPath + strDirPath))
	{
		m_wndFileView.DeleteItem(hItem);
		return TRUE;
	}

	return FALSE;
}

CString CFileView::GetFileRelativePath(HTREEITEM hItem)
{
	CString strRelativePath = m_wndFileView.GetItemText(hItem);

	hItem = m_wndFileView.GetParentItem(hItem);
	while(hItem)
	{
		strRelativePath = m_wndFileView.GetItemText(hItem) + _T("\\") + strRelativePath;
		hItem = m_wndFileView.GetParentItem(hItem);
	}

	return strRelativePath;
}

void CFileView::OpenUIFile(HTREEITEM hItem)
{
	if(m_wndFileView.GetItemData(hItem) != INFO_FILE)
		return;

	CString strFilePath = GetFileRelativePath(hItem) + _T(".xml");
	AfxGetApp()->OpenDocumentFile(CGlobalVariable::m_strProjectPath + strFilePath);
}

CDocument* CFileView::FindUIFile(CString& strPath)
{
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	while (pos != NULL)
	{
		CDocTemplate *pDocTemplate = theApp.GetNextDocTemplate(pos);
		POSITION posDoc = pDocTemplate->GetFirstDocPosition();
		while (posDoc != NULL)
		{
			CDocument* pDoc = pDocTemplate->GetNextDoc(posDoc);
			if ((pDoc != NULL) && (pDoc->GetPathName() == strPath))
			{
				return pDoc;
			}
		}
	}

	return NULL;
}

BOOL CFileView::CloseUIFile(CString& strPath)
{
	CDocument* pDoc = FindUIFile(strPath);
	if(pDoc == NULL)
		return FALSE;

	pDoc->OnCloseDocument();

	return TRUE;
}

void CFileView::OnFileOpen()
{
	// TODO: 在此处添加命令处理程序代码
	HTREEITEM hSelectedItem = m_wndFileView.GetSelectedItem();
	OpenUIFile(hSelectedItem);
}

void CFileView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndFileView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CFileView::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROJECT_EXPLORER_24 : IDR_PROJECT_EXPLORER, 0, 0, TRUE /* 锁定 */);

	m_FileViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("无法加载位图: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}

void CFileView::OnDirectoryNew()
{
	// TODO: 在此添加命令处理程序代码
	HTREEITEM hSelectedItem = m_wndFileView.GetSelectedItem();
	DWORD dwInfo = m_wndFileView.GetItemData(hSelectedItem);
	HTREEITEM hParent = (dwInfo != INFO_FILE)?hSelectedItem:m_wndFileView.GetParentItem(hSelectedItem);
	CString strDirName = _T("新建文件夹");
	CString strDirPath = CGlobalVariable::m_strProjectPath + GetFileRelativePath(hParent) + _T("\\");
	int nCount = 2;
	while(!CreateDirectory(strDirPath + strDirName, NULL))
	{
		strDirName.Format(_T("新建文件夹(%d)"), nCount++);
	}
	HTREEITEM hNewItem = m_wndFileView.InsertItem(strDirName, 0, 0, hParent);
	m_wndFileView.SetItemData(hNewItem, INFO_DIRECTORY);
	m_wndFileView.Expand(hParent, TVE_EXPAND);
}

void CFileView::OnCreateCopy()
{
	// TODO: 在此添加命令处理程序代码
	HTREEITEM hSelectedItem = m_wndFileView.GetSelectedItem();
	if(m_wndFileView.GetItemData(hSelectedItem) != INFO_FILE)
		return;

	CString strFilePath= GetFileRelativePath(hSelectedItem) + _T(".xml");
	int nFind = strFilePath.ReverseFind('\\');
	if(nFind == -1)
		return;
	CString strNewFilePath = strFilePath;
	strNewFilePath.Insert(nFind + 1, _T("复件 "));
	int nCount = 0;
	while(!CopyFile(CGlobalVariable::m_strProjectPath + strFilePath,
		CGlobalVariable::m_strProjectPath + strNewFilePath, TRUE))
	{
		strNewFilePath = strFilePath;
		CString strCount;
		strCount.Format(_T("复件(%d) "), nCount++);
		strNewFilePath.Insert(nFind + 1, strCount);
	}
	CString strCopyName = strNewFilePath.Mid(nFind + 1, strNewFilePath.GetLength() - nFind - 5);
	HTREEITEM hParent  = m_wndFileView.GetParentItem(hSelectedItem);
	HTREEITEM hNewItem = m_wndFileView.InsertItem(strCopyName, 1, 1, hParent);
	m_wndFileView.SetItemData(hNewItem, INFO_FILE);
}

void CFileView::OnFileRename()
{
	// TODO: 在此添加命令处理程序代码
	HTREEITEM hSelectedItem = m_wndFileView.GetSelectedItem();
	m_wndFileView.EditLabel(hSelectedItem);
}

BOOL CFileView::RenameFile(CString strNewName, HTREEITEM hItem)
{
	CString strFilePath = GetFileRelativePath(hItem);
	int nFind = strFilePath.ReverseFind('\\');
	if(nFind == -1)
		return FALSE;
	CString strOldName = strFilePath.Right(strFilePath.GetLength() - nFind- 1);
	strFilePath = CGlobalVariable::m_strProjectPath + strFilePath.Left(nFind + 1);
	DWORD dwInfo = m_wndFileView.GetItemData(hItem);
	if(dwInfo == INFO_FILE)
	{
		strOldName += _T(".xml");
		strNewName += _T(".xml");

		if(FindUIFile(strFilePath + strOldName))
		{
			MessageBox(_T("此文件正处于打开状态，请先关闭后再进行重命名。"), _T("提示"), MB_ICONINFORMATION);
			return FALSE;
		}
	}
	if(!MoveFile(strFilePath + strOldName, strFilePath + strNewName))
	{
		MessageBox(_T("此文件已经存在，不能重复命名。"), _T("提示"), MB_ICONINFORMATION);
		return FALSE;
	}

	return TRUE;
}

void CFileView::OnTreeCollapse()
{
	// TODO: 在此添加命令处理程序代码
	ExpandFileViewTree(m_wndFileView.GetRootItem(), TVE_COLLAPSE);
}

void CFileView::OnTreeExpand()
{
	// TODO: 在此添加命令处理程序代码
	ExpandFileViewTree(m_wndFileView.GetRootItem(), TVE_EXPAND);
}

void CFileView::ExpandFileViewTree(HTREEITEM hItem, UINT nCode)
{
	HTREEITEM hChild = m_wndFileView.GetChildItem(hItem);
	while(hChild != NULL)
	{
		ExpandFileViewTree(hChild, nCode);
		m_wndFileView.Expand(hItem, nCode);
		hChild = m_wndFileView.GetNextItem(hChild, TVGN_NEXT);
	}
}