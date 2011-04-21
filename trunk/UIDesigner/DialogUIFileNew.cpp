// DialogUIFileNew.cpp : implementation file
//

#include "stdafx.h"
#include "UIDesigner.h"
#include "DialogUIFileNew.h"


// CDialogUIFileNew dialog

IMPLEMENT_DYNAMIC(CDialogUIFileNew, CDialog)

CDialogUIFileNew::CDialogUIFileNew(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogUIFileNew::IDD, pParent)
	, m_strFileName(_T(""))
	, m_strStyleFile(_T(""))
	, m_strStyleDir(_T(""))
{

}

CDialogUIFileNew::~CDialogUIFileNew()
{
}

void CDialogUIFileNew::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE_NAME, m_strFileName);
	DDX_Control(pDX, IDC_LIST_STYLE, m_lstStyle);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_Preview);
}


BEGIN_MESSAGE_MAP(CDialogUIFileNew, CDialog)
	ON_LBN_DBLCLK(IDC_LIST_STYLE, &CDialogUIFileNew::OnLbnDblclkListStyle)
	ON_BN_CLICKED(IDOK, &CDialogUIFileNew::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogUIFileNew message handlers

BOOL CDialogUIFileNew::OnInitDialog()
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

CString CDialogUIFileNew::GetStyleFilePath() const
{
	return m_strStyleFile;
}

void CDialogUIFileNew::FindStyleFiles(CString& strDir)
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

void CDialogUIFileNew::OnLbnDblclkListStyle()
{
	// TODO: Add your control notification handler code here
	CString strStyleFileName;
	int nIndex = m_lstStyle.GetCurSel();
	m_lstStyle.GetText(nIndex, strStyleFileName);
	m_strStyleFile = (strStyleFileName == _T("(нч)")) ? _T(""):m_strStyleDir + strStyleFileName;
}

void CDialogUIFileNew::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	OnOK();
}
