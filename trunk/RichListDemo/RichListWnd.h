#pragma once
#include "WinImplBase.hpp"

class CRichListWnd : public WindowImplBase
{
public:
	CRichListWnd(void);
	~CRichListWnd(void);

	virtual void OnFinalMessage( HWND );
	virtual tString GetSkinFolder();
	virtual tString GetSkinFile();
	virtual LPCTSTR GetWindowClassName( void ) const;
	virtual void Notify( TNotifyUI &msg );
	virtual LRESULT OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual void Init();
	virtual LRESULT OnMouseHover( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual LRESULT OnChar( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual tString GetZIPFileName() const;

private:
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
};
