
// UIDesignerDoc.cpp : CUIDesignerDoc 类的实现
//

#include "stdafx.h"
#include "UIDesigner.h"

#include "UIDesignerDoc.h"
#include "DialogSkinFileNew.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUIDesignerDoc

IMPLEMENT_DYNCREATE(CUIDesignerDoc, CDocument)

BEGIN_MESSAGE_MAP(CUIDesignerDoc, CDocument)
	ON_COMMAND(ID_TEMPLATE_SAVE, &CUIDesignerDoc::OnTemplateSave)
END_MESSAGE_MAP()


// CUIDesignerDoc 构造/析构

CUIDesignerDoc::CUIDesignerDoc()
{
	// TODO: 在此添加一次性构造代码

}

CUIDesignerDoc::~CUIDesignerDoc()
{
}

BOOL CUIDesignerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	CDialogSkinFileNew dlg;
	dlg.m_strFileName = this->GetTitle();
	if(dlg.DoModal() == IDOK)
	{
		CString strFilePath = dlg.GetStyleFilePath();
		if(!strFilePath.IsEmpty())
			this->SetPathName(dlg.GetStyleFilePath());
		if(!dlg.m_strFileName.IsEmpty())
			this->SetTitle(dlg.m_strFileName);
	}
	else
		return FALSE;

	return TRUE;
}




// CUIDesignerDoc 序列化

void CUIDesignerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}


// CUIDesignerDoc 诊断

#ifdef _DEBUG
void CUIDesignerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CUIDesignerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CUIDesignerDoc 命令

void CUIDesignerDoc::OnTemplateSave()
{
	// TODO: 在此添加命令处理程序代码
}

BOOL CUIDesignerDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: 在此添加专用代码和/或调用基类
	POSITION pos = this->GetFirstViewPosition();
	while(pos)
	{
		CView* pView = this->GetNextView(pos);
		CUIDesignerView* pUIView = DYNAMIC_DOWNCAST(CUIDesignerView, pView);
		ASSERT(pUIView);
		pUIView->OnSaveSkinFile(lpszPathName);
	}

	return TRUE/*CDocument::OnSaveDocument(lpszPathName)*/;
}
