#pragma once


// CDialogTemplateSaveAs 对话框

class CDialogTemplateSaveAs : public CDialog
{
	DECLARE_DYNAMIC(CDialogTemplateSaveAs)

public:
	CDialogTemplateSaveAs(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogTemplateSaveAs();

// 对话框数据
	enum { IDD = IDD_DIALOG_TEMPLATE_SAVE_AS };

public:
	CString GetTemplateName() const { return m_strName; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CString m_strName;
};
