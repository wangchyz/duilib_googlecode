// TemplateImagePreview.cpp : 实现文件
//

#include "stdafx.h"
#include "UIDesigner.h"
#include "TemplateImagePreview.h"


// CTemplateImagePreview

IMPLEMENT_DYNAMIC(CTemplateImagePreview, CStatic)

CTemplateImagePreview::CTemplateImagePreview()
{

}

CTemplateImagePreview::~CTemplateImagePreview()
{
}


BEGIN_MESSAGE_MAP(CTemplateImagePreview, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CTemplateImagePreview 消息处理程序

void CTemplateImagePreview::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CStatic::OnPaint()
	CMemDC memDC(dc, this);
	CDC* pDC = &memDC.GetDC();

	CRect rectClient;
	CRgn rgnClip;
	this->GetClientRect(&rectClient);
	rgnClip.CreateRectRgnIndirect(&rectClient);
	pDC->SelectClipRgn(&rgnClip);
	pDC->FillRect(&rectClient, &afxGlobalData.brBtnFace);
	if(!m_imgPreview.IsNull())
		m_imgPreview.BitBlt(pDC->GetSafeHdc(), 0, 0);
}

void CTemplateImagePreview::SetPreviewImage(LPCTSTR pstrImage)
{
	if(!m_imgPreview.IsNull())
		m_imgPreview.Destroy();

	if(*pstrImage != _T('\0'))
		m_imgPreview.Load(pstrImage);
	this->Invalidate(TRUE);
}