//
// UIFriends.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2011 achellies (achellies at 163 dot com)
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef UIFRIENDS_HPP
#define UIFRIENDS_HPP

#include "UIListCommonDefine.hpp"

namespace DuiLib
{
struct FriendListItemInfo
{
	bool folder;
	bool empty;
	tString id;
	tString logo;
	tString nick_name;
	tString description;
};


class CFriendsUI : public CListUI
{
public:
	enum {SCROLL_TIMERID = 10};

	CFriendsUI(CPaintManagerUI& paint_manager);

	~CFriendsUI();

	bool Add(CControlUI* pControl);

	bool AddAt(CControlUI* pControl, int iIndex);

	bool Remove(CControlUI* pControl);

	bool RemoveAt(int iIndex);

	void RemoveAll();

	void DoEvent(TEventUI& event);

	Node* GetRoot();

	Node* AddNode(const FriendListItemInfo& item, Node* parent = NULL);

	void RemoveNode(Node* node);

	void SetChildVisible(Node* node, bool visible);

	bool CanExpand(Node* node) const;

	bool SelectItem(int iIndex);

private:
	Node*	root_node_;
	LONG	delay_deltaY_;
	DWORD	delay_number_;
	DWORD	delay_left_;
	CRect	text_padding_;
	int level_text_start_pos_;
	tString level_expand_image_;
	tString level_collapse_image_;
	CPaintManagerUI& paint_manager_;
};

} // DuiLib

#endif // UIFRIENDS_HPP