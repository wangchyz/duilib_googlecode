#ifndef __UIACTIVEX_H__
#define __UIACTIVEX_H__

#pragma once

struct IOleObject;

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class CActiveXCtrl;

template< class T >
class CSafeRelease
{
public:
    CSafeRelease(T* p) : m_p(p) { };
    ~CSafeRelease() { if( m_p != NULL ) m_p->Release(); };
    T* Detach() { T* t = m_p; m_p = NULL; return t; };
    T* m_p;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CActiveXUI : public CControlUI, public IMessageFilterUI
{
    friend CActiveXCtrl;
public:
    CActiveXUI();
    virtual ~CActiveXUI();

    LPCTSTR GetClass() const;

    bool CreateControl(const CLSID clsid, bool bDelayedCreate = false);
    bool CreateControl(LPCTSTR pstrCLSID, bool bDelayedCreate = false);
    HRESULT GetControl(const IID iid, LPVOID* ppRet);

    void SetVisible(bool bVisible = true);
    void SetInternVisible(bool bVisible = true);
    void SetPos(RECT rc);
    void DoPaint(HDC hDC, const RECT& rcPaint);

    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

protected:
    void ReleaseControl();
    bool DelayedControlCreation();

protected:
    CLSID m_clsid;
    bool m_bCreated;
    IOleObject* m_pUnk;
    CActiveXCtrl* m_pControl;
    HWND m_hwndHost;
};

} // namespace DuiLib

#endif // __UIACTIVEX_H__
