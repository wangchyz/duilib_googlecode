#include "StdAfx.h"

namespace DuiLib {

CControlUI* CDialogBuilder::Create(STRINGorID xml, STRINGorID type, IDialogBuilderCallback* pCallback, CPaintManagerUI* pManager)
{
    m_pCallback = pCallback;
    if( HIWORD(xml.m_lpstr) != NULL ) {
        if( *(xml.m_lpstr) == _T('<') ) {
            if( !m_xml.Load(xml.m_lpstr) ) return NULL;
        }
        else {
            if( !m_xml.LoadFromFile(xml.m_lpstr) ) return NULL;
        }
    }
    else {

        HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), xml.m_lpstr, type.m_lpstr);
        if( hResource == NULL ) return NULL;
        HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
        if( hGlobal == NULL ) {
            FreeResource(hResource);
            return NULL;
        }

        m_pCallback = pCallback;
        if( !m_xml.LoadFromMem((BYTE*)::LockResource(hGlobal), ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource) )) return NULL;
        ::FreeResource(hResource);
    }

    CMarkupNode root = m_xml.GetRoot();
    if( pManager ) {
        LPCTSTR pstrClass = root.GetName();
        if( _tcscmp(pstrClass, _T("Window")) == 0 ) {
            int nAttributes = 0;
            LPCTSTR pstrName = NULL;
            LPCTSTR pstrValue = NULL;
            LPTSTR pstr = NULL;

            for( CMarkupNode node = root.GetChild() ; node.IsValid(); node = node.GetSibling() ) {
                pstrClass = node.GetName();
                if( _tcscmp(pstrClass, _T("Image")) == 0 ) {
                    nAttributes = node.GetAttributeCount();
                    LPCTSTR pImageName = NULL;
                    LPCTSTR pImageResType = NULL;
                    DWORD mask = 0;
                    for( int i = 0; i < nAttributes; i++ ) {
                        pstrName = node.GetAttributeName(i);
                        pstrValue = node.GetAttributeValue(i);
                        if( _tcscmp(pstrName, _T("name")) == 0 ) {
                            pImageName = pstrValue;
                        }
                        else if( _tcscmp(pstrName, _T("restype")) == 0 ) {
                            pImageResType = pstrValue;
                        }
                        else if( _tcscmp(pstrName, _T("mask")) == 0 ) {
                            if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
                            mask = _tcstoul(pstrValue, &pstr, 16);
                        }
                    }
                    if( pImageName ) pManager->AddImage(pImageName, pImageResType, mask);
                }
                else if( _tcscmp(pstrClass, _T("Font")) == 0 ) {
                    nAttributes = node.GetAttributeCount();
                    LPCTSTR pFontName = NULL;
                    int size = 12;
                    bool bold = false;
                    bool underline = false;
                    bool italic = false;
                    bool defaultfont = false;
                    bool defaultboldfont = false;
                    bool defaultlinkfont = false;
					DWORD dwDefLinkColor = 0xFF0000FF;
					DWORD dwDefLinkHoverColor = 0xFFD3215F;
                    for( int i = 0; i < nAttributes; i++ ) {
                        pstrName = node.GetAttributeName(i);
                        pstrValue = node.GetAttributeValue(i);
                        if( _tcscmp(pstrName, _T("name")) == 0 ) {
                            pFontName = pstrValue;
                        }
                        else if( _tcscmp(pstrName, _T("size")) == 0 ) {
                            size = _tcstol(pstrValue, &pstr, 10);
                        }
                        else if( _tcscmp(pstrName, _T("bold")) == 0 ) {
                            bold = (_tcscmp(pstrValue, _T("true")) == 0);
                        }
                        else if( _tcscmp(pstrName, _T("underline")) == 0 ) {
                            underline = (_tcscmp(pstrValue, _T("true")) == 0);
                        }
                        else if( _tcscmp(pstrName, _T("italic")) == 0 ) {
                            italic = (_tcscmp(pstrValue, _T("true")) == 0);
                        }
                        else if( _tcscmp(pstrName, _T("default")) == 0 ) {
                            defaultfont = (_tcscmp(pstrValue, _T("true")) == 0);
                        }
                        else if( _tcscmp(pstrName, _T("defaultbold")) == 0 ) {
                            defaultboldfont = (_tcscmp(pstrValue, _T("true")) == 0);
                        }
                        else if( _tcscmp(pstrName, _T("defaultlink")) == 0 ) {
                            defaultlinkfont = (_tcscmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcscmp(pstrName, _T("defaultlinkcolor")) == 0 ) {
							if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
							LPTSTR pstr = NULL;
							dwDefLinkColor = _tcstoul(pstrValue, &pstr, 16);
						}
						else if( _tcscmp(pstrName, _T("defaultlinkhovercolor")) == 0 ) {
							if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
							LPTSTR pstr = NULL;
							dwDefLinkHoverColor = _tcstoul(pstrValue, &pstr, 16);
						}
                    }
                    if( pFontName ) {
                        HFONT hFont = pManager->AddFont(pFontName, size, bold, underline, italic);
                        if( hFont != NULL ) {
                            if( defaultfont ) pManager->SetDefaultFont(hFont);
                            if( defaultboldfont ) pManager->SetDefaultBoldFont(hFont);
                            if( defaultlinkfont ) pManager->SetDefaultLinkFont(hFont,dwDefLinkColor,dwDefLinkHoverColor);
                        }
                    }
                }
                else if( _tcscmp(pstrClass, _T("Default")) == 0 ) {
                    nAttributes = node.GetAttributeCount();
                    LPCTSTR pControlName = NULL;
                    LPCTSTR pControlValue = NULL;
                    for( int i = 0; i < nAttributes; i++ ) {
                        pstrName = node.GetAttributeName(i);
                        pstrValue = node.GetAttributeValue(i);
                        if( _tcscmp(pstrName, _T("name")) == 0 ) {
                            pControlName = pstrValue;
                        }
                        else if( _tcscmp(pstrName, _T("value")) == 0 ) {
                            pControlValue = pstrValue;
                        }
                    }
                    if( pControlName ) {
                        pManager->AddDefaultAttributeList(pControlName, pControlValue);
                    }
                }
            }

            if( pManager->GetPaintWindow() ) {
                int nAttributes = root.GetAttributeCount();
                for( int i = 0; i < nAttributes; i++ ) {
                    pstrName = root.GetAttributeName(i);
                    pstrValue = root.GetAttributeValue(i);
                    if( _tcscmp(pstrName, _T("size")) == 0 ) {
                        LPTSTR pstr = NULL;
                        int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
                        int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
                        pManager->SetInitSize(cx, cy);
                    } 
                    else if( _tcscmp(pstrName, _T("sizebox")) == 0 ) {
                        RECT rcSizeBox = { 0 };
                        LPTSTR pstr = NULL;
                        rcSizeBox.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
                        rcSizeBox.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
                        rcSizeBox.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
                        rcSizeBox.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
                        pManager->SetSizeBox(rcSizeBox);
                    }
                    else if( _tcscmp(pstrName, _T("caption")) == 0 ) {
                        RECT rcCaption = { 0 };
                        LPTSTR pstr = NULL;
                        rcCaption.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
                        rcCaption.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
                        rcCaption.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
                        rcCaption.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
                        pManager->SetCaptionRect(rcCaption);
                    }
                    else if( _tcscmp(pstrName, _T("roundcorner")) == 0 ) {
                        LPTSTR pstr = NULL;
                        int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
                        int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
                        pManager->SetRoundCorner(cx, cy);
                    } 
                    else if( _tcscmp(pstrName, _T("mininfo")) == 0 ) {
                        LPTSTR pstr = NULL;
                        int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
                        int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
                        pManager->SetMinMaxInfo(cx, cy);
                    }
                    else if( _tcscmp(pstrName, _T("showdirty")) == 0 ) {
                        pManager->SetShowUpdateRect(_tcscmp(pstrValue, _T("true")) == 0);
                    } 
                    else if( _tcscmp(pstrName, _T("alpha")) == 0 ) {
                        pManager->SetTransparent(_ttoi(pstrValue));
                    } 
                    else if( _tcscmp(pstrName, _T("bktrans")) == 0 ) {
                        pManager->SetBackgroundTransparent(_tcscmp(pstrValue, _T("true")) == 0);
                    } 
                }
            }
        }
    }
    return _Parse(&root, NULL, pManager);
}

void CDialogBuilder::GetLastErrorMessage(LPTSTR pstrMessage, SIZE_T cchMax) const
{
    return m_xml.GetLastErrorMessage(pstrMessage, cchMax);
}

void CDialogBuilder::GetLastErrorLocation(LPTSTR pstrSource, SIZE_T cchMax) const
{
    return m_xml.GetLastErrorLocation(pstrSource, cchMax);
}

CControlUI* CDialogBuilder::_Parse(CMarkupNode* pRoot, CControlUI* pParent, CPaintManagerUI* pManager)
{
    CDialogLayoutUI* pDialogLayout = NULL;
    IContainerUI* pContainer = NULL;
    CControlUI* pReturn = NULL;
    for( CMarkupNode node = pRoot->GetChild() ; node.IsValid(); node = node.GetSibling() ) {
        LPCTSTR pstrClass = node.GetName();
        if( _tcscmp(pstrClass, _T("Image")) == 0 || _tcscmp(pstrClass, _T("Font")) == 0 \
            || _tcscmp(pstrClass, _T("Default")) == 0 ) continue;

        SIZE_T cchLen = _tcslen(pstrClass);
        CControlUI* pControl = NULL;
        switch( cchLen ) {
        case 4:
            if( _tcscmp(pstrClass, _T("Edit")) == 0 )                   pControl = new CEditUI;
            else if( _tcscmp(pstrClass, _T("List")) == 0 )              pControl = new CListUI;
            else if( _tcscmp(pstrClass, _T("Text")) == 0 )              pControl = new CTextUI;
            break;
        case 5:
            if( _tcscmp(pstrClass, _T("Combo")) == 0 )                  pControl = new CComboUI;
            else if( _tcscmp(pstrClass, _T("Label")) == 0 )             pControl = new CLabelUI;
            break;
        case 6:
            if( _tcscmp(pstrClass, _T("Button")) == 0 )                 pControl = new CButtonUI;
            else if( _tcscmp(pstrClass, _T("Option")) == 0 )            pControl = new COptionUI;
            else if( _tcscmp(pstrClass, _T("Slider")) == 0 )            pControl = new CSliderUI;
            break;
        case 7:
            if( _tcscmp(pstrClass, _T("Control")) == 0 )                pControl = new CControlUI;
            else if( _tcscmp(pstrClass, _T("ActiveX")) == 0 )           pControl = new CActiveXUI;
            break;
        case 8:
            if( _tcscmp(pstrClass, _T("Progress")) == 0 )               pControl = new CProgressUI;
            else if(  _tcscmp(pstrClass, _T("RichEdit")) == 0 )         pControl = new CRichEditUI;
            break;
        case 9:
            if( _tcscmp(pstrClass, _T("Container")) == 0 )              pControl = new CContainerUI;
            else if( _tcscmp(pstrClass, _T("TabLayout")) == 0 )         pControl = new CTabLayoutUI;
            break;
        case 10:
            if( _tcscmp(pstrClass, _T("ListHeader")) == 0 )             pControl = new CListHeaderUI;
            else if( _tcscmp(pstrClass, _T("TileLayout")) == 0 )        pControl = new CTileLayoutUI;
            break;
        case 12:
            if( _tcscmp(pstrClass, _T("DialogLayout")) == 0 )           pControl = new CDialogLayoutUI;
            break;
        case 14:
            if( _tcscmp(pstrClass, _T("VerticalLayout")) == 0 )         pControl = new CVerticalLayoutUI;
            else if( _tcscmp(pstrClass, _T("ListHeaderItem")) == 0 )    pControl = new CListHeaderItemUI;
            break;
        case 15:
            if( _tcscmp(pstrClass, _T("ListTextElement")) == 0 )        pControl = new CListTextElementUI;
            break;
        case 16:
            if( _tcscmp(pstrClass, _T("HorizontalLayout")) == 0 )       pControl = new CHorizontalLayoutUI;
            else if( _tcscmp(pstrClass, _T("ListLabelElement")) == 0 )  pControl = new CListLabelElementUI;
            break;
        case 17:
            if( _tcscmp(pstrClass, _T("ListExpandElement")) == 0 )      pControl = new CListExpandElementUI;
            break;
        case 20:
            if( _tcscmp(pstrClass, _T("ListContainerElement")) == 0 )   pControl = new CListContainerElementUI;
            break;
        }
        // User-supplied control factory
        if( pControl == NULL && m_pCallback != NULL ) {
            pControl = m_pCallback->CreateControl(pstrClass);
        }
        ASSERT(pControl);
        if( pControl == NULL ) continue;

        // Add children
        if( node.HasChildren() ) {
            _Parse(&node, pControl, pManager);
        }
        // Attach to parent
        // 因为某些属性和父窗口相关，比如selected，必须先Add到父窗口
        if( pParent != NULL ) {
            if( pContainer == NULL ) pContainer = static_cast<IContainerUI*>(pParent->GetInterface(_T("IContainer")));
            ASSERT(pContainer);
            if( pContainer == NULL ) return NULL;
            if( !pContainer->Add(pControl) ) {
                delete pControl;
                continue;
            }
        }
        // Init default attributes
        if( pManager ) {
            pControl->SetManager(pManager, NULL, false);
            LPCTSTR pDefaultAttributes = pManager->GetDefaultAttributeList(pstrClass);
            if( pDefaultAttributes ) {
                pControl->ApplyAttributeList(pDefaultAttributes);
            }
        }
        // Process attributes
        if( node.HasAttributes() ) {
            TCHAR szValue[500] = { 0 };
            SIZE_T cchLen = lengthof(szValue) - 1;
            // Set ordinary attributes
            int nAttributes = node.GetAttributeCount();
            for( int i = 0; i < nAttributes; i++ ) {
                pControl->SetAttribute(node.GetAttributeName(i), node.GetAttributeValue(i));
            }

            // Very custom attributes
            if ( node.GetAttributeValue(_T("stretch"), szValue, cchLen) ) {
                if( pParent == NULL ) continue;

                if( pDialogLayout == NULL ) pDialogLayout = static_cast<CDialogLayoutUI*>(pParent->GetInterface(_T("DialogLayout")));
                ASSERT(pDialogLayout);
                if( pDialogLayout == NULL ) continue;

                UINT uMode = 0;
                if( _tcsstr(szValue, _T("move_x")) != NULL ) uMode |= UISTRETCH_MOVE_X;
                if( _tcsstr(szValue, _T("move_y")) != NULL ) uMode |= UISTRETCH_MOVE_Y;
                if( _tcsstr(szValue, _T("move_xy")) != NULL ) uMode |= UISTRETCH_MOVE_X | UISTRETCH_MOVE_Y;
                if( _tcsstr(szValue, _T("size_x")) != NULL ) uMode |= UISTRETCH_SIZE_X;
                if( _tcsstr(szValue, _T("size_y")) != NULL ) uMode |= UISTRETCH_SIZE_Y;
                if( _tcsstr(szValue, _T("size_xy")) != NULL ) uMode |= UISTRETCH_SIZE_X | UISTRETCH_SIZE_Y;
                if( _tcsstr(szValue, _T("group")) != NULL ) uMode |= UISTRETCH_NEWGROUP;
                if( _tcsstr(szValue, _T("line")) != NULL ) uMode |= UISTRETCH_NEWLINE;
                pDialogLayout->SetStretchMode(pControl, uMode);
            }
        }
        if( pManager ) {
            pControl->SetManager(NULL, NULL, false);
        }
        // Return first item
        if( pReturn == NULL ) pReturn = pControl;
    }
    return pReturn;
}

} // namespace DuiLib
