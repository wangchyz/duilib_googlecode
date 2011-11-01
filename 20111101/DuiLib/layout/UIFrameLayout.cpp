#include "stdafx.h"
#include "UIFrameLayout.h"

namespace DuiLib {

	CFrameLayoutUI::CFrameLayoutUI()
	{
	}

	CFrameLayoutUI::~CFrameLayoutUI()
	{
	}

	LPCTSTR CFrameLayoutUI::GetClass() const
	{
		return _T("FrameLayoutUI");
	}

	LPVOID CFrameLayoutUI::GetInterface( LPCTSTR pstrName )
	{
		if(_tcscmp(pstrName, _T("FrameLayout")) == 0)
			return static_cast<CFrameLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	void CFrameLayoutUI::SetPos( RECT rc )
	{
		CControlUI::SetPos(rc);

		RECT stClient = rc;
		CommonUtils::stInner(m_rcPaddings, stClient);

		int i;
		for(i = 0; i < m_items.GetSize(); i++)
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
			if(pControl->IsVisible())
			{
				RECT stRect = stClient;
				const CLayoutParams *pLayoutParams = pControl->GetLayoutParams();
				LONG nWidth = pLayoutParams->GetWidth(), nHeight = pLayoutParams->GetHeight();
				CommonUtils::stInner(pLayoutParams->GetMargins(), stRect);
				if(nWidth)
					stRect.right = stRect.left + nWidth;
				if(nHeight)
					stRect.bottom = stRect.top + nHeight;
				pControl->SetPos(stRect);
			}
		}
	}
}