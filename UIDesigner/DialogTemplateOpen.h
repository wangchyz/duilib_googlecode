#pragma once
#include "afxwin.h"
#include "TemplateImagePreview.h"

// CDialogTemplateOpen 对话框

class CDialogTemplateOpen : public CDialog
{
	DECLARE_DYNAMIC(CDialogTemplateOpen)

public:
	CDialogTemplateOpen(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogTemplateOpen();

// 对话框数据
	enum { IDD = IDD_DIALOG_TEMPLATE_OPEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CListBox m_lstTemplates;
	CTemplateImagePreview m_TemplatePreview;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListTemplates();
	afx_msg void OnBnClickedButtonTemplateDelete();
	afx_msg void OnBnClickedOk();
};
