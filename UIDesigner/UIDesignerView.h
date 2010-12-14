
// UIDesignerView.h : CUIDesignerView 类的接口
//


#pragma once


class CUIDesignerView : public CView
{
protected: // 仅从序列化创建
	CUIDesignerView();
	DECLARE_DYNCREATE(CUIDesignerView)

// 属性
public:
	CUIDesignerDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CUIDesignerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // UIDesignerView.cpp 中的调试版本
inline CUIDesignerDoc* CUIDesignerView::GetDocument() const
   { return reinterpret_cast<CUIDesignerDoc*>(m_pDocument); }
#endif

