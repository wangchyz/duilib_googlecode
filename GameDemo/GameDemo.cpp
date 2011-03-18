// App.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <exdisp.h>
#include <comdef.h>

#include "MiniDumper.h"
CMiniDumper g_miniDumper( true );

#include "ControlEx.h"

class CLoginFrameWnd : public CWindowWnd, public INotifyUI
{
public:
    CLoginFrameWnd() { };
    LPCTSTR GetWindowClassName() const { return _T("UILoginFrame"); };
    UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
    void OnFinalMessage(HWND /*hWnd*/) { delete this; };

    void Init() {
        CComboUI* pAccountCombo = static_cast<CComboUI*>(m_pm.FindControl(_T("accountcombo")));
        CEditUI* pAccountEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
        if( pAccountCombo && pAccountEdit ) pAccountEdit->SetText(pAccountCombo->GetText());
    }

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("click") ) {
            if( msg.pSender->GetName() == _T("closebtn") ) { PostQuitMessage(0); return; }
            else if( msg.pSender->GetName() == _T("loginBtn") ) { Close(); return; }
        }
        else if( msg.sType == _T("itemselect") ) {
            if( msg.pSender->GetName() == _T("accountcombo") ) {
                CEditUI* pAccountEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
                if( pAccountEdit ) pAccountEdit->SetText(msg.pSender->GetText());
            }
        }
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        RECT rcClient;
        ::GetClientRect(*this, &rcClient);
        ::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
            rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

        m_pm.Init(m_hWnd);
        CDialogBuilder builder;
        CDialogBuilderCallbackEx cb;
        CControlUI* pRoot = builder.Create(_T("login.xml"), (UINT)0, &cb, &m_pm);
        ASSERT(pRoot && "Failed to parse XML");
        m_pm.AttachDialog(pRoot);
        m_pm.AddNotifier(this);

        SIZE szInitWindowSize = m_pm.GetInitSize();
        if( szInitWindowSize.cx != 0 ) {
            ::SetWindowPos(*this, NULL, 0, 0, szInitWindowSize.cx, szInitWindowSize.cy, SWP_NOZORDER | SWP_NOMOVE);
        }

        Init();
        return 0;
    }

    LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return (wParam == 0) ? TRUE : FALSE;
    }

    LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return 0;
    }

    LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return 0;
    }

    LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
        ::ScreenToClient(*this, &pt);

        RECT rcClient;
        ::GetClientRect(*this, &rcClient);

        RECT rcCaption = m_pm.GetCaptionRect();
        if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
            && pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
                CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
                if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 )
                    return HTCAPTION;
        }

        return HTCLIENT;
    }

    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SIZE szRoundCorner = m_pm.GetRoundCorner();
        if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
            RECT rcClient;
            ::GetClientRect(*this, &rcClient);
            HRGN hRgn = ::CreateRoundRectRgn(rcClient.left, rcClient.top, rcClient.right + 1, rcClient.bottom + 1, szRoundCorner.cx, szRoundCorner.cy);
            ::SetWindowRgn(*this, hRgn, TRUE);
            ::DeleteObject(hRgn);
        }

        bHandled = FALSE;
        return 0;
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT lRes = 0;
        BOOL bHandled = TRUE;
        switch( uMsg ) {
        case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
        case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
        case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
        case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
        case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
        case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
        default:
            bHandled = FALSE;
        }
        if( bHandled ) return lRes;
        if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

public:
    CPaintManagerUI m_pm;
};

class CGameFrameWnd : public CWindowWnd, public INotifyUI
{
public:
    CGameFrameWnd() { };
    LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
    UINT GetClassStyle() const { return CS_DBLCLKS; };
    void OnFinalMessage(HWND /*hWnd*/) { delete this; };

    void Init() {
        m_pCloseBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")));
        m_pMaxBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("maxbtn")));
        m_pRestoreBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("restorebtn")));
        m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minbtn")));

        CActiveXUI* pActiveXUI = static_cast<CActiveXUI*>(m_pm.FindControl(_T("ie")));
        if( pActiveXUI ) {
            IWebBrowser2* pWebBrowser = NULL;
            pActiveXUI->GetControl(IID_IWebBrowser2, (void**)&pWebBrowser);
            if( pWebBrowser != NULL ) {
                pWebBrowser->Navigate(L"http://www.baidu.com",NULL,NULL,NULL,NULL);  
                //pWebBrowser->Navigate(L"about:blank",NULL,NULL,NULL,NULL); 
                pWebBrowser->Release();
            }
        }
    }

    void OnPrepare() { 
        GameListUI* pGameList = static_cast<GameListUI*>(m_pm.FindControl(_T("gamelist")));
        GameListUI::Node* pCategoryNode = NULL;
        GameListUI::Node* pGameNode = NULL;
        GameListUI::Node* pServerNode = NULL;
        GameListUI::Node* pRoomNode = NULL;
        pCategoryNode = pGameList->AddNode(_T("{x 4}{i gameicons.png 3}{x 4}推荐游戏"));
        for( int i = 0; i < 4; ++i )
        {
            pGameNode = pGameList->AddNode(_T("{x 4}{i gameicons.png 10}{x 4}四人斗地主"), pCategoryNode);
            for( int i = 0; i < 3; ++i )
            {
                pServerNode = pGameList->AddNode(_T("{x 4}{i gameicons.png 10}{x 4}测试服务器"), pGameNode);
                for( int i = 0; i < 3; ++i )
                {
                    pRoomNode = pGameList->AddNode(_T("{x 4}{i gameicons.png 10}{x 4}测试房间"), pServerNode);
                }
            }
        }
        pCategoryNode = pGameList->AddNode(_T("{x 4}{i gameicons.png 3}{x 4}最近玩过的游戏"));
        for( int i = 0; i < 2; ++i )
        {
            pGameList->AddNode(_T("三缺一"), pCategoryNode);
        }
        pCategoryNode = pGameList->AddNode(_T("{x 4}{i gameicons.png 3}{x 4}棋牌游戏"));
        for( int i = 0; i < 8; ++i )
        {
            pGameList->AddNode(_T("双扣"), pCategoryNode);
        }
        pCategoryNode = pGameList->AddNode(_T("{x 4}{i gameicons.png 3}{x 4}休闲游戏"));
        for( int i = 0; i < 8; ++i )
        {
            pGameList->AddNode(_T("飞行棋"), pCategoryNode);
        }

        CLoginFrameWnd* pLoginFrame = new CLoginFrameWnd();
        if( pLoginFrame == NULL ) { Close(); return; }
        pLoginFrame->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, UI_WNDSTYLE_EX_DIALOG, 0, 0, 0, 0, NULL);
        pLoginFrame->CenterWindow();
        pLoginFrame->ShowModal();
    }

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) OnPrepare();
        else if( msg.sType == _T("click") ) {
            if( msg.pSender == m_pCloseBtn ) { 
                COptionUI* pControl = static_cast<COptionUI*>(m_pm.FindControl(_T("hallswitch")));
                if( pControl && pControl->IsSelected() == false ) {
                    CControlUI* pFadeControl = m_pm.FindControl(_T("fadeEffect"));
                    if( pFadeControl ) pFadeControl->SetVisible(true);
                }
                else {
                    /*Close()*/PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
                }
                return; 
            }
            else if( msg.pSender == m_pMinBtn ) { SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); return; }
            else if( msg.pSender == m_pMaxBtn ) { SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; }
            else if( msg.pSender == m_pRestoreBtn ) { SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); return; }
            CStdString name = msg.pSender->GetName();
            if( name == _T("quitbtn") ) {
                /*Close()*/PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
            }
            else if( name == _T("returnhallbtn") ) {
                CControlUI* pFadeControl = m_pm.FindControl(_T("fadeEffect"));
                if( pFadeControl ) pFadeControl->SetVisible(false); 

                COptionUI* pControl = static_cast<COptionUI*>(m_pm.FindControl(_T("hallswitch")));
                pControl->Activate();
                pControl = static_cast<COptionUI*>(m_pm.FindControl(_T("roomswitch")));
                if( pControl ) pControl->SetVisible(false);
            }
            else if( name == _T("fontswitch") ) {
                if( m_pm.GetDefaultFont() == m_pm.GetFont(0) ) {
                    m_pm.SetDefaultFont(m_pm.GetFont(3));
                    m_pm.SetDefaultBoldFont(m_pm.GetFont(4));
                    m_pm.SetDefaultLinkFont(m_pm.GetFont(5));
                }
                else {
                    m_pm.SetDefaultFont(m_pm.GetFont(0));
                    m_pm.SetDefaultBoldFont(m_pm.GetFont(1));
                    m_pm.SetDefaultLinkFont(m_pm.GetFont(2));
                }
                m_pm.GetRoot()->NeedUpdate();
            }
            else if( name == _T("leaveBtn") ) {
                COptionUI* pControl = static_cast<COptionUI*>(m_pm.FindControl(_T("hallswitch")));
                if( pControl && pControl->IsSelected() == false ) {
                    pControl->Activate();
                    pControl = static_cast<COptionUI*>(m_pm.FindControl(_T("roomswitch")));
                    if( pControl ) pControl->SetVisible(false);
                }
            }
            else if( name == _T("sendbtn") ) {
                CVerticalLayoutUI* pControl = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("chatmsglist")));
                CTextUI* text = new CTextUI;
                text->SetText(_T("7月22日消息，盛大网络旗下盛大在线宣布，目前已与酷6网就其旗下的PHPCMS业务整体移交一事达成一致，交接工作正在进行中。这也意味着盛大开始在站长行业进行战略布局。"));
                text->SetTextColor(0xFF0000CF);
                pControl->Add(text);
                pControl->SetPos(pControl->GetPos());
                pControl->EndDown();
            }
            else if( name == _T("deletebtn") ) {
                CVerticalLayoutUI* pControl = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("chatmsglist")));
                pControl->RemoveAt(pControl->GetCount() - 1);
            }
        }
        else if( msg.sType == _T("selectchanged") ) {
            CStdString name = msg.pSender->GetName();
            if( name == _T("hallswitch") ) {
                CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
                if( pControl && pControl->GetCurSel() != 0 ) pControl->SelectItem(0);
            }
            else if( name == _T("roomswitch") ) {
                CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));
                if( pControl && pControl->GetCurSel() != 1 ) {
                    pControl->SelectItem(1);
                    CVerticalLayoutUI* pChatMsgList = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("chatmsglist")));
                    if( pChatMsgList ) pChatMsgList->RemoveAll();
                }
            }
        }
        else if( msg.sType == _T("itemclick") ) {
            GameListUI* pGameList = static_cast<GameListUI*>(m_pm.FindControl(_T("gamelist")));
            if( pGameList->GetItemIndex(msg.pSender) != -1 )
            {
                if( _tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0 ) {
                    GameListUI::Node* node = (GameListUI::Node*)msg.pSender->GetTag();

                    POINT pt = { 0 };
                    ::GetCursorPos(&pt);
                    ::ScreenToClient(m_pm.GetPaintWindow(), &pt);
                    pt.x -= msg.pSender->GetX();
                    SIZE sz = pGameList->GetExpanderSizeX(node);
                    if( pt.x >= sz.cx && pt.x < sz.cy )                     
                        pGameList->SetChildVisible(node, !node->data()._child_visible);
                }
            }
        }
        else if( msg.sType == _T("itemactivate") ) {
            GameListUI* pGameList = static_cast<GameListUI*>(m_pm.FindControl(_T("gamelist")));
            if( pGameList->GetItemIndex(msg.pSender) != -1 )
            {
                if( _tcscmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0 ) {
                    GameListUI::Node* node = (GameListUI::Node*)msg.pSender->GetTag();
                    pGameList->SetChildVisible(node, !node->data()._child_visible);
                    if( node->data()._level == 3 ) {
                        COptionUI* pControl = static_cast<COptionUI*>(m_pm.FindControl(_T("roomswitch")));
                        if( pControl ) {
                            pControl->SetVisible(true);
                            pControl->Activate();
                        }
                    }
                }
            }

        }
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
        styleValue &= ~WS_CAPTION;
        ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        RECT rcClient;
        ::GetClientRect(*this, &rcClient);
        ::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
            rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

        m_pm.Init(m_hWnd);
        CDialogBuilder builder;
        CDialogBuilderCallbackEx cb;
        CControlUI* pRoot = builder.Create(_T("hall.xml"), (UINT)0, &cb, &m_pm);
        ASSERT(pRoot && "Failed to parse XML");
        m_pm.AttachDialog(pRoot);
        m_pm.AddNotifier(this);

        Init();
        return 0;
    }

    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ::PostQuitMessage(0L);

        bHandled = FALSE;
        return 0;
    }

    LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return (wParam == 0) ? TRUE : FALSE;
    }

    LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return 0;
    }

    LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return 0;
    }

    LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
        ::ScreenToClient(*this, &pt);

        RECT rcClient;
        ::GetClientRect(*this, &rcClient);

        if( !::IsZoomed(*this) ) {
            RECT rcSizeBox = m_pm.GetSizeBox();
            if( pt.y < rcClient.top + rcSizeBox.top ) {
                if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
                if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
                return HTTOP;
            }
            else if( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
                if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
                if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
                return HTBOTTOM;
            }
            if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
            if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
        }

        RECT rcCaption = m_pm.GetCaptionRect();
        if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
            && pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
                CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
                if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 && 
                    _tcscmp(pControl->GetClass(), _T("OptionUI")) != 0 &&
                    _tcscmp(pControl->GetClass(), _T("TextUI")) != 0 )
                    return HTCAPTION;
        }

        return HTCLIENT;
    }

    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SIZE szRoundCorner = m_pm.GetRoundCorner();
        if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
            RECT rcClient;
            ::GetClientRect(*this, &rcClient);
            HRGN hRgn = ::CreateRoundRectRgn(rcClient.left, rcClient.top, rcClient.right + 1, rcClient.bottom + 1, szRoundCorner.cx, szRoundCorner.cy);
            ::SetWindowRgn(*this, hRgn, TRUE);
            ::DeleteObject(hRgn);
        }

        bHandled = FALSE;
        return 0;
    }

    LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        MONITORINFO oMonitor = {};
        oMonitor.cbSize = sizeof(oMonitor);
        ::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
        CRect rcWork = oMonitor.rcWork;
        LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
        lpMMI->ptMaxPosition.x = rcWork.left;
        lpMMI->ptMaxPosition.y = rcWork.top;
        lpMMI->ptMaxSize.x = rcWork.right - rcWork.left;
        lpMMI->ptMaxSize.y = rcWork.bottom - rcWork.top;
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        // 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
        if( wParam == SC_CLOSE ) {
            ::PostQuitMessage(0L);
            bHandled = TRUE;
            return 0;
        }
        BOOL bZoomed = ::IsZoomed(*this);
        LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
        if( ::IsZoomed(*this) != bZoomed ) {
            if( !bZoomed ) {
                CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
                if( pControl ) pControl->SetVisible(false);
                pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
                if( pControl ) pControl->SetVisible(true);
            }
            else {
                CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
                if( pControl ) pControl->SetVisible(true);
                pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
                if( pControl ) pControl->SetVisible(false);
            }
        }
        return lRes;
    }

    LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        // 解决ie控件收不到滚动消息的问题
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ::ScreenToClient(m_pm.GetPaintWindow(), &pt);
        CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("ie")));
        if( pControl && pControl->IsVisible() ) {
            RECT rc = pControl->GetPos();
            if( ::PtInRect(&rc, pt) ) {
                return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
            }
        }

        bHandled = FALSE;
        return 0;
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT lRes = 0;
        BOOL bHandled = TRUE;
        switch( uMsg ) {
        case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
        case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
        case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
        case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
        case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
        case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
        case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
        case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
        case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
        case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
        case WM_MOUSEWHEEL:    lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled); break;
        default:
            bHandled = FALSE;
        }
        if( bHandled ) return lRes;
        if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

public:
    CPaintManagerUI m_pm;

private:
    CButtonUI* m_pCloseBtn;
    CButtonUI* m_pMaxBtn;
    CButtonUI* m_pRestoreBtn;
    CButtonUI* m_pMinBtn;
    //...
};


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetResourcePath(_T("skin"));
    CPaintManagerUI::SetResourceZip(_T("GameRes.zip"));

    HRESULT Hr = ::CoInitialize(NULL);
    if( FAILED(Hr) ) return 0;

    CGameFrameWnd* pFrame = new CGameFrameWnd();
    if( pFrame == NULL ) return 0;
    pFrame->Create(NULL, _T(""), UI_WNDSTYLE_FRAME, 0L, 0, 0, 1024, 738);
    pFrame->CenterWindow();
    ::ShowWindow(*pFrame, SW_SHOWMAXIMIZED);

    CPaintManagerUI::MessageLoop();

    ::CoUninitialize();
    return 0;
}
