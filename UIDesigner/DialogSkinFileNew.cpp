// DialogSkinFileNew.cpp : implementation file
//

#include "stdafx.h"
#include "UIDesigner.h"
#include "DialogSkinFileNew.h"


// CDialogSkinFileNew dialog

IMPLEMENT_DYNAMIC(CDialogSkinFileNew, CDialog)

CDialogSkinFileNew::CDialogSkinFileNew(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogSkinFileNew::IDD, pParent)
	, m_strFileName(_T(""))
	, m_strStyleFile(_T(""))
	, m_strStyleDir(_T(""))
{

}

CDialogSkinFileNew::~CDialogSkinFileNew()
{
}

void CDialogSkinFileNew::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE_NAME, m_strFileName);
	DDX_Control(pDX, IDC_LIST_STYLE, m_lstStyle);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_Preview);
}


BEGIN_MESSAGE_MAP(CDialogSkinFileNew, CDialog)
	ON_LBN_DBLCLK(IDC_LIST_STYLE, &CDialogSkinFileNew::OnLbnDblclkListStyle)
	ON_BN_CLICKED(IDOK, &CDialogSkinFileNew::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogSkinFileNew message handlers

BOOL CDialogSkinFileNew::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_lstStyle.AddString(_T("(нч)"));
	m_lstStyle.SetCurSel(0);
	m_strStyleDir = CGlobalVariable::GetCurPath() + DIR_STYLES;
	FindStyleFiles(m_strStyleDir);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

CString CDialogSkinFileNew::GetStyleFilePath() const
{
	return m_strStyleFile;
}

void CDialogSkinFileNew::FindStyleFiles(CString& strDir)
{
	WIN32_FIND_DATA FindFileData = {0};
	CString strFind = strDir + _T("*.xml");
	HANDLE hFind = ::FindFirstFile(strFind, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
		return;
	do
	{
		m_lstStyle.AddString(FindFileData.cFileName);
	}while(::FindNextFile(hFind, &FindFileData));
	::FindClose(hFind);
}

void CDialogSkinFileNew::OnLbnDblclkListStyle()
{
	// TODO: Add your control notification handler code here
	CString strStyleFileName;
	int nIndex = m_lstStyle.GetCurSel();
	m_lstStyle.GetText(nIndex, strStyleFileName);
	m_strStyleFile = (strStyleFileName == _T("(нч)")) ? _T(""):m_strStyleDir + strStyleFileName;
}

void CDialogSkinFileNew::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	OnOK();
}
