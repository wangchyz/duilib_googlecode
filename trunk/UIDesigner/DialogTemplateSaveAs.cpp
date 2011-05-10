// DialogTemplateSaveAs.cpp : 实现文件
//

#include "stdafx.h"
#include "UIDesigner.h"
#include "DialogTemplateSaveAs.h"


// CDialogTemplateSaveAs 对话框

IMPLEMENT_DYNAMIC(CDialogTemplateSaveAs, CDialog)

CDialogTemplateSaveAs::CDialogTemplateSaveAs(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogTemplateSaveAs::IDD, pParent)
	, m_strName(_T("未命名1"))
{

}

CDialogTemplateSaveAs::~CDialogTemplateSaveAs()
{
}

void CDialogTemplateSaveAs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TEMPLATE_NAME, m_strName);
}


BEGIN_MESSAGE_MAP(CDialogTemplateSaveAs, CDialog)
END_MESSAGE_MAP()


// CDialogTemplateSaveAs 消息处理程序
