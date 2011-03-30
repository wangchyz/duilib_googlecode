
#include "StdAfx.h"
#include "UIManager.h"
#include <zmouse.h>

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//

static UINT MapKeyState()
{
    UINT uState = 0;
    if( ::GetKeyState(VK_CONTROL) < 0 ) uState |= MK_CONTROL;
    if( ::GetKeyState(VK_RBUTTON) < 0 ) uState |= MK_LBUTTON;
    if( ::GetKeyState(VK_LBUTTON) < 0 ) uState |= MK_RBUTTON;
    if( ::GetKeyState(VK_SHIFT) < 0 ) uState |= MK_SHIFT;
    if( ::GetKeyState(VK_MENU) < 0 ) uState |= MK_ALT;
    return uState;
}

typedef struct tagFINDTABINFO
{
    CControlUI* pFocus;
    CControlUI* pLast;
    bool bForward;
    bool bNextIsIt;
} FINDTABINFO;

typedef struct tagFINDSHORTCUT
{
    TCHAR ch;
    bool bPickNext;
} FINDSHORTCUT;

typedef struct tagTIMERINFO
{
    CControlUI* pSender;
    UINT nLocalID;
    HWND hWnd;
    UINT uWinTimer;
    bool bKilled;
} TIMERINFO;


/////////////////////////////////////////////////////////////////////////////////////

HPEN m_hUpdateRectPen = NULL;
HINSTANCE CPaintManagerUI::m_hInstance = NULL;
HINSTANCE CPaintManagerUI::m_hResourceInstance = NULL;
CStdString CPaintManagerUI::m_pStrResourcePath;
CStdString CPaintManagerUI::m_pStrResourceZip;
CStdPtrArray CPaintManagerUI::m_aPreMessages;


CPaintManagerUI::CPaintManagerUI() :
m_hWndPaint(NULL),
m_hDcPaint(NULL),
m_hDcOffscreen(NULL),
m_hDcBackground(NULL),
m_hbmpOffscreen(NULL),
m_hbmpBackground(NULL),
m_hwndTooltip(NULL),
m_bShowUpdateRect(false),
m_uTimerID(0x1000),
m_pRoot(NULL),
m_pFocus(NULL),
m_pEventHover(NULL),
m_pEventClick(NULL),
m_pEventKey(NULL),
m_bFirstLayout(true),
m_bFocusNeeded(false),
m_bUpdateNeeded(false),
m_bMouseTracking(false),
m_bMouseCapture(false),
m_bOffscreenPaint(true),
m_bAlphaBackground(false),
m_pParentResourcePM(NULL)
{
    m_dwDefalutDisabledColor = 0xFFA7A6AA;
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    lf.lfHeight = -12;
    m_hDefalutFont = ::CreateFontIndirect(&lf);
    m_dwDefalutFontColor = 0xFF000000;
    ::ZeroMemory(&m_hDefalutFontTextMetric, sizeof(m_hDefalutFontTextMetric));
    
    lf.lfUnderline = TRUE;
    m_hDefalutLinkFont = ::CreateFontIndirect(&lf);
    m_dwDefalutLinkFontColor = 0xFF0000FF;
    m_dwDefalutLinkFontHoverColor = 0xFFD3215F;
    ::ZeroMemory(&m_hDefaluttBoldFontTextMetric, sizeof(m_hDefaluttBoldFontTextMetric));

    lf.lfUnderline = FALSE;
    lf.lfWeight += FW_BOLD;
    m_hDefalutBoldFont = ::CreateFontIndirect(&lf);
    m_dwDefalutBoldFontColor = 0xFF000000;
    ::ZeroMemory(&m_hDefalutLinkFontTextMetric, sizeof(m_hDefalutLinkFontTextMetric));

    if( m_hUpdateRectPen == NULL ) {
        m_hUpdateRectPen = ::CreatePen(PS_SOLID, 1, RGB(220, 0, 0));
        // Boot Windows Common Controls (for the ToolTip control)
        ::InitCommonControls();
        ::LoadLibrary(_T("msimg32.dll"));
    }

    m_szMinWindow.cx = 0;
    m_szMinWindow.cy = 0;
    m_szInitWindowSize.cx = 0;
    m_szInitWindowSize.cy = 0;
    m_szRoundCorner.cx = m_szRoundCorner.cy = 0;
    ::ZeroMemory(&m_rcSizeBox, sizeof(m_rcSizeBox));
    ::ZeroMemory(&m_rcCaption, sizeof(m_rcCaption));
    m_ptLastMousePos.x = m_ptLastMousePos.y = -1;
}

CPaintManagerUI::~CPaintManagerUI()
{
    // Delete the control-tree structures
    for( int i = 0; i < m_aDelayedCleanup.GetSize(); i++ ) delete static_cast<CControlUI*>(m_aDelayedCleanup[i]);
    m_mNameHash.Resize(0);

    for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
        TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->bKilled == false ) {
            KillTimer(pTimer->pSender, pTimer->nLocalID);
        }

        delete static_cast<TIMERINFO*>(m_aTimers[i]);
    }

    delete m_pRoot;

    ::DeleteObject(m_hDefalutFont);
    ::DeleteObject(m_hDefalutBoldFont);
    ::DeleteObject(m_hDefalutLinkFont);
    RemoveAllFonts();
    RemoveAllImages();
    RemoveAllDefaultAttributeList();

    // Reset other parts...
    if( m_hwndTooltip != NULL ) ::DestroyWindow(m_hwndTooltip);
    if( m_hDcOffscreen != NULL ) ::DeleteDC(m_hDcOffscreen);
    if( m_hDcBackground != NULL ) ::DeleteDC(m_hDcBackground);
    if( m_hbmpOffscreen != NULL ) ::DeleteObject(m_hbmpOffscreen);
    if( m_hbmpBackground != NULL ) ::DeleteObject(m_hbmpBackground);
    if( m_hDcPaint != NULL ) ::ReleaseDC(m_hWndPaint, m_hDcPaint);
    m_aPreMessages.Remove(m_aPreMessages.Find(this));
}

void CPaintManagerUI::Init(HWND hWnd)
{
    ASSERT(::IsWindow(hWnd));
    // Remember the window context we came from
    m_hWndPaint = hWnd;
    m_hDcPaint = ::GetDC(hWnd);
    // We'll want to filter messages globally too
    m_aPreMessages.Add(this);
}

HINSTANCE CPaintManagerUI::GetInstance()
{
    return m_hInstance;
}

CStdString CPaintManagerUI::GetInstancePath()
{
    if( m_hInstance == NULL ) return _T('\0');
    
    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(m_hInstance, tszModule, MAX_PATH);
    CStdString sInstancePath = tszModule;
    int pos = sInstancePath.ReverseFind(_T('\\'));
    sInstancePath = sInstancePath.Left(pos);
    return sInstancePath;
}

CStdString CPaintManagerUI::GetCurrentPath()
{
    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetCurrentDirectory(MAX_PATH, tszModule);
    return tszModule;
}

HINSTANCE CPaintManagerUI::GetResourceDll()
{
    if( m_hResourceInstance == NULL ) return m_hInstance;
    return m_hResourceInstance;
}

const CStdString& CPaintManagerUI::GetResourcePath()
{
    return m_pStrResourcePath;
}

const CStdString& CPaintManagerUI::GetResourceZip()
{
    return m_pStrResourceZip;
}

void CPaintManagerUI::SetInstance(HINSTANCE hInst)
{
    m_hInstance = hInst;
}

void CPaintManagerUI::SetCurrentPath(LPCTSTR pStrPath)
{
    ::SetCurrentDirectory(pStrPath);
}

void CPaintManagerUI::SetResourceDll(HINSTANCE hInst)
{
    m_hResourceInstance = hInst;
}

void CPaintManagerUI::SetResourcePath(LPCTSTR pStrPath)
{
    m_pStrResourcePath = pStrPath;
    if( m_pStrResourcePath.IsEmpty() ) return;
    TCHAR cEnd = m_pStrResourcePath.GetAt(m_pStrResourcePath.GetLength() - 1);
    if( cEnd != _T('\\') && cEnd != _T('/') ) m_pStrResourcePath += _T('\\');
}

void CPaintManagerUI::SetResourceZip(LPCTSTR pStrPath)
{
    m_pStrResourceZip = pStrPath;
}

HWND CPaintManagerUI::GetPaintWindow() const
{
    return m_hWndPaint;
}

HDC CPaintManagerUI::GetPaintDC() const
{
    return m_hDcPaint;
}

POINT CPaintManagerUI::GetMousePos() const
{
    return m_ptLastMousePos;
}

SIZE CPaintManagerUI::GetClientSize() const
{
    RECT rcClient = { 0 };
    ::GetClientRect(m_hWndPaint, &rcClient);
    return CSize(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
}

SIZE CPaintManagerUI::GetInitSize()
{
    return m_szInitWindowSize;
}

void CPaintManagerUI::SetInitSize(int cx, int cy)
{
    m_szInitWindowSize.cx = cx;
    m_szInitWindowSize.cy = cy;
}

RECT& CPaintManagerUI::GetSizeBox()
{
    return m_rcSizeBox;
}

void CPaintManagerUI::SetSizeBox(RECT& rcSizeBox)
{
    m_rcSizeBox = rcSizeBox;
}

RECT& CPaintManagerUI::GetCaptionRect()
{
    return m_rcCaption;
}

void CPaintManagerUI::SetCaptionRect(RECT& rcCaption)
{
    m_rcCaption = rcCaption;
}

SIZE CPaintManagerUI::GetRoundCorner() const
{
    return m_szRoundCorner;
}

void CPaintManagerUI::SetRoundCorner(int cx, int cy)
{
    m_szRoundCorner.cx = cx;
    m_szRoundCorner.cy = cy;
}

SIZE CPaintManagerUI::GetMinMaxInfo() const
{
	return m_szMinWindow;
}

void CPaintManagerUI::SetMinMaxInfo(int cx, int cy)
{
    ASSERT(cx>=0 && cy>=0);
    m_szMinWindow.cx = cx;
    m_szMinWindow.cy = cy;
}

void CPaintManagerUI::SetTransparent(int nOpacity)
{
    if( m_hWndPaint != NULL ) {
        typedef BOOL (__stdcall *PFUNCSETLAYEREDWINDOWATTR)(HWND, COLORREF, BYTE, DWORD);
        PFUNCSETLAYEREDWINDOWATTR fSetLayeredWindowAttributes;

        HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
        if (hUser32)
        {
            fSetLayeredWindowAttributes = 
                (PFUNCSETLAYEREDWINDOWATTR)::GetProcAddress(hUser32, "SetLayeredWindowAttributes");
            if( fSetLayeredWindowAttributes == NULL ) return;
        }

        DWORD dwStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
        DWORD dwNewStyle = dwStyle;
        if( nOpacity >= 0 && nOpacity < 256 ) dwNewStyle |= WS_EX_LAYERED;
        else dwNewStyle &= ~WS_EX_LAYERED;
        if(dwStyle != dwNewStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewStyle);
        fSetLayeredWindowAttributes(m_hWndPaint, 0, nOpacity, LWA_ALPHA);
    }
}

void CPaintManagerUI::SetBackgroundTransparent(bool bTrans)
{
    m_bAlphaBackground = bTrans;
}

bool CPaintManagerUI::IsShowUpdateRect() const
{
	return m_bShowUpdateRect;
}

void CPaintManagerUI::SetShowUpdateRect(bool show)
{
    m_bShowUpdateRect = show;
}

bool CPaintManagerUI::PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& /*lRes*/)
{
    switch( uMsg ) {
   case WM_KEYDOWN:
       {
           // Tabbing between controls
           if( wParam == VK_TAB ) {
               if( m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && _tcsstr(m_pFocus->GetClass(), _T("RichEditUI")) != NULL ) {
                   if( static_cast<CRichEditUI*>(m_pFocus)->IsWantTab() ) return false;
               }
               SetNextTabControl(::GetKeyState(VK_SHIFT) >= 0);
               return true;
           }
       }
       break;
   case WM_SYSCHAR:
       {
           // Handle ALT-shortcut key-combinations
           FINDSHORTCUT fs = { 0 };
           fs.ch = toupper((int)wParam);
           CControlUI* pControl = m_pRoot->FindControl(__FindControlFromShortcut, &fs, UIFIND_ENABLED | UIFIND_ME_FIRST | UIFIND_TOP_FIRST);
           if( pControl != NULL ) {
               pControl->SetFocus();
               pControl->Activate();
               return true;
           }
       }
       break;
   case WM_SYSKEYDOWN:
       {
           if( m_pFocus != NULL ) {
               TEventUI event = { 0 };
               event.Type = UIEVENT_SYSKEY;
               event.chKey = (TCHAR)wParam;
               event.ptMouse = m_ptLastMousePos;
               event.wKeyState = MapKeyState();
               event.dwTimestamp = ::GetTickCount();
               m_pFocus->Event(event);
           }
       }
       break;
    }
    return false;
}

bool CPaintManagerUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes)
{
//#ifdef _DEBUG
//    switch( uMsg ) {
//    case WM_NCPAINT:
//    case WM_NCHITTEST:
//    case WM_SETCURSOR:
//       break;
//    default:
//       TRACE(_T("MSG: %-20s (%08ld)"), TRACEMSG(uMsg), ::GetTickCount());
//    }
//#endif
    // Not ready yet?
    if( m_hWndPaint == NULL ) return false;
    // Cycle through listeners
    for( int i = 0; i < m_aMessageFilters.GetSize(); i++ ) 
    {
        bool bHandled = false;
        LRESULT lResult = static_cast<IMessageFilterUI*>(m_aMessageFilters[i])->MessageHandler(uMsg, wParam, lParam, bHandled);
        if( bHandled ) {
            lRes = lResult;
            return true;
        }
    }
    // Custom handling of events
    switch( uMsg ) {
   case WM_APP + 1:
       {
           // Delayed control-tree cleanup. See AttachDialog() for details.
           for( int i = 0; i < m_aDelayedCleanup.GetSize(); i++ ) delete static_cast<CControlUI*>(m_aDelayedCleanup[i]);
           m_aDelayedCleanup.Empty();
       }
       break;
   case WM_CLOSE:
       {
           // Make sure all matching "closing" events are sent
           TEventUI event = { 0 };
           event.ptMouse = m_ptLastMousePos;
           event.dwTimestamp = ::GetTickCount();
           if( m_pEventHover != NULL ) {
               event.Type = UIEVENT_MOUSELEAVE;
               event.pSender = m_pEventHover;
               m_pEventHover->Event(event);
           }
           if( m_pEventClick != NULL ) {
               event.Type = UIEVENT_BUTTONUP;
               event.pSender = m_pEventClick;
               m_pEventClick->Event(event);
           }

           SetFocus(NULL);

           // Hmmph, the usual Windows tricks to avoid
           // focus loss...
           HWND hwndParent = GetWindowOwner(m_hWndPaint);
           if( hwndParent != NULL ) ::SetFocus(hwndParent);
       }
       break;
   case WM_ERASEBKGND:
       {
           // We'll do the painting here...
           lRes = 1;
       }
       return true;
   case WM_PAINT:
       {
           // Should we paint?
           RECT rcPaint = { 0 };
           if( !::GetUpdateRect(m_hWndPaint, &rcPaint, FALSE) ) return true;
           // Do we need to resize anything?
           // This is the time where we layout the controls on the form.
           // We delay this even from the WM_SIZE messages since resizing can be
           // a very expensize operation.
           if( m_bUpdateNeeded ) {
               m_bUpdateNeeded = false;
               RECT rcClient = { 0 };
               ::GetClientRect(m_hWndPaint, &rcClient);
               if( !::IsRectEmpty(&rcClient) ) {
                   if( m_pRoot->IsUpdateNeeded() ) {
                       m_pRoot->SetPos(rcClient);
                       if( m_hDcOffscreen != NULL ) ::DeleteDC(m_hDcOffscreen);
                       if( m_hDcBackground != NULL ) ::DeleteDC(m_hDcBackground);
                       if( m_hbmpOffscreen != NULL ) ::DeleteObject(m_hbmpOffscreen);
                       if( m_hbmpBackground != NULL ) ::DeleteObject(m_hbmpBackground);
                       m_hDcOffscreen = NULL;
                       m_hDcBackground = NULL;
                       m_hbmpOffscreen = NULL;
                       m_hbmpBackground = NULL;
                   }
                   else {
                       CControlUI* pControl = NULL;
                       while( pControl = m_pRoot->FindControl(__FindControlFromUpdate, NULL, UIFIND_VISIBLE | UIFIND_ME_FIRST) ) {
                           pControl->SetPos( pControl->GetPos() );
                       }
                   }
                   // We'll want to notify the window when it is first initialized
                   // with the correct layout. The window form would take the time
                   // to submit swipes/animations.
                   if( m_bFirstLayout ) {
                       m_bFirstLayout = false;
                       SendNotify(m_pRoot, _T("windowinit"));
                   }
               }
           }
           // Set focus to first control?
           if( m_bFocusNeeded ) {
               SetNextTabControl();
           }
           //
           // Render screen
           //
           // Prepare offscreen bitmap?
           if( m_bOffscreenPaint && m_hbmpOffscreen == NULL )
           {
               RECT rcClient = { 0 };
               ::GetClientRect(m_hWndPaint, &rcClient);
               m_hDcOffscreen = ::CreateCompatibleDC(m_hDcPaint);
               m_hbmpOffscreen = ::CreateCompatibleBitmap(m_hDcPaint, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top); 
               ASSERT(m_hDcOffscreen);
               ASSERT(m_hbmpOffscreen);
           }
           // Begin Windows paint
           PAINTSTRUCT ps = { 0 };
           ::BeginPaint(m_hWndPaint, &ps);
           if( m_bOffscreenPaint )
           {
               HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(m_hDcOffscreen, m_hbmpOffscreen);
               int iSaveDC = ::SaveDC(m_hDcOffscreen);
               if( m_bAlphaBackground ) {
                   if( m_hbmpBackground == NULL ) {
                       RECT rcClient = { 0 };
                       ::GetClientRect(m_hWndPaint, &rcClient);
                       m_hDcBackground = ::CreateCompatibleDC(m_hDcPaint);;
                       m_hbmpBackground = ::CreateCompatibleBitmap(m_hDcPaint, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top); 
                       ASSERT(m_hDcBackground);
                       ASSERT(m_hbmpBackground);
                       ::SelectObject(m_hDcBackground, m_hbmpBackground);
                       ::BitBlt(m_hDcBackground, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
                           ps.rcPaint.bottom - ps.rcPaint.top, ps.hdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
                   }
                   else
                       ::SelectObject(m_hDcBackground, m_hbmpBackground);
                   ::BitBlt(m_hDcOffscreen, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
                       ps.rcPaint.bottom - ps.rcPaint.top, m_hDcBackground, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
               }
               m_pRoot->DoPaint(m_hDcOffscreen, ps.rcPaint);
               for( int i = 0; i < m_aPostPaintControls.GetSize(); i++ ) {
                   CControlUI* pPostPaintControl = static_cast<CControlUI*>(m_aPostPaintControls[i]);
                   pPostPaintControl->DoPostPaint(m_hDcOffscreen, ps.rcPaint);
               }
               ::RestoreDC(m_hDcOffscreen, iSaveDC);
               ::BitBlt(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
                   ps.rcPaint.bottom - ps.rcPaint.top, m_hDcOffscreen, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
               ::SelectObject(m_hDcOffscreen, hOldBitmap);

               if( m_bShowUpdateRect ) {
                   HPEN hOldPen = (HPEN)::SelectObject(ps.hdc, m_hUpdateRectPen);
                   ::SelectObject(ps.hdc, ::GetStockObject(HOLLOW_BRUSH));
                   ::Rectangle(ps.hdc, rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
                   ::SelectObject(ps.hdc, hOldPen);
               }
           }
           else
           {
               // A standard paint job
               int iSaveDC = ::SaveDC(ps.hdc);
               m_pRoot->DoPaint(ps.hdc, ps.rcPaint);
               ::RestoreDC(ps.hdc, iSaveDC);
           }
           // All Done!
           ::EndPaint(m_hWndPaint, &ps);
       }
       // If any of the painting requested a resize again, we'll need
       // to invalidate the entire window once more.
       if( m_bUpdateNeeded ) {
           ::InvalidateRect(m_hWndPaint, NULL, FALSE);
       }
       return true;
   case WM_PRINTCLIENT:
       {
           RECT rcClient;
           ::GetClientRect(m_hWndPaint, &rcClient);
           HDC hDC = (HDC) wParam;
           int save = ::SaveDC(hDC);
           m_pRoot->DoPaint(hDC, rcClient);
           // Check for traversing children. The crux is that WM_PRINT will assume
           // that the DC is positioned at frame coordinates and will paint the child
           // control at the wrong position. We'll simulate the entire thing instead.
           if( (lParam & PRF_CHILDREN) != 0 ) {
               HWND hWndChild = ::GetWindow(m_hWndPaint, GW_CHILD);
               while( hWndChild != NULL ) {
                   RECT rcPos = { 0 };
                   ::GetWindowRect(hWndChild, &rcPos);
                   ::MapWindowPoints(HWND_DESKTOP, m_hWndPaint, reinterpret_cast<LPPOINT>(&rcPos), 2);
                   ::SetWindowOrgEx(hDC, -rcPos.left, -rcPos.top, NULL);
                   // NOTE: We use WM_PRINT here rather than the expected WM_PRINTCLIENT
                   //       since the latter will not print the nonclient correctly for
                   //       EDIT controls.
                   ::SendMessage(hWndChild, WM_PRINT, wParam, lParam | PRF_NONCLIENT);
                   hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
               }
           }
           ::RestoreDC(hDC, save);
       }
       break;
   case WM_GETMINMAXINFO:
       {
           LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
           lpMMI->ptMinTrackSize.x = m_szMinWindow.cx;
           lpMMI->ptMinTrackSize.y = m_szMinWindow.cy;
       }
       break;
   case WM_SIZE:
       {
           if( m_pFocus != NULL ) {
               TEventUI event = { 0 };
               event.Type = UIEVENT_WINDOWSIZE;
               event.dwTimestamp = ::GetTickCount();
               m_pFocus->Event(event);
           }

           m_pRoot->NeedUpdate();
       }
       return true;
   case WM_TIMER:
       {
           for( int i = 0; i < m_aTimers.GetSize(); i++ ) {
               const TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
               if( pTimer->hWnd == m_hWndPaint && pTimer->uWinTimer == LOWORD(wParam) && pTimer->bKilled == false) {
                   TEventUI event = { 0 };
                   event.Type = UIEVENT_TIMER;
                   event.wParam = pTimer->nLocalID;
                   event.dwTimestamp = ::GetTickCount();
                   pTimer->pSender->Event(event);
                   break;
               }
           }
       }
       break;
   case WM_MOUSEHOVER:
       {
           m_bMouseTracking = false;
           POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
           CControlUI* pHover = FindControl(pt);
           if( pHover == NULL ) break;
           // Generate mouse hover event
           if( m_pEventHover != NULL ) {
               TEventUI event = { 0 };
               event.ptMouse = pt;
               event.Type = UIEVENT_MOUSEHOVER;
               event.pSender = pHover;
               event.dwTimestamp = ::GetTickCount();
               m_pEventHover->Event(event);
           }
           // Create tooltip information
           CStdString sToolTip = pHover->GetToolTip();
           if( sToolTip.IsEmpty() ) return true;
           ::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
           m_ToolTip.cbSize = sizeof(TOOLINFO);
           m_ToolTip.uFlags = TTF_IDISHWND;
           m_ToolTip.hwnd = m_hWndPaint;
           m_ToolTip.uId = (UINT_PTR) m_hWndPaint;
           m_ToolTip.hinst = m_hInstance;
           m_ToolTip.lpszText = const_cast<LPTSTR>( (LPCTSTR) sToolTip );
           m_ToolTip.rect = pHover->GetPos();
           if( m_hwndTooltip == NULL ) {
               m_hwndTooltip = ::CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hWndPaint, NULL, m_hInstance, NULL);
               ::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM) &m_ToolTip);
           }
           ::SendMessage(m_hwndTooltip, TTM_SETTOOLINFO, 0, (LPARAM) &m_ToolTip);
           ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM) &m_ToolTip);
       }
       return true;
   case WM_MOUSELEAVE:
       {
           if( m_hwndTooltip != NULL ) ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
           if( m_bMouseTracking ) ::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM) -1);
           m_bMouseTracking = false;
       }
       break;
   case WM_MOUSEMOVE:
       {
           // Start tracking this entire window again...
           if( !m_bMouseTracking ) {
               TRACKMOUSEEVENT tme = { 0 };
               tme.cbSize = sizeof(TRACKMOUSEEVENT);
               tme.dwFlags = TME_HOVER | TME_LEAVE;
               tme.hwndTrack = m_hWndPaint;
               tme.dwHoverTime = m_hwndTooltip == NULL ? 1000UL : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
               _TrackMouseEvent(&tme);
               m_bMouseTracking = true;
           }
           // Generate the appropriate mouse messages
           POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
           m_ptLastMousePos = pt;
           CControlUI* pNewHover = FindControl(pt);
           if( pNewHover != NULL && pNewHover->GetManager() != this ) break;
           TEventUI event = { 0 };
           event.ptMouse = pt;
           event.dwTimestamp = ::GetTickCount();
           if( pNewHover != m_pEventHover && m_pEventHover != NULL ) {
               event.Type = UIEVENT_MOUSELEAVE;
               event.pSender = pNewHover;
               m_pEventHover->Event(event);
               m_pEventHover = NULL;
               if( m_hwndTooltip != NULL ) ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
           }
           if( pNewHover != m_pEventHover && pNewHover != NULL ) {
               event.Type = UIEVENT_MOUSEENTER;
               event.pSender = m_pEventHover;
               pNewHover->Event(event);
               m_pEventHover = pNewHover;
           }
           if( m_pEventClick != NULL ) {
               event.Type = UIEVENT_MOUSEMOVE;
               event.pSender = NULL;
               m_pEventClick->Event(event);
           }
           else if( pNewHover != NULL ) {
               event.Type = UIEVENT_MOUSEMOVE;
               event.pSender = NULL;
               pNewHover->Event(event);
           }
       }
       break;
   case WM_LBUTTONDOWN:
       {
           // We alway set focus back to our app (this helps
           // when Win32 child windows are placed on the dialog
           // and we need to remove them on focus change).
           ::SetFocus(m_hWndPaint);
           POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
           m_ptLastMousePos = pt;
           CControlUI* pControl = FindControl(pt);
           if( pControl == NULL ) break;
           if( pControl->GetManager() != this ) break;
           m_pEventClick = pControl;
           pControl->SetFocus();
           SetCapture();
           m_bMouseCapture = true;
           TEventUI event = { 0 };
           event.Type = UIEVENT_BUTTONDOWN;
           event.wParam = wParam;
           event.lParam = lParam;
           event.ptMouse = pt;
           event.wKeyState = (WORD)wParam;
           event.dwTimestamp = ::GetTickCount();
           pControl->Event(event);
           
       }
       break;
   case WM_LBUTTONUP:
       {
           POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
           m_ptLastMousePos = pt;
           if( m_pEventClick == NULL ) break;
           ReleaseCapture();
           m_bMouseCapture = false;
           TEventUI event = { 0 };
           event.Type = UIEVENT_BUTTONUP;
           event.wParam = wParam;
           event.lParam = lParam;
           event.ptMouse = pt;
           event.wKeyState = (WORD)wParam;
           event.dwTimestamp = ::GetTickCount();
           m_pEventClick->Event(event);
           m_pEventClick = NULL;
       }
       break;
   case WM_LBUTTONDBLCLK:
       {
           ::SetFocus(m_hWndPaint);
           POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
           m_ptLastMousePos = pt;
           CControlUI* pControl = FindControl(pt);
           if( pControl == NULL ) break;
           if( pControl->GetManager() != this ) break;
           ::SetCapture(m_hWndPaint);
           m_bMouseCapture = true;
           TEventUI event = { 0 };
           event.Type = UIEVENT_DBLCLICK;
           event.ptMouse = pt;
           event.wKeyState = (WORD)wParam;
           event.dwTimestamp = ::GetTickCount();
           pControl->Event(event);
           m_pEventClick = pControl;
       }
       break;
   case WM_MOUSEWHEEL:
       {
           if( m_pFocus == NULL ) break;
           int zDelta = (int) (short) HIWORD(wParam);
           TEventUI event = { 0 };
           event.Type = UIEVENT_SCROLLWHEEL;
           event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
           event.lParam = lParam;
           event.wKeyState = MapKeyState();
           event.dwTimestamp = ::GetTickCount();
           m_pFocus->Event(event);

           // Let's make sure that the scroll item below the cursor is the same as before...
           ::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM) MAKELPARAM(m_ptLastMousePos.x, m_ptLastMousePos.y));
       }
       break;
   case WM_CONTEXTMENU:
       {
           ::SetFocus(m_hWndPaint);
           POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
           m_ptLastMousePos = pt;
           CControlUI* pControl = FindControl(pt);
           if( pControl == NULL ) break;
           if( pControl->GetManager() != this ) break;
           m_pEventClick = pControl;
           pControl->SetFocus();
           TEventUI event = { 0 };
           event.Type = UIEVENT_CONTEXTMENU;
           event.ptMouse = pt;
           event.wKeyState = (WORD)wParam;
           event.dwTimestamp = ::GetTickCount();
           pControl->Event(event);
       }
       break;
   case WM_CHAR:
       {
           if( m_pFocus == NULL ) break;
           TEventUI event = { 0 };
           event.Type = UIEVENT_CHAR;
           event.chKey = (TCHAR)wParam;
           event.ptMouse = m_ptLastMousePos;
           event.wKeyState = MapKeyState();
           event.dwTimestamp = ::GetTickCount();
           m_pFocus->Event(event);
       }
       break;
   case WM_KEYDOWN:
       {
           if( m_pFocus == NULL ) break;
           TEventUI event = { 0 };
           event.Type = UIEVENT_KEYDOWN;
           event.chKey = (TCHAR)wParam;
           event.ptMouse = m_ptLastMousePos;
           event.wKeyState = MapKeyState();
           event.dwTimestamp = ::GetTickCount();
           m_pFocus->Event(event);
           m_pEventKey = m_pFocus;
       }
       break;
   case WM_KEYUP:
       {
           if( m_pEventKey == NULL ) break;
           TEventUI event = { 0 };
           event.Type = UIEVENT_KEYUP;
           event.chKey = (TCHAR)wParam;
           event.ptMouse = m_ptLastMousePos;
           event.wKeyState = MapKeyState();
           event.dwTimestamp = ::GetTickCount();
           m_pEventKey->Event(event);
           m_pEventKey = NULL;
       }
       break;
   case WM_SETCURSOR:
       {
           if( LOWORD(lParam) != HTCLIENT ) break;
           if( m_bMouseCapture ) return true;

           POINT pt = { 0 };
           ::GetCursorPos(&pt);
           ::ScreenToClient(m_hWndPaint, &pt);
           CControlUI* pControl = FindControl(pt);
           if( pControl == NULL ) break;
           if( (pControl->GetControlFlags() & UIFLAG_SETCURSOR) == 0 ) break;
           TEventUI event = { 0 };
           event.Type = UIEVENT_SETCURSOR;
           event.wParam = wParam;
           event.lParam = lParam;
           event.ptMouse = pt;
           event.wKeyState = MapKeyState();
           event.dwTimestamp = ::GetTickCount();
           pControl->Event(event);
       }
       return true;
   case WM_NOTIFY:
       {
           LPNMHDR lpNMHDR = (LPNMHDR) lParam;
           if( lpNMHDR != NULL ) lRes = ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
           return true;
       }
       break;
   case WM_COMMAND:
       {
           if( lParam == 0 ) break;
           HWND hWndChild = (HWND) lParam;
           lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
           return true;
       }
       break;
   case  WM_CTLCOLOREDIT:
       {
           if( lParam == 0 ) break;
           HWND hWndChild = (HWND) lParam;
           lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
           return true;
       }
       break;
   default:
       break;
    }

    return false;
}

void CPaintManagerUI::NeedUpdate()
{
    m_bUpdateNeeded = true;
}

void CPaintManagerUI::Invalidate(RECT& rcItem)
{
    ::InvalidateRect(m_hWndPaint, &rcItem, FALSE);
}

bool CPaintManagerUI::AttachDialog(CControlUI* pControl)
{
    ASSERT(::IsWindow(m_hWndPaint));
    // Reset any previous attachment
    SetFocus(NULL);
    m_pEventKey = NULL;
    m_pEventHover = NULL;
    m_pEventClick = NULL;
    // Remove the existing control-tree. We might have gotten inside this function as
    // a result of an event fired or similar, so we cannot just delete the objects and
    // pull the internal memory of the calling code. We'll delay the cleanup.
    if( m_pRoot != NULL ) {
        m_aPostPaintControls.Empty();
        m_aDelayedCleanup.Add(m_pRoot);
        ::PostMessage(m_hWndPaint, WM_APP + 1, 0, 0L);
    }
    // Set the dialog root element
    m_pRoot = pControl;
    // Go ahead...
    m_bUpdateNeeded = true;
    m_bFirstLayout = true;
    m_bFocusNeeded = true;
    // Initiate all control
    return InitControls(pControl);
}

bool CPaintManagerUI::InitControls(CControlUI* pControl, CControlUI* pParent /*= NULL*/)
{
    ASSERT(pControl);
    if( pControl == NULL ) return false;
    pControl->SetManager(this, pParent != NULL ? pParent : pControl->GetParent(), true);
    pControl->FindControl(__FindControlFromNameHash, this, UIFIND_ALL);
    return true;
}

void CPaintManagerUI::ReapObjects(CControlUI* pControl)
{
    if( pControl == m_pEventKey ) m_pEventKey = NULL;
    if( pControl == m_pEventHover ) m_pEventHover = NULL;
    if( pControl == m_pEventClick ) m_pEventClick = NULL;
    if( pControl == m_pFocus ) m_pFocus = NULL;
    
    const CStdString& sName = pControl->GetName();
    if( !sName.IsEmpty() ) m_mNameHash.Remove(sName);
}

void CPaintManagerUI::MessageLoop()
{
    MSG msg = { 0 };
    while( ::GetMessage(&msg, NULL, 0, 0) ) {
        if( !CPaintManagerUI::TranslateMessage(&msg) ) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
}

bool CPaintManagerUI::TranslateMessage(const LPMSG pMsg)
{
    // Pretranslate Message takes care of system-wide messages, such as
    // tabbing and shortcut key-combos. We'll look for all messages for
    // each window and any child control attached.
    HWND hwndParent = ::GetParent(pMsg->hwnd);
    UINT uStyle = GetWindowStyle(pMsg->hwnd);
    LRESULT lRes = 0;
    for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) {
        CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
        if( pMsg->hwnd == pT->GetPaintWindow()
            || (hwndParent == pT->GetPaintWindow() && ((uStyle & WS_CHILD) != 0)) )
        {
            if( pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes) ) return true;
        }
    }
    return false;
}

CControlUI* CPaintManagerUI::GetFocus() const
{
    return m_pFocus;
}

void CPaintManagerUI::SetFocus(CControlUI* pControl)
{
    // Paint manager window has focus?
    if( ::GetFocus() != m_hWndPaint ) ::SetFocus(m_hWndPaint);
    // Already has focus?
    if( pControl == NULL || pControl == m_pFocus ) return;
    // Remove focus from old control
    if( m_pFocus != NULL ) 
    {
        TEventUI event = { 0 };
        event.Type = UIEVENT_KILLFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, _T("killfocus"));
        m_pFocus = NULL;
    }
    // Set focus to new control
    if( pControl != NULL 
        && pControl->GetManager() == this 
        && pControl->IsVisible() 
        && pControl->IsEnabled() ) 
    {
        m_pFocus = pControl;
        TEventUI event = { 0 };
        event.Type = UIEVENT_SETFOCUS;
        event.pSender = pControl;
        event.dwTimestamp = ::GetTickCount();
        m_pFocus->Event(event);
        SendNotify(m_pFocus, _T("setfocus"));
    }
}

bool CPaintManagerUI::SetTimer(CControlUI* pControl, UINT nTimerID, UINT uElapse)
{
    ASSERT(pControl!=NULL);
    ASSERT(uElapse>0);
    for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
        TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->pSender == pControl
            && pTimer->hWnd == m_hWndPaint
            && pTimer->nLocalID == nTimerID )
        {
            if( pTimer->bKilled == true ) {
                if( ::SetTimer(m_hWndPaint, pTimer->uWinTimer, uElapse, NULL) ) {
                    pTimer->bKilled = false;
                    return true;
                }

                return false;
            }

            return false;
        }
    }

    m_uTimerID = (++m_uTimerID) % 0xFF;
    if( !::SetTimer(m_hWndPaint, m_uTimerID, uElapse, NULL) ) return FALSE;
    TIMERINFO* pTimer = new TIMERINFO;
    if( pTimer == NULL ) return FALSE;
    pTimer->hWnd = m_hWndPaint;
    pTimer->pSender = pControl;
    pTimer->nLocalID = nTimerID;
    pTimer->uWinTimer = m_uTimerID;
    pTimer->bKilled = false;
    return m_aTimers.Add(pTimer);
}

bool CPaintManagerUI::KillTimer(CControlUI* pControl, UINT nTimerID)
{
    ASSERT(pControl!=NULL);
    for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
        TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
        if( pTimer->pSender == pControl
            && pTimer->hWnd == m_hWndPaint
            && pTimer->nLocalID == nTimerID )
        {
            if( pTimer->bKilled == false ) {
                ::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
                pTimer->bKilled = true;
                return true;
            }
        }
    }
    return false;
}

void CPaintManagerUI::SetCapture()
{
    ::SetCapture(m_hWndPaint);
    m_bMouseCapture = true;
}

void CPaintManagerUI::ReleaseCapture()
{
    ::ReleaseCapture();
    m_bMouseCapture = false;
}

bool CPaintManagerUI::IsCaptured()
{
    return m_bMouseCapture;
}

bool CPaintManagerUI::SetNextTabControl(bool bForward)
{
    // If we're in the process of restructuring the layout we can delay the
    // focus calulation until the next repaint.
    if( m_bUpdateNeeded && bForward ) {
        m_bFocusNeeded = true;
        ::InvalidateRect(m_hWndPaint, NULL, FALSE);
        return true;
    }
    // Find next/previous tabbable control
    FINDTABINFO info1 = { 0 };
    info1.pFocus = m_pFocus;
    info1.bForward = bForward;
    CControlUI* pControl = m_pRoot->FindControl(__FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
    if( pControl == NULL ) {  
        if( bForward ) {
            // Wrap around
            FINDTABINFO info2 = { 0 };
            info2.pFocus = bForward ? NULL : info1.pLast;
            info2.bForward = bForward;
            pControl = m_pRoot->FindControl(__FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
        }
        else {
            pControl = info1.pLast;
        }
    }
    if( pControl != NULL ) SetFocus(pControl);
    m_bFocusNeeded = false;
    return true;
}

bool CPaintManagerUI::AddNotifier(INotifyUI* pNotifier)
{
    ASSERT(m_aNotifiers.Find(pNotifier)<0);
    return m_aNotifiers.Add(pNotifier);
}

bool CPaintManagerUI::RemoveNotifier(INotifyUI* pNotifier)
{
    for( int i = 0; i < m_aNotifiers.GetSize(); i++ ) {
        if( static_cast<INotifyUI*>(m_aNotifiers[i]) == pNotifier ) {
            return m_aNotifiers.Remove(i);
        }
    }
    return false;
}

bool CPaintManagerUI::AddMessageFilter(IMessageFilterUI* pFilter)
{
    ASSERT(m_aMessageFilters.Find(pFilter)<0);
    return m_aMessageFilters.Add(pFilter);
}

bool CPaintManagerUI::RemoveMessageFilter(IMessageFilterUI* pFilter)
{
    for( int i = 0; i < m_aMessageFilters.GetSize(); i++ ) {
        if( static_cast<IMessageFilterUI*>(m_aMessageFilters[i]) == pFilter ) {
            return m_aMessageFilters.Remove(i);
        }
    }
    return false;
}

int CPaintManagerUI::GetPostPaintCount() const
{
    return m_aPostPaintControls.GetSize();
}

bool CPaintManagerUI::AddPostPaint(CControlUI* pControl)
{
    ASSERT(m_aPostPaintControls.Find(pControl) < 0);
    return m_aPostPaintControls.Add(pControl);
}

bool CPaintManagerUI::RemovePostPaint(CControlUI* pControl)
{
    for( int i = 0; i < m_aPostPaintControls.GetSize(); i++ ) {
        if( static_cast<CControlUI*>(m_aPostPaintControls[i]) == pControl ) {
            return m_aPostPaintControls.Remove(i);
        }
    }
    return false;
}

bool CPaintManagerUI::SetPostPaintIndex(CControlUI* pControl, int iIndex)
{
    RemovePostPaint(pControl);
    return m_aPostPaintControls.InsertAt(iIndex, pControl);
}

void CPaintManagerUI::SendNotify(CControlUI* pControl, LPCTSTR pstrMessage, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    TNotifyUI Msg;
    Msg.pSender = pControl;
    Msg.sType = pstrMessage;
    Msg.wParam = wParam;
    Msg.lParam = lParam;
    SendNotify(Msg);
}

void CPaintManagerUI::SendNotify(TNotifyUI& Msg)
{
    Msg.ptMouse = m_ptLastMousePos;
    Msg.dwTimestamp = ::GetTickCount();
    // Send to all listeners
    for( int i = 0; i < m_aNotifiers.GetSize(); i++ ) {
        static_cast<INotifyUI*>(m_aNotifiers[i])->Notify(Msg);
    }
}

bool CPaintManagerUI::UseParentResource(CPaintManagerUI* pm)
{
    if( pm == NULL ) {
        m_pParentResourcePM = NULL;
        return true;
    }
    if( pm == this ) return false;
    
    CPaintManagerUI* pParentPM = pm->GetParentResource();
    while( pParentPM ) {
        if( pParentPM == this ) return false;
        pParentPM = pParentPM->GetParentResource();
    }
    m_pParentResourcePM = pm;
    return true;
}

CPaintManagerUI* CPaintManagerUI::GetParentResource() const
{
    return m_pParentResourcePM;
}

DWORD CPaintManagerUI::GetDefaultDisabledColor()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultDisabledColor();
    return m_dwDefalutDisabledColor;
}

void CPaintManagerUI::SetDefaultDisabledColor(DWORD dwColor)
{
    m_dwDefalutDisabledColor = dwColor;
}

HFONT CPaintManagerUI::GetDefaultFont()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultFont();
    return m_hDefalutFont;
}

DWORD CPaintManagerUI::GetDefaultFontColor()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultFontColor();
    return m_dwDefalutFontColor;
}

const TEXTMETRIC& CPaintManagerUI::GetDefaultFontInfo()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultFontInfo();
    if( m_hDefalutFontTextMetric.tmHeight == 0 ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_hDefalutFont);
        ::GetTextMetrics(m_hDcPaint, &m_hDefalutFontTextMetric);
        ::SelectObject(m_hDcPaint, hOldFont);
    }

    return m_hDefalutFontTextMetric;
}

void CPaintManagerUI::SetDefaultFont(HFONT hFont, DWORD dwColor)
{
    if( hFont == NULL ) return;

    if( m_hDcPaint ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &m_hDefalutFontTextMetric);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    else {
        ::ZeroMemory(&m_hDefalutFontTextMetric, sizeof(m_hDefalutFontTextMetric));
    }

    if( m_hDefalutFont && !FindFont(m_hDefalutFont) ) ::DeleteObject(m_hDefalutFont);
    m_hDefalutFont = hFont;
    m_dwDefalutFontColor = dwColor;
}

HFONT CPaintManagerUI::GetDefaultBoldFont()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultBoldFont();
    return m_hDefalutBoldFont;
}

DWORD CPaintManagerUI::GetDefaultBoldFontColor()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultBoldFontColor();
    return m_dwDefalutBoldFontColor;
}

const TEXTMETRIC& CPaintManagerUI::GetDefaultBoldFontInfo()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultBoldFontInfo();
    if( m_hDefaluttBoldFontTextMetric.tmHeight == 0 ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_hDefalutBoldFont);
        ::GetTextMetrics(m_hDcPaint, &m_hDefaluttBoldFontTextMetric);
        ::SelectObject(m_hDcPaint, hOldFont);
    }

    return m_hDefaluttBoldFontTextMetric;
}

void CPaintManagerUI::SetDefaultBoldFont(HFONT hFont, DWORD dwColor)
{
    if( hFont == NULL ) return;

    if( m_hDcPaint ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &m_hDefaluttBoldFontTextMetric);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    else {
        ::ZeroMemory(&m_hDefaluttBoldFontTextMetric, sizeof(m_hDefaluttBoldFontTextMetric));
    }

    if( m_hDefalutBoldFont && !FindFont(m_hDefalutBoldFont) ) ::DeleteObject(m_hDefalutBoldFont);
    m_hDefalutBoldFont = hFont;
    m_dwDefalutBoldFontColor = dwColor;
}

HFONT CPaintManagerUI::GetDefaultLinkFont()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultLinkFont();
    return m_hDefalutLinkFont;
}

DWORD CPaintManagerUI::GetDefaultLinkFontColor()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultLinkFontColor();
    return m_dwDefalutLinkFontColor;
}

DWORD CPaintManagerUI::GetDefaultLinkFontHoverColor()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultLinkFontHoverColor();
    return m_dwDefalutLinkFontHoverColor;
}

const TEXTMETRIC& CPaintManagerUI::GetDefaultLinkFontInfo()
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultLinkFontInfo();
    if( m_hDefalutLinkFontTextMetric.tmHeight == 0 ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_hDefalutLinkFont);
        ::GetTextMetrics(m_hDcPaint, &m_hDefalutLinkFontTextMetric);
        ::SelectObject(m_hDcPaint, hOldFont);
    }

    return m_hDefalutLinkFontTextMetric;
}

void CPaintManagerUI::SetDefaultLinkFont(HFONT hFont, DWORD dwColor, DWORD dwHoverColor)
{
    if( hFont == NULL ) return;

    if( m_hDcPaint ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &m_hDefalutLinkFontTextMetric);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    else {
        ::ZeroMemory(&m_hDefalutLinkFontTextMetric, sizeof(m_hDefalutLinkFontTextMetric));
    }

    if( m_hDefalutLinkFont && !FindFont(m_hDefalutLinkFont) ) ::DeleteObject(m_hDefalutLinkFont);
    m_hDefalutLinkFont = hFont;
    m_dwDefalutLinkFontColor = dwColor;
    m_dwDefalutLinkFontHoverColor = dwHoverColor;
}

bool CPaintManagerUI::AddFont(HFONT hFont)
{
    if( hFont == NULL ) return false;

    TFontInfo* pFontInfo = new TFontInfo;
    if( !pFontInfo ) return false;
    ::ZeroMemory(pFontInfo, sizeof(TFontInfo));
    pFontInfo->hFont = hFont;
    if( m_hDcPaint ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }

    if( !m_aCustomFonts.Add(pFontInfo) ) {
        delete pFontInfo;
        return false;
    }

    return true;
}

HFONT CPaintManagerUI::AddFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    _tcscpy(lf.lfFaceName, pStrFontName);
    lf.lfHeight = -nSize;
    if( bBold ) lf.lfWeight += FW_BOLD;
    if( bUnderline ) lf.lfUnderline = TRUE;
    if( bItalic ) lf.lfItalic = TRUE;
    HFONT hFont = ::CreateFontIndirect(&lf);
    if( hFont == NULL ) return NULL;
    if( !AddFont(hFont) ) {
        ::DeleteObject(hFont);
        return NULL;
    }

    return hFont;
}

bool CPaintManagerUI::AddFontAt(int index, HFONT hFont)
{
    if( hFont == NULL ) return false;

    TFontInfo* pFontInfo = new TFontInfo;
    if( !pFontInfo ) return false;
    ::ZeroMemory(pFontInfo, sizeof(TFontInfo));
    pFontInfo->hFont = hFont;
    if( m_hDcPaint ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }

    if( !m_aCustomFonts.InsertAt(index, pFontInfo) ) {
        delete pFontInfo;
        return false;
    }

    return true;
}

HFONT CPaintManagerUI::AddFontAt(int index, LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline)
{
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    _tcscpy(lf.lfFaceName, pStrFontName);
    lf.lfHeight = -nSize;
    if( bBold ) lf.lfWeight += FW_BOLD;
    if( bUnderline ) lf.lfUnderline = TRUE;
    HFONT hFont = ::CreateFontIndirect(&lf);
    if( hFont == NULL ) return NULL;
    if( !AddFontAt(index, hFont) ) {
        ::DeleteObject(hFont);
        return NULL;
    }

    return hFont;
}

HFONT CPaintManagerUI::GetFont(int index)
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetFont(index);
    if( index < 0 || index >= m_aCustomFonts.GetSize() ) return GetDefaultFont();
    TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[index]);
    return pFontInfo->hFont;
}

bool CPaintManagerUI::FindFont(HFONT hFont)
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->FindFont(hFont);

    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->hFont == hFont ) return true;
    }

    return false;
}

bool CPaintManagerUI::RemoveFont(HFONT hFont)
{
    TFontInfo* pFontInfo = NULL;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        if( pFontInfo->hFont == hFont ) {
            ::DeleteObject(pFontInfo->hFont);
            delete pFontInfo;
            return m_aCustomFonts.Remove(it);
        }
    }

    return false;
}

bool CPaintManagerUI::RemoveFontAt(int index)
{
    if( index < 0 || index >= m_aCustomFonts.GetSize() ) return false;
    TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[index]);
    ::DeleteObject(pFontInfo->hFont);
    delete pFontInfo;
    return m_aCustomFonts.Remove(index);
}

void CPaintManagerUI::RemoveAllFonts()
{
    TFontInfo* pFontInfo;
    for( int it = 0; it < m_aCustomFonts.GetSize(); it++ ) {
        pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[it]);
        ::DeleteObject(pFontInfo->hFont);
        delete pFontInfo;
    }
    m_aCustomFonts.Empty();
}

const TEXTMETRIC& CPaintManagerUI::GetFontInfo(int index)
{
    if( m_pParentResourcePM ) return m_pParentResourcePM->GetFontInfo(index);
    if( index < 0 || index >= m_aCustomFonts.GetSize() ) return GetDefaultFontInfo();
    TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_aCustomFonts[index]);
    if( pFontInfo->tm.tmHeight == 0 ) {
        HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, pFontInfo->hFont);
        ::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
        ::SelectObject(m_hDcPaint, hOldFont);
    }
    return pFontInfo->tm;
}

TImageInfo* CPaintManagerUI::GetImage(LPCTSTR bitmap)
{
    TImageInfo* data = static_cast<TImageInfo*>(m_mImageHash.Find(bitmap));
    if( !data && m_pParentResourcePM ) return m_pParentResourcePM->GetImage(bitmap);
    else return data;
}

TImageInfo* CPaintManagerUI::GetImageEx(LPCTSTR bitmap, LPCTSTR type, DWORD mask)
{
    TImageInfo* data = static_cast<TImageInfo*>(m_mImageHash.Find(bitmap));
    if( !data ) {
        if( AddImage(bitmap, type, mask) ) {
            data = static_cast<TImageInfo*>(m_mImageHash.Find(bitmap));
        }
    }

    return data;
}

TImageInfo* CPaintManagerUI::AddImage(LPCTSTR bitmap, LPCTSTR type, DWORD mask)
{
    TImageInfo* data = NULL;
    if( type != NULL ) {
        if( isdigit(*bitmap) ) {
            LPTSTR pstr = NULL;
            int iIndex = _tcstol(bitmap, &pstr, 10);
            data = CRenderEngine::LoadImage(iIndex, type, mask);
        }
    }
    else {
        data = CRenderEngine::LoadImage(bitmap, type, mask);
    }

    if( !data ) return NULL;
    if( !m_mImageHash.Insert(bitmap, data) ) {
        ::DeleteObject(data->hBitmap);
        delete data;
    }

    return data;
}

bool CPaintManagerUI::RemoveImage(LPCTSTR bitmap)
{
    TImageInfo* data = GetImage(bitmap);
    if( !data ) return false;

    ::DeleteObject(data->hBitmap);
    delete data;

    return m_mImageHash.Remove(bitmap);
}

void CPaintManagerUI::RemoveAllImages()
{
    TImageInfo* data;
    for( int i = 0; i< m_mImageHash.GetSize(); i++ ) {
        if(LPCTSTR key = m_mImageHash.GetAt(i)) {
            data = static_cast<TImageInfo*>(m_mImageHash.Find(key));
            ::DeleteObject(data->hBitmap);
            delete data;
        }
    }

    m_mImageHash.Resize();
}

void CPaintManagerUI::AddDefaultAttributeList(LPCTSTR pStrControlName, LPCTSTR pStrControlAttrList)
{
    CStdString *pDefaultAttr = new CStdString(pStrControlAttrList);
    m_DefaultAttrHash.Set(pStrControlName, (LPVOID)pDefaultAttr);
}

LPCTSTR CPaintManagerUI::GetDefaultAttributeList(LPCTSTR pStrControlName) const
{
    CStdString* pDefaultAttr = static_cast<CStdString*>(m_DefaultAttrHash.Find(pStrControlName));
    if( !pDefaultAttr && m_pParentResourcePM ) return m_pParentResourcePM->GetDefaultAttributeList(pStrControlName);
    
    if( pDefaultAttr ) return pDefaultAttr->GetData();
    else return NULL;
}

bool CPaintManagerUI::RemoveDefaultAttributeList(LPCTSTR pStrControlName)
{
    CStdString* pDefaultAttr = static_cast<CStdString*>(m_DefaultAttrHash.Find(pStrControlName));
    if( !pDefaultAttr ) return false;

    delete pDefaultAttr;
    return m_DefaultAttrHash.Remove(pStrControlName);
}

void CPaintManagerUI::RemoveAllDefaultAttributeList()
{
    CStdString* pDefaultAttr;
    for( int i = 0; i< m_DefaultAttrHash.GetSize(); i++ ) {
        if(LPCTSTR key = m_DefaultAttrHash.GetAt(i)) {
            pDefaultAttr = static_cast<CStdString*>(m_DefaultAttrHash.Find(key));
            delete pDefaultAttr;
        }
    }

    m_DefaultAttrHash.Resize();
}

CControlUI* CPaintManagerUI::GetRoot() const
{
    ASSERT(m_pRoot);
    return m_pRoot;
}

CControlUI* CPaintManagerUI::FindControl(LPCTSTR pstrName)
{
    ASSERT(m_pRoot);
    return static_cast<CControlUI*>(m_mNameHash.Find(pstrName));
}

CControlUI* CPaintManagerUI::FindControl(POINT pt) const
{
    ASSERT(m_pRoot);
    return m_pRoot->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromCount(CControlUI* /*pThis*/, LPVOID pData)
{
    int* pnCount = static_cast<int*>(pData);
    (*pnCount)++;
    return NULL;  // Count all controls
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromTab(CControlUI* pThis, LPVOID pData)
{
    FINDTABINFO* pInfo = static_cast<FINDTABINFO*>(pData);
    if( pInfo->pFocus == pThis ) {
        if( pInfo->bForward ) pInfo->bNextIsIt = true;
        return pInfo->bForward ? NULL : pInfo->pLast;
    }
    if( (pThis->GetControlFlags() & UIFLAG_TABSTOP) == 0 ) return NULL;
    pInfo->pLast = pThis;
    if( pInfo->bNextIsIt ) return pThis;
    if( pInfo->pFocus == NULL ) return pThis;
    return NULL;  // Examine all controls
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromNameHash(CControlUI* pThis, LPVOID pData)
{
    CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(pData);
    const CStdString& sName = pThis->GetName();
    if( sName.IsEmpty() ) return NULL;
    // Add this control to the hash list
    pManager->m_mNameHash.Insert(sName, pThis);
    return NULL; // Attempt to add all controls
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromShortcut(CControlUI* pThis, LPVOID pData)
{
    if( !pThis->IsVisible() ) return NULL; 
    FINDSHORTCUT* pFS = static_cast<FINDSHORTCUT*>(pData);
    if( pFS->ch == toupper(pThis->GetShortcut()) ) pFS->bPickNext = true;
    if( _tcsstr(pThis->GetClass(), _T("LabelUI")) != NULL ) return NULL;   // Labels never get focus!
    return pFS->bPickNext ? pThis : NULL;
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromPoint(CControlUI* pThis, LPVOID pData)
{
    LPPOINT pPoint = static_cast<LPPOINT>(pData);
    return ::PtInRect(&pThis->GetPos(), *pPoint) ? pThis : NULL;
}

CControlUI* CALLBACK CPaintManagerUI::__FindControlFromUpdate(CControlUI* pThis, LPVOID pData)
{
    return pThis->IsUpdateNeeded() ? pThis : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CControlUI::CControlUI() : 
m_pManager(NULL), 
m_pParent(NULL), 
m_bUpdateNeeded(true),
m_bVisible(true), 
m_bInternVisible(true),
m_bFocused(false),
m_bEnabled(true),
m_bMouseEnabled(true),
m_bFloat(false),
m_bFloatSetPos(false),
m_chShortcut('\0'),
m_pTag(NULL),
m_dwBackColor(0),
m_dwBackColor2(0),
m_dwBackColor3(0),
m_dwBorderColor(0),
m_nBorderSize(1)
{
    m_cXY.cx = m_cXY.cy = 0;
    m_cxyFixed.cx = m_cxyFixed.cy = 0;
    m_cxyMin.cx = m_cxyMin.cy = 0;
    m_cxyMax.cx = m_cxyMax.cy = 9999;
    m_cxyBorderRound.cx = m_cxyBorderRound.cy = 0;

    ::ZeroMemory(&m_rcPadding, sizeof(m_rcPadding));
    ::ZeroMemory(&m_rcItem, sizeof(RECT));
    ::ZeroMemory(&m_rcPaint, sizeof(RECT));
	::ZeroMemory(&m_tRelativePos, sizeof(TRelativePosUI));
}

CControlUI::~CControlUI()
{
    if( m_pManager != NULL ) m_pManager->ReapObjects(this);
}

CStdString CControlUI::GetName() const
{
    return m_sName;
}

void CControlUI::SetName(LPCTSTR pstrName)
{
    m_sName = pstrName;
}

LPVOID CControlUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, _T("Control")) == 0 ) return this;
    return NULL;
}

LPCTSTR CControlUI::GetClass() const
{
    return _T("ControlUI");
}

UINT CControlUI::GetControlFlags() const
{
    return 0;
}

bool CControlUI::Activate()
{
    if( !IsVisible() ) return false;
    if( !IsEnabled() ) return false;
    return true;
}

CPaintManagerUI* CControlUI::GetManager() const
{
    return m_pManager;
}

void CControlUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    m_pManager = pManager;
    m_pParent = pParent;
    if( bInit && m_pParent ) Init();
}

CControlUI* CControlUI::GetParent() const
{
    return m_pParent;
}

CStdString CControlUI::GetText() const
{
    return m_sText;
}

void CControlUI::SetText(LPCTSTR pstrText)
{
    if( m_sText == pstrText ) return;

    m_sText = pstrText;
    Invalidate();
}

DWORD CControlUI::GetBkColor() const
{
    return m_dwBackColor;
}

void CControlUI::SetBkColor(DWORD dwBackColor)
{
    if( m_dwBackColor == dwBackColor ) return;

    m_dwBackColor = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetBkColor2() const
{
    return m_dwBackColor2;
}

void CControlUI::SetBkColor2(DWORD dwBackColor)
{
    if( m_dwBackColor2 == dwBackColor ) return;

    m_dwBackColor2 = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetBkColor3() const
{
    return m_dwBackColor3;
}

void CControlUI::SetBkColor3(DWORD dwBackColor)
{
    if( m_dwBackColor3 == dwBackColor ) return;

    m_dwBackColor3 = dwBackColor;
    Invalidate();
}

LPCTSTR CControlUI::GetBkImage()
{
    return m_sBkImage;
}

void CControlUI::SetBkImage(LPCTSTR pStrImage)
{
    if( m_sBkImage == pStrImage ) return;

    m_sBkImage = pStrImage;
    Invalidate();
}

DWORD CControlUI::GetBorderColor() const
{
    return m_dwBorderColor;
}

void CControlUI::SetBorderColor(DWORD dwBorderColor)
{
    if( m_dwBorderColor == dwBorderColor ) return;

    m_dwBorderColor = dwBorderColor;
    Invalidate();
}

int CControlUI::GetBorderSize() const
{
    return m_nBorderSize;
}

void CControlUI::SetBorderSize(int nSize)
{
    if( m_nBorderSize == nSize ) return;

    m_nBorderSize = nSize;
    Invalidate();
}

SIZE CControlUI::GetBorderRound() const
{
    return m_cxyBorderRound;
}

void CControlUI::SetBorderRound(SIZE cxyRound)
{
    m_cxyBorderRound = cxyRound;
    Invalidate();
}

bool CControlUI::DrawImage(HDC hDC, LPCTSTR pStrImage, LPCTSTR pStrModify)
{
    return CRenderEngine::DrawImageString(hDC, m_pManager, m_rcItem, m_rcPaint, pStrImage, pStrModify);
}

const RECT& CControlUI::GetPos() const
{
    return m_rcItem;
}

void CControlUI::SetPos(RECT rc)
{
    if( rc.right < rc.left ) rc.right = rc.left;
    if( rc.bottom < rc.top ) rc.bottom = rc.top;

    CRect invalidateRc = m_rcItem;
    if( ::IsRectEmpty(&invalidateRc) ) invalidateRc = rc;

    m_rcItem = rc;
    if( m_pManager == NULL ) return;

    if( m_bFloat ) {
        if( !m_bFloatSetPos ) {
            m_bFloatSetPos = true;
            m_pManager->SendNotify(this, _T("setpos"));
            m_bFloatSetPos = false;
        }

        CControlUI* pParent = GetParent();
        if( pParent != NULL ) {
            RECT rcParentPos = pParent->GetPos();
            if( m_cXY.cx >= 0 ) m_cXY.cx = m_rcItem.left - rcParentPos.left;
            else m_cXY.cx = m_rcItem.right - rcParentPos.right;

            if( m_cXY.cy >= 0 ) m_cXY.cy = m_rcItem.top - rcParentPos.top;
            else m_cXY.cy = m_rcItem.bottom - rcParentPos.bottom;
        }
    }

    m_bUpdateNeeded = false;

    // NOTE: SetPos() is usually called during the WM_PAINT cycle where all controls are
    //       being laid out. Calling UpdateLayout() again would be wrong. Refreshing the
    //       window won't hurt (if we're already inside WM_PAINT we'll just validate it out).
    invalidateRc.Join(m_rcItem);

    CControlUI* pParent = this;
    RECT rcTemp;
    RECT rcParent;
    while( pParent = pParent->GetParent() )
    {
        rcTemp = invalidateRc;
        rcParent = pParent->GetPos();
        if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
        {
            return;
        }
    }
    m_pManager->Invalidate(invalidateRc);
}

int CControlUI::GetWidth() const
{
    return m_rcItem.right - m_rcItem.left;
}

int CControlUI::GetHeight() const
{
    return m_rcItem.bottom - m_rcItem.top;
}

int CControlUI::GetX() const
{
    return m_rcItem.left;
}

int CControlUI::GetY() const
{
    return m_rcItem.top;
}

RECT CControlUI::GetPadding() const
{
    return m_rcPadding;
}

void CControlUI::SetPadding(RECT rcPadding)
{
    m_rcPadding = rcPadding;
    NeedParentUpdate();
}

SIZE CControlUI::GetFixedXY() const
{
    return m_cXY;
}

void CControlUI::SetFixedXY(SIZE szXY)
{
    m_cXY.cx = szXY.cx;
    m_cXY.cy = szXY.cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetFixedWidth() const
{
    return m_cxyFixed.cx;
}

void CControlUI::SetFixedWidth(int cx)
{
    if( cx < 0 ) return; 
    m_cxyFixed.cx = cx;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetFixedHeight() const
{
    return m_cxyFixed.cy;
}

void CControlUI::SetFixedHeight(int cy)
{
    if( cy < 0 ) return; 
    m_cxyFixed.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMinWidth() const
{
    return m_cxyMin.cx;
}

void CControlUI::SetMinWidth(int cx)
{
    if( m_cxyMin.cx == cx ) return;

    if( cx < 0 ) return; 
    m_cxyMin.cx = cx;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMaxWidth() const
{
    return m_cxyMax.cx;
}

void CControlUI::SetMaxWidth(int cx)
{
    if( m_cxyMax.cx == cx ) return;

    if( cx < 0 ) return; 
    m_cxyMax.cx = cx;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMinHeight() const
{
    return m_cxyMin.cy;
}

void CControlUI::SetMinHeight(int cy)
{
    if( m_cxyMin.cy == cy ) return;

    if( cy < 0 ) return; 
    m_cxyMin.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMaxHeight() const
{
    return m_cxyMax.cy;
}

void CControlUI::SetMaxHeight(int cy)
{
    if( m_cxyMax.cy == cy ) return;

    if( cy < 0 ) return; 
    m_cxyMax.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

void CControlUI::SetRelativePos(SIZE szMove,SIZE szZoom)
{
	m_tRelativePos.bRelative = TRUE;
	m_tRelativePos.nMoveXPercent = szMove.cx;
	m_tRelativePos.nMoveYPercent = szMove.cy;
	m_tRelativePos.nZoomXPercent = szZoom.cx;
	m_tRelativePos.nZoomYPercent = szZoom.cy;
}

void CControlUI::SetRelativeParentSize(SIZE sz)
{
	m_tRelativePos.szParent = sz;
}

TRelativePosUI CControlUI::GetRelativePos() const
{
	return m_tRelativePos;
}

bool CControlUI::IsRelativePos() const
{
	return m_tRelativePos.bRelative;
}

CStdString CControlUI::GetToolTip() const
{
    return m_sToolTip;
}

void CControlUI::SetToolTip(LPCTSTR pstrText)
{
    m_sToolTip = pstrText;
}


TCHAR CControlUI::GetShortcut() const
{
    return m_chShortcut;
}

void CControlUI::SetShortcut(TCHAR ch)
{
    m_chShortcut = ch;
}

const CStdString& CControlUI::GetUserData()
{
    return m_sUserData;
}

void CControlUI::SetUserData(LPCTSTR pstrText)
{
    m_sUserData = pstrText;
}

UINT_PTR CControlUI::GetTag() const
{
    return m_pTag;
}

void CControlUI::SetTag(UINT_PTR pTag)
{
    m_pTag = pTag;
}

bool CControlUI::IsVisible() const
{
    return m_bVisible && m_bInternVisible;
}

void CControlUI::SetVisible(bool bVisible)
{
    if( m_bVisible == bVisible ) return;

    bool v = IsVisible();
    m_bVisible = bVisible;
    if( m_bFocused ) m_bFocused = false;
    if( IsVisible() != v ) {
        if( !m_bFloat ) NeedParentUpdate();
        else NeedUpdate();
    }
}

void CControlUI::SetInternVisible(bool bVisible)
{
    m_bInternVisible = bVisible;
}

bool CControlUI::IsEnabled() const
{
    return m_bEnabled;
}

void CControlUI::SetEnabled(bool bEnabled)
{
    if( m_bEnabled == bEnabled ) return;

    m_bEnabled = bEnabled;
    Invalidate();
}

bool CControlUI::IsMouseEnabled() const
{
    return m_bMouseEnabled;
}

void CControlUI::SetMouseEnabled(bool bEnabled)
{
    m_bMouseEnabled = bEnabled;
}

bool CControlUI::IsFocused() const
{
    return m_bFocused;
}
void CControlUI::SetFocus()
{
    if( m_pManager != NULL ) m_pManager->SetFocus(this);
}

bool CControlUI::IsFloat() const
{
    return m_bFloat;
}

void CControlUI::SetFloat(bool bFloat)
{
    if( m_bFloat == bFloat ) return;

    m_bFloat = bFloat;
    NeedParentUpdate();
}

CControlUI* CControlUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
    if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
    if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
    if( (uFlags & UIFIND_HITTEST) != 0 && (!m_bMouseEnabled || !::PtInRect(&m_rcItem, * static_cast<LPPOINT>(pData))) ) return NULL;
    return Proc(this, pData);
}

void CControlUI::Invalidate()
{
    if( !IsVisible() ) return;

    RECT invalidateRc = m_rcItem;

    CControlUI* pParent = this;
    RECT rcTemp;
    RECT rcParent;
    while( pParent = pParent->GetParent() )
    {
        rcTemp = invalidateRc;
        rcParent = pParent->GetPos();
        if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
        {
            return;
        }
    }

    if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
}

bool CControlUI::IsUpdateNeeded() const
{
    return m_bUpdateNeeded;
}

void CControlUI::NeedUpdate()
{
    m_bUpdateNeeded = true;
    Invalidate();

    if( m_pManager != NULL ) m_pManager->NeedUpdate();
}

void CControlUI::NeedParentUpdate()
{
    if( GetParent() ) {
        GetParent()->NeedUpdate();
        GetParent()->Invalidate();
    }
    else {
        NeedUpdate();
    }

    if( m_pManager != NULL ) m_pManager->NeedUpdate();
}

void CControlUI::Init()
{

}

void CControlUI::Event(TEventUI& event)
{
    if( event.Type == UIEVENT_SETCURSOR )
    {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
        return;
    }
    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        m_bFocused = true;
        Invalidate();
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        m_bFocused = false;
        Invalidate();
        return;
    }
    if( event.Type == UIEVENT_TIMER )
    {
        m_pManager->SendNotify(this, _T("timer"), event.wParam, event.lParam);
        return;
    }
    if( m_pParent != NULL ) m_pParent->Event(event);
}

void CControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("pos")) == 0 ) {
        RECT rcPos = { 0 };
        LPTSTR pstr = NULL;
        rcPos.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcPos.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcPos.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcPos.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SIZE szXY = {rcPos.left >= 0 ? rcPos.left : rcPos.right, rcPos.top >= 0 ? rcPos.top : rcPos.bottom};
        SetFixedXY(szXY);
        SetFixedWidth(rcPos.right - rcPos.left);
        SetFixedHeight(rcPos.bottom - rcPos.top);
    }
	else if( _tcscmp(pstrName, _T("relativepos")) == 0 ) {
		SIZE szMove,szZoom;
		LPTSTR pstr = NULL;
		szMove.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		szMove.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		szZoom.cx = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		szZoom.cy = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr); 
		SetRelativePos(szMove,szZoom);
	}
    else if( _tcscmp(pstrName, _T("padding")) == 0 ) {
        RECT rcPadding = { 0 };
        LPTSTR pstr = NULL;
        rcPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SetPadding(rcPadding);
    }
    else if( _tcscmp(pstrName, _T("bkcolor")) == 0 || _tcscmp(pstrName, _T("bkcolor1")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("bkcolor2")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor2(clrColor);
    }
    else if( _tcscmp(pstrName, _T("bkcolor3")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor3(clrColor);
    }
    else if( _tcscmp(pstrName, _T("bordercolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBorderColor(clrColor);
    }
    else if( _tcscmp(pstrName, _T("bordersize")) == 0 ) SetBorderSize(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("borderround")) == 0 ) {
        SIZE cxyRound = { 0 };
        LPTSTR pstr = NULL;
        cxyRound.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        cxyRound.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);     
        SetBorderRound(cxyRound);
    }
    else if( _tcscmp(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
    else if( _tcscmp(pstrName, _T("width")) == 0 ) SetFixedWidth(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("height")) == 0 ) SetFixedHeight(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("minwidth")) == 0 ) SetMinWidth(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("minheight")) == 0 ) SetMinHeight(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("maxwidth")) == 0 ) SetMaxWidth(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("maxheight")) == 0 ) SetMaxHeight(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("name")) == 0 ) SetName(pstrValue);
    else if( _tcscmp(pstrName, _T("text")) == 0 ) SetText(pstrValue);
    else if( _tcscmp(pstrName, _T("tooltip")) == 0 ) SetToolTip(pstrValue);
    else if( _tcscmp(pstrName, _T("userdata")) == 0 ) SetUserData(pstrValue);
    else if( _tcscmp(pstrName, _T("enabled")) == 0 ) SetEnabled(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("mouse")) == 0 ) SetMouseEnabled(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("visible")) == 0 ) SetVisible(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("float")) == 0 ) SetFloat(_tcscmp(pstrValue, _T("true")) == 0);
    else if( _tcscmp(pstrName, _T("shortcut")) == 0 ) SetShortcut(pstrValue[0]);
}

CControlUI* CControlUI::ApplyAttributeList(LPCTSTR pstrList)
{
    CStdString sItem;
    CStdString sValue;
    while( *pstrList != _T('\0') ) {
        sItem.Empty();
        sValue.Empty();
        while( *pstrList != _T('\0') && *pstrList != _T('=') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sItem += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('=') );
        if( *pstrList++ != _T('=') ) return this;
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return this;
        while( *pstrList != _T('\0') && *pstrList != _T('\"') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sValue += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return this;
        SetAttribute(sItem, sValue);
        if( *pstrList++ != _T(' ') ) return this;
    }
    return this;
}

SIZE CControlUI::EstimateSize(SIZE szAvailable)
{
    return m_cxyFixed;
}

void CControlUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;

    // »æÖÆÑ­Ðò£º±³¾°ÑÕÉ«->±³¾°Í¼->×´Ì¬Í¼->ÎÄ±¾
    PaintBkColor(hDC);
    PaintBkImage(hDC);
    PaintStatusImage(hDC);
    PaintText(hDC);
    PaintBorder(hDC);
}

void CControlUI::PaintBkColor(HDC hDC)
{
    if( m_dwBackColor != 0 ) {
        if( m_dwBackColor2 != 0 ) {
            if( m_dwBackColor3 != 0 ) {
                RECT rc = m_rcItem;
                rc.bottom = (rc.bottom + rc.top) / 2;
                CRenderEngine::DrawGradient(hDC, rc, m_dwBackColor, m_dwBackColor2, true, 8);
                rc.top = rc.bottom;
                rc.bottom = m_rcItem.bottom;
                CRenderEngine::DrawGradient(hDC, rc, m_dwBackColor2, m_dwBackColor3, true, 8);
            }
            else 
                CRenderEngine::DrawGradient(hDC, m_rcItem, m_dwBackColor, m_dwBackColor2, true, 16);
        }
        else if( m_dwBackColor >= 0xFF000000 ) CRenderEngine::DrawColor(hDC, m_rcPaint, m_dwBackColor);
        else CRenderEngine::DrawColor(hDC, m_rcItem, m_dwBackColor);
    }
}

void CControlUI::PaintBkImage(HDC hDC)
{
    if( m_sBkImage.IsEmpty() ) return;
    if( !DrawImage(hDC, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
}

void CControlUI::PaintStatusImage(HDC hDC)
{
    return;
}

void CControlUI::PaintText(HDC hDC)
{
    return;
}

void CControlUI::PaintBorder(HDC hDC)
{
    if( m_dwBorderColor != 0 && m_nBorderSize > 0 ) {
        if( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 )
            CRenderEngine::DrawRoundRect(hDC, m_rcItem, m_nBorderSize, m_cxyBorderRound.cx, m_cxyBorderRound.cy, m_dwBorderColor);
        else CRenderEngine::DrawRect(hDC, m_rcItem, m_nBorderSize, m_dwBorderColor);
    }
}

void CControlUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
{
    return;
}

} // namespace DuiLib
