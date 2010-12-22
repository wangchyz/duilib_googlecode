// ToolBoxWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "UIDesigner.h"
#include "ToolBoxWnd.h"

#include <memory>

// CToolBoxWnd

IMPLEMENT_DYNAMIC(CToolBoxWnd, CDockablePane)

CToolBoxWnd::CToolBoxWnd()
{

}

CToolBoxWnd::~CToolBoxWnd()
{
}


BEGIN_MESSAGE_MAP(CToolBoxWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CToolBoxWnd 消息处理程序

int CToolBoxWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	if (!m_ctlToolList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 12))
	{
		TRACE0("Failed to create ToolBox\n");
		return -1;      // fail to create
	}
	InitToolList();

	return 0;
}

void CToolBoxWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	AdjustLayout();
}

void CToolBoxWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	m_ctlToolList.SetWindowPos(NULL, rectClient.left,rectClient.top,rectClient.Width(),rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CToolBoxWnd::InitToolList()
{
	std::auto_ptr<CToolElement> apTab1(new CToolElement(_T("对话框")));
	CToolElement* pTool=new CToolElement(_T("指针1"),IDI_PROPERTIES_WND_HC);
	apTab1->AddSubTool(pTool);
	apTab1->AddSubTool(new CToolElement(_T("指针2"),IDI_PROPERTIES_WND_HC));
	apTab1->AddSubTool(new CToolElement(_T("指针3"),IDI_PROPERTIES_WND_HC));
	apTab1->AddSubTool(new CToolElement(_T("指针4"),IDI_PROPERTIES_WND_HC));
	apTab1->AddSubTool(new CToolElement(_T("指针5"),IDI_PROPERTIES_WND_HC));
	apTab1->AddSubTool(new CToolElement(_T("指针6"),IDI_PROPERTIES_WND_HC));
	apTab1->AddSubTool(new CToolElement(_T("指针7"),IDI_PROPERTIES_WND_HC));
	apTab1->AddSubTool(new CToolElement(_T("指针8"),IDI_PROPERTIES_WND_HC));
	apTab1->AddSubTool(new CToolElement(_T("指针9"),IDI_PROPERTIES_WND_HC));
	apTab1->AddSubTool(new CToolElement(_T("指针10"),IDI_PROPERTIES_WND_HC));

	m_ctlToolList.AddToolTab(apTab1.release());

	std::auto_ptr<CToolElement> apTab2(new CToolElement(_T("对话框")));
	apTab2->AddSubTool(new CToolElement(_T("指针1"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针2"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针3"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针4"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针5"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针6"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针7"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针8"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针9"),IDI_PROPERTIES_WND_HC));
	apTab2->AddSubTool(new CToolElement(_T("指针10"),IDI_PROPERTIES_WND_HC));

	m_ctlToolList.AddToolTab(apTab2.release());
	m_ctlToolList.SetCurSel(pTool);
}