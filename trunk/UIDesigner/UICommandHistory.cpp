#include "StdAfx.h"
#include "UICommandHistory.h"

//////////////////////////////////////////////////////////////////////////
//CUICommandElement

CUICommandElement::CUICommandElement(CArray<CControlUI*,CControlUI*>& arrSelected)
	: m_pElementXml(NULL)
{
	m_pElementXml = new TiXmlElement("ButtonUI");
	m_pElementXml->SetAttribute("name", "button");
}

CUICommandElement::CUICommandElement(const CUICommandElement& copy)
{
	m_pElementXml = new TiXmlElement(*copy.m_pElementXml);
}

CUICommandElement::~CUICommandElement()
{
	if(m_pElementXml)
		delete m_pElementXml;
}

//////////////////////////////////////////////////////////////////////////
//CUICommandNode

CUICommandNode::CUICommandNode(ActionType type)
	: m_ActionType(type), m_pBefore(NULL), m_pAfter(NULL), m_pAllSelected(NULL)
{

}

CUICommandNode::CUICommandNode(CUICommandElement* pBefore, CUICommandElement* pAfter, ActionType type)
	: m_ActionType(type), m_pBefore(NULL), m_pAfter(NULL), m_pAllSelected(NULL)
{
	m_pBefore = new CUICommandElement(*pBefore);
	m_pAfter = new CUICommandElement(*pAfter);

	switch(type)
	{
	case actionAdd:
		break;
	case actionModify:
		break;
	case actionDelete:
		break;
	}
}

CUICommandNode::~CUICommandNode()
{
	if(m_pBefore)
		delete m_pBefore;
	if(m_pAfter)
		delete m_pAfter;
}

void CUICommandNode::Begin(CArray<CControlUI*,CControlUI*>& arrSelected)
{
	ASSERT(m_pBefore == NULL && m_pAfter == NULL);
	ASSERT(m_pAllSelected == NULL);

	m_pAllSelected = &arrSelected;
	switch(m_ActionType)
	{
	case actionAdd:
		m_pBefore = NULL;
		break;
	case actionModify:
	case actionDelete:
		m_pBefore = new CUICommandElement(arrSelected);
		break;
	}
}

void CUICommandNode::End()
{
	ASSERT(m_pAfter == NULL);
	ASSERT(m_pAllSelected);

	switch(m_ActionType)
	{
	case actionAdd:
		m_pAfter = new CUICommandElement(*m_pAllSelected);
	case actionModify:
		m_pAfter = new CUICommandElement(*m_pAllSelected);
		RemoveSameProperties(m_pBefore, m_pAfter);
		break;
	case actionDelete:
		m_pAfter = NULL;
		break;
	}

	m_pAllSelected = NULL;
}

BOOL CUICommandNode::RemoveSameProperties(CUICommandElement* pBefore, CUICommandElement* pAfter)
{
	if(pBefore==NULL || pAfter==NULL || m_ActionType!=actionModify)
		return FALSE;
// 	if(pBefore->m_pElementXml)

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CUICommandHistory

CUICommandHistory::CUICommandHistory(void)
	: m_pNode(NULL), m_nCommandIndex(0)
{
}

CUICommandHistory::~CUICommandHistory(void)
{
	while(!m_lstCommandNodes.IsEmpty())
		delete m_lstCommandNodes.RemoveHead();
}

void CUICommandHistory::Begin(CArray<CControlUI*,CControlUI*>& arrSelected, ActionType type)
{
	ASSERT(m_pNode == NULL);

	m_pNode = new CUICommandNode(type);
	m_pNode->Begin(arrSelected);
}

void CUICommandHistory::End()
{
	ASSERT(m_pNode);

	m_pNode->End();
	AddUICommand(m_pNode);
	m_pNode = NULL;
}

BOOL CUICommandHistory::AddUICommand(CUICommandNode* pNode)
{
	switch(pNode->m_ActionType)
	{
	case actionAdd:
		if(pNode->m_pAfter == NULL)
			return FALSE;
	case actionModify:
		if(pNode->m_pBefore==NULL || pNode->m_pAfter==NULL)
			return FALSE;
		break;
	case actionDelete:
		if(pNode->m_pBefore == NULL)
			return FALSE;
		break;
	}

	int count = m_lstCommandNodes.GetCount();
	while(m_nCommandIndex < count--)
		delete m_lstCommandNodes.RemoveTail();
	if(m_lstCommandNodes.GetCount() >= UI_COMMAND_HISTORY)
		delete m_lstCommandNodes.RemoveHead();
	m_lstCommandNodes.AddTail(pNode);
	m_nCommandIndex = m_lstCommandNodes.GetCount();

	return TRUE;
}

void CUICommandHistory::Undo()
{
	UICommandAction(cmdUndo);
}

void CUICommandHistory::Redo()
{
	UICommandAction(cmdRedo);
}

void CUICommandHistory::UICommandAction(CommandType type)
{
	CUICommandNode* pOldNode;
	CUICommandNode* pNewNode;

	if(type == cmdRedo)
	{
		if(!CanRedo())
			return;

		POSITION pos = m_lstCommandNodes.FindIndex(m_nCommandIndex);
		pOldNode = m_lstCommandNodes.GetAt(pos);
		pNewNode = new CUICommandNode(pOldNode->m_pBefore, pOldNode->m_pAfter, pOldNode->m_ActionType);
		m_nCommandIndex++;
	}
	else
	{
		if(!CanUndo())
			return;

		m_nCommandIndex--;
		pOldNode = m_lstCommandNodes.GetTail();
		ActionType action;
		switch(pOldNode->m_ActionType)
		{
		case actionAdd:
			action = actionDelete;
			break;
		case actionModify:
			action = actionModify;
			break;
		case actionDelete:
			action = actionAdd;
			break;
		}
		pNewNode = new CUICommandNode(pOldNode->m_pAfter, pOldNode->m_pBefore, action);
	}

	switch(pNewNode->m_ActionType)
	{
	case actionAdd:
		ActionAdd(pNewNode->m_pAfter);
		break;
	case actionModify:
		ActionModify(pNewNode->m_pBefore, pNewNode->m_pAfter);
		break;
	case actionDelete:
		ActionDelete(pNewNode->m_pBefore);
		break;
	}

	delete pNewNode;
}

BOOL CUICommandHistory::CanUndo()
{
	return m_nCommandIndex != 0;
}

BOOL CUICommandHistory::CanRedo()
{
	return m_nCommandIndex != m_lstCommandNodes.GetCount();
}

void CUICommandHistory::ActionAdd(CUICommandElement* pAfter)
{

}

void CUICommandHistory::ActionModify(CUICommandElement* pBefore, CUICommandElement* pAfter)
{

}

void CUICommandHistory::ActionDelete(CUICommandElement* pBefore)
{

}