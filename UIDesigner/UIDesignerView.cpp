
// UIDesignerView.cpp : CUIDesignerView 类的实现
//

#include "stdafx.h"
#include "UIDesigner.h"

#include "UIDesignerDoc.h"
#include "UIDesignerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUIDesignerView

IMPLEMENT_DYNCREATE(CUIDesignerView, CView)

BEGIN_MESSAGE_MAP(CUIDesignerView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CUIDesignerView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CUIDesignerView 构造/析构

CUIDesignerView::CUIDesignerView()
{
	// TODO: 在此处添加构造代码

}

CUIDesignerView::~CUIDesignerView()
{
}

BOOL CUIDesignerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CUIDesignerView 绘制

void CUIDesignerView::OnDraw(CDC* /*pDC*/)
{
	CUIDesignerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CUIDesignerView 打印


void CUIDesignerView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CUIDesignerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CUIDesignerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CUIDesignerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CUIDesignerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CUIDesignerView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CUIDesignerView 诊断

#ifdef _DEBUG
void CUIDesignerView::AssertValid() const
{
	CView::AssertValid();
}

void CUIDesignerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CUIDesignerDoc* CUIDesignerView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUIDesignerDoc)));
	return (CUIDesignerDoc*)m_pDocument;
}
#endif //_DEBUG


// CUIDesignerView 消息处理程序
