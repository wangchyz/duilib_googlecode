#pragma once
#include "tinyxml.h"
#include "..\DuiLib\UIlib.h"
using DuiLib::CContainerUI;

enum CommandType
{
	cmdUndo,
	cmdRedo,
};
enum ActionType
{
	actionAdd,
	actionDelete,
	actionModify,
};

//////////////////////////////////////////////////////////////////////////
//CUICommandElement

class CUICommandNode;

class CUICommandElement
{
	friend CUICommandNode;

public:
	CUICommandElement(CArray<CControlUI*,CControlUI*>& arrSelected);
	CUICommandElement(const CUICommandElement& copy);

	~CUICommandElement();

public:
protected:
	TiXmlElement* m_pElementXml;
};

//////////////////////////////////////////////////////////////////////////
//CUICommandNode

class CUICommandHistory;

class CUICommandNode
{
	friend CUICommandHistory;

public:
	CUICommandNode(ActionType type);
	CUICommandNode(CUICommandElement* pBefore, CUICommandElement* pAfter, ActionType type);

	~CUICommandNode();

public:
	void Begin(CArray<CControlUI*,CControlUI*>& arrSelected);
	void End();

protected:
	BOOL RemoveSameProperties(CUICommandElement* pBefore, CUICommandElement* pAfter);

protected:
	ActionType m_ActionType;
	CUICommandElement* m_pBefore;
	CUICommandElement* m_pAfter;
	CArray<CControlUI*,CControlUI*>* m_pAllSelected;
};

//////////////////////////////////////////////////////////////////////////
//CUICommandHistory

class CUICommandHistory
{
public:
	CUICommandHistory(void);
	~CUICommandHistory(void);

public:
	void Begin(CArray<CControlUI*,CControlUI*>& arrSelected, ActionType type);
	void End();

	void Undo();
	void Redo();
	BOOL CanUndo();
	BOOL CanRedo();

protected:
	BOOL AddUICommand(CUICommandNode* pNode);
	void UICommandAction(CommandType type);

	void ActionAdd(CUICommandElement* pAfter);
	void ActionModify(CUICommandElement* pBefore, CUICommandElement* pAfter);
	void ActionDelete(CUICommandElement* pBefore);

protected:
	CUICommandNode* m_pNode;
	int m_nCommandIndex;
	CList<CUICommandNode*, CUICommandNode*> m_lstCommandNodes;
};