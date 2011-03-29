#include "StdAfx.h"
#include "LayoutManager.h"

//////////////////////////////////////////////////////////////////////////
//CFormUI

CFormUI::CFormUI()
{
}

CFormUI::~CFormUI()
{

}

LPCTSTR CFormUI::GetClass() const
{
	return _T("FormUI");
}

LPVOID CFormUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("Form")) == 0 )
		return static_cast<CFormUI*>(this);

	return CContainerUI::GetInterface(pstrName);
}

void CFormUI::SetManager(CPaintManagerUI* pPaintManager)
{
	ASSERT(pPaintManager);

	m_pManager=pPaintManager;
}

CPaintManagerUI* CFormUI::GetManager() const
{
	return m_pManager;
}

SIZE CFormUI::GetInitSize()
{
	return m_pManager->GetInitSize();
}

void CFormUI::SetInitSize(int cx, int cy)
{
	m_pManager->SetInitSize(cx,cy);

	SetPos(CRect(0,0,cx,cy));
}

RECT& CFormUI::GetSizeBox()
{
	return m_pManager->GetSizeBox();
}

void CFormUI::SetSizeBox(RECT& rcSizeBox)
{
	m_pManager->SetSizeBox(rcSizeBox);
}

RECT& CFormUI::GetCaptionRect()
{
	return m_pManager->GetCaptionRect();
}

void CFormUI::SetCaptionRect(RECT& rcCaption)
{
	m_pManager->SetCaptionRect(rcCaption);
}

SIZE CFormUI::GetRoundCorner() const
{
	return m_pManager->GetRoundCorner();
}

void CFormUI::SetRoundCorner(int cx, int cy)
{
	m_pManager->SetRoundCorner(cx,cy);
}

SIZE CFormUI::GetMinMaxInfo() const
{
	return m_pManager->GetMinMaxInfo();
}

void CFormUI::SetMinMaxInfo(int cx, int cy)
{
	ASSERT(cx>=0 && cy>=0);
	m_pManager->SetMinMaxInfo(cx,cy);
}

bool CFormUI::IsShowUpdateRect() const
{
	return m_pManager->IsShowUpdateRect();
}

void CFormUI::SetShowUpdateRect(bool show)
{
	m_pManager->SetShowUpdateRect(show);
}

void CFormUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);

	if(m_items.GetSize()==0)
		return;
	CControlUI* pControl=static_cast<CControlUI*>(m_items[0]);
	if(pControl==NULL)
		return;
	pControl->SetPos(rc);//放大到整个客户区
}

void CFormUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("size")) == 0 ) {
		LPTSTR pstr = NULL;
		int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
		SetInitSize(cx, cy);
	} 
	else if( _tcscmp(pstrName, _T("sizebox")) == 0 ) {
		RECT rcSizeBox = { 0 };
		LPTSTR pstr = NULL;
		rcSizeBox.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcSizeBox.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcSizeBox.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcSizeBox.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetSizeBox(rcSizeBox);
	}
	else if( _tcscmp(pstrName, _T("caption")) == 0 ) {
		RECT rcCaption = { 0 };
		LPTSTR pstr = NULL;
		rcCaption.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcCaption.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcCaption.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcCaption.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetCaptionRect(rcCaption);
	}
	else if( _tcscmp(pstrName, _T("roundcorner")) == 0 ) {
		LPTSTR pstr = NULL;
		int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
		SetRoundCorner(cx, cy);
	} 
	else if( _tcscmp(pstrName, _T("mininfo")) == 0 ) {
		LPTSTR pstr = NULL;
		int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
		SetMinMaxInfo(cx, cy);
	}
	else if( _tcscmp(pstrName, _T("showdirty")) == 0 ) {
		SetShowUpdateRect(_tcscmp(pstrValue, _T("true")) == 0);
	}
}

//////////////////////////////////////////////////////////////////////////
//CFormTestWnd

CFormTestWnd::CFormTestWnd():m_pManager(NULL),m_pRoot(NULL)
{

}

CFormTestWnd::~CFormTestWnd()
{
	if(m_pManager)
		delete m_pManager;
}

LPCTSTR CFormTestWnd::GetWindowClassName() const
{
	 return _T("UIFormTest");
}

UINT CFormTestWnd::GetClassStyle() const
{
	return CS_DBLCLKS;
}

void CFormTestWnd::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}

void CFormTestWnd::SetManager(CPaintManagerUI* pPaintManager)
{
	ASSERT(pPaintManager);

	m_pManager=pPaintManager;
}

CPaintManagerUI* CFormTestWnd::GetManager() const
{
	return m_pManager;
}

void CFormTestWnd::SetRoot(CControlUI* pControl)
{
	m_pRoot=pControl;
}

void CFormTestWnd::Notify(TNotifyUI& msg)
{
}

LRESULT CFormTestWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	RECT rcClient;
	::GetClientRect(*this, &rcClient);
	::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
		rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

	m_pManager->Init(m_hWnd);
	m_pManager->AttachDialog(m_pRoot);
	m_pManager->AddNotifier(this);

	SIZE szInitWindowSize = m_pManager->GetInitSize();
	if( szInitWindowSize.cx != 0 ) {
		::SetWindowPos(m_hWnd, NULL, 0, 0, szInitWindowSize.cx, szInitWindowSize.cy, SWP_NOMOVE | SWP_NOZORDER);
	}

	return 0;
}

LRESULT CFormTestWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CFormTestWnd::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CFormTestWnd::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CFormTestWnd::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	RECT rcCaption = m_pManager->GetCaptionRect();
	if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pManager->FindControl(pt));
			if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
				_tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
				_tcscmp(pControl->GetClass(), _T("TextUI")) != 0 )
				return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CFormTestWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pManager->GetRoundCorner();
	if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
		RECT rcClient;
		::GetClientRect(*this, &rcClient);
		RECT rc = { rcClient.left, rcClient.top + szRoundCorner.cx, rcClient.right, rcClient.bottom };
		HRGN hRgn1 = ::CreateRectRgnIndirect( &rc );
		HRGN hRgn2 = ::CreateRoundRectRgn(rcClient.left, rcClient.top, rcClient.right + 1, rcClient.bottom - szRoundCorner.cx, szRoundCorner.cx, szRoundCorner.cy);
		::CombineRgn( hRgn1, hRgn1, hRgn2, RGN_OR );
		::SetWindowRgn(*this, hRgn1, TRUE);
		::DeleteObject(hRgn1);
		::DeleteObject(hRgn2);
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CFormTestWnd::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	CRect rcWork = oMonitor.rcWork;
	rcWork.OffsetRect(-rcWork.left, -rcWork.top);

	LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
	lpMMI->ptMaxPosition.x	= rcWork.left;
	lpMMI->ptMaxPosition.y	= rcWork.top;
	lpMMI->ptMaxSize.x		= rcWork.right;
	lpMMI->ptMaxSize.y		= rcWork.bottom;

	bHandled = FALSE;
	return 0;
}

LRESULT CFormTestWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(wParam==VK_ESCAPE)
		Close();

	bHandled=FALSE;
	return 0;
}

LRESULT CFormTestWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch( uMsg ) {
		case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:	   lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
		default:
			bHandled = FALSE;
	}
	if( bHandled )
		return lRes;
	if( m_pManager->MessageHandler(uMsg, wParam, lParam, lRes) )
		return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
//CDelayResize

CDelayRepos::CDelayRepos()
{
}

CDelayRepos::~CDelayRepos()
{
	m_arrDelay.RemoveAll();
}

BOOL CDelayRepos::AddParent(CControlUI* pControl)
{
	if(pControl==NULL)
		return FALSE;

	for(int i=0;i<m_arrDelay.GetSize();i++)
	{
		CControlUI* pParent=m_arrDelay.GetAt(i);

		if(pControl==pParent)
			return FALSE;
	}
	m_arrDelay.Add(pControl);

	return TRUE;
}

void CDelayRepos::Repos()
{
	for(int i=0;i<m_arrDelay.GetSize();i++)
	{
		CControlUI* pParent=m_arrDelay.GetAt(i);

		pParent->SetPos(pParent->GetPos());
	}
}

//////////////////////////////////////////////////////////////////////////
//CLayoutManager

CLayoutManager::CLayoutManager(void):m_bShowGrid(false),m_bShowAuxBorder(true)
{
}

CLayoutManager::~CLayoutManager(void)
{
}

void CLayoutManager::Init(HWND hWnd,LPCTSTR pstrLoad)
{
	CFormUI* pForm=static_cast<CFormUI*>(NewControl(classForm,
		CRect(0,0,FORM_INIT_WIDTH,FORM_INIT_HEIGHT),&m_Manager,NULL)->GetInterface(_T("Form")));
	ASSERT(pForm);
	pForm->SetManager(&m_Manager);
	pForm->SetInitSize(FORM_INIT_WIDTH,FORM_INIT_HEIGHT);

	m_Manager.Init(hWnd);
	if(*pstrLoad!='\0')
	{
		CString strSkinDir(pstrLoad);
		int nFind=strSkinDir.ReverseFind(_T('\\'));
		strSkinDir=strSkinDir.Left(nFind);
		nFind=strSkinDir.ReverseFind(_T('\\'));
		strSkinDir=strSkinDir.Left(nFind);
		strSkinDir+=_T("\\");
		
		g_HookAPI.SetSkinDir(strSkinDir);
		g_HookAPI.EnableCreateFile(true);

		CDialogBuilder builder;
		CControlUI* pRoot=builder.Create(pstrLoad,(UINT)0,NULL,&m_Manager);
		ASSERT(pRoot && "Failed to parse XML");
		pForm->Add(pRoot);

		SIZE size=m_Manager.GetInitSize();
		pForm->SetInitSize(size.cx,size.cy);
	}
	m_Manager.AttachDialog(pForm);
}

void CLayoutManager::Draw(CDC* pDC)
{
	CSize szForm=GetForm()->GetInitSize();
	CRect rcPaint(0,0,szForm.cx,szForm.cy);
	CControlUI* pForm=m_Manager.GetRoot();

	pForm->DoPaint(pDC->GetSafeHdc(),rcPaint);

	CContainerUI* pContainer=static_cast<CContainerUI*>(pForm->GetInterface(_T("Container")));
	ASSERT(pContainer);
	if(m_bShowAuxBorder)
		DrawAuxBorder(pDC,pContainer->GetItemAt(0));
}

void CLayoutManager::DrawAuxBorder(CDC* pDC,CControlUI* pControl)
{
	if(pControl==NULL||!pControl->IsVisible())
		return;

	CContainerUI* pContainer=static_cast<CContainerUI*>(pControl->GetInterface(_T("Container")));

	//draw auxBorder
	if(pControl->GetBorderColor()==0||pControl->GetBorderSize()<=0)
	{
		pDC->SetBkMode(TRANSPARENT);
		CPen DotedPen(PS_SOLID,1,pContainer?RGB(255,0,0):RGB(0,255,0));
		CPen* pOldPen;
		CBrush* pOldBrush;
		pOldPen=pDC->SelectObject(&DotedPen);
		pOldBrush=(CBrush*)pDC->SelectStockObject(NULL_BRUSH);
		CRect rect=pControl->GetPos();
		pDC->Rectangle(&rect);
		pDC->SelectObject(pOldBrush);
		pDC->SelectObject(pOldPen);
	}

	if(pContainer==NULL)
		return;
	for(int i=0;i<pContainer->GetCount();i++)
	{
		DrawAuxBorder(pDC,pContainer->GetItemAt(i));
	}
}

CControlUI* CLayoutManager::NewControl(int nClass,CRect& rect,CPaintManagerUI* pManager,CControlUI* pParent)
{
	CControlUI* pControl=NULL;
	
	ExtendedAttributes* pExtended=new ExtendedAttributes;
	ZeroMemory(pExtended,sizeof(ExtendedAttributes));
	switch(nClass)
	{
	case classForm:
		pControl=new CFormUI;
		pExtended->nClass=classForm;
		break;
	case classControl:
		pControl=new CControlUI;
		pExtended->nClass=classControl;
		pControl->SetFloat(true);
		break;
	case classLabel:
		pControl=new CLabelUI;
		pExtended->nClass=classLabel;
		pControl->SetFloat(true);
		break;
	case classText:
		pControl=new CTextUI;
		pExtended->nClass=classText;
		pControl->SetFloat(true);
		break;
	case classButton:
		pControl=new CButtonUI;
		pExtended->nClass=classButton;
		pControl->SetFloat(true);
		break;
	case classEdit:
		pControl=new CEditUI;
		pExtended->nClass=classEdit;
		pControl->SetFloat(true);
		break;
	case classOption:
		pControl=new COptionUI;
		pExtended->nClass=classOption;
		pControl->SetFloat(true);
		break;
	case classProgress:
		pControl=new CProgressUI;
		pExtended->nClass=classProgress;
		pControl->SetFloat(true);
		break;
	case classSlider:
		pControl=new CSliderUI;
		pExtended->nClass=classSlider;
		pControl->SetFloat(true);
		break;
	case classCombo:
		pControl=new CComboUI;
		pExtended->nClass=classCombo;
		pControl->SetFloat(true);
		break;
	case classActiveX:
		pControl=new CActiveXUI;
		pExtended->nClass=classActiveX;
		break;
	case classContainer:
		pControl=new CContainerUI;
		pExtended->nClass=classContainer;
		break;
	case classVerticalLayout:
		pControl=new CVerticalLayoutUI;
		pExtended->nClass=classVerticalLayout;
		break;
	case classHorizontalLayout:
		pControl=new CHorizontalLayoutUI;
		pExtended->nClass=classHorizontalLayout;
		break;
	case classDialogLayout:
		pControl=new CDialogLayoutUI;
		pExtended->nClass=classDialogLayout;
		break;
	case classTileLayout:
		pControl=new CTileLayoutUI;
		pExtended->nClass=classTileLayout;
		break;
	case classTabLayout:
		pControl=new CTabLayoutUI;
		pExtended->nClass=classTabLayout;
		break;
	default:
		delete pExtended;
		return NULL;
	}
	if(pControl==NULL)
	{
		delete pExtended;
		return NULL;
	}

	pControl->SetManager(pManager,pParent);
	pControl->SetTag((UINT_PTR)pExtended);

	//pos
	CRect rcParent=pParent?pParent->GetPos():CRect(0,0,0,0);
	pControl->SetFixedXY(CSize(rect.left-rcParent.left,rect.top-rcParent.top));
	pControl->SetFixedWidth(rect.right - rect.left);
	pControl->SetFixedHeight(rect.bottom - rect.top);

	if(pParent)
	{
		CContainerUI* pContainer = static_cast<CContainerUI*>(pParent->GetInterface(_T("Container")));
		ASSERT(pContainer);
		if(!pContainer->Add(pControl))
		{
			delete pControl;
			return NULL;
		}
		pParent->SetPos(pParent->GetPos());
	}

	return pControl;
}

BOOL CLayoutManager::RemoveControl(CControlUI* pControl)
{
	if(pControl==NULL||pControl==GetForm())
		return FALSE;

	ExtendedAttributes* pExtended=(ExtendedAttributes*)pControl->GetTag();
	delete pExtended;
	pControl->SetTag(NULL);

	CContainerUI* pParent=static_cast<CContainerUI*>(pControl->GetParent()->GetInterface(_T("Container")));
	ASSERT(pParent);
	pParent->Remove(pControl);

	return TRUE;
}

CFormUI* CLayoutManager::GetForm() const
{ 
	CControlUI* pRoot=m_Manager.GetRoot();
	if(pRoot==NULL)
		return NULL;

	return static_cast<CFormUI*>(pRoot->GetInterface(_T("Form")));
}

CControlUI* CLayoutManager::FindControl(CPoint point) const
{
	return m_Manager.FindControl(point);
}

void CLayoutManager::TestForm()
{
	CFormTestWnd* pFrame=new CFormTestWnd();
	CPaintManagerUI* pManager=new CPaintManagerUI();
	SIZE size=m_Manager.GetInitSize();
	pManager->SetInitSize(size.cx,size.cy);
	pManager->SetSizeBox(m_Manager.GetSizeBox());
	pManager->SetCaptionRect(m_Manager.GetCaptionRect());
	size=m_Manager.GetRoundCorner();
	pManager->SetRoundCorner(size.cx,size.cy);
	size=m_Manager.GetMinMaxInfo();
	pManager->SetMinMaxInfo(size.cx,size.cy);
	pManager->SetShowUpdateRect(m_Manager.IsShowUpdateRect());

	if( pFrame == NULL )
		return;

	CControlUI* pRoot=CopyControls(GetForm()->GetItemAt(0));
	if(pRoot==NULL)
		return;

	g_HookAPI.EnableInvalidate(false);
	g_HookAPI.EnableAddImage(false);

	pFrame->SetManager(pManager);
	pRoot->SetManager(NULL,NULL);
	pFrame->SetRoot(pRoot);
	pFrame->Create(m_Manager.GetPaintWindow(),_T("FormTest"),UI_WNDSTYLE_FRAME,0,0,0,size.cx,size.cy);
	pFrame->CenterWindow();

	HWND m_hWnd=pFrame->GetHWND();
	MSG msg = { 0 };
	while( ::IsWindow(m_hWnd) && ::GetMessage(&msg, NULL, 0, 0) ) {
		if( msg.hwnd != m_hWnd ) {
			if( (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) )
				continue;
			if( msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST ) 
				continue;
			if( msg.message == WM_SETCURSOR )
				continue;
		}
		if( !CPaintManagerUI::TranslateMessage(&msg) ) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		if( msg.message == WM_QUIT ) break;
	}
	if( msg.message == WM_QUIT ) ::PostQuitMessage(msg.wParam);

	g_HookAPI.EnableInvalidate(true);
	g_HookAPI.EnableAddImage(true);
}

BOOL CLayoutManager::IsEmptyForm() const
{
	return GetForm()->GetItemAt(0)==NULL;
}

CControlUI* CLayoutManager::CopyControls(CControlUI* pControl)
{
	if(pControl==NULL)
		return NULL;

	CContainerUI* pContainer=static_cast<CContainerUI*>(pControl->GetInterface(_T("Container")));
	if(pContainer==NULL)
		return CopyControl(pControl);

	CContainerUI* pCopyContainer=static_cast<CContainerUI*>(CopyControl(pContainer)->GetInterface(_T("Container")));
	pCopyContainer->SetAutoDestroy(false);
	pCopyContainer->RemoveAll();
	pCopyContainer->SetAutoDestroy(true);
	for(int i=0;i<pContainer->GetCount();i++)
	{
		CControlUI* pCopyControl=CopyControls(pContainer->GetItemAt(i));
		pCopyControl->SetManager(NULL,pCopyContainer);
		pCopyContainer->Add(pCopyControl);
	}

	return pCopyContainer;
}

CControlUI* CLayoutManager::CopyControl(CControlUI* pControl)
{
	LPCTSTR pstrClass=pControl->GetClass();
	SIZE_T cchLen = _tcslen(pstrClass);
	CControlUI* pCopyControl = NULL;
	switch( cchLen ) {
		case 6:
			if( _tcscmp(pstrClass, _T("EditUI")) == 0 )                   pCopyControl = new CEditUI(*static_cast<CEditUI*>(pControl->GetInterface(_T("Edit"))));
			else if( _tcscmp(pstrClass, _T("ListUI")) == 0 )              pCopyControl = new CListUI(*static_cast<CListUI*>(pControl->GetInterface(_T("List"))));
			else if( _tcscmp(pstrClass, _T("TextUI")) == 0 )              pCopyControl = new CTextUI(*static_cast<CTextUI*>(pControl->GetInterface(_T("Text"))));
			break;
		case 7:
			if( _tcscmp(pstrClass, _T("ComboUI")) == 0 )                  pCopyControl = new CComboUI(*static_cast<CComboUI*>(pControl->GetInterface(_T("Combo"))));
			else if( _tcscmp(pstrClass, _T("LabelUI")) == 0 )             pCopyControl = new CLabelUI(*static_cast<CLabelUI*>(pControl->GetInterface(_T("Label"))));
		case 8:
			if( _tcscmp(pstrClass, _T("ButtonUI")) == 0 )                 pCopyControl = new CButtonUI(*static_cast<CButtonUI*>(pControl->GetInterface(_T("Button"))));
			else if( _tcscmp(pstrClass, _T("OptionUI")) == 0 )            pCopyControl = new COptionUI(*static_cast<COptionUI*>(pControl->GetInterface(_T("Option"))));
			else if( _tcscmp(pstrClass, _T("SliderUI")) == 0 )            pCopyControl = new CSliderUI(*static_cast<CSliderUI*>(pControl->GetInterface(_T("Slider"))));
			break;
		case 9:
			if( _tcscmp(pstrClass, _T("ControlUI")) == 0 )                pCopyControl = new CControlUI(*static_cast<CControlUI*>(pControl->GetInterface(_T("Control"))));
			else if( _tcscmp(pstrClass, _T("ActiveXUI")) == 0 )           pCopyControl = new CActiveXUI(*static_cast<CActiveXUI*>(pControl->GetInterface(_T("ActiveX"))));
			break;
		case 10:
			if( _tcscmp(pstrClass, _T("ProgressUI")) == 0 )               pCopyControl = new CProgressUI(*static_cast<CProgressUI*>(pControl->GetInterface(_T("Progress"))));
		case 11:
			if( _tcscmp(pstrClass, _T("ContainerUI")) == 0 )              pCopyControl = new CContainerUI(*static_cast<CContainerUI*>(pControl->GetInterface(_T("Container"))));
			else if( _tcscmp(pstrClass, _T("TabLayoutUI")) == 0 )         pCopyControl = new CTabLayoutUI(*static_cast<CTabLayoutUI*>(pControl->GetInterface(_T("TabLayout"))));
			break;
		case 12:
			if( _tcscmp(pstrClass, _T("ListHeaderUI")) == 0 )             pCopyControl = new CListHeaderUI(*static_cast<CListHeaderUI*>(pControl->GetInterface(_T("ListHeader"))));
			else if( _tcscmp(pstrClass, _T("TileLayoutUI")) == 0 )        pCopyControl = new CTileLayoutUI(*static_cast<CTileLayoutUI*>(pControl->GetInterface(_T("TileLayout"))));
			break;
		case 14:
			if( _tcscmp(pstrClass, _T("DialogLayoutUI")) == 0 )           pCopyControl = new CDialogLayoutUI(*static_cast<CDialogLayoutUI*>(pControl->GetInterface(_T("DialogLayout"))));
			break;
		case 16:
			if( _tcscmp(pstrClass, _T("VerticalLayoutUI")) == 0 )         pCopyControl = new CVerticalLayoutUI(*static_cast<CVerticalLayoutUI*>(pControl->GetInterface(_T("VerticalLayout"))));
			else if( _tcscmp(pstrClass, _T("ListHeaderItemUI")) == 0 )    pCopyControl = new CListHeaderItemUI(*static_cast<CListHeaderItemUI*>(pControl->GetInterface(_T("ListHeaderItem"))));
			break;
		case 17:
			if( _tcscmp(pstrClass, _T("ListTextElementUI")) == 0 )        pCopyControl = new CListTextElementUI(*static_cast<CListTextElementUI*>(pControl->GetInterface(_T("ListTextElement"))));
			break;
		case 18:
			if( _tcscmp(pstrClass, _T("HorizontalLayoutUI")) == 0 )       pCopyControl = new CHorizontalLayoutUI(*static_cast<CHorizontalLayoutUI*>(pControl->GetInterface(_T("HorizontalLayout"))));
			else if( _tcscmp(pstrClass, _T("ListLabelElementUI")) == 0 )  pCopyControl = new CListLabelElementUI(*static_cast<CListLabelElementUI*>(pControl->GetInterface(_T("ListLabelElement"))));
			break;
		case 19:
			if( _tcscmp(pstrClass, _T("ListExpandElementUI")) == 0 )      pCopyControl = new CListExpandElementUI(*static_cast<CListExpandElementUI*>(pControl->GetInterface(_T("ListExpandElement"))));
			break;
		case 22:
			if( _tcscmp(pstrClass, _T("ListContainerElementUI")) == 0 )   pCopyControl = new CListContainerElementUI(*static_cast<CListContainerElementUI*>(pControl->GetInterface(_T("ListContainerElement"))));
			break;
	}

	return pCopyControl;
}

void CLayoutManager::AlignLeft(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	SIZE szXYLeft=pFocused->GetFixedXY();

	for (int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl->GetParent()!=pFocused->GetParent())
			continue;

		SIZE szXY=pControl->GetFixedXY();
		szXY.cx=szXYLeft.cx;
		pControl->SetFixedXY(szXY);
	}

	CControlUI* pParent=pFocused->GetParent();
	if(pParent)
		pParent->SetPos(pParent->GetPos());
}

void CLayoutManager::AlignRight(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	SIZE szXYRight=pFocused->GetFixedXY();
	int nWidth=pFocused->GetFixedWidth();

	for (int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl->GetParent()!=pFocused->GetParent())
			continue;

		SIZE szXY=pControl->GetFixedXY();
		szXY.cx=szXYRight.cx+nWidth-pControl->GetFixedWidth();
		pControl->SetFixedXY(szXY);
	}

	CControlUI* pParent=pFocused->GetParent();
	if(pParent)
		pParent->SetPos(pParent->GetPos());
}

void CLayoutManager::AlignTop(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	SIZE szXYTop=pFocused->GetFixedXY();

	for (int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl->GetParent()!=pFocused->GetParent())
			continue;

		SIZE szXY=pControl->GetFixedXY();
		szXY.cy=szXYTop.cy;
		pControl->SetFixedXY(szXY);
	}

	CControlUI* pParent=pFocused->GetParent();
	if(pParent)
		pParent->SetPos(pParent->GetPos());
}

void CLayoutManager::AlignBottom(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	SIZE szXYBottom=pFocused->GetFixedXY();
	int nHeight=pFocused->GetFixedHeight();

	for (int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl->GetParent()!=pFocused->GetParent())
			continue;

		SIZE szXY=pControl->GetFixedXY();
		szXY.cy=szXYBottom.cy+nHeight-pControl->GetFixedHeight();
		pControl->SetFixedXY(szXY);
	}

	CControlUI* pParent=pFocused->GetParent();
	if(pParent)
		pParent->SetPos(pParent->GetPos());
}

void CLayoutManager::AlignCenterVertically(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	CControlUI* pParent=pFocused->GetParent();
	if(!pParent)
		return;

	RECT rcParent=pParent->GetPos();

	CRect rectUnion;
	rectUnion.SetRectEmpty();
	for(int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl->GetParent()!=pFocused->GetParent())
			continue;

		rectUnion.UnionRect(&rectUnion,&pControl->GetPos());
	}

	int nOffsetY;
	nOffsetY=(rcParent.top+rcParent.bottom)/2-(rectUnion.top+rectUnion.bottom)/2;
	for(int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl->GetParent()!=pFocused->GetParent())
			continue;

		SIZE szXY=pControl->GetFixedXY();
		szXY.cy+=nOffsetY;
		pControl->SetFixedXY(szXY);
	}

	pParent->SetPos(pParent->GetPos());
}

void CLayoutManager::AlignCenterHorizontally(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	CControlUI* pParent=pFocused->GetParent();
	if(!pParent)
		return;

	RECT rcParent=pParent->GetPos();

	CRect rectUnion;
	rectUnion.SetRectEmpty();
	for(int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl->GetParent()!=pFocused->GetParent())
			continue;

		rectUnion.UnionRect(&rectUnion,&pControl->GetPos());
	}

	int nOffsetX;
	nOffsetX=(rcParent.left+rcParent.right)/2-(rectUnion.left+rectUnion.right)/2;
	for(int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl->GetParent()!=pFocused->GetParent())
			continue;

		SIZE szXY=pControl->GetFixedXY();
		szXY.cx+=nOffsetX;
		pControl->SetFixedXY(szXY);
	}

	pParent->SetPos(pParent->GetPos());
}

void CLayoutManager::AlignHorizontal(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{

}

void CLayoutManager::AlignVertical(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{

}

void CLayoutManager::AlignSameWidth(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	int nWidth=pFocused->GetFixedWidth();

	CDelayRepos DelayPos;
	for (int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl==pFocused)
			continue;
		
		pControl->SetFixedWidth(nWidth);
		DelayPos.AddParent(pControl->GetParent());
	}
	DelayPos.Repos();
}

void CLayoutManager::AlignSameHeight(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	int nHeight=pFocused->GetFixedHeight();

	CDelayRepos DelayPos;
	for (int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl==pFocused)
			continue;

		pControl->SetFixedHeight(nHeight);
		DelayPos.AddParent(pControl->GetParent());
	}
	DelayPos.Repos();
}

void CLayoutManager::AlignSameSize(CControlUI* pFocused,CArray<CControlUI*,CControlUI*>& arrSelected)
{
	int nWidth=pFocused->GetFixedWidth();
	int nHeight=pFocused->GetFixedHeight();

	CDelayRepos DelayPos;
	for (int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);
		if(pControl==pFocused)
			continue;

		pControl->SetFixedWidth(nWidth);
		pControl->SetFixedHeight(nHeight);
		DelayPos.AddParent(pControl->GetParent());
	}
	DelayPos.Repos();
}

void CLayoutManager::ShowGrid(bool bShow)
{
	m_bShowGrid=bShow;
}

bool CLayoutManager::IsShowGrid() const
{
	return m_bShowGrid;
}

void CLayoutManager::ShowAuxBorder(bool bShow)
{
	m_bShowAuxBorder=bShow;
}

bool CLayoutManager::IsShowAuxBorder() const
{
	return m_bShowAuxBorder;
}

void CLayoutManager::MicoMoveUp(CArray<CControlUI*,CControlUI*>& arrSelected,int nMoved)
{
	CDelayRepos DelayPos;
	for(int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);

		SIZE szXY=pControl->GetFixedXY();
		szXY.cy-=nMoved;
		pControl->SetFixedXY(szXY);
		DelayPos.AddParent(pControl->GetParent());
	}
	DelayPos.Repos();
}

void CLayoutManager::MicoMoveDown(CArray<CControlUI*,CControlUI*>& arrSelected,int nMoved)
{
	CDelayRepos DelayPos;
	for(int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);

		SIZE szXY=pControl->GetFixedXY();
		szXY.cy+=nMoved;
		pControl->SetFixedXY(szXY);
		DelayPos.AddParent(pControl->GetParent());
	}
	DelayPos.Repos();
}

void CLayoutManager::MicoMoveLeft(CArray<CControlUI*,CControlUI*>& arrSelected,int nMoved)
{
	CDelayRepos DelayPos;
	for(int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);

		SIZE szXY=pControl->GetFixedXY();
		szXY.cx-=nMoved;
		pControl->SetFixedXY(szXY);
		DelayPos.AddParent(pControl->GetParent());
	}
	DelayPos.Repos();
}

void CLayoutManager::MicoMoveRight(CArray<CControlUI*,CControlUI*>& arrSelected,int nMoved)
{
	CDelayRepos DelayPos;
	for(int i=0;i<arrSelected.GetSize();i++)
	{
		CControlUI* pControl=arrSelected.GetAt(i);

		SIZE szXY=pControl->GetFixedXY();
		szXY.cx+=nMoved;
		pControl->SetFixedXY(szXY);
		DelayPos.AddParent(pControl->GetParent());
	}
	DelayPos.Repos();
}

#if defined(EXPORT_UI_SCRIPT)
void CLayoutManager::CControlUI_Properties(CControlUI* pControlUI, TiXmlElement* pNode)
{
	TCHAR szBuf[MAX_PATH] = {0};

	pNode->SetAttribute("name", StringConvertor::TcharToUtf8(pControlUI->GetName()));
	pNode->SetAttribute("text", StringConvertor::TcharToUtf8(pControlUI->GetText()));
	pNode->SetAttribute("tooltip", StringConvertor::TcharToUtf8(pControlUI->GetToolTip()));
	pNode->SetAttribute("float", pControlUI->IsFloat()?"true":"false");

	_stprintf(szBuf, _T("%d"),pControlUI->GetBorderSize());
	pNode->SetAttribute("bordersize", StringConvertor::TcharToUtf8(szBuf));

	if (pControlUI->IsFloat())
	{
		_stprintf(szBuf, _T("%d,%d,%d,%d"), pControlUI->GetFixedXY().cx, pControlUI->GetFixedXY().cy, pControlUI->GetFixedXY().cx + pControlUI->GetFixedWidth(), pControlUI->GetFixedXY().cy + pControlUI->GetFixedHeight());
		pNode->SetAttribute("pos", StringConvertor::TcharToUtf8(szBuf));
	}
	else if ((pControlUI->GetFixedHeight() > 0) && (pControlUI->GetFixedWidth() > 0))
	{
		_stprintf(szBuf, _T("%d"), pControlUI->GetFixedWidth(), pControlUI->GetFixedHeight());
		pNode->SetAttribute("width", StringConvertor::TcharToUtf8(szBuf));

		_stprintf(szBuf, _T("%d"), pControlUI->GetFixedHeight());
		pNode->SetAttribute("height", StringConvertor::TcharToUtf8(szBuf));
	}

	RECT rcPadding = pControlUI->GetPadding();
	if (!IsRectEmpty(&rcPadding))
	{
		_stprintf(szBuf, _T("%d,%d,%d,%d"), rcPadding.left, rcPadding.top, rcPadding.right, rcPadding.bottom);
		pNode->SetAttribute("padding", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pControlUI->GetBkImage() && _tcslen(pControlUI->GetBkImage()) > 0)
	{
		pNode->SetAttribute("bkimage", StringConvertor::TcharToUtf8(pControlUI->GetBkImage()));
	}

	if (pControlUI->GetBkColor() != 0)
	{
		DWORD dwBkColor = pControlUI->GetBkColor();					
		_stprintf(szBuf, _T("#%02X%02X%02X%02X"), HIBYTE(HIWORD(dwBkColor)), static_cast<BYTE>(GetBValue(dwBkColor)), GetGValue(dwBkColor), GetRValue(dwBkColor));
		pNode->SetAttribute("bkcolor", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pControlUI->GetBkColor2() != 0)
	{
		DWORD dwBkColor = pControlUI->GetBkColor2();
		_stprintf(szBuf, _T("#%02X%02X%02X%02X"), HIBYTE(HIWORD(dwBkColor)), static_cast<BYTE>(GetBValue(dwBkColor)), GetGValue(dwBkColor), GetRValue(dwBkColor));

		pNode->SetAttribute("bkcolor2", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pControlUI->GetBorderColor() != 0)
	{
		DWORD dwBkColor = pControlUI->GetBorderColor();					
		_stprintf(szBuf, _T("#%02X%02X%02X%02X"), HIBYTE(HIWORD(dwBkColor)), static_cast<BYTE>(GetBValue(dwBkColor)), GetGValue(dwBkColor), GetRValue(dwBkColor));
		pNode->SetAttribute("bordercolor", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pControlUI->GetFocusBorderColor() != 0)
	{
		DWORD dwBkColor = pControlUI->GetFocusBorderColor();					
		_stprintf(szBuf, _T("#%02X%02X%02X%02X"), HIBYTE(HIWORD(dwBkColor)), static_cast<BYTE>(GetBValue(dwBkColor)), GetGValue(dwBkColor), GetRValue(dwBkColor));
		pNode->SetAttribute("focusbordercolor", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pControlUI->GetMaxHeight() != 9999)
	{
		_stprintf(szBuf, _T("%d"), pControlUI->GetMaxHeight());
		pNode->SetAttribute("maxheight", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pControlUI->GetMaxWidth() != 9999)
	{
		_stprintf(szBuf, _T("%d"), pControlUI->GetMaxWidth());
		pNode->SetAttribute("maxwidth", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pControlUI->GetMinWidth() != 0)
	{
		_stprintf(szBuf, _T("%d"), pControlUI->GetMinWidth());
		pNode->SetAttribute("minwidth", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pControlUI->GetMinHeight() != 0)
	{
		_stprintf(szBuf, _T("minheight=\"%d\" "), pControlUI->GetMinHeight());
		pNode->SetAttribute("minheight", StringConvertor::TcharToUtf8(szBuf));
	}
}

void CLayoutManager::CLabelUI_Properties(CControlUI* pControlUI, TiXmlElement* pNode)
{
	CControlUI_Properties(pControlUI, pNode);

	CLabelUI* pLabelUI = static_cast<CLabelUI*>(pControlUI->GetInterface(_T("Label")));

	TCHAR szBuf[MAX_PATH] = {0};

	RECT rcTextPadding = pLabelUI->GetTextPadding();
	if (!IsRectEmpty(&rcTextPadding))
	{
		_stprintf(szBuf, _T("%d,%d,%d,%d"), rcTextPadding.left, rcTextPadding.top, rcTextPadding.right, rcTextPadding.bottom);
		pNode->SetAttribute("textpadding", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pLabelUI->GetTextColor() != 0)
	{
		DWORD dwColor = pLabelUI->GetTextColor();					
		_stprintf(szBuf, _T("#%02X%02X%02X%02X"), HIBYTE(HIWORD(dwColor)), static_cast<BYTE>(GetBValue(dwColor)), GetGValue(dwColor), GetRValue(dwColor));
		pNode->SetAttribute("textcolor", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pLabelUI->GetDisabledTextColor() != 0)
	{
		DWORD dwColor = pLabelUI->GetDisabledTextColor();					
		_stprintf(szBuf, _T("#%02X%02X%02X%02X"), HIBYTE(HIWORD(dwColor)), static_cast<BYTE>(GetBValue(dwColor)), GetGValue(dwColor), GetRValue(dwColor));
		pNode->SetAttribute("disabledtextcolor", StringConvertor::TcharToUtf8(szBuf));
	}
}

void CLayoutManager::CButtonUI_Properties(CControlUI* pControlUI, TiXmlElement* pNode)
{
	CLabelUI_Properties(pControlUI, pNode);

	CButtonUI* pButtonUI = static_cast<CButtonUI*>(pControlUI->GetInterface(_T("Button")));
	if (pButtonUI->GetNormalImage() && _tcslen(pButtonUI->GetNormalImage()) > 0)
		pNode->SetAttribute("normalimage", StringConvertor::TcharToUtf8(pButtonUI->GetNormalImage()));

	if (pButtonUI->GetHotImage() && _tcslen(pButtonUI->GetHotImage()) > 0)
		pNode->SetAttribute("hotimage", StringConvertor::TcharToUtf8(pButtonUI->GetHotImage()));

	if (pButtonUI->GetPushedImage() && _tcslen(pButtonUI->GetPushedImage()) > 0)
		pNode->SetAttribute("pushedimage", StringConvertor::TcharToUtf8(pButtonUI->GetPushedImage()));

	if (pButtonUI->GetFocusedImage() && _tcslen(pButtonUI->GetFocusedImage()) > 0)
		pNode->SetAttribute("focusedimage", StringConvertor::TcharToUtf8(pButtonUI->GetFocusedImage()));

	if (pButtonUI->GetDisabledImage() && _tcslen(pButtonUI->GetDisabledImage()) > 0)
		pNode->SetAttribute("disabledimage", StringConvertor::TcharToUtf8(pButtonUI->GetDisabledImage()));

	if (pButtonUI->GetForeImage() && _tcslen(pButtonUI->GetForeImage()) > 0)
		pNode->SetAttribute("foreimage", StringConvertor::TcharToUtf8(pButtonUI->GetForeImage()));
}


void CLayoutManager::COptionUI_Properties(CControlUI* pControlUI, TiXmlElement* pNode)
{
	CButtonUI_Properties(pControlUI, pNode);
	COptionUI* pOptionUI = static_cast<COptionUI*>(pControlUI->GetInterface(_T("Option")));

	TCHAR szBuf[MAX_PATH] = {0};

	if (pOptionUI->IsGroup())
		pNode->SetAttribute("group", pOptionUI->IsGroup()?"true":"false");

	if (pOptionUI->IsSelected())
		pNode->SetAttribute("selected", pOptionUI->IsSelected()?"true":"false");

	if (pOptionUI->GetSelectedTextColor() != 0)
	{
		DWORD dwColor = pOptionUI->GetSelectedTextColor();					
		_stprintf(szBuf, _T("#%02X%02X%02X%02X"), HIBYTE(HIWORD(dwColor)), static_cast<BYTE>(GetBValue(dwColor)), GetGValue(dwColor), GetRValue(dwColor));
		pNode->SetAttribute("selectedtextcolor", StringConvertor::TcharToUtf8(szBuf));
	}

	if (pOptionUI->GetSelectedImage() && _tcslen(pOptionUI->GetSelectedImage()) > 0)
		pNode->SetAttribute("selectedimage", StringConvertor::TcharToUtf8(pOptionUI->GetSelectedImage()));

}

void CLayoutManager::CContainerUI_Properties(CControlUI* pControlUI, TiXmlElement* pNode)
{
	CControlUI_Properties(pControlUI, pNode);

	CContainerUI* pContainerUI = static_cast<CContainerUI*>(pControlUI->GetInterface(_T("Container")));
	for( int it = 0; it < pContainerUI->GetCount(); it++ )
	{
		CControlUI* pControl = static_cast<CControlUI*>(pContainerUI->GetItemAt(it));

		TiXmlElement* pNodeChild = new TiXmlElement(StringConvertor::TcharToUtf8(pControl->GetTypeName()));
		SaveProperties(pControl, pNodeChild);
		TiXmlNode* pNodeElement = pNode->InsertEndChild(*pNodeChild);
		delete pNodeChild;
		pNodeChild = NULL;
	}
}


void CLayoutManager::SaveContainerProperties(CControlUI* pControlUI, TiXmlElement* pParentNode)
{
	TiXmlElement* pNode = NULL;
	if ((static_cast<IContainerUI*>(pControlUI->GetInterface(_T("IContainer"))) != NULL) && (static_cast<IContainerUI*>(pControlUI->GetInterface(_T("Container"))) != NULL))
	{
		pNode = new TiXmlElement(StringConvertor::TcharToUtf8(pControlUI->GetTypeName()));		
		CContainerUI_Properties(pControlUI, pNode);
		TiXmlNode* pNodeElement = pParentNode->InsertEndChild(*pNode);
		delete pNode;
		pNode = NULL;

	}
}

void CLayoutManager::SaveSingleProperties(CControlUI* pControlUI, TiXmlElement* pParentNode)
{
	TiXmlElement* pNode = NULL;
	if (_tcsicmp(pControlUI->GetClass(), _T("ControlUI")) == 0)
	{
		pNode = new TiXmlElement(StringConvertor::TcharToUtf8(pControlUI->GetTypeName()));		
		CControlUI_Properties(pControlUI, pNode);
		TiXmlNode* pNodeElement = pParentNode->InsertEndChild(*pNode);
	}
	else if (_tcsicmp(pControlUI->GetClass(), _T("LabelUI")) == 0)
	{
		pNode = new TiXmlElement(StringConvertor::TcharToUtf8(pControlUI->GetTypeName()));		
		CLabelUI_Properties(pControlUI, pNode);
		TiXmlNode* pNodeElement = pParentNode->InsertEndChild(*pNode);
	}
	else if (_tcsicmp(pControlUI->GetClass(), _T("ButtonUI")) == 0)
	{
		pNode = new TiXmlElement(StringConvertor::TcharToUtf8(pControlUI->GetTypeName()));		
		CButtonUI_Properties(pControlUI, pNode);
		TiXmlNode* pNodeElement = pParentNode->InsertEndChild(*pNode);
	}
	else if (_tcsicmp(pControlUI->GetClass(), _T("OptionUI")) == 0)
	{
		pNode = new TiXmlElement(StringConvertor::TcharToUtf8(pControlUI->GetTypeName()));		
		COptionUI_Properties(pControlUI, pNode);
		TiXmlNode* pNodeElement = pParentNode->InsertEndChild(*pNode);
	}

	if (pNode)
	{
		delete pNode;
		pNode = NULL;
	}
}

void CLayoutManager::SaveProperties(CControlUI* pControlUI, TiXmlElement* pParentNode)
{
	if ((pControlUI != NULL) && (pParentNode != NULL))
	{
		if ((static_cast<IContainerUI*>(pControlUI->GetInterface(_T("IContainer"))) != NULL) && (static_cast<IContainerUI*>(pControlUI->GetInterface(_T("Container"))) != NULL))
		{
			SaveContainerProperties(pControlUI, pParentNode);
		}
		else
		{
			SaveSingleProperties(pControlUI, pParentNode);
		}
	}
}
#endif