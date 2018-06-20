// Serial.cpp: implementation of the CSerial class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Serial.h"


// ---------------------------CSerial类---------------------------------//


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::CSerial
// 功能描述     : 构造函数，用来初始化整个实例对象
// 参数         : BOOL bNeedThread			// 指定是否需要创建辅助线程
// 返回值       : None
//
///////////////////////////////////////////////////////////////////////
CSerial::CSerial(BOOL bNeedThread)
{
	m_hCom = INVALID_HANDLE_VALUE;
	m_bExit = FALSE;
	m_hThread = NULL;
	m_dwMaskEvent = DEFAULT_COM_MASK_EVENT; 
	m_dwNotifyNum = 0;
	m_dcb.DCBlength = sizeof(DCB);
	m_hNotifyWnd = NULL;

	// 初始化重叠结构
	memset(&m_ReadOl, 0, sizeof(m_ReadOl));
	memset(&m_WriteOl, 0, sizeof(m_WriteOl));
	memset(&m_WaitOl, 0, sizeof(m_WaitOl));
	m_ReadOl.hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_WriteOl.hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_WaitOl.hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_bNeedThread = bNeedThread ;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::~CSerial
// 功能描述     : 析构函数，用来销毁整个实例对象
// 返回值       : None
//
///////////////////////////////////////////////////////////////////////
CSerial::~CSerial()
{
	// 关闭串口
	Close();
	// 释放事件对象
	::CloseHandle(m_ReadOl.hEvent);
	::CloseHandle(m_WriteOl.hEvent);
	::CloseHandle(m_WaitOl.hEvent);
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::Open
// 功能描述     : 打开端口
// 参数         : DWORD dwPort			要打开的COM端口
// 参数         : DWORD dwBaudRate		要设置的波特率
// 返回值       : BOOL					是否成功
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::Open(DWORD dwPort, DWORD dwBaudRate)
{
	if(dwPort<1 || dwPort>1024)
	{
		AfxMessageBox(L"串口号不对");
		return FALSE;
	}

	// 打开串口
	if(!OpenCommPort(dwPort))
	{
		AfxMessageBox(L"串口打不开");
		return FALSE;
	}

	// 采用默认设置
	// 设置输入输出缓冲区大小
	if(!::SetupComm(m_hCom, 4096, 4096))
	{
		AfxMessageBox(L"串口设置缓冲区不成功");
		return FALSE;
	}
	// 设置超时
	COMMTIMEOUTS co;
	if(!::GetCommTimeouts(m_hCom, &co))
		return FALSE;

	co.ReadIntervalTimeout = 50;
	co.ReadTotalTimeoutMultiplier = 0;
	co.ReadTotalTimeoutConstant = 0;
	co.WriteTotalTimeoutMultiplier = 1;
	co.WriteTotalTimeoutConstant = 1000;
	if (!::SetCommTimeouts(m_hCom, &co))
	{
		AfxMessageBox(L"串口设置超时不成功");
		return FALSE;

	}
	// 清空输入输出缓冲区
	if (!::PurgeComm(m_hCom, PURGE_TXCLEAR | PURGE_RXCLEAR))///PURGE_TXABORT|PURGE_RXABORT|
	{
		AfxMessageBox(L"串口设置清空输入输出缓冲区不成功");
		return FALSE;
	}
	
	if (!SetState(dwBaudRate))
	{
		AfxMessageBox(L"串口设置波特率不成功");
		return FALSE;
	}

 //	AfxMessageBox(L"启动串口监测线程");
	if(m_bNeedThread)
	{
		DWORD dwId;
		m_bExit = FALSE;
		m_hThread = ::CreateThread(NULL, 0, SerialThreadProc, this, 0, &dwId);
		if(m_hThread == NULL)
		{
		 	AfxMessageBox(L"启动串口监测线程不成功");

			::CloseHandle(m_hCom);
			m_hCom = INVALID_HANDLE_VALUE; //
			return FALSE;
		}
	}
 //	AfxMessageBox(L"启动串口监测线程成功");


	//return SetState(dwBaudRate);
	return TRUE;
}



///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::Purge
// 功能描述     : 清空输入输出缓冲区
// 参数         : DWORD dwFlags				 清空属性
// 返回值       : BOOL						 是否成功
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::Purge(DWORD dwFlags)
{
	if(!IsOpen())
		return FALSE;

	return ::PurgeComm(m_hCom, dwFlags);
}



///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OpenCommPort	
// 功能描述     : 打开指定端口
// 参数         : DWORD dwPort		要打开的端口
// 返回值       : BOOL				是否成功		
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::OpenCommPort(DWORD dwPort)
{
	if(m_hCom != INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	wcscpy_s(m_szComm, _T("\\\\.\\COM"));
	WCHAR sz[5];
	_ltow_s	(dwPort, sz, 10);

	wcscat_s(m_szComm, sz);
	 
	CString str;
	str = m_szComm;
	;
//	AfxMessageBox(str + L" will be opened");

	m_hCom = ::CreateFile(m_szComm, GENERIC_READ | GENERIC_WRITE, 
						0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}


//	AfxMessageBox(str +L"打开成功");

	
/*	if(m_bNeedThread)
	{
		DWORD dwId;
		m_bExit = FALSE;
		m_hThread = ::CreateThread(NULL, 0, SerialThreadProc, this, 0, &dwId);
		if(m_hThread == NULL)
		{
			::CloseHandle(m_hCom);
			m_hCom = INVALID_HANDLE_VALUE; // 
			return FALSE;
		}
	}
*/
	// here to 
	return TRUE;
}



///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::Close
// 功能描述     : 关闭打开的端口
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::Close()
{
	if(!IsOpen())
		return;

	if(m_bNeedThread)
	{
		// wait thread to exit
		m_bExit = TRUE;
		::SetEvent(m_WaitOl.hEvent);
		DWORD dwExitCode;
		int n = 0;
		do
		{
			::Sleep(10);
			if(n++ > 5)
				break;
		}while(!::GetExitCodeThread(m_hThread, &dwExitCode));
		
		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	::CloseHandle(m_hCom);
	m_hCom = INVALID_HANDLE_VALUE;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetState
// 功能描述     : 设置状态
// 参数         : DWORD dwBaudRate			指定波特率
// 参数         : DWORD dwByteSize          指定字节长度
// 参数         : DWORD dwPatity			指定校验位
// 参数         : DWORD dwStopBits          指定停止位
// 返回值       : BOOL						是否成功
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetState(DWORD dwBaudRate,DWORD dwByteSize, DWORD dwPatity, DWORD dwStopBits)
{
	if(IsOpen())
	{
		if(!::GetCommState(m_hCom, &m_dcb))
		{
			return FALSE;
		}
		m_dcb.BaudRate = dwBaudRate;
		m_dcb.ByteSize = (UCHAR)dwByteSize;
		m_dcb.StopBits = (UCHAR)dwStopBits;
		m_dcb.Parity = (UCHAR)dwPatity;
		if(dwPatity != NOPARITY)
			m_dcb.fParity = TRUE;

		//新VS2013以后，需要控制DSR其他信号
		m_dcb.fRtsControl = RTS_CONTROL_DISABLE;//不使用握手信号
		m_dcb.fDtrControl = DTR_CONTROL_DISABLE;//不使用握手信号

		return ::SetCommState(m_hCom, &m_dcb);
	}
	return FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::ThreadProc
// 功能描述     : 线程函数
// 返回值       : DWORD       线程退出代码
//
///////////////////////////////////////////////////////////////////////
DWORD CSerial::ThreadProc()
{
	if(!::SetCommMask(m_hCom, m_dwMaskEvent))
	{
		::MessageBox(m_hNotifyWnd,_T( " SetCommMask failed"), _T(""), 0);
		return -1;
	}

	DWORD dwError, dwMask, dwTrans;
	COMSTAT Stat;

	while(!m_bExit)
	{
//		AfxMessageBox(L"准备等待串口事件");
		dwMask = 0;
		if(!::WaitCommEvent(m_hCom, &dwMask, &m_WaitOl))
		{
			if(::GetLastError() == ERROR_IO_PENDING)
			{
				::GetOverlappedResult(m_hCom, &m_WaitOl, &dwTrans, TRUE);
			}
			else
				continue;
		}
	//	AfxMessageBox(L"发生串口事件");
		switch(dwMask)
		{
			case EV_RXCHAR:
				{
					// 有数据进来，  获取数据大小
					::ClearCommError(m_hCom, &dwError, &Stat);
					if(Stat.cbInQue >= m_dwNotifyNum)
					{
					//	AfxMessageBox(L"发生串口接收事件");
						OnReceive();
					}
				}
				break;
			
			case EV_TXEMPTY:		// 输出缓冲区中的最后一个字符被发送了
				OnTXEmpty();
				break;
			case EV_CTS:			// clear-to-send信号改变
				OnCTS();
				break;

			case EV_DSR:			// data-set-ready信号改变
				OnDSR();
				break;

			case EV_RING:		
				OnRing();
				break;

			case EV_RLSD:		
				OnRLSD();
				break;

			case EV_BREAK:
				OnBreak();
				break;

			case EV_ERR:
				OnError();
				break;
			default:
				continue;
		}
	}
	return 0;
}
	

///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SerialThreadProc
// 功能描述     : 
// 参数         : LPVOID lpParam
// 返回值       : DWORD WINAPI 
//
///////////////////////////////////////////////////////////////////////
DWORD WINAPI CSerial::SerialThreadProc(LPVOID lpParam)
{
	CSerial *pThis = (CSerial *)lpParam;
	return pThis->ThreadProc();
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::ReadData
// 功能描述     : 读取指定长度的数据
// 参数         : LPVOID Buffer					数据缓冲区
// 参数         : DWORD dwReadLen				要读取的长度
// 参数         : DWORD dwWaitTime				要等待时间
// 返回值       : BOOL							是否成功
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::ReadData(LPVOID Buffer, DWORD dwReadLen, DWORD dwWaitTime)
{
	if(!IsOpen())
		return FALSE;

	COMSTAT Stat;
	DWORD dwError;
	BOOL bError = FALSE;  // 

	DWORD dwRead = 0;
	while(TRUE)
	{
		// 检查是否有错误发生
		if(::ClearCommError(m_hCom, &dwError, &Stat))
		{
			if(dwError > 0)
			{
				 bError = TRUE;  
			}
			
			// 查看输入缓冲区中的数据长度
			if(Stat.cbInQue >= dwReadLen)
			{
				// 读取输入缓冲区中的数据
				dwRead = Read(Buffer, dwReadLen, dwWaitTime);
				break;
			}
			else
			{
				::Sleep(10);
			 	dwWaitTime -= 10;
			}
		}
		else
		{
			return FALSE;
		}

		if(dwWaitTime <= 0)
			return FALSE;
	}


	if(dwRead == dwReadLen && !bError)
		return TRUE;
	else
		return FALSE; 
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::Read
// 功能描述     : 读取缓冲区中的数据
// 参数         : LPVOID Buffer
// 参数         : DWORD dwBufferLen
// 参数         : DWORD dwWaitTime
// 返回值       : DWORD 
//
///////////////////////////////////////////////////////////////////////
DWORD CSerial::Read(LPVOID Buffer, DWORD dwBufferLen, DWORD dwWaitTime)
{
	if(!IsOpen())
		return 0;


	COMSTAT Stat;
	DWORD dwError;
	// 有错误发生！
	if(::ClearCommError(m_hCom, &dwError, &Stat) && dwError > 0)
	{
		// 清除缓冲区，清除所有抛出的读操作
		::PurgeComm(m_hCom, PURGE_RXABORT|PURGE_RXCLEAR);
		return 0;
	}

	if(!Stat.cbInQue)
	{
		return 0;
	}

	DWORD dwReadLen = 0;
	if(dwBufferLen > Stat.cbInQue)
		dwBufferLen = Stat.cbInQue;

	if(!::ReadFile(m_hCom, Buffer, dwBufferLen, &dwReadLen, &m_ReadOl))
	{
		if(::GetLastError() == ERROR_IO_PENDING)
		{
			// 等待操作完成
			::WaitForSingleObject(m_ReadOl.hEvent, dwWaitTime);
			// 获取操作结果
			if(!::GetOverlappedResult(m_hCom, &m_ReadOl, &dwReadLen, FALSE))
			{
				if(::GetLastError() != ERROR_IO_INCOMPLETE)
				{
					dwReadLen  = 0;
				}
			}
		}
		else
		{
			dwReadLen  = 0;
		}
	}
	return dwReadLen;
}



///////////////////////////////////////////////////////////////////////
//
// 函数名       : *CSerial::ReadString
// 功能描述     : 读取字符串
// 参数         : TCHAR *szBuffer
// 参数         : DWORD dwBufferLen
// 参数         : DWORD dwWaitTime
// 返回值       : TCHAR 
//
///////////////////////////////////////////////////////////////////////
TCHAR *CSerial::ReadString(TCHAR *szBuffer, DWORD dwBufferLen, DWORD dwWaitTime)
{
	DWORD dwRead = Read(szBuffer, dwBufferLen - 1, dwWaitTime);
	szBuffer[dwRead] = '\0';
	return szBuffer;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::Write
// 功能描述     : 写入指定长度的数据
// 参数         : LPVOID Buffer			数据缓冲区
// 参数         : DWORD dwBufferLen		缓冲区长度
// 返回值       : DWORD					实际写入的数量
//
///////////////////////////////////////////////////////////////////////
DWORD CSerial::Write(LPVOID Buffer, DWORD dwBufferLen)
{
	if(!IsOpen())
		return 0;


	COMSTAT Stat;
	DWORD dwError;
	// 有错误发生！
	if(::ClearCommError(m_hCom, &dwError, &Stat) && dwError > 0)
	{
		// 清除缓冲区，清除所有抛出的读操作
		::PurgeComm(m_hCom, PURGE_RXABORT|PURGE_RXCLEAR);
		return 0;
	}

	DWORD dwWrite = 0;
	if(!::WriteFile(m_hCom, Buffer, dwBufferLen, &dwWrite, &m_WriteOl))
	{
		if( ::GetLastError() == ERROR_IO_PENDING)
		{
			::GetOverlappedResult(m_hCom, &m_WriteOl, &dwWrite, TRUE);
		}
	}
	return dwWrite;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::WriteString
// 功能描述     : 写入字符串
// 参数         : LPTSTR szBuffer
// 返回值       : DWORD 
//
///////////////////////////////////////////////////////////////////////
DWORD CSerial::WriteString(LPTSTR szBuffer)
{
	return Write(szBuffer, wcslen(szBuffer));
}

DWORD CSerial::WriteStringA(LPTSTR szBuffer)
{
	return Write(szBuffer, strlen((char*)szBuffer));
}

///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetBufferSize
// 功能描述     : 设置缓冲区大小
// 参数         : DWORD dwInputSize
// 参数         : DWORD dwOutputSize
// 返回值       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetBufferSize(DWORD dwInputSize, DWORD dwOutputSize)
{
	return IsOpen() ? ::SetupComm(m_hCom, dwInputSize, dwOutputSize) : FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetWnd
// 功能描述     : 设置窗口句柄
// 参数         : HWND hWnd
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::SetWnd(HWND hWnd)
{
	m_hNotifyWnd = hWnd;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetNotifyNum
// 功能描述     : 设置通知数量
// 参数         : DWORD dwNum
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::SetNotifyNum(DWORD dwNum)
{
	m_dwNotifyNum = dwNum;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetTimeouts
// 功能描述     : 设置超时
// 参数         : LPCOMMTIMEOUTS lpCO
// 返回值       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetTimeouts(LPCOMMTIMEOUTS lpCO)
{
	return IsOpen() ? ::SetCommTimeouts(m_hCom, lpCO) : FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetDTR
// 功能描述     : 
// 参数         : BOOL OnOrOff
// 返回值       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetDTR(BOOL OnOrOff)
{
	return IsOpen() ? ::EscapeCommFunction(m_hCom, OnOrOff ? SETDTR:CLRDTR): FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetRTS
// 功能描述     : 
// 参数         : BOOL OnOrOff
// 返回值       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetRTS(BOOL OnOrOff)
{
	return IsOpen() ? ::EscapeCommFunction(m_hCom, OnOrOff ? SETRTS:CLRRTS):FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetBreak
// 功能描述     : 
// 参数         : BOOL OnOrOff
// 返回值       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetBreak(BOOL OnOrOff)
{
	return IsOpen() ? ::EscapeCommFunction(m_hCom, OnOrOff ? SETBREAK:CLRBREAK):FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::SetMaskEvent
// 功能描述     : 
// 参数         : DWORD dwMask
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::SetMaskEvent(DWORD dwMask)
{
	m_dwMaskEvent = dwMask;
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OnCTS
// 功能描述     : 
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnCTS()
{
	if(::IsWindow(m_hNotifyWnd))
	{
		DWORD dwStatus;
		if(GetCommModemStatus(m_hCom, &dwStatus))
			::SendMessage(m_hNotifyWnd, WM_COM_CTS, 0, dwStatus&MS_CTS_ON ? 1:0);
	}
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OnDSR
// 功能描述     : 
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnDSR()
{
	if(::IsWindow(m_hNotifyWnd))
	{
		DWORD dwStatus;
		if(::GetCommModemStatus(m_hCom, &dwStatus))
		{
			::SendMessage(m_hNotifyWnd, WM_COM_DSR, 0, dwStatus&MS_DSR_ON ? 1:0);
		}
	}
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OnRing
// 功能描述     : 
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnRing()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_RING, 0, 0);
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OnRLSD
// 功能描述     : 
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnRLSD()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_RLSD, 0, 0);
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OnBreak
// 功能描述     : 
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnBreak()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_BREAK, 0, 0);
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OnError
// 功能描述     : 
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnError()
{
	DWORD dwError;
	::ClearCommError(m_hCom, &dwError, NULL);
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_ERROR, 0, dwError);
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OnReceive
// 功能描述     : 
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnReceive()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_RECEIVE, 0, 0);
}


///////////////////////////////////////////////////////////////////////
//
// 函数名       : CSerial::OnTXEmpty
// 功能描述     : 
// 返回值       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnTXEmpty()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_TXEMPTY, 0, 0);
}
