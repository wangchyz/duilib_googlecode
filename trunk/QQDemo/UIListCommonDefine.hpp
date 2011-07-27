//
// UIListCommonDefine.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
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

#ifndef UILISTCOMMONDEFINE_HPP
#define UILISTCOMMONDEFINE_HPP

#include <math.h>

namespace DuiLib
{

struct NodeData
{
	int level_;
	bool folder_;
	bool child_visible_;
	bool has_child_;
	tString text_;
	tString value;
	CListContainerElementUI* list_elment_;
};

double CalculateDelay(double state);

class Node
{
public:
	Node();
	explicit Node(NodeData t);
	Node(NodeData t, Node* parent);
	~Node();
	NodeData& data();
	int num_children() const;
	Node* child(int i);
	Node* parent();
	bool folder() const;
	bool has_children() const;
	void add_child(Node* child);
	void remove_child(Node* child);
	Node* get_last_child();

private:
	void set_parent(Node* parent);

private:
	typedef std::vector <Node*>	Children;

	Children	children_;
	Node*		parent_;

	NodeData    data_;
};

} // DuiLib

#endif // UILISTCOMMONDEFINE_HPP