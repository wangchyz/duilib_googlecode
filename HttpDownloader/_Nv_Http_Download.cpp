#include "StdAfx.h"
#include "_Nv_Http_Download.h"



//////////////////////////////////////////////////////////////////////////
//线程函数
void Thread_Download_Task(void *vpPtr)
{
	_Nv_Download_Task *Nv_Download_ = (_Nv_Download_Task*)vpPtr;
	
	//下载接收数据
	BOOL bRet = Nv_Download_->NvDownload_Task(Nv_Download_->strFilePath_.c_str());

	if (bRet == FALSE)
	{
		//有个线程下载出现错误
		//安全关闭 或 打开迅雷
		Nv_Download_->Nv_Download_Ptr_->thread_SecurityCloseTask();
	}

	//暂停下载
	DWORD dwRet = WaitForSingleObject(Nv_Download_->Handle_Stop_Download_, 0);

	//触发成功
	if (dwRet == WAIT_OBJECT_0)
	{
		DegMsg("线程安全关闭");
		Nv_Download_->Handle_Stop_Download_ = NULL;
		Nv_Download_->NvInternetCloseHandle(NULL);
		//关闭线程 设置代码
		_endthreadex(Thread_download_Out);
		return;
	}
	
	//任务完成
	Nv_Download_->NvInternetCloseHandle(NULL);
	Nv_Download_->bTaskOk_ = TRUE;
	_endthreadex(Thread_download_Ok);
}

//线程函数
void Thread_Download(void *vpPtr)
{
	_Nv_Download *Nv_Download_ = (_Nv_Download*)vpPtr;
	
	if (Nv_Download_->strCoolie_.length() != 0)
		Nv_Download_->thread_Task(Nv_Download_->strUrl_.c_str(), Nv_Download_->strFileSavePath_.c_str(), Nv_Download_->strCoolie_.c_str());
	else
		Nv_Download_->thread_Task(Nv_Download_->strUrl_.c_str(), Nv_Download_->strFileSavePath_.c_str(), NULL);

	_endthreadex(Thread_Download_Fun);
}


//线程函数
void Thread_DetectTask(void *vpPtr)
{
	_Nv_Download *Nv_Download_ = (_Nv_Download*)vpPtr;

	while (TRUE)
	{
		//
		DWORD dwRet = WaitForSingleObject(Nv_Download_->Handle_DetectTask_Out_, 0);

		//触发成功
		if (dwRet == WAIT_OBJECT_0)
		{
			//销毁线程句柄
			BOOL bRet = CloseHandle(Nv_Download_->Handle_DetectTask_Out_);

			Nv_Download_->Handle_DetectTask_Out_ = NULL;
			//关闭线程 设置代码
			_endthreadex(Thread_download_Ok);
			return;
		}

		Nv_Download_->detect_vtThread_Task();
		Sleep(10);
	}
}



//////////////////////////////////////////////////////////////////////////
//************************************************************************
//
// 函数:	
// 参数:	
// 返回值:	
// 功能:	构造
//
//
//************************************************************************
_Nv_Download_Task::_Nv_Download_Task()
{
	Init();
}




//************************************************************************
//
// 函数:	
// 参数:	
// 返回值:	
// 功能:	接口 构造 下载任务
//
//
//************************************************************************
_Nv_Download_Task::_Nv_Download_Task(
							__in CONST CHAR *strDownloadUrl, 
							__in CONST CHAR *strSaveFilePath, 
							__in __int64 _n64X,
							__in __int64 _n64Size,
							__out __int64 *n64CallCnt,
							__in CONST CHAR *strCoolie,
							__in BOOL bFalg
)
{
	Init();

	n64CallByteCnt_ = n64CallCnt;

	strUrl_ = strDownloadUrl;
	strFilePath_ = strSaveFilePath;

	//偏移量
	n64X_ = _n64X;
	
	//
	set_Coolie(strCoolie);

	if (bFalg == Run_Mode_2)
	{	
		n64DownloadSize_ = _n64Size;
	}

	Thread_Task();
}



//************************************************************************
//
// 函数:	
// 参数:	
// 返回值:	
// 功能:	
//
//
//************************************************************************
_Nv_Download_Task::~_Nv_Download_Task()
{

}



//************************************************************************
//
// 函数:	Init
// 参数:	
// 返回值:	
// 功能:	初始化类的变量
//
//
//************************************************************************
VOID _Nv_Download_Task::Init(VOID)
{
	memset(strFileName_, 0, sizeof(strFileName_));
	memset(strHostName_, 0, sizeof(strHostName_));

	n64DownloadSize_ = 0;

	Handle_Stop_Download_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	bStop_Download_ = FALSE;

	dwSleep_ = NULL;

	strUrl_ = "";
	strFilePath_ = "";

	n64FileSize_ = 0;

	n64X_ = n64Y_ = 0;

	n64_Download_Byte_Cnt_ = 0;

	bTaskOk_ = FALSE;

	bCoolie_ = FALSE;

	strCoolie_ = "";

	Handle_Download_Ok = NULL;

	Nv_Download_Ptr_ = NULL;
}



//************************************************************************
//
// 函数:	Thread_Task
// 参数:	
// 返回值:	成功返回TRUE 否则FALSE
// 功能:	任务下载
//
//
//************************************************************************
BOOL _Nv_Download_Task::Thread_Task(VOID)
{
	int nRetCode = 0;

	if (strCoolie_.length() != 0)
		nRetCode = GetHttpReturnData(strUrl_.c_str(), strFilePath_.c_str(), n64X_, n64Y_, strCoolie_.c_str());
	else
		nRetCode = GetHttpReturnData(strUrl_.c_str(), strFilePath_.c_str(), n64X_, n64Y_, NULL);

	if (nRetCode == Http_Return_206)
	{
		//开始接收数据
		Handle_Download_Ok = (HANDLE)_beginthreadex(
			NULL, 
			0, 
			(unsigned int (__stdcall *)(void *))Thread_Download_Task, 
			this, 
			0, 
			NULL
			);

		return TRUE;
	}

	return FALSE;
}

//************************************************************************
//
// 函数:	NvInternetOpen
// 参数:	__in char *strHttpName 输入HTTP请求名字 可以随意 __in int nFalg 输入规则
// 返回值:	成功返回TRUE 否则FALSE
// 功能:	调用系统API InternetOpenA
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvInternetOpen(__in CONST CHAR *strHttpName, __in INT nFalg)
{
	Hinternet_Main_ = InternetOpenA(strHttpName, nFalg, NULL, NULL, 0);

	if (Hinternet_Main_ == NULL)
	{
		dwError_ = ::GetLastError();
		return FALSE;
	}

	return TRUE;
}



//************************************************************************
//
// 函数:	NvInternetOpenUrl
// 参数:	__in char *strUrl 要打开的url  int nFalg 规则
// 返回值:	成功返回TRUE 否则FALSE
// 功能:	调用系统API InternetOpenA
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvInternetOpenUrl(__in CONST CHAR *strUrl, __in INT nFalg)
{
	Hinternet_OpenUrl_ = InternetOpenUrl(Hinternet_Main_, strUrl, NULL, 0, nFalg, 0);

	if (Hinternet_OpenUrl_ == NULL)
	{
		return FALSE;
	}

	return TRUE;

}



//************************************************************************
//
// 函数:	NvInternetConnect
// 参数:	__in char *strHostName 输入hostname 该名字从NvAnalyzeURL函数构造
// 返回值:  成功返回TRUE 否则FALSE
// 功能:	调用系统API InternetConnectA
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvInternetConnect(__in CHAR *strHostName)
{
	Hinternet_Connect_ = InternetConnectA(Hinternet_Main_, strHostName, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	
	if (Hinternet_Connect_ == NULL)
	{
		dwError_ = ::GetLastError();
		return FALSE;
	}

	return TRUE;
}

//************************************************************************
//
// 函数:	NvAnalyzeURL
// 参数:    __in char *strUrl 输入下载地址URL
// 返回值:	成功返回TRUE 否则FALSE
// 功能:	解析URL
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvAnalyzeURL(__in CONST CHAR *strUrl)
{
	URL_COMPONENTSA stURLComponents;
	ZeroMemory(&stURLComponents, sizeof(stURLComponents));

	stURLComponents.dwStructSize = sizeof(stURLComponents);
	stURLComponents.dwExtraInfoLength	= 1;
	stURLComponents.dwHostNameLength	= 1;
	stURLComponents.dwPasswordLength	= 1;
	stURLComponents.dwSchemeLength		= 1;
	stURLComponents.dwUrlPathLength		= 1;
	stURLComponents.dwUserNameLength	= 1;

	if (!InternetCrackUrlA(strUrl, 0, 0, &stURLComponents) || 
		stURLComponents.nScheme != INTERNET_SCHEME_HTTP || stURLComponents.dwUrlPathLength == 1)
	{
		dwError_ = ::GetLastError();
		return FALSE;
	}

	// 获得文件所在服务器的服务器名
	ZeroMemory(strHostName_, sizeof(strHostName_));
	ZeroMemory(strFileName_, sizeof(strFileName_));

	strncpy(strHostName_, 
		stURLComponents.lpszHostName, 
		min(sizeof(strHostName_) / sizeof(char) - 1, 
		stURLComponents.dwHostNameLength)
		);
	strcpy(strFileName_, stURLComponents.lpszUrlPath);
	return TRUE;
}



//************************************************************************
//
// 函数:	NvHttpOpenRequest
// 参数:	__in const char *strAcceptTypes HTTP报文Accept字段 __in char *strFileName 从NvAnalyzeURL函数构造 __in DWORD dwFalg 是否使用coolie
//			__in const char *strHttpType 缺省。
// 返回值:  成功返回TRUE 否则FALSE
// 功能:    调用系统API HttpOpenRequestA
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvHttpOpenRequest(__in CONST CHAR *strAcceptTypes, __in CHAR *strFileName, __in CONST CHAR *strHttpType, __in DWORD dwFalg)
{
	if (dwFalg != INTERNET_FLAG_NO_COOKIES)
	{
      Hinternet_Request_ = HttpOpenRequestA(Hinternet_Connect_, 
			("GET"), 
			strFileName, 
			HTTP_VERSIONA, 
			NULL,
			&strAcceptTypes,
			INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION,
			0
		  );
	}
	else
	{
		Hinternet_Request_ = HttpOpenRequestA(Hinternet_Connect_, 
			("GET"), 
			strFileName, 
			HTTP_VERSIONA, 
			NULL,
			&strAcceptTypes,
			INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION,
			INTERNET_FLAG_NO_COOKIES
			);
	}

	if (Hinternet_Request_ == NULL)
	{
		dwError_ = ::GetLastError();
		return FALSE;
	}
	
	return TRUE;
}



//************************************************************************
//
// 函数:	NvHttpSendRequest
// 参数:	HINTERNET hRequest 从NvHttpOpenRequest 或其他函数构造的句柄
// 返回值:  成功返回TRUE 否则FALSE
// 功能:    调用系统API HttpSendRequestA 发送报文
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvHttpSendRequest(VOID)
{
	BOOL bRet = ::HttpSendRequestA(Hinternet_Request_, NULL, 0, 0, 0);

	if (bRet == FALSE)
	{
		dwError_ = ::GetLastError();
		return bRet;
	}

	return bRet;
}



//************************************************************************
//
// 函数:	NvHttpAddRequestHeaders
// 参数:	__in HINTERNET hRequest 从NvHttpOpenRequest 或其他函数构造的句柄 
//			__in DWORD dwMinPos, __in DWORD dwMaxPos 下载点  __in int nFalg 规则
// 返回值:	成功返回TRUE 否则FALSE
// 功能:	构造一个断点传送的下载点 x - y
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvHttpAddRequestHeaders(__in HINTERNET hRequest, __in __int64 _n64MinPos, __in __int64 _n64MaxPos, __in BOOL bFalg)
{
	char strPos[128] = {0};
	
	if (n64DownloadSize_ == 0)
	{
		sprintf(strPos, ("Range:bytes=%lld-\r\n"), _n64MinPos);
	}
	else
	{	
		(_n64MaxPos == 0) && (_n64MaxPos=_n64MinPos);
		sprintf(strPos, ("Range:bytes=%lld-%lld\r\n"), _n64MinPos, _n64MaxPos + n64DownloadSize_);
	}

	BOOL bRet = HttpAddRequestHeadersA(hRequest, strPos, -1, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);

	if (bRet == FALSE)
	{
		dwError_ = ::GetLastError();
		return bRet;
	}

	return bRet;
}



//************************************************************************
//
// 函数:	NvHttpQueryInfo 
// 参数:	__in HINTERNET hRequest, 从NvHttpOpenRequest 或其他函数构造的句柄 __in int nFalg 状态码
// 返回值:	返回状态代码
// 功能:	查询一个返回的HTTP报文中的状态数据
//
//
//************************************************************************
__int64 _Nv_Download_Task::NvHttpQueryInfo(__in HINTERNET hRequest, __in int nFalg)
{
	char strBuf[1024] = {0};
	DWORD dwLen = sizeof(strBuf);
	if (FALSE == ::HttpQueryInfoA(hRequest, nFalg, strBuf, &dwLen , 0))
	{
		dwError_ = ::GetLastError();
		return 0;
	}

	__int64 int64Ret = _atoi64(strBuf);

	return int64Ret;
}



//************************************************************************
//
// 函数:	NvHttpQueryInfo 
// 参数:	__in HINTERNET hRequest, 从NvHttpOpenRequest 或其他函数构造的句柄 __in int nFalg 状态码 OUT char *strFileName 保存文件名
// 返回值:	返回状态代码
// 功能:	查询一个返回的HTTP报文中的状态数据
//
//
//************************************************************************
void _Nv_Download_Task::NvHttpQueryInfo(__in HINTERNET hRequest, __in INT nFalg, OUT CHAR *strFileName)
{
	char strBuf[1024] = {0};
	DWORD dwLen = sizeof(strBuf);
	if (FALSE == ::HttpQueryInfoA(hRequest, nFalg, strBuf, &dwLen , 0))
	{
		strFileName = NULL;
		dwError_ = ::GetLastError();
		return;
	}

	strcpy(strFileName, strBuf);

	return;
}



//************************************************************************
//
// 函数:	NvInternetCloseHandle
// 参数:	__in HINTERNET hDelete 是否断开连接
// 返回值:	成功返回TRUE 否则FALSE
// 功能:	删除所有句柄
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvInternetCloseHandle(__in HINTERNET hDelete)
{
	if (hDelete == NULL)
	{
		Hinternet_Main_ != NULL && (InternetCloseHandle(Hinternet_Main_), Hinternet_Main_ = NULL);
		Hinternet_Connect_ != NULL && (InternetCloseHandle(Hinternet_Connect_), Hinternet_Connect_ = NULL);
		Hinternet_Request_ != NULL && (InternetCloseHandle(Hinternet_Request_), Hinternet_Request_ = NULL);
	}
	else
	{
		InternetCloseHandle(hDelete);
	}

	return TRUE;
}



//************************************************************************
//
// 函数:	GetHttpReturnData
// 参数:	__in char *strUrl 下载的URL __in char *strFileSavePath 文件保存路径
//			__in DWORD dwMinPos 下载点 __in DWORD dwMaxPos 下载点 __in BOOL bCoolie 是否有coolie __in char *strCoolie coolie内容
// 返回值:	返回状态码
// 功能:	测试一个服务器是否支持 断点续传
//
//
//************************************************************************
DWORD _Nv_Download_Task::GetHttpReturnData(
								__in	CONST CHAR	*strUrl, 
								__in	CONST CHAR	*strFileSavePath, 
								__in	__int64 dwMinPos, 
								__in	__int64 dwMaxPos,
								__in	CONST CHAR	*strCoolie
)
{
	//随机生成一个HTTP 名称
	LARGE_INTEGER   t1;
	QueryPerformanceCounter(&t1);
	char strTimeName[64] = {0};
	sprintf(strTimeName, ("NV-%d"), t1.QuadPart);
	
	//连接
	NvInternetOpen(strTimeName, INTERNET_OPEN_TYPE_PRECONFIG);
	//解析URL
	NvAnalyzeURL(strUrl);
	//连接
	NvInternetConnect(strHostName_);

		//构造报文
	if (strCoolie == FALSE)
	{
		NvHttpOpenRequest(NULL, strFileName_, "HTTP/1.1", 0);
		//添加一个报文命令
		HttpAddRequestHeadersA(Hinternet_Request_, "Accept: */*\r\n", -1, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
	}
	else
	{
		NvHttpOpenRequest(NULL, strFileName_, "HTTP/1.1", INTERNET_FLAG_NO_COOKIES);
		//添加一个报文命令
		HttpAddRequestHeadersA(Hinternet_Request_, "Accept: */*\r\n", -1, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
		//添加一个报文命令
		HttpAddRequestHeadersA(Hinternet_Request_, strCoolie, -1, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
	}

	//设置断点续传
	NvHttpAddRequestHeaders(Hinternet_Request_, dwMinPos, dwMaxPos, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
	
	//发送报文
	NvHttpSendRequest();
	
	//测试状态
	__int64 int64Recv = NvHttpQueryInfo(Hinternet_Request_, HTTP_QUERY_STATUS_CODE);

	if (int64Recv == Http_Return_206)
	{
		//连接成功
		return Http_Return_206;
	}
	else
	{
		//错误 销毁
		dwError_ = ::GetLastError();
		NvInternetCloseHandle(NULL);
		return 0xFFFF;
	}
}


//************************************************************************
//
// 函数:	NvGetFileSize
// 参数:	__in char *strUrl 下载地址 URL __in BOOL bCoolie 是否有coolie __in char *strCoolie coolie内容
// 返回值:	返回文件大小 错误返回0xFFFF
// 功能:	获取远程文件大小
//
//
//************************************************************************
__int64 _Nv_Download_Task::NvGetFileSize(__in CONST CHAR *strUrl, OUT CHAR *strFileName, __in BOOL bCoolie, __in CONST CHAR *strCoolie)
{
	
	//随机生成一个HTTP 名称
	LARGE_INTEGER   t1;
	QueryPerformanceCounter(&t1);
	char strTimeName[64] = {0};
	sprintf(strTimeName, ("NV-%d"), t1.QuadPart);
	
	//连接
	NvInternetOpen(strTimeName, INTERNET_OPEN_TYPE_PRECONFIG);
	//解析URL
	NvAnalyzeURL(strUrl);
	//连接
	NvInternetConnect(strHostName_);

		//构造报文
	if (bCoolie == FALSE)
	{
		NvHttpOpenRequest(NULL, strFileName_, "HTTP/1.1", 0);
		//添加一个报文命令
		HttpAddRequestHeadersA(Hinternet_Request_, "Accept: */*\r\n", -1, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
	}
	else
	{
		NvHttpOpenRequest(NULL, strFileName_, "HTTP/1.1", INTERNET_FLAG_NO_COOKIES);
		//添加一个报文命令
		HttpAddRequestHeadersA(Hinternet_Request_, "Accept: */*\r\n", -1, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
		//添加一个报文命令
		HttpAddRequestHeadersA(Hinternet_Request_, strCoolie, -1, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
	}

	//发送报文
	NvHttpSendRequest();

	//延时
	Sleep(500);
	//获取状态码 HTTP_QUERY_FLAG_NUMBER| HTTP_QUERY_STATUS_CODE);
	__int64 int64Recv = NvHttpQueryInfo(Hinternet_Request_, HTTP_QUERY_STATUS_CODE); 

	if (int64Recv == 403 || int64Recv == 404)
	{
		NvInternetCloseHandle(NULL);
		return 0xFFFF;
	}

	//获取文件大小
	__int64 int64FileSize = NvHttpQueryInfo(Hinternet_Request_, HTTP_QUERY_CONTENT_LENGTH);

	if (strFileName != NULL && int64Recv == 302)
	{
		//获取文件名
		NvHttpQueryInfo(Hinternet_Request_, HTTP_QUERY_LOCATION, strFileName);
	}
	else if (strFileName != NULL)
	{
		//获取文件名
		NvHttpQueryInfo(Hinternet_Request_, HTTP_QUERY_CONTENT_DISPOSITION, strFileName);
	}

	NvInternetCloseHandle(NULL);
	
	//文件大小为0 错误 设置错误代码
	if (int64FileSize <= 0)
	{
		dwError_ = ::GetLastError();
	}

	return int64FileSize;
}




//************************************************************************
//
// 函数:	NvDownload_Task
// 参数:	__in CONST CHAR *strSaveFilePath 保存文件路径
// 返回值:	完成任务或用户控制 返回TRUE 否则返回FALSE
// 功能:	开始接收下载字节
//
//
//************************************************************************
BOOL _Nv_Download_Task::NvDownload_Task(__in CONST CHAR *strSaveFilePath)
{
	//有下载指令
	//m_nTaskOk = -1;

	//打开文件开始写入数据
	HANDLE hFile = CreateFileA(strSaveFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwError_ = ::GetLastError();
		return FALSE;
	}

	// 根据断点位置处理文件大小
	LARGE_INTEGER largeFileSize;
	largeFileSize.QuadPart = n64X_;
	SetFilePointer(hFile, largeFileSize.LowPart, &largeFileSize.HighPart, FILE_BEGIN);

	//开始下载

	//错误
	BOOL	bRecv	= FALSE;
	BOOL	bWrite	= FALSE;
	DWORD	dwError = 0;
	//实际收到
	DWORD	dwRecvSize;
	//写入数据
	DWORD	dwWriteSize = 1;
	//连接次数
	DWORD	dwLinkCnt = 0;
	
	//接收buf
	char	*strRecvBuf = new char[1024*2];

	if (NULL == strRecvBuf)
	{
		return FALSE;
	}

	while (dwWriteSize != 0)
	{
		bRecv = InternetReadFile(Hinternet_Request_, strRecvBuf, 2047, &dwRecvSize);
		dwError = ::GetLastError();
		
		//错误处理
		if (dwError == 12002)
		{
			//设置一个错误 这个错误由线程管理器重新执行该线程
			dwLinkCnt++;

			if (dwLinkCnt == 20) { 
				NvInternetCloseHandle(NULL); CloseHandle(hFile); 
				delete []strRecvBuf;  return FALSE; 
			}
			else continue;
			
		}

		bWrite = WriteFile(hFile, strRecvBuf, dwRecvSize, &dwWriteSize, NULL);

		//累计当前工作线程下载了多少字节
		n64_Download_Byte_Cnt_ += dwWriteSize;
		*n64CallByteCnt_ += dwWriteSize;
		
		//设置退出下载
		if (bStop_Download_ == TRUE)
		{
			DegMsg("设置退出下载...");
			Nv_Download_Ptr_->get_ThreadTaskData(n64X_, n64DownloadSize_, n64_Download_Byte_Cnt_);
			//退出
			//销毁资源
			delete []strRecvBuf;
			CloseHandle(hFile);
			NvInternetCloseHandle(NULL);
			return TRUE;
		}
	}
	
	delete []strRecvBuf;
	CloseHandle(hFile);
	NvInternetCloseHandle(NULL);

	return TRUE;
}




//************************************************************************
//
// 函数:	stop_Task
// 参数:	
// 返回值:	
// 功能:	关闭该下载任务
//
//
//************************************************************************
BOOL _Nv_Download_Task::stop_Task(VOID)
{
	//停止下载
	stop_Download();
	//关闭该下载任务
	SetEvent(Handle_Stop_Download_);
	//等待正常关闭
	DWORD dwOutCode;
	while (TRUE)
	{
		::GetExitCodeThread(Handle_Download_Ok, &dwOutCode);
		if ((dwOutCode == Thread_download_Out) || (Thread_download_Ok == dwOutCode))
		{
			return TRUE;
		}
		Sleep(5);
	}

	return FALSE;
}


//************************************************************************
//
// 函数:	stop_Download
// 参数:	
// 返回值:	
// 功能:	停止下载
//
//
//************************************************************************
VOID _Nv_Download_Task::stop_Download(VOID)
{
	bStop_Download_ = TRUE;
}



//************************************************************************
//
// 函数:	get_Download_Byte_Cnt
// 参数:	
// 返回值:	
// 功能:	
//
//
//************************************************************************
__int64 _Nv_Download_Task::get_Download_Byte_Cnt(VOID)
{
	return n64_Download_Byte_Cnt_;
}



////************************************************************************
////
//// 函数:	get_Download_X
//// 参数:	
//// 返回值:	
//// 功能:	获取该类下载的起始位置
////
////
////************************************************************************
//__int64 _Nv_Download_Task::get_Download_X(VOID)
//{
//	return n64X_;
//}
//
//
//
////************************************************************************
////
//// 函数:	get_Task_Yes
//// 参数:	
//// 返回值:	
//// 功能:	获取该类任务是否完成
////
////
////************************************************************************
//BOOL _Nv_Download_Task::get_Task_Yes(VOID)
//{
//	return bTaskOk_;
//}



//************************************************************************
//
// 函数:	set_Coolie
// 参数:	
// 返回值:	
// 功能:	设置Coolie
//
//
//************************************************************************
VOID _Nv_Download_Task::set_Coolie(__in CONST CHAR *strCoolie)
{
	if (strCoolie != NULL)
	{
		strCoolie_ = strCoolie;
		bCoolie_ = TRUE;
	}
}



//************************************************************************
//
// 函数:	get_DownloadThisPtr
// 参数:	
// 返回值:	
// 功能:	获取下载管理器类指针
//
//
//************************************************************************
BOOL _Nv_Download_Task::get_DownloadThisPtr(__in _Nv_Download *nv_this)
{
	if (nv_this != NULL)
	{
		Nv_Download_Ptr_ = nv_this;
		return TRUE;
	}

	return FALSE;
}





//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// _Nv_Download 类实现


//************************************************************************
//
// 函数:	
// 参数:	
// 返回值:	
// 功能:	使用类构造函数直接下载 将无法设置线程数量 线程默认工作量 它会采用默认设置
//
//
//************************************************************************
_Nv_Download::_Nv_Download( 
			__in CONST CHAR *strDownloadUrl, 
			__in CONST CHAR *strSaveFilePath,
			__in CONST CHAR *strCoolie,
			__in BOOL bModeFalg /* = Run_Mode_2 */ 
)
{
	Init();

	if (strCoolie != NULL)
	{
		strCoolie_ = strCoolie;
		bCoolie_ = TRUE;
	}

	//获取文件大小
	__int64 n64FileSize = get_TaskFileSize(strDownloadUrl);

	//测试断点续传文件
	read_DwonloadFile();
	
	//获取url
	strUrl_ = strDownloadUrl;
	//获取保存文件路径
	strFileSavePath_ = strSaveFilePath;

	//测试单线程下载任务
	//开始接收数据
	Handle_ThreadTask_ = (HANDLE)_beginthreadex(
		NULL, 
		0, 
		(unsigned int (__stdcall *)(void *))Thread_Download, 
		this, 
		0, 
		NULL
		);

	Handle_DetectTask_ = (HANDLE)_beginthreadex(
		NULL, 
		0, 
		(unsigned int (__stdcall *)(void *))Thread_DetectTask, 
		this, 
		0, 
		NULL
		);

}




//************************************************************************
//
// 函数:	
// 参数:	
// 返回值:	
// 功能:	构造
//
//
//************************************************************************
_Nv_Download::_Nv_Download()
{
	Init();
}



//************************************************************************
//
// 函数:	
// 参数:	
// 返回值:	
// 功能:	
//
//
//************************************************************************
_Nv_Download::~_Nv_Download()
{

}


//************************************************************************
//
// 函数:	Init
// 参数:	
// 返回值:	
// 功能:	初始化类变量
//
//
//************************************************************************
VOID _Nv_Download::Init()
{
	bCoolie_ = FALSE;
	strCoolie_ = "";

	vtThread_Task_.clear();
	vtTagTaskData.clear();
	
	//线程数量
	dwThreadCnt_ = 10;
	//线程工作量
	n64ThreadDownloadCnt_ = 204800;
	n64ThreadTaskSion_ = 0 - n64ThreadDownloadCnt_;

	n64FileSize_ = 0;

	strUrl_ = "";
	strFileSavePath_ = "";

	n64DownloadCount_ = 0;

	Handle_DetectTask_Out_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	Handle_ThreadTask_Out_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	n64JiDonwloadCnt_ = 0;

	n64TimeDownloadCnt_ = 0;

	dwIsSpeed_ = 0;

	bTaskStop_ = FALSE;
}



//************************************************************************
//
// 函数:	Download
// 参数:	
// 返回值:	
// 功能:	外部接口  下载
//
//
//************************************************************************
BOOL _Nv_Download::Download(__in CONST CHAR *strUrl, __in CONST CHAR *strFileSavePath, __in CONST CHAR *strCoolie, __in BOOL bModeFalg)
{
	if (strCoolie != NULL)
	{
		strCoolie_ = strCoolie;
		bCoolie_ = TRUE;
	}

	//获取文件大小
	__int64 n64FileSize = get_TaskFileSize(strUrl);

	//获取url
	strUrl_ = strUrl;
	//获取保存文件路径
	strFileSavePath_ = strFileSavePath;

	//测试断点续传文件
	read_DwonloadFile();

	//测试单线程下载任务
	//开始接收数据
	Handle_ThreadTask_ = (HANDLE)_beginthreadex(
		NULL, 
		0, 
		(unsigned int (__stdcall *)(void *))Thread_Download, 
		this, 
		0, 
		NULL
		);

	Handle_DetectTask_ = (HANDLE)_beginthreadex(
		NULL, 
		0, 
		(unsigned int (__stdcall *)(void *))Thread_DetectTask, 
		this, 
		0, 
		NULL
		);

	return TRUE;
}


//************************************************************************
//
// 函数:	get_TaskFileSize
// 参数:	__in CONST CHAR *strDownloadUrl  下载URL
// 返回值:	返回文件大小
// 功能:	获取要下载的文件大小
//
//
//************************************************************************
__int64 _Nv_Download::get_TaskFileSize(__in CONST CHAR *strDownloadUrl)
{
	_Nv_Download_Task *Nv_FileSize_ = new _Nv_Download_Task;
	__int64 n64FileSize = Nv_FileSize_->NvGetFileSize(strDownloadUrl, NULL, bCoolie_, strCoolie_.c_str());
	n64FileSize_ = n64FileSize;

	delete Nv_FileSize_;

	return n64FileSize;
}



//************************************************************************
//
// 函数:	call_AssignTasks
// 参数:	
// 返回值:	
// 功能:	设置下载属性
//
//
//************************************************************************
BOOL _Nv_Download::call_AssignTasks(__in __int64 &n641, __in __int64 &n642, __in __int64 &n643)
{
	if ((n641 + n642) == n643)
	{
		return FALSE;
	}
	if ((n641 + n642) < n643)
	{
		n641 += n642;
	}

	//最后
	if ((n641 + n642) > n643)
	{
		n642 = n643 - n641;
	}

	return TRUE;
}
//************************************************************************
//
// 函数:	thread_Task
// 参数:	
// 返回值:	
// 功能:	核心函数
//
//
//************************************************************************
BOOL _Nv_Download::thread_Task( 
			__in CONST CHAR *strDownloadUrl, 
			__in CONST CHAR *strSaveFilePath,
			__in CONST CHAR *strCoolie,
			__in BOOL bModeFalg /* = Run_Mode_2 */ 
)
{
	while(TRUE)
	{
		//任务线程创建已满？
		if (dwThreadCnt_ > vtThread_Task_.size())
		{
			//退出
			DWORD dwRet = WaitForSingleObject(Handle_ThreadTask_Out_, 0);

			//触发成功
			if (dwRet == WAIT_OBJECT_0)
			{
				DegMsg("thread_Task线程安全关闭");
				//停止任务检查
				stop_DetectTask();
				//关闭线程 设置代码
				_endthreadex(Thread_ThreadTask_Out);
				return TRUE;
			}
			
			//任务量分配
			if (FALSE == call_AssignTasks(n64ThreadTaskSion_, n64ThreadDownloadCnt_, n64FileSize_))
			{
				//下载完成
				break;
			}

			//断点续传任务
			__int64 n64GetTaskCnt = 0;
			__int64 n64RetCode = set_ThreadTaskData(n64ThreadTaskSion_, n64ThreadDownloadCnt_, n64GetTaskCnt);
			
			//创建一个工作线程任务;
			_Nv_Download_Task *Nv_Task_ = new _Nv_Download_Task(
				strDownloadUrl, strSaveFilePath, n64ThreadTaskSion_, 
					n64ThreadDownloadCnt_ - 1, &n64DownloadCount_, strCoolie
				);

			if ((n64RetCode != T_X) && (n64RetCode != T_Z))
			{
				//最后
				n64ThreadTaskSion_ = n64RetCode;
			}

			Nv_Task_->get_DownloadThisPtr(this);
			
			//
			if (Nv_Task_ != NULL)
			{
				//追踪该任务
				vtThread_Task_.push_back(Nv_Task_);
			}
		}

		Sleep(100);
	}
	
	//停止检查
	stop_DetectTask();

	return TRUE;
}



//************************************************************************
//
// 函数:	detect_vtThread_Task
// 参数:	
// 返回值:	
// 功能:	线程管理器检查函数 它判断一个线程是否已经完成自己的任务
//
//
//************************************************************************
VOID _Nv_Download::detect_vtThread_Task(VOID)
{
	DWORD dwSize = vtThread_Task_.size();

	for (int i=0; i<dwSize; ++i)
	{
		if (vtThread_Task_[i]->bTaskOk_ == TRUE)
		{
			//获取线程退出代码
			//检查退出代码
			DWORD dwOutCode = 0;
			::GetExitCodeThread(vtThread_Task_[i]->Handle_Download_Ok, &dwOutCode);
			if (dwOutCode == Thread_download_Ok)
			{
				//该线程已经安全退出
				delete vtThread_Task_[i];
				vtThread_Task_[i] = NULL;
				vtThread_Task_.erase(vtThread_Task_.begin() + i);
				dwSize = vtThread_Task_.size();
				break;
			}
		}
	}
}



//************************************************************************
//
// 函数:	stop_DetectTask
// 参数:	
// 返回值:	
// 功能:	关闭线程管理器检查线程
//
//
//************************************************************************
VOID _Nv_Download::stop_DetectTask(VOID)
{
	//关闭
	SetEvent(Handle_DetectTask_Out_);
}




//************************************************************************
//
// 函数:	stop_ThreadTask
// 参数:	
// 返回值:	
// 功能:	停止线程任务管理器
//
//
//************************************************************************
BOOL _Nv_Download::stop_ThreadTask(VOID)
{
	//关闭
	SetEvent(Handle_ThreadTask_Out_);
	
	DWORD dwOutCode;
	while (TRUE)
	{
		::GetExitCodeThread(Handle_ThreadTask_, &dwOutCode);
		if (dwOutCode == Thread_ThreadTask_Out)
		{
			return TRUE;
		}
		Sleep(5);
	}

	return FALSE;
}



//************************************************************************
//
// 函数:	get_DownloadCount
// 参数:	
// 返回值:	
// 功能:	获取当前已经下载多少字节
//
//
//************************************************************************
__int64 _Nv_Download::get_DownloadCount(VOID)
{
	return n64DownloadCount_ + n64JiDonwloadCnt_;
}



//************************************************************************
//
// 函数:	get_DownloadSpeed
// 参数:	
// 返回值:	
// 功能:	获取下载速率
//
//
//************************************************************************
__int64 _Nv_Download::get_DownloadSpeed(VOID)
{
	dwIsSpeed_ = (DWORD)(get_DownloadCount() - n64TimeDownloadCnt_) / 1024;
	set_TimeCount();
	return dwIsSpeed_;
}




//************************************************************************
//
// 函数:	set_TimeCount
// 参数:	
// 返回值:	
// 功能:	设置时间差
//
//
//************************************************************************
VOID _Nv_Download::set_TimeCount(VOID)
{
	n64TimeDownloadCnt_ = get_DownloadCount();
}


//************************************************************************
//
// 函数:	set_ThreadCount
// 参数:	
// 返回值:	
// 功能:	设置线程数
//
//
//************************************************************************
VOID _Nv_Download::set_ThreadCount(__in DWORD dwThreadCount)
{
	if (dwThreadCount > 12)
	{
		dwThreadCnt_ = 12;
	}
	else if(dwThreadCount == 0)
	{
		dwThreadCnt_ = 6;
	}
}



//************************************************************************
//
// 函数:	set_HttpLinkCount
// 参数:	
// 返回值:	
// 功能:	设置连接数
//
//
//************************************************************************
VOID _Nv_Download::set_HttpLinkCount(__in DWORD dwCount)
{
	HKEY hKey;
	::RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",0,KEY_WRITE,&hKey);
	::RegSetValueEx(hKey,"MaxConnectionsPer1_0Server",0,REG_DWORD,(CONST BYTE *)&dwCount, 4);
	::RegSetValueEx(hKey,"MaxConnectionsPerServer",0,REG_DWORD,(CONST BYTE *)&dwCount, 4);
	::RegCloseKey(hKey);
}



//************************************************************************
//
// 函数:	set_DownloadTaskCnt
// 参数:	
// 返回值:	
// 功能:	设置每个线程的任务量
//
//
//************************************************************************
VOID _Nv_Download::set_DownloadTaskCnt(__in DWORD dwDownloadTaskCnt)
{
	if (dwDownloadTaskCnt > 1020000/3)
	{
		n64ThreadDownloadCnt_ = 204800;
	}
	else if(dwDownloadTaskCnt == 0)
	{
		n64ThreadDownloadCnt_ = 204800;
	}
}


//************************************************************************
//
// 函数:	get_SurplusDownloadTime
// 参数:	无
// 返回值:	返回 剩余时间
// 功能:	计算剩余时间
//
//
//************************************************************************
CHAR *_Nv_Download::get_SurplusDownloadTime(VOID)
{
	
	//当前下载速率
	__int64	dwSize = dwIsSpeed_;

	if (dwSize == 0)
	{
		return "无法估算..";
	}

	//还有多少么有下载完
	__int64	dwOutSize = n64FileSize_ - get_DownloadCount();
	//大约还剩下多少秒
	double  bOutTime = (double)(dwOutSize / 1024 / (double)(dwSize));
	BOOL	bFeng = FALSE;

	DWORD	dwFeng = (DWORD)(bOutTime / 60.0);
	DWORD	dwShi  = 0;
	
	if (dwFeng >= 60)
	{
		bFeng	= TRUE;
		dwShi	= dwFeng / 60;
		dwFeng	= dwFeng % 60;
	}

	DWORD	dwMiao = (DWORD)bOutTime % 60;

	static char strOut[64] = {0};
	if (bFeng == TRUE)
		sprintf(strOut, "%d小时%d分钟%d秒", dwShi, dwFeng, dwMiao);
	else
		sprintf(strOut, "%d分钟%d秒", dwFeng, dwMiao);

	return strOut;
}


//************************************************************************
//
// 函数:	get_SurplusDownloadTime_Dword
// 参数:	无
// 返回值:	返回 剩余时间
// 功能:	计算剩余时间
//
//
//************************************************************************
DOUBLE _Nv_Download::get_SurplusDownloadTime_Dword(VOID)
{
	//当前下载速率
	__int64	dwSize = dwIsSpeed_;

	if (dwSize == 0)
	{
		return 0;
	}

	//还有多少么有下载完
	__int64	dwOutSize = n64FileSize_ - get_DownloadCount();
	//大约还剩下多少秒
	DOUBLE  bOutTime = (double)(dwOutSize / 1024 / (double)(dwSize));
	
	return bOutTime;
}




//************************************************************************
//
// 函数:	detect_TaskComplete
// 参数:	
// 返回值:	 
// 功能:	检查任务是否已完成
//
//
//************************************************************************
BOOL _Nv_Download::detect_TaskComplete(VOID)
{
	if (n64FileSize_ == get_DownloadCount())
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



//************************************************************************
//
// 函数:	thread_SecurityCloseTask
// 参数:	
// 返回值:	 
// 功能:	安全关闭线程
//
//
//************************************************************************
BOOL _Nv_Download::thread_SecurityCloseTask(VOID)
{
	//清理数据保器
	vtTagTaskData.clear();

	//设置停止
	bTaskStop_ = TRUE;

	//先停止线程管理器
	stop_ThreadTask();
	//停止所有正在运行的线程
	DWORD dwSize = vtThread_Task_.size();

	for (int i=0; i<dwSize; ++i)
	{
		vtThread_Task_[i]->stop_Task();
		DegMsg("已安全关闭一个线程。");
	}

	create_DwonloadFile();

	return TRUE;
}


//************************************************************************
//
// 函数:	read_DwonloadFile
// 参数:	
// 返回值:	
// 功能:	读取断点续传文件
//
//
//************************************************************************
BOOL _Nv_Download::read_DwonloadFile(VOID)
{
	//构造路径
	strDownloadFile_ = strFileSavePath_ + FILE_SAVE_STRCAT_BUF;

	FILE *fp = fopen(strDownloadFile_.c_str(), "r");

	if (fp != NULL)
	{
		DWORD dwSize = 0;
		fscanf(fp, "%u\n", &dwSize);
		//已经下载了多少
		fscanf(fp, "%lld\n", &n64JiDonwloadCnt_);
		
		_Nv_Tag_Download_Data Nv_Tag_Task_Data_;
		for (int i=0; i<dwSize; ++i)
		{
			fscanf(fp, "%lld\n%lld\n%lld\n", 
				&Nv_Tag_Task_Data_.n64X_, &Nv_Tag_Task_Data_.n64TaskCnt_, &Nv_Tag_Task_Data_.n64DownloadCnt_);

			vtTagTaskData.push_back(Nv_Tag_Task_Data_);
		}

		fclose(fp);

		//排序
		//greater
		sort(vtTagTaskData.begin(), vtTagTaskData.end(), std::less<_Nv_Tag_Download_Data>());
		//删掉断点续传文件
		return DeleteFile(strDownloadFile_.c_str());

	}

	return FALSE;
}




//************************************************************************
//
// 函数:	create_DwonloadFile
// 参数:	
// 返回值:	
// 功能:	创建断点续传文件
//
//
//************************************************************************
BOOL _Nv_Download::create_DwonloadFile(VOID)
{
	//构造路径
	strDownloadFile_ = strFileSavePath_ + FILE_SAVE_STRCAT_BUF;

	FILE *fp = fopen(strDownloadFile_.c_str(), "w+");

	if (fp != NULL)
	{
		//保存当前任务正在下载的任务数据
		DWORD dwSize = vtTagTaskData.size();

		//保存当前任务的线程数
		fprintf(fp, "%u\n", dwSize);

		fprintf(fp, "%lld\n", get_DownloadCount());

		for (int i=0; i<dwSize; ++i)
		{
			fprintf(fp, "%lld\n%lld\n%lld\n", 
				vtTagTaskData[i].n64X_, vtTagTaskData[i].n64TaskCnt_,vtTagTaskData[i].n64DownloadCnt_);
		}

		fclose(fp);
		return TRUE;
	}

	return FALSE;
}


//************************************************************************
//
// 函数:	get_ThreadTaskData
// 参数:	
// 返回值:	
// 功能:	获取线程的下载状态
//
//
//************************************************************************
VOID _Nv_Download::get_ThreadTaskData(__in __int64 n64X, __in __int64 n64TaskCnt, __in __int64 n64DownloadCnt)
{
	_Nv_Tag_Download_Data Nv_Tag_Task_Data_;
	Nv_Tag_Task_Data_.n64X_ = n64X;
	Nv_Tag_Task_Data_.n64DownloadCnt_ = n64DownloadCnt;
	Nv_Tag_Task_Data_.n64TaskCnt_ = n64TaskCnt + 1;

	vtTagTaskData.push_back(Nv_Tag_Task_Data_);

	return;
}



//************************************************************************
//
// 函数:	set_ThreadTaskData
// 参数:	
// 返回值:	
// 功能:	
//
//
//************************************************************************
__int64 _Nv_Download::set_ThreadTaskData(__out __int64 &n64X, __out __int64 &n64TaskCnt, __out __int64 &n64Count)
{
	DWORD dwSize = vtTagTaskData.size();
	
	if (dwSize == 0)
	{
		return T_X;
	}

	for (int i=0; i<dwSize; ++i)
	{
		//从这里开始下载
		n64X = vtTagTaskData[i].n64X_ + vtTagTaskData[i].n64DownloadCnt_;
		//这次的任务量
		n64TaskCnt = vtTagTaskData[i].n64TaskCnt_ - vtTagTaskData[i].n64DownloadCnt_;

		if (dwSize == 1)
		{
			n64Count = vtTagTaskData[i].n64TaskCnt_;
			vtTagTaskData.erase(vtTagTaskData.begin() + i);
			return n64X;
		}

		vtTagTaskData.erase(vtTagTaskData.begin() + i);

		return T_Z;
	}

	return T_X;
}


//************************************************************************
//
// 函数:	get_TaskStop
// 参数:	
// 返回值:	
// 功能:	
//
//
//************************************************************************
BOOL _Nv_Download::get_TaskStop(VOID)
{
	return bTaskStop_;
}