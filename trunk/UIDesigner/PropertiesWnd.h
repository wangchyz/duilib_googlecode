#pragma once
#include "../DuiLib/UIlib.h"

//////////////////////////////////////////////////////////////////////////
//CMFCPropertyGridColor32Property

#define ARGB(a,r,g,b)        ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16))|(((DWORD)(BYTE)(a))<<24))
#define GetAValue(argb)      (LOBYTE((argb)>>24))

class CMFCPropertyGridColor32Property : public CMFCPropertyGridColorProperty
{
public:
	CMFCPropertyGridColor32Property(const CString& strName,const COLORREF& color,CPalette* pPalette=NULL,LPCTSTR lpszDescr=NULL,DWORD_PTR dwData=0);
	virtual ~CMFCPropertyGridColor32Property();

public:
	virtual BOOL OnUpdateValue();
	virtual void OnDrawValue(CDC* pDC, CRect rect);
	virtual CString FormatProperty();
};

//////////////////////////////////////////////////////////////////////////
//CMFCPropertyGridImageProperty

#define AFX_PROP_HAS_BUTTON 0x0002

class CMFCPropertyGridImageProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridImageProperty)

public:
	CMFCPropertyGridImageProperty(const CString& strName, const CString& strImage, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	virtual ~CMFCPropertyGridImageProperty();

public:
	virtual void OnClickButton(CPoint point);
};

//////////////////////////////////////////////////////////////////////////
//CMFCPropertyGridCustomFontsProperty

class CMFCPropertyGridCustomFontsProperty : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CMFCPropertyGridCustomFontsProperty)

public:
	CMFCPropertyGridCustomFontsProperty(const CString& strName, const CString& strFonts, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);
	virtual ~CMFCPropertyGridCustomFontsProperty();

public:
	virtual void OnClickButton(CPoint point);
};

//////////////////////////////////////////////////////////////////////////
//CPropertiesToolBar

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

//////////////////////////////////////////////////////////////////////////
//CPropertiesWnd

#define UPDATE_POS 0x01
#define UPDATE_REDRAW_CONTROL 0x02
#define UPDATE_REDRAW_PARENT 0x03

enum PropertyTag
{
	tagNULL=0,

	//Form
	tagForm,
	tagFormSize=tagForm,
	tagCaption,
	tagSizeBox,
	tagRoundCorner,
	tagMinInfo,
	tagMaxInfo,
	tagShowDirty,
	tagCustomFonts,

	//Control
	tagControl,
	tagName=tagControl,
	tagText,
	tagPos,
	tagRelativePos,
	tagSize,
	tagMinSize,
	tagMaxSize,
	tagPadding,
	tagBkImage,
	tagBkColor,
	tagBkColor2,
	tagBorderColor,
	tagBorderSize,
	tagEnabled,
	tagFloat,
	tagVisible,

	//Label
	tagLabel,
	tagAlign=tagLabel,
	tagTextColor,
	tagDisabledTextColor,
	tagFont,
	tagTextPadding,
	tagShowHtml,

	//Button
	tagButton,
	tagNormalImage=tagButton,
	tagHotImage,
	tagPushedImage,
	tagFocusedImage,
	tagDisabledImage,

	//Edit
	tagEdit,
	tagEditNormalImage=tagEdit,
	tagEditHotImage,
	tagEditFocusedImage,
	tagEditDisabledImage,
	tagReadOnly,
	tagPassword,

	//Option
	tagOption,
	tagSelectedImage=tagOption,
	tagSelected,
	tagGroup,

	//Progress
	tagProgress,
	tagFgImage=tagProgress,
	tagMinMax,
	tagValue,
	tagHor,

	//Slider
	tagSlider,
	tagThumbImage=tagSlider,
	tagThumbHotImage,
	tagThumbPushedImage,
	tagThumbSize,

	//Combo
	tagCombo,
	tagComboNormalImage=tagCombo,
	tagComboHotImage,
	tagComboPushedImage,
	tagComboFocusedImage,
	tagComboDisabledImage,
	tagItemImage,
	tagItemHotImage,
	tagItemSelectedImage,
	tagItemDisabledImage,
	tagComboTextPadding,
	tagItemTextPadding,
	tagItemAlign,
	tagItemTextColor,
	tagItemBkColor,
	tagItemSelectedTextColor,
	tagItemSelectedBkColor,
	tagItemHotTextColor,
	tagItemHotBkColor,
	tagItemDisabledTextColor,
	tagItemDisabledBkColor,
	tagItemLineColor,
	tagItemFont,
	tagItemShowHtml,

	//ActiveX
	tagActiveX,
	tagClsid=tagActiveX,

	//Container
	tagContainer,
	tagInset=tagContainer,
	tagChildPadding,
	tagMouseChild,
	tagHScrollBar,
	tagVScrollBar,

	//HorizontalLayout
	tagHorizontalLayout,
	tagSepWidth=tagHorizontalLayout,
	tagSepImm,

	//TileLayout
	tagTileLayout,
	tagColumns=tagTileLayout,
};

class CPropertiesWnd : public CDockablePane
{
// 构造
public:
	CPropertiesWnd();
	virtual ~CPropertiesWnd();

// 实现
public:
	void SetVSDotNetLook(BOOL bSet);
	void AdjustLayout();

	void HideAllProperties(BOOL bHide=TRUE,BOOL bAdjustLayout=FALSE);
	void ShowProperty(CControlUI* pControl);
	void SetPropValue(CControlUI* pControl,int nTag);

protected:
	void InitPropList();
	void SetPropListFont();

	//FormProperty
	void ShowFormProperty(CControlUI* pControl);

	//Control Property
	void ShowControlProperty(CControlUI* pControl);
	void ShowLabelProperty(CControlUI* pControl);
	void ShowButtonProperty(CControlUI* pControl);
	void ShowEditProperty(CControlUI* pControl);
	void ShowOptionProperty(CControlUI* pControl);
	void ShowProgressProperty(CControlUI* pControl);
	void ShowSliderProperty(CControlUI* pControl);
	void ShowComboProperty(CControlUI* pControl);
	void ShowActiveXProperty(CControlUI* pControl);

	//Container Property
	void ShowContainerProperty(CControlUI* pControl);
	void ShowHorizontalLayoutProperty(CControlUI* pControl);
	void ShowTileLayoutProperty(CControlUI* pControl);

	void SetUIValue(CMFCPropertyGridProperty* pProp,int nTag);

	CString FormatOrigProperty(CMFCPropertyGridProperty* pProp);

protected:
	CFont m_fntPropList;
	CPropertiesToolBar m_wndToolBar;
	CMFCPropertyGridCtrl m_wndPropList;

	CControlUI* m_pControl;//save the control pointer

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPropertiesSort();
	afx_msg void OnUpdatePropertiesSort(CCmdUI* pCmdUI);
	afx_msg void OnPropertiesAlphaBetic();
	afx_msg void OnUpdatePropertiesAlphaBetic(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnPropertyChanged(WPARAM wp, LPARAM lp);

	DECLARE_MESSAGE_MAP()
};

