// HttpDownloader.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <exdisp.h>
#include <comdef.h>
#include "MainDlg.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);

	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + "skin");

	HRESULT Hr = ::CoInitialize(NULL);
	if( FAILED(Hr) ) return 0;

	CMainDlg* pFrame = new CMainDlg();
	if( pFrame == NULL ) return 0;
	pFrame->Create(NULL, _T("Download"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 800, 572);

	pFrame->CenterWindow();
	::ShowWindow(*pFrame, SW_SHOW);

	CPaintManagerUI::MessageLoop();

	::CoUninitialize();
	return 0;
}
