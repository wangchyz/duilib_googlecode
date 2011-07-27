//
// UIMicroBlog.hpp
// ~~~~~~~~~~~~~~~
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

#ifndef UIMICROBLOG_HPP
#define UIMICROBLOG_HPP

namespace DuiLib
{
class CMicroBlogUI : public CListUI
{
public:
	CMicroBlogUI(CPaintManagerUI& paint_manager);
	~CMicroBlogUI();

private:
	CPaintManagerUI& paint_manager_;
};

} // DuiLib

#endif // UIMICROBLOG_HPP