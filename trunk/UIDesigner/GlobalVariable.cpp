#include "StdAfx.h"
#include "GlobalVariable.h"

bool CGlobalVariable::m_bIsProjectExist = false;
CString CGlobalVariable::m_strProjectName;
CString CGlobalVariable::m_strProjectPath;
CString CGlobalVariable::m_strCurPath;

CGlobalVariable::CGlobalVariable(void)
{
}

CGlobalVariable::~CGlobalVariable(void)
{
}

CString& CGlobalVariable::GetCurPath()
{
	if(m_strCurPath.IsEmpty())
	{
		TCHAR szFileName[MAX_PATH] = {0};
		::GetModuleFileName(NULL, szFileName, MAX_PATH);
		m_strCurPath = szFileName;
		int nPos = m_strCurPath.ReverseFind('\\');
		if(nPos != -1)
			m_strCurPath = m_strCurPath.Left(nPos + 1);
	}

	return m_strCurPath;
}