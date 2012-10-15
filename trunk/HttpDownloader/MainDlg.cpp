#include "StdAfx.h"
#include <Commdlg.h>

#include "MainDlg.h"

CMainDlg::CMainDlg(void)
{
}

CMainDlg::~CMainDlg(void)
{
	::PostQuitMessage(0);
}

//线程函数
void Thread_Count(void *vpPtr)
{
	CMainDlg *Nv_Download_ = (CMainDlg*)vpPtr;

	while (TRUE)
	{
		Sleep(100);
		Nv_Download_->SendMessage(WM_USER_EDIT);

		if ((Nv_Download_->Nv_Download_Task_->detect_TaskComplete() == TRUE) ||
			(Nv_Download_->Nv_Download_Task_->get_TaskStop() == TRUE))
		{
			return;
		}
	}
}


void Thread_Download_Data(void *vpPtr)
{
	CMainDlg *Nv_Download_ = (CMainDlg*)vpPtr;

	while (TRUE)
	{
		Sleep(1000);
		Nv_Download_->SendMessage(WM_USER_PRINT);
		if ((Nv_Download_->Nv_Download_Task_->detect_TaskComplete() == TRUE) ||
			(Nv_Download_->Nv_Download_Task_->get_TaskStop() == TRUE))
		{
			return;
		}
	}
}


//线程函数
void Thread_OpenFileDlg_File(void *vpPtr)
{

	//主类this
	CMainDlg *pdlg = (CMainDlg*)vpPtr;
	memset(pdlg->m_strFilePath, 0, 1024);

	pdlg->OpenDlgFile(pdlg->m_strFilePath);

	if (strlen(pdlg->m_strFilePath) < 1)
	{
		return;
	}

	pdlg->m_pEditFilePath->SetText(pdlg->m_strFilePath);

	return;
}

void Thread_Download_Print(void *vpPtr)
{
	CMainDlg *Nv_Download_ = (CMainDlg*)vpPtr;

	while (TRUE)
	{
		Sleep(100);
		Nv_Download_->SendMessage(WM_USER_PROG);

		if ((Nv_Download_->Nv_Download_Task_->detect_TaskComplete() == TRUE) ||
			(Nv_Download_->Nv_Download_Task_->get_TaskStop() == TRUE))
		{
			return;
		}
	}
}

void CMainDlg::InitWindow()
{
	init_BindControls();
	init_ControlsData();
}

DuiLib::CDuiString CMainDlg::GetSkinFolder()
{
	return _T("skin\\HttpDownloader\\");
}

DuiLib::CDuiString CMainDlg::GetSkinFile()
{
	return _T("skin.xml");
}

LPCTSTR CMainDlg::GetWindowClassName( void ) const
{
	return _T("UIHttpDownloader");
}

void CMainDlg::Notify( TNotifyUI& msg )
{
	if(msg.sType == _T("click"))
	{
		//单击了这个按钮
		if( msg.pSender == m_pBtnClose) 
		{
			::PostQuitMessage(0);
			return; 
		}

		if (m_pBtnRun == msg.pSender)
		{
			m_pBtnOpenFile->SetVisible(false);
			m_pEditFilePath->SetVisible(false);
			m_pEditInput->SetVisible(false);
			m_pTextUrl->SetVisible(false);
			m_pTextOutFile->SetVisible(false);

			m_pTextPrint->SetVisible(true);
			m_pProgPrint->SetVisible(true);
			m_pBtnTieTu->SetVisible(true);

			CDuiString	strUrl = m_pEditInput->GetText();
			CDuiString	strFile = m_pEditFilePath->GetText();

			_Nv_Download *Nv_Download_T = new _Nv_Download;
			Nv_Download_Task_ = Nv_Download_T;
			//设置线程数
			Nv_Download_T->set_ThreadCount(10);
			//设置连接数
			Nv_Download_T->set_HttpLinkCount(12);
			//设置每个线程需要下载的字节 任务量
			m_pProgPrint->SetMinValue(0);
			Nv_Download_T->set_DownloadTaskCnt(205000);
			Nv_Download_T->Download(strUrl.GetData(), strFile.GetData(), NULL);

			m_pProgPrint->SetMaxValue(Nv_Download_T->n64FileSize_);


			(HANDLE)_beginthreadex(
				NULL, 
				0, 
				(unsigned int (__stdcall *)(void *))Thread_Count, 
				this, 
				0, 
				NULL
				);

			(HANDLE)_beginthreadex(
				NULL, 
				0, 
				(unsigned int (__stdcall *)(void *))Thread_Download_Print, 
				this, 
				0, 
				NULL
				);

			(HANDLE)_beginthreadex(
				NULL, 
				0, 
				(unsigned int (__stdcall *)(void *))Thread_Download_Data, 
				this, 
				0, 
				NULL
				);
		}

		if (m_pBtnStop == msg.pSender)
		{
			if (Nv_Download_Task_ == NULL)
			{
				return;
			}
			Nv_Download_Task_->thread_SecurityCloseTask();
			return;
		}

		if (m_pBtnOpenFile == msg.pSender)
		{
			::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_OpenFileDlg_File, this, 0, NULL);
			return;
		}
	}
}

LRESULT CMainDlg::HandleCustomMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	LRESULT lRes = 0;
	switch( uMsg )
	{
	case WM_USER_EDIT:	   lRes = OnUserEdit(uMsg, wParam, lParam, bHandled); break;
	case WM_USER_PROG:	   lRes = OnUserProg(uMsg, wParam, lParam, bHandled); break;
	case WM_USER_PRINT:	   lRes = OnUserPrint(uMsg, wParam, lParam, bHandled); break;
	}
	return lRes;
}

void CMainDlg::OnFinalMessage( HWND hWnd )
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

LRESULT CMainDlg::OnUserEdit( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	//CHAR strCnt[1024] = {0};
	//sprintf(strCnt, "已下载 %lld....", Nv_Download_Task_->get_DownloadCount());
	//m_pEditOut->SetText(strCnt);

	return 0;
}

LRESULT CMainDlg::OnUserProg( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	m_pProgPrint->SetValue(Nv_Download_Task_->get_DownloadCount());
	return 0;
}

LRESULT CMainDlg::OnUserPrint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CHAR strCnt[1024] = {0};
	sprintf(strCnt, "已下载 %0.2fMB, %lld/KBs, 还有:%s", 
		(float)((float)Nv_Download_Task_->get_DownloadCount() / (float)1024 / (float)1024),
		Nv_Download_Task_->get_DownloadSpeed(),
		Nv_Download_Task_->get_SurplusDownloadTime()
		);
	m_pTextPrint->SetText(strCnt);
	return 0;
}

void CMainDlg::set_WidthPos( IN CRichEditUI *pdlg, IN int nWidth )
{
	RECT rc = pdlg->GetPos();
	RECT rect;
	rect.top  = rc.top;
	rect.left = nWidth;

	//宽度 长度
	rect.bottom = pdlg->GetFixedHeight() + rect.top;
	rect.right = pdlg->GetFixedWidth() + rect.left;
	pdlg->SetPos(rect);
}

void CMainDlg::set_WidthPos( IN CButtonUI *pdlg, IN int nWidth )
{
	RECT rc = pdlg->GetPos();
	RECT rect;
	rect.top  = rc.top;
	rect.left = nWidth;

	//宽度 长度
	rect.bottom = pdlg->GetFixedHeight() + rect.top;
	rect.right = pdlg->GetFixedWidth() + rect.left;
	pdlg->SetPos(rect);
}

void CMainDlg::set_WidthPos( IN CTextUI *pdlg, IN int nWidth )
{
	RECT rc = pdlg->GetPos();
	RECT rect;
	rect.top  = rc.top;
	rect.left = nWidth;

	//宽度 长度
	rect.bottom = pdlg->GetFixedHeight() + rect.top;
	rect.right = pdlg->GetFixedWidth() + rect.left;
	pdlg->SetPos(rect);
}

void CMainDlg::set_Width( IN CRichEditUI *pdlg, IN int nWidth )
{
	RECT rc = pdlg->GetPos();
	RECT rect;
	rect.top  = rc.top;
	rect.left = rc.left;

	//宽度 长度
	rect.bottom = pdlg->GetFixedHeight() + rect.top;
	rect.right = pdlg->GetFixedWidth() + nWidth;
	pdlg->SetPos(rect);
}

void CMainDlg::set_Width( IN CHorizontalLayoutUI *pdlg, IN int nWidth )
{
	RECT rc = pdlg->GetPos();
	RECT rect;
	rect.top  = rc.top;
	rect.left = rc.left;

	//宽度 长度
	rect.bottom = pdlg->GetFixedHeight() + rect.top;
	rect.right = pdlg->GetFixedWidth() + nWidth;
	pdlg->SetPos(rect);
}

//初始化 按钮 等等
void CMainDlg::init_BindControls( void )
{
	//根据xml里面的字符串绑定类
	m_pBtnClose = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
	m_pBtnRun = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("run")));
	m_pBtnOpenFile = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("file")));
	m_pEditInput = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("editurl")));
	m_pEditFilePath	= static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("editoutFile")));
	m_pProgPrint = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("jdt")));
	m_pBtnTieTu = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("tietu")));
	m_pTextPrint = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("print")));
	m_pBtnStop = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("stop")));
	m_pTextUrl = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("url")));
	m_pTextOutFile = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("outFile")));
}

//初始化信息 数据
void CMainDlg::init_ControlsData( void )
{

}

void CMainDlg::OpenDlgFile( char *strPathName )
{
	OPENFILENAME    ofn		=	{0};
	char   szFile[1024] =	{0};

	// 开始设置OPENFILENAME结构成员
	ofn.lStructSize		=	sizeof(OPENFILENAME); 
	ofn.hwndOwner		=	this->m_hWnd;
	ofn.lpstrFile		=	szFile; 
	ofn.nMaxFile		=	1024; 
	ofn.lpstrFilter		=	"ESRI Shape文件(*.SHP)/0*.shp/0AutoCAD交换文件(*.DXF;*.DXF.GZ)/0*.dxf;*.dxf.gz /0所有图像文件/0*.jpg;*.jpeg;*.png;*.gif;*.bmp;*.tif;*.tiff/0所有文件(*.*)/0*.*/0/0";

	ofn.nFilterIndex	=	1;
	ofn.lpstrTitle		=   "请选择保存路径以及文件名称";
	ofn.lpstrInitialDir =	0;

	ofn.Flags	=	 OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	char strDwonPath[1024] = {0};

	if (::GetSaveFileName(&ofn))
	{
		strcpy(strPathName, ofn.lpstrFile);
	}

	return;
}

