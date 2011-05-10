#pragma once
#include <atlimage.h>

// CTemplateImagePreview

class CTemplateImagePreview : public CStatic
{
	DECLARE_DYNAMIC(CTemplateImagePreview)

public:
	CTemplateImagePreview();
	virtual ~CTemplateImagePreview();

private:
	CImage m_imgPreview;

public:
	void SetPreviewImage(LPCTSTR pstrImage);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};