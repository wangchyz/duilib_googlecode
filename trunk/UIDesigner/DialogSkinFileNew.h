#pragma once
#include "afxwin.h"

// CDialogSkinFileNew dialog

class CDialogSkinFileNew : public CDialog
{
	DECLARE_DYNAMIC(CDialogSkinFileNew)

public:
	CDialogSkinFileNew(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogSkinFileNew();

// Dialog Data
	enum { IDD = IDD_SKINFIlE_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	CString GetStyleFilePath() const;

private:
	void FindStyleFiles(CString& strDir);

	DECLARE_MESSAGE_MAP()
public:
	CString m_strFileName;

private:
	CListBox m_lstStyle;
	CStatic m_Preview;
	CString m_strStyleFile;
	CString m_strStyleDir;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnDblclkListStyle();
	afx_msg void OnBnClickedOk();
};
