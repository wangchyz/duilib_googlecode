#include "stdafx.h"
#include "UILinearLayout.h"

namespace DuiLib {
	CLinearLayoutUI::CLinearLayoutUI()
		: m_bVertical(0)
	{
	}

	CLinearLayoutUI::~CLinearLayoutUI()
	{
	}

	LPCTSTR CLinearLayoutUI::GetClass() const
	{
		return _T("LinearLayoutUI");
	}

	LPVOID CLinearLayoutUI::GetInterface( LPCTSTR pstrName )
	{
		if(_tcscmp(pstrName, _T("LinearLayout")) == 0)
			return static_cast<CLinearLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	void CLinearLayoutUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;

		int i;
		int nFixed = 0;
		int nTotalWeight = 0;

		SIZE szAvailable = { rc.right - rc.left - m_rcPaddings.left - m_rcPaddings.right, rc.bottom - rc.top - m_rcPaddings.top - m_rcPaddings.bottom };

		bool bMatchParent = false;
		for(i = 0; i < m_items.GetSize(); i++)
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);

			if(!pControl->IsVisible())
				continue;

			const CLayoutParams *pLayoutParams = pControl->GetLayoutParams();
			if(m_bVertical)
			{
				nFixed += pLayoutParams->GetHeight() +  pLayoutParams->GetMargins().top + pLayoutParams->GetMargins().bottom;
				bMatchParent = bMatchParent || pLayoutParams->IsHeightMatchParent();
			}
			else
			{
				nFixed += pLayoutParams->GetWidth() +  pLayoutParams->GetMargins().left + pLayoutParams->GetMargins().right;
				bMatchParent = bMatchParent || pLayoutParams->IsWidthMatchParent();
			}

			nTotalWeight += pControl->GetLayoutParams()->GetWeight();
		}

		RECT stRect = {rc.left + m_rcPaddings.left, rc.top + m_rcPaddings.top, rc.right - m_rcPaddings.right, rc.bottom - m_rcPaddings.bottom};
		int nRemaining = m_bVertical ? stRect.bottom - stRect.top - nFixed : stRect.right - stRect.left - nFixed;

		if(!m_bVertical && m_stGravity.IsCenterHorizontal() && !bMatchParent)
		{
			stRect.left = stRect.left + (stRect.right - stRect.left - nFixed) / 2;
			stRect.right = stRect.left + nFixed;
		}
		if(m_bVertical && m_stGravity.IsCenterVertical() && !bMatchParent)
		{
			stRect.top = stRect.top + (stRect.bottom - stRect.top - nFixed) / 2;
			stRect.bottom = stRect.top + nFixed;
		}

		for(i = 0; i < m_items.GetSize(); i++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[(m_bVertical ? m_stGravity.IsBottom() : m_stGravity.IsRight()) ? m_items.GetSize() - 1 - i : i]);

			if(!pControl->IsVisible())
				continue;

			const CLayoutParams *pLayoutParams = pControl->GetLayoutParams();

			SIZE sz = pControl->EstimateSize(szAvailable);

			if(m_bVertical)
			{
				LONG height = pLayoutParams->GetHeight();
				if(height == 0)
					height =  pLayoutParams->IsHeightMatchParent() ? nRemaining : (nTotalWeight ? nRemaining * pLayoutParams->GetWeight() / nTotalWeight : sz.cy);
				if(pLayoutParams->IsWidthMatchParent())
				{
					sz.cx = szAvailable.cx;
					pLayoutParams->MarginIn(sz);
				}
				else
					sz.cy = pLayoutParams->IsWidthWrapContent() ? sz.cx : pLayoutParams->GetWidth();
				sz.cy = height;
			}
			else
			{
				LONG width = pLayoutParams->GetWidth();
				if(width == 0)
					width =  pLayoutParams->IsWidthMatchParent() ? nRemaining : (nTotalWeight ? nRemaining * pLayoutParams->GetWeight() / nTotalWeight : sz.cx);
				if(pLayoutParams->IsHeightMatchParent())
				{
					sz.cy = szAvailable.cy;
					pLayoutParams->MarginIn(sz);
				}
				else
					sz.cy = pLayoutParams->IsHeightWrapContent() ? sz.cy : pLayoutParams->GetHeight();
				sz.cx = width;
			}

			pLayoutParams->MarginOut(sz);

			RECT stItem = doGravityLayout(m_stGravity, sz, m_bVertical, stRect);
			pLayoutParams->MarginIn(stItem);
			pControl->SetPos(stItem);
		}
	}

	RECT CLinearLayoutUI::doGravityLayout(const CLayoutGravity &stGravity, const SIZE &stSize, bool isVertical, RECT &stRect)
	{
		LONG width = stSize.cx;
		LONG height = stSize.cy;
		RECT stItem = stRect;

		if(stGravity.IsLeft())
			stItem.left = stRect.left;
		else if(stGravity.IsCenterHorizontal())
		{
			if(isVertical)
				stItem.left = stItem.left + (stRect.right - stRect.left - width) / 2;
			else
				stItem.left = stRect.left;
		}
		else if(stGravity.IsRight())
			stItem.left = stRect.right - width;

		if(stGravity.IsTop())
			stItem.top = stRect.top;
		else if(stGravity.IsCenterVertical())
		{
			if(isVertical)
				stItem.top = stRect.top;
			else
				stItem.top = stRect.top + (stRect.bottom - stRect.top - height) / 2;
		}
		else if(stGravity.IsBottom())
			stItem.top = stRect.bottom - height;

		stItem.right = stItem.left + width;
		stItem.bottom = stItem.top + height;

		if(isVertical)
		{
			stRect.top = (stGravity.IsTop() || stGravity.IsCenterVertical()) ? stItem.bottom : stRect.top;
			stRect.bottom = stGravity.IsBottom() ? stItem.top : stRect.bottom;
		}
		else
		{
			stRect.left = (stGravity.IsLeft() || stGravity.IsCenterHorizontal()) ? stItem.right : stRect.left;
			stRect.right = stGravity.IsRight() ? stItem.left : stRect.right;
		}
		return stItem;
	}

	void CLinearLayoutUI::SetAttribute( LPCTSTR lpszName, LPCTSTR lpszValue )
	{
		if(_tcscmp(lpszName, _T("gravity")) == 0)
			m_stGravity.SetGravity(lpszValue);
		else if(_tcscmp(lpszName, _T("vertical")) == 0)
			m_bVertical = CommonUtils::toBoolean(lpszValue);
		else if(_tcscmp(lpszName, _T("orientation")) == 0)
			m_bVertical = !_tcscmp(lpszValue, _T("vertical"));
		else
			CContainerUI::SetAttribute(lpszName, lpszValue);
	}
}