// Serial.h

#ifndef _SERIAL__H__
#define _SERIAL__H__


//  CSerial类向主窗口发送的消息
#define WM_COM_RECEIVE	WM_USER	+ 301
#define WM_COM_CTS		WM_USER	+ 302		
#define WM_COM_DSR		WM_USER	+ 303
#define WM_COM_RING		WM_USER	+ 304
#define WM_COM_RLSD		WM_USER	+ 306
#define WM_COM_BREAK	WM_USER	+ 307
#define WM_COM_TXEMPTY	WM_USER	+ 308
#define WM_COM_ERROR	WM_USER	+ 309

#define DEFAULT_COM_MASK_EVENT EV_RXCHAR | EV_ERR  \
				| EV_CTS | EV_DSR | EV_BREAK | EV_TXEMPTY | EV_RING | EV_RLSD

// 串口控制类
class CSerial  
{
public:
// 构造函数和析构函数
	CSerial(BOOL bNeedThread = TRUE);
	virtual ~CSerial();

	// 判断端口是否打开
	BOOL IsOpen() { return m_hCom != INVALID_HANDLE_VALUE; }

	// 使用默认设置打开串口
	BOOL Open(DWORD dwPort, DWORD dwBaudRate = 115200);
	// 关闭端口
	void Close();

	// 设置串口状态
	BOOL SetState(DWORD dwBaudRate, DWORD dwByteSize = 8, 
				DWORD dwPatity = NOPARITY, DWORD dwStopBits = ONESTOPBIT);
	BOOL SetTimeouts(LPCOMMTIMEOUTS lpCO);
	BOOL SetBufferSize(DWORD dwInputSize, DWORD dwOutputSize);
	void SetWnd(HWND hWnd);
	void SetNotifyNum(DWORD dwNum);
	void SetMaskEvent(DWORD dwMask);

	// 读写串口
	DWORD Write(LPVOID Buffer, DWORD dwBufferLen);
	DWORD WriteString(LPTSTR szBuffer);
	DWORD WriteStringA(LPTSTR szBuffer);
	DWORD Read(LPVOID Buffer, DWORD dwBufferLen, DWORD dwWaitTime = 20);
	// 读取指定字节的数据
	BOOL  ReadData(LPVOID Buffer, DWORD dwReadLen, DWORD dwWaitTime = 100);
	TCHAR *ReadString(TCHAR *szBuffer, DWORD dwBufferLen, DWORD dwWaitTime = 20);

	BOOL Purge(DWORD dwFlags = PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

	// 设置电平
	BOOL SetDTR(BOOL OnOrOff);
	BOOL SetRTS(BOOL OnOrOff);
	BOOL SetBreak(BOOL OnOrOff);

protected:		
	BOOL OpenCommPort(DWORD dwPort);

	// 事件处理函数
	virtual void OnCTS();
	virtual void OnDSR();
	virtual void OnRing();
	virtual void OnRLSD();
	virtual void OnBreak();
	virtual void OnError();
	virtual void OnReceive();
	virtual void OnTXEmpty();

	// 线程函数，等待处理串口上发生的事件
	virtual DWORD ThreadProc();	
	static DWORD WINAPI SerialThreadProc(LPVOID lpParam);
protected:
	HANDLE m_hCom;
	OVERLAPPED m_ReadOl;
	OVERLAPPED m_WriteOl;
	OVERLAPPED m_WaitOl;
	BOOL m_bExit;
	HANDLE m_hThread;
	DCB m_dcb;
	DWORD m_dwMaskEvent;
	HWND m_hNotifyWnd;
	DWORD m_dwNotifyNum;
	WCHAR m_szComm[16];

	BOOL m_bNeedThread;
};

#endif // _SERIAL__H__
