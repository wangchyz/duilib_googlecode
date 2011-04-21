
// UIDesignerDoc.cpp : CUIDesignerDoc 类的实现
//

#include "stdafx.h"
#include "UIDesigner.h"

#include "UIDesignerDoc.h"
#include "DialogUIFileNew.h"

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
	CDialogUIFileNew dlg;
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
