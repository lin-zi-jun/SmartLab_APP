// Serial.cpp: implementation of the CSerial class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Serial.h"


// ---------------------------CSerial��---------------------------------//


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::CSerial
// ��������     : ���캯����������ʼ������ʵ������
// ����         : BOOL bNeedThread			// ָ���Ƿ���Ҫ���������߳�
// ����ֵ       : None
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

	// ��ʼ���ص��ṹ
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
// ������       : CSerial::~CSerial
// ��������     : ����������������������ʵ������
// ����ֵ       : None
//
///////////////////////////////////////////////////////////////////////
CSerial::~CSerial()
{
	// �رմ���
	Close();
	// �ͷ��¼�����
	::CloseHandle(m_ReadOl.hEvent);
	::CloseHandle(m_WriteOl.hEvent);
	::CloseHandle(m_WaitOl.hEvent);
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::Open
// ��������     : �򿪶˿�
// ����         : DWORD dwPort			Ҫ�򿪵�COM�˿�
// ����         : DWORD dwBaudRate		Ҫ���õĲ�����
// ����ֵ       : BOOL					�Ƿ�ɹ�
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::Open(DWORD dwPort, DWORD dwBaudRate)
{
	if(dwPort<1 || dwPort>1024)
	{
		AfxMessageBox(L"���ںŲ���");
		return FALSE;
	}

	// �򿪴���
	if(!OpenCommPort(dwPort))
	{
		AfxMessageBox(L"���ڴ򲻿�");
		return FALSE;
	}

	// ����Ĭ������
	// �������������������С
	if(!::SetupComm(m_hCom, 4096, 4096))
	{
		AfxMessageBox(L"�������û��������ɹ�");
		return FALSE;
	}
	// ���ó�ʱ
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
		AfxMessageBox(L"�������ó�ʱ���ɹ�");
		return FALSE;

	}
	// ����������������
	if (!::PurgeComm(m_hCom, PURGE_TXCLEAR | PURGE_RXCLEAR))///PURGE_TXABORT|PURGE_RXABORT|
	{
		AfxMessageBox(L"���������������������������ɹ�");
		return FALSE;
	}
	
	if (!SetState(dwBaudRate))
	{
		AfxMessageBox(L"�������ò����ʲ��ɹ�");
		return FALSE;
	}

 //	AfxMessageBox(L"�������ڼ���߳�");
	if(m_bNeedThread)
	{
		DWORD dwId;
		m_bExit = FALSE;
		m_hThread = ::CreateThread(NULL, 0, SerialThreadProc, this, 0, &dwId);
		if(m_hThread == NULL)
		{
		 	AfxMessageBox(L"�������ڼ���̲߳��ɹ�");

			::CloseHandle(m_hCom);
			m_hCom = INVALID_HANDLE_VALUE; //
			return FALSE;
		}
	}
 //	AfxMessageBox(L"�������ڼ���̳߳ɹ�");


	//return SetState(dwBaudRate);
	return TRUE;
}



///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::Purge
// ��������     : ����������������
// ����         : DWORD dwFlags				 �������
// ����ֵ       : BOOL						 �Ƿ�ɹ�
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
// ������       : CSerial::OpenCommPort	
// ��������     : ��ָ���˿�
// ����         : DWORD dwPort		Ҫ�򿪵Ķ˿�
// ����ֵ       : BOOL				�Ƿ�ɹ�		
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


//	AfxMessageBox(str +L"�򿪳ɹ�");

	
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
// ������       : CSerial::Close
// ��������     : �رմ򿪵Ķ˿�
// ����ֵ       : void 
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
// ������       : CSerial::SetState
// ��������     : ����״̬
// ����         : DWORD dwBaudRate			ָ��������
// ����         : DWORD dwByteSize          ָ���ֽڳ���
// ����         : DWORD dwPatity			ָ��У��λ
// ����         : DWORD dwStopBits          ָ��ֹͣλ
// ����ֵ       : BOOL						�Ƿ�ɹ�
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

		//��VS2013�Ժ���Ҫ����DSR�����ź�
		m_dcb.fRtsControl = RTS_CONTROL_DISABLE;//��ʹ�������ź�
		m_dcb.fDtrControl = DTR_CONTROL_DISABLE;//��ʹ�������ź�

		return ::SetCommState(m_hCom, &m_dcb);
	}
	return FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::ThreadProc
// ��������     : �̺߳���
// ����ֵ       : DWORD       �߳��˳�����
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
//		AfxMessageBox(L"׼���ȴ������¼�");
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
	//	AfxMessageBox(L"���������¼�");
		switch(dwMask)
		{
			case EV_RXCHAR:
				{
					// �����ݽ�����  ��ȡ���ݴ�С
					::ClearCommError(m_hCom, &dwError, &Stat);
					if(Stat.cbInQue >= m_dwNotifyNum)
					{
					//	AfxMessageBox(L"�������ڽ����¼�");
						OnReceive();
					}
				}
				break;
			
			case EV_TXEMPTY:		// ����������е����һ���ַ���������
				OnTXEmpty();
				break;
			case EV_CTS:			// clear-to-send�źŸı�
				OnCTS();
				break;

			case EV_DSR:			// data-set-ready�źŸı�
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
// ������       : CSerial::SerialThreadProc
// ��������     : 
// ����         : LPVOID lpParam
// ����ֵ       : DWORD WINAPI 
//
///////////////////////////////////////////////////////////////////////
DWORD WINAPI CSerial::SerialThreadProc(LPVOID lpParam)
{
	CSerial *pThis = (CSerial *)lpParam;
	return pThis->ThreadProc();
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::ReadData
// ��������     : ��ȡָ�����ȵ�����
// ����         : LPVOID Buffer					���ݻ�����
// ����         : DWORD dwReadLen				Ҫ��ȡ�ĳ���
// ����         : DWORD dwWaitTime				Ҫ�ȴ�ʱ��
// ����ֵ       : BOOL							�Ƿ�ɹ�
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
		// ����Ƿ��д�����
		if(::ClearCommError(m_hCom, &dwError, &Stat))
		{
			if(dwError > 0)
			{
				 bError = TRUE;  
			}
			
			// �鿴���뻺�����е����ݳ���
			if(Stat.cbInQue >= dwReadLen)
			{
				// ��ȡ���뻺�����е�����
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
// ������       : CSerial::Read
// ��������     : ��ȡ�������е�����
// ����         : LPVOID Buffer
// ����         : DWORD dwBufferLen
// ����         : DWORD dwWaitTime
// ����ֵ       : DWORD 
//
///////////////////////////////////////////////////////////////////////
DWORD CSerial::Read(LPVOID Buffer, DWORD dwBufferLen, DWORD dwWaitTime)
{
	if(!IsOpen())
		return 0;


	COMSTAT Stat;
	DWORD dwError;
	// �д�������
	if(::ClearCommError(m_hCom, &dwError, &Stat) && dwError > 0)
	{
		// �������������������׳��Ķ�����
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
			// �ȴ��������
			::WaitForSingleObject(m_ReadOl.hEvent, dwWaitTime);
			// ��ȡ�������
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
// ������       : *CSerial::ReadString
// ��������     : ��ȡ�ַ���
// ����         : TCHAR *szBuffer
// ����         : DWORD dwBufferLen
// ����         : DWORD dwWaitTime
// ����ֵ       : TCHAR 
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
// ������       : CSerial::Write
// ��������     : д��ָ�����ȵ�����
// ����         : LPVOID Buffer			���ݻ�����
// ����         : DWORD dwBufferLen		����������
// ����ֵ       : DWORD					ʵ��д�������
//
///////////////////////////////////////////////////////////////////////
DWORD CSerial::Write(LPVOID Buffer, DWORD dwBufferLen)
{
	if(!IsOpen())
		return 0;


	COMSTAT Stat;
	DWORD dwError;
	// �д�������
	if(::ClearCommError(m_hCom, &dwError, &Stat) && dwError > 0)
	{
		// �������������������׳��Ķ�����
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
// ������       : CSerial::WriteString
// ��������     : д���ַ���
// ����         : LPTSTR szBuffer
// ����ֵ       : DWORD 
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
// ������       : CSerial::SetBufferSize
// ��������     : ���û�������С
// ����         : DWORD dwInputSize
// ����         : DWORD dwOutputSize
// ����ֵ       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetBufferSize(DWORD dwInputSize, DWORD dwOutputSize)
{
	return IsOpen() ? ::SetupComm(m_hCom, dwInputSize, dwOutputSize) : FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::SetWnd
// ��������     : ���ô��ھ��
// ����         : HWND hWnd
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::SetWnd(HWND hWnd)
{
	m_hNotifyWnd = hWnd;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::SetNotifyNum
// ��������     : ����֪ͨ����
// ����         : DWORD dwNum
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::SetNotifyNum(DWORD dwNum)
{
	m_dwNotifyNum = dwNum;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::SetTimeouts
// ��������     : ���ó�ʱ
// ����         : LPCOMMTIMEOUTS lpCO
// ����ֵ       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetTimeouts(LPCOMMTIMEOUTS lpCO)
{
	return IsOpen() ? ::SetCommTimeouts(m_hCom, lpCO) : FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::SetDTR
// ��������     : 
// ����         : BOOL OnOrOff
// ����ֵ       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetDTR(BOOL OnOrOff)
{
	return IsOpen() ? ::EscapeCommFunction(m_hCom, OnOrOff ? SETDTR:CLRDTR): FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::SetRTS
// ��������     : 
// ����         : BOOL OnOrOff
// ����ֵ       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetRTS(BOOL OnOrOff)
{
	return IsOpen() ? ::EscapeCommFunction(m_hCom, OnOrOff ? SETRTS:CLRRTS):FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::SetBreak
// ��������     : 
// ����         : BOOL OnOrOff
// ����ֵ       : BOOL 
//
///////////////////////////////////////////////////////////////////////
BOOL CSerial::SetBreak(BOOL OnOrOff)
{
	return IsOpen() ? ::EscapeCommFunction(m_hCom, OnOrOff ? SETBREAK:CLRBREAK):FALSE;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::SetMaskEvent
// ��������     : 
// ����         : DWORD dwMask
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::SetMaskEvent(DWORD dwMask)
{
	m_dwMaskEvent = dwMask;
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::OnCTS
// ��������     : 
// ����ֵ       : void 
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
// ������       : CSerial::OnDSR
// ��������     : 
// ����ֵ       : void 
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
// ������       : CSerial::OnRing
// ��������     : 
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnRing()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_RING, 0, 0);
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::OnRLSD
// ��������     : 
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnRLSD()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_RLSD, 0, 0);
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::OnBreak
// ��������     : 
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnBreak()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_BREAK, 0, 0);
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::OnError
// ��������     : 
// ����ֵ       : void 
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
// ������       : CSerial::OnReceive
// ��������     : 
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnReceive()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_RECEIVE, 0, 0);
}


///////////////////////////////////////////////////////////////////////
//
// ������       : CSerial::OnTXEmpty
// ��������     : 
// ����ֵ       : void 
//
///////////////////////////////////////////////////////////////////////
void CSerial::OnTXEmpty()
{
	if(::IsWindow(m_hNotifyWnd))
		::SendMessage(m_hNotifyWnd, WM_COM_TXEMPTY, 0, 0);
}
