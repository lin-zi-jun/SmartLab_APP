
// IAPDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "IAP.h"
#include "IAPDlg.h"
#include "afxdialogex.h"

#include <devguid.h>
#include <setupapi.h> 

#pragma comment(lib, "setupapi") 
#define WM_REV_FRAME WM_USER + 10


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NAK                     (0x15)  /* negative acknowledge */
#define CA                      (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1                  (0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  (0x61)  /* 'a' == 0x61, abort by user */

#define TIMER1		1
///////////////////////////////////////////////////////////////////////////////
static unsigned short ccitt_table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

unsigned short crc_ccitt(BYTE *q, int len)
{
	unsigned short crc = 0;

	while (len-- > 0)
		crc = ccitt_table[(crc >> 8 ^ *q++) & 0xff] ^ (crc << 8);
	return ((~crc)&0xFFFF);
}
///////////////////////////////////////////////////////////////////////////////
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIAPDlg 对话框



CIAPDlg::CIAPDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIAPDlg::IDD, pParent)
	, m_FWFile(_T(""))
	, m_FWBuf(NULL)
	, m_Bootloader(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIAPDlg::DoDataExchange(CDataExchange* pDX)		//控件之间相互关联
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_FWFile);
	DDX_Text(pDX, IDC_EDIT2, m_Bootloader);
	DDX_Control(pDX, IDC_COMBO1, m_CommboBoxCommName);
	DDX_Control(pDX, IDC_BTN_OPEN_COMM, m_BtnOpenCommPort);
	DDX_Control(pDX, IDC_COMBO2, m_ComboBaudrate);
	DDX_Control(pDX, IDC_EDIT3, m_RecvInfo);
	DDX_Control(pDX, IDC_EDIT_SN, m_SN);					//m_SN编辑变量关联 编辑框ID			
	DDX_Control(pDX, IDC_BTN_CHSN, m_BtnChSN);				//SN按钮 关联 按钮ID
	DDX_Control(pDX, IDC_EDIT4, m_RemainTime);
	DDX_Control(pDX, IDC_BTN_STATE, m_BtnState);
	DDX_Control(pDX, IDC_BTN_RELAY, m_BtnRelay);
	DDX_Control(pDX, IDC_BTN_TIME, m_BtnTime);
	DDX_Control(pDX, IDC_BTN_CL_OFFLINE, m_BtnClOffline);
	DDX_Control(pDX, IDC_BTN_CL_ALL, m_BtnClAll);
	DDX_Control(pDX, IDC_BTN_DLFW, m_BtnDL);
	DDX_Control(pDX, IDC_DL_STATE, m_DLState);
	DDX_Control(pDX, IDC_BTN_BOOT, m_BtnBoot);
	DDX_Control(pDX, IDC_BUTTON_JUMP_IAP, m_BtnJumpIap);

	DDX_Control(pDX, IDC_BUTTON_SUPER_CARD_TIME, m_BtnSuperCardTime);			//超级卡使用的时间
	DDX_Control(pDX, IDC_EDIT_SUPER_CARD_TIME,m_EditSuperCardTime);
	
	DDX_Control(pDX, IDC_BUTTON_LOG_OUT_TIME, m_BtnLogOutTime);			
	DDX_Control(pDX, IDC_EDIT_LOG_OUT_TIME, m_EditLogOutTime);

	DDX_Control(pDX, IDC_BUTTON_SET_SUPER_CARD, m_BtnSetSuperCard);
	DDX_Control(pDX, IDC_EDIT_SET_SUPER_CARD, m_EditSetSuperCard);

	DDX_Control(pDX, IDC_BUTTON_CLEAR_SUPER_CARD, m_BtnClearSuperCard);
	DDX_Control(pDX, IDC_BUTTON_DELETE_SUPER_CARD, m_BtntDeleteSuperCard);
	DDX_Control(pDX, IDC_EDIT_DELETE_SUPER_CARD, m_EditDeleteSuperCard);

	DDX_Control(pDX, IDC_BUTTON_SET_DNS, m_BtnSetDns);
	DDX_Control(pDX, IDC_BUTTON_SET_IP, m_BtnSetIP);
	DDX_Control(pDX, IDC_BUTTON_SET_TCP, m_BtntSetTCP);
	DDX_Control(pDX, IDC_EDIT_SET_DNS, m_EditSetDns);
	DDX_Control(pDX, IDC_EDIT_SET_IP, m_EditSetIP);
	DDX_Control(pDX, IDC_EDIT_SET_TCP, m_EditSetTCP);

	DDX_Control(pDX, IDC_BUTTON_SET_WIFI, m_BtntSetWIFI);
	DDX_Control(pDX, IDC_EDIT_SET_WIFI, m_EditSetWIFI);
	DDX_Control(pDX, IDC_BUTTON_CLEAR, m_BtntClear);
	DDX_Control(pDX, IDC_BUTTON_LOG_OFF, m_BtntLogOff);
	
}






BEGIN_MESSAGE_MAP(CIAPDlg, CDialogEx)				//消息处理机制
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CIAPDlg::OnBnClickedBtnBrowse)
	ON_WM_CLOSE()
	ON_WM_DEVICECHANGE()
	ON_MESSAGE(WM_COM_RECEIVE, &CIAPDlg::OnRevFrame)
	ON_BN_CLICKED(IDC_BTN_BROWSE2, &CIAPDlg::OnBnClickedBtnBrowse2)
	ON_BN_CLICKED(IDC_BTN_OPEN_COMM, &CIAPDlg::OnBnClickedBtnOpenComm)
	ON_BN_CLICKED(IDC_BTN_CHSN, &CIAPDlg::OnBnClickedBtnChsn)				//SN号按钮
	ON_BN_CLICKED(IDC_BTN_STATE, &CIAPDlg::OnBnClickedBtnState)
	ON_BN_CLICKED(IDC_BTN_RELAY, &CIAPDlg::OnBnClickedBtnRelay)
	ON_BN_CLICKED(IDC_BTN_CL_OFFLINE, &CIAPDlg::OnBnClickedBtnClOffline)
	ON_BN_CLICKED(IDC_BTN_CL_ALL, &CIAPDlg::OnBnClickedBtnClAll)
	ON_BN_CLICKED(IDC_BTN_TIME, &CIAPDlg::OnBnClickedBtnTime)
	ON_CBN_DROPDOWN(IDC_COMBO1, &CIAPDlg::OnCbnDropdownCombo1)
	ON_BN_CLICKED(IDC_BTN_DLFW, &CIAPDlg::OnBnClickedBtnDlfw)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_BOOT, &CIAPDlg::OnBnClickedBtnBoot)
	ON_BN_CLICKED(IDC_BUTTON_JUMP_IAP, &CIAPDlg::OnBnClickedButtonJumpIap)

	ON_BN_CLICKED(IDC_BUTTON_SUPER_CARD_TIME, &CIAPDlg::OnBnClickedButtonSuperCardTime)

	ON_BN_CLICKED(IDC_BUTTON_LOG_OUT_TIME, &CIAPDlg::OnBnClickedButtonLogOutTime)

	ON_BN_CLICKED(IDC_BUTTON_SET_SUPER_CARD, &CIAPDlg::OnBnClickedButtonSetSuperCard)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_SUPER_CARD, &CIAPDlg::OnBnClickedButtonDeleteSuperCard)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_SUPER_CARD, &CIAPDlg::OnBnClickedButtonClearSuperCard)

	ON_BN_CLICKED(IDC_BUTTON_SET_DNS, &CIAPDlg::OnBnClickedButtonSetDns)
	ON_BN_CLICKED(IDC_BUTTON_SET_TCP, &CIAPDlg::OnBnClickedButtonSetTcp)
	ON_BN_CLICKED(IDC_BUTTON_SET_IP, &CIAPDlg::OnBnClickedButtonSetIp)

	ON_BN_CLICKED(IDC_BUTTON_SET_WIFI, &CIAPDlg::OnBnClickedButtonSetWifi)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CIAPDlg::OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CIAPDlg::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_LOG_OFF, &CIAPDlg::OnBnClickedButtonLogOff)
END_MESSAGE_MAP()


// CIAPDlg 消息处理程序

BOOL CIAPDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	TCHAR strBuff[256];
	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_FWBuf = NULL;
	m_FWFileLen = 0;
	m_FWFilePos = 0;
	m_Session = _T("Config");
	m_FWKey = _T("FW_PATH");
	m_BLKey = _T("BootLoader");
	m_COMKey = _T("COM");
	m_BaudKey = _T("Buadrate");
	GetCurrentDirectory(256, strBuff);   //获取当前路径
	m_IniFileName.Format(_T("%s//Config.ini"),strBuff);

	GetPrivateProfileString(m_Session, m_FWKey, 
		NULL,strBuff,256,m_IniFileName); //读取ini文件中相应字段的内容
	m_FWFile = strBuff;
	GetPrivateProfileString(m_Session, m_BLKey, 
		NULL,strBuff,256,m_IniFileName); //读取ini文件中相应字段的内容
	m_Bootloader = strBuff;
	GetPrivateProfileString(m_Session, m_COMKey, 
		NULL,strBuff,256,m_IniFileName); //读取ini文件中相应字段的内容
	m_ComName = strBuff;
	GetPrivateProfileString(m_Session, m_BaudKey, 
		NULL,strBuff,256,m_IniFileName); //读取ini文件中相应字段的内容
	m_Baudrate = strBuff;

	//初始化串口组合框
	GetDevCommInfo();

	TCHAR bauds[][16] = {_T("9600"), _T("19200"), _T("38400"), _T("57600"), _T("115200"), _T("230400"), _T("460800"), _T("921600")};
	int nSel = 0;
	for (int i=0; i<8; i++) {
		m_ComboBaudrate.AddString(bauds[i]);
		if (m_Baudrate.GetLength()>0 && m_Baudrate.Find(bauds[i])>=0) {
			nSel = i;
		}
	}
	m_ComboBaudrate.SetCurSel(nSel);

	//SetTimer(TIMER1,1000,0);

	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CIAPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIAPDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CIAPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int   CIAPDlg::GetDevCommInfo()
{
	int cnt;
	int nPos = 0, nSel = 0;
	cnt = m_CommboBoxCommName.GetCount();
	while (cnt)
	{ 
		m_CommboBoxCommName.DeleteString(0);
		m_CommboBoxCommName.UpdateData(1);
		cnt = m_CommboBoxCommName.GetCount();
	}//   得到设备信息集    

	//   得到设备信息集    
//	GUID GUID_DEVCLASS_PORTS;

	HDEVINFO   hDevInfo = SetupDiGetClassDevs(	(LPGUID)&GUID_DEVCLASS_PORTS,
													NULL,
													0,
													DIGCF_PRESENT/*   |   DIGCF_ALLCLASSES*/
											);
	CString str;
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		str.Format(_T("Error!   SetupDiGetClassDevs()   return   %d\n"), GetLastError());
		AfxMessageBox(str);
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return     0;
	}   

	TCHAR   szBuf[MAX_PATH];

	SP_DEVINFO_DATA   spDevInfoData = { sizeof(SP_DEVINFO_DATA) };
	//   开始列举设备    
	DWORD   i = 0;
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &spDevInfoData); i++)
	{
		//   得到设备类描述    
		if (SetupDiGetClassDescription(
			&spDevInfoData.ClassGuid,
			szBuf,
			MAX_PATH,
			NULL))
		{
			str.Format(_T("\n%03d   :   ClassDescription   =   %s\n"), i, szBuf);
		//	AfxMessageBox(str);
		}
		//   得到设备实例的唯一标识ID(由注册表路径组成)    
		if (SetupDiGetDeviceInstanceId(
			hDevInfo,
			&spDevInfoData,
			szBuf,
			MAX_PATH,
			NULL))
		{
			str.Format(_T("%03d   :   InstanceId   =   %s\n"), i, szBuf);
		//	AfxMessageBox(str);
		}
		//   得到设备名称    

		if (SetupDiGetDeviceRegistryProperty(
			hDevInfo,
			&spDevInfoData,
			SPDRP_DEVICEDESC,
			NULL,
			(PBYTE)szBuf,
			MAX_PATH,
			NULL))
		{
			str.Format(_T("%03d   :   DeviceDesc   =   %s\n"), i, szBuf);
		//	AfxMessageBox(str);
		}
		//   得到设备型号    

		if (SetupDiGetDeviceRegistryProperty(

			hDevInfo,
			&spDevInfoData,
			SPDRP_FRIENDLYNAME,
			NULL,
			(PBYTE)szBuf,
			MAX_PATH,
			NULL))
		{

		//	str.Format(_T("%03d   :   FriendName   =   %s\n"), i, szBuf);
			str.Format(_T("%s"),szBuf);
			m_CommboBoxCommName.AddString(str);
			if (m_ComName.GetLength()>0 && str.Find(_T("(")+m_ComName+_T(")"))>0)
			{
				nSel = nPos;;
			}
			nPos++;
		//	AfxMessageBox(str);

		}
		//   得到设备在注册表中的子路径    

		if (SetupDiGetDeviceRegistryProperty(
			hDevInfo,
			&spDevInfoData,
			SPDRP_DRIVER,
			NULL,
			(PBYTE)szBuf,
			MAX_PATH,
			NULL))
		{
			str.Format(_T("%03d   :   Regedit   Path   =   %s\n"), i, szBuf);
			//AfxMessageBox(str);
		}
	}

	m_CommboBoxCommName.SetCurSel(nSel);

	str.Format(_T("\nPort  count   =   %d\n"), i);
//	AfxMessageBox(str);

	return   0;

}
CString CIAPDlg::GetCommName()
{
	CString str, strCommName;
	int nPos;
	nPos = m_CommboBoxCommName.GetCount();

	if (nPos == 0)
		return NULL;


	nPos = m_CommboBoxCommName.GetCurSel();
	m_CommboBoxCommName.GetLBText(nPos, str);
	nPos = 0;
	str.MakeUpper();
	nPos = str.Find(_T("(COM"), 0);
	strCommName = str.Right(str.GetLength() - nPos);
	strCommName.Remove('(');
	strCommName.Remove(')');

	m_strCommName = strCommName;

//	m_strCommName = _T("\\\\.\\") + m_strCommName;
	return m_strCommName;
}

void CIAPDlg::ChBtnState(BOOL state)				//打开串口使能按键
{
	m_BtnChSN.EnableWindow(state);
	m_BtnState.EnableWindow(state);
	m_BtnRelay.EnableWindow(state);
	m_BtnClOffline.EnableWindow(state);
	m_BtnClAll.EnableWindow(state);
	m_BtnTime.EnableWindow(state);
	m_BtnDL.EnableWindow(state);
	m_BtnBoot.EnableWindow(state);
	m_BtnJumpIap.EnableWindow(state);
	m_BtnSuperCardTime.EnableWindow(state);
	m_BtnLogOutTime.EnableWindow(state);

	m_BtnSetSuperCard.EnableWindow(state);

	m_BtnClearSuperCard.EnableWindow(state);
	m_BtntDeleteSuperCard.EnableWindow(state);

	m_BtnSetDns.EnableWindow(state);
	m_BtnSetIP.EnableWindow(state);
	m_BtntSetTCP.EnableWindow(state);

	m_BtntSetWIFI.EnableWindow(state);
	m_BtntLogOff.EnableWindow(state);
	m_ComboBaudrate.EnableWindow(!state);
	m_CommboBoxCommName.EnableWindow(!state);

}

void CIAPDlg::OnBnClickedBtnOpenComm()
{
	CString nameStr;
	m_BtnOpenCommPort.GetWindowTextW(nameStr);
	

	CString str,Token;
	int nPos = 0;
	if (nameStr == L"关闭串口")
	{
		CloseComm();
		nameStr = L"打开串口";
		m_BtnOpenCommPort.SetWindowTextW(nameStr);
		ChBtnState(FALSE);
		return;
	}
	CleanDlState();

	str = GetCommName();
	m_ComName = str;
	Token = _T("COM");
	str = str.Tokenize(Token, nPos);

	DWORD dwPortNum;
	dwPortNum = _wtoi(str);

	m_SerialPort.SetWnd(this->m_hWnd);
	m_SerialPort.SetNotifyNum(1);
	m_SerialPort.Open(dwPortNum);
	m_ComboBaudrate.GetWindowTextW(m_Baudrate);
	m_SerialPort.SetState( _wtoi(m_Baudrate));

	if (m_SerialPort.IsOpen()) {
		nameStr = L"关闭串口";
		m_BtnOpenCommPort.SetWindowTextW(nameStr);
		ChBtnState(TRUE);
		WritePrivateProfileString(m_Session, m_COMKey, m_ComName.GetBuffer(), m_IniFileName);   //写入ini文件中相应字段
		WritePrivateProfileString(m_Session, m_BaudKey, m_Baudrate.GetBuffer(), m_IniFileName);   //写入ini文件中相应字段
	} else {
		nameStr = L"打开串口";
		m_BtnOpenCommPort.SetWindowTextW(nameStr);
		ChBtnState(FALSE);
	}
}

void CIAPDlg::CloseComm()
{
	m_SerialPort.Close();
}

BOOL CIAPDlg::Contains(BYTE *buf, BYTE key, DWORD len)			//把字符送入数组
{
	for (DWORD i=0; i<len; i++) {
		if (buf[i] == key)
			return TRUE;
	}
	return FALSE;
}

void CIAPDlg::CleanDlState(void) {				//清除文件信息
	m_FWFileLen = 0;
	m_FWFilePos = 0;
	if (m_FWBuf!=NULL)
		delete m_FWBuf;
	m_FWBuf = NULL;
}

afx_msg LRESULT CIAPDlg::OnRevFrame(WPARAM wParam, LPARAM lParam)
{
	DWORD dwBufLen = 1024;
	DWORD dwRetLen;
	TCHAR wBuf[1024];
	TCHAR pBuf[20];
	int cmd=0, RemainTime=0, nCnt=0, match=0;
	CString str, SN;

	char h = 0, m = 0, s = 0;

	//	MessageBox(L"发生串口接收事件,准备读数据");
	memset(m_COMBuf, 0, 1024);
	ZeroMemory(wBuf, 1024);
	dwRetLen = m_SerialPort.Read(m_COMBuf, dwBufLen);//读出一帧数据
	MultiByteToWideChar(CP_ACP,0, (LPCCH)m_COMBuf,dwRetLen, wBuf, 1024);			//转换成款字节序
	str.Format(_T("%s"), wBuf);
	AppendTextToRich(str.GetBuffer());			//操作主文本框
	
	if (m_FWFileLen>0) {					//文件长度大于0  开始
		// YMODEM mode
		CString state;
		BYTE buf[3+1024+2];
		memset(buf, 0x20, 1029);
		
		if (Contains(m_COMBuf, ACK, dwRetLen)) {
			if (m_FWPkg>0) {
				m_FWFilePos+=1024;
			}
			if (m_FWFilePos>=m_FWFileLen) {
				// Transmit complete
				CleanDlState();
				state.Format(_T("下载完成！"));				
				m_DLState.SetWindowTextW(state);				
			}
			m_FWPkg++;
		}

		if (Contains(m_COMBuf, CRC16, dwRetLen) && m_FWFileLen>0) {
			buf[0] = STX;
			buf[1] = m_FWPkg & 0xFF;
			buf[2] = 0xFF - (m_FWPkg & 0xFF);

			state.Format(_T("下载进度：%d\\%d"), m_FWFilePos, m_FWFileLen);
			m_DLState.SetWindowTextW(state);
			if (m_FWPkg ==0) {					//第一个包
				DWORD p;
				p = sprintf((char*)(buf+3), "test.bin");
				sprintf((char*)(buf+5+p), "%d", m_FWFileLen);
			} else {
				memcpy(buf+3, m_FWBuf+m_FWFilePos, 1024);
			}

			unsigned short crc = crc_ccitt(buf+3, 1024);
			memcpy(buf+1027, &crc, 2);
			m_SerialPort.Write(buf, 1029);				//串口发送
		}
		

	} else {
		// CMD mode
		while (AfxExtractSubString(str, wBuf, nCnt++, _T(','))) {
			switch (nCnt) {
			case 1:
				if (str.Compare(_T("$MCU"))==0)
					match++;
				break;
			case 2:
				if (str.Compare(_T("3"))==0)
					cmd = _ttoi(str);
					match++;
				break;
			case 3:
				if (match==2) {
					SN =str;
					match++;
				}
				break;
			case 4:
				if (match==3) {
					RemainTime = _ttoi(str);
				}
				break;
			default:
				break;
			}
		}

		//MessageBox(str);
		if (match==3) {
			if (cmd == 3) {
				// 心跳包
				m_SN.SetWindowTextW(SN);
	
				str.Format(_T("%d"), RemainTime);
				m_RemainTime.SetWindowTextW(str);
			}
		}
	}
	

	return 0;
}

BOOL CIAPDlg::AppendTextToRich(TCHAR *text)			//操作主文本框
{
    //1、自动换行
    //2、自动滚动
    //3、尾行添加文本
    //4、重新开始时自动清除所有内容

	HWND hWndRich=::GetDlgItem(AfxGetMainWnd()->m_hWnd, IDC_EDIT3);     
         
    ::SendMessage(hWndRich, EM_SETSEL, -2, -1);
    ::SendMessage(hWndRich, EM_REPLACESEL, 0, (LPARAM)(text));
 
    //自动滚动到最后一行
    ::SendMessage(hWndRich, WM_VSCROLL, SB_BOTTOM, 0);
    return TRUE;   

}




void CIAPDlg::ReadFWFile(CString filePath)		//读文件函数
{
	CString str;
	if (filePath.GetLength()>0) {
		//读取文件
		CFile file(filePath,  CFile::modeRead);
		m_FWFileLen = file.GetLength();				//文件大小      
		m_FWFilePos = 0;
		m_FWPkg = 0;
		m_FWBuf = new BYTE[m_FWFileLen +1];
		memset(m_FWBuf, 0, m_FWFileLen+1);
		file.Read(m_FWBuf, m_FWFileLen);			///读取文件大小
		file.Close();
	}
}

void CIAPDlg::OnClose()				//关闭串口
{
	if (m_FWBuf != NULL)
	{
		delete m_FWBuf;
	}
	if (m_SerialPort.IsOpen())
		m_SerialPort.Close();
	CDialogEx::OnClose();
}











void CIAPDlg::OnCbnDropdownCombo1()
{
	GetDevCommInfo();
}


void CIAPDlg::OnBnClickedBtnDlfw()						//点击下载固件
{
	UpdateData();
	if (m_FWFile.GetLength()>0) {					//文件路径长度>0
		ReadFWFile(m_FWFile);
	} else {
		MessageBox(_T("请先选择固件！"));
	}
}


int CIAPDlg::GetComNum(CString ComName) {
	DWORD dwPortNum = 0;
	int nPos = 0;
	if (ComName.GetLength()>0) {
		CString Token = _T("COM");
		CString str;
		str = str.Tokenize(Token, nPos);

		
		dwPortNum = _wtoi(str);
	}
	return dwPortNum;
}

void CIAPDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent) {
	case TIMER1:
		if (!m_SerialPort.IsOpen()) {
			ChBtnState(FALSE);
			m_BtnOpenCommPort.SetWindowTextW(L"打开串口");
		}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CIAPDlg::OnBnClickedBtnBrowse()			//浏览APP文件
{
	CFileDialog *lpszOpenFile;    //定义一个CfileDialog对象
	lpszOpenFile = new CFileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, _T("文件类型(*.bin)|*.bin||"));//生成一个对话框
	if (lpszOpenFile->DoModal() == IDOK)//点击对话框确定按钮,读文件
	{
		m_FWFile = lpszOpenFile->GetPathName();//得到保存文件的路径
		WritePrivateProfileString(m_Session, m_FWKey, m_FWFile.GetBuffer(256), m_IniFileName);   //写入ini文件中相应字段
		UpdateData(FALSE);
	}

	delete lpszOpenFile;//释放分配的对话框
}

void CIAPDlg::OnBnClickedBtnBrowse2()			//浏览IAP文件
{
	CFileDialog *lpszOpenFile;    //定义一个CfileDialog对象
	lpszOpenFile = new CFileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, _T("文件类型(*.bin)|*.bin||"));//生成一个对话框
	if (lpszOpenFile->DoModal() == IDOK)//点击对话框确定按钮,读文件
	{
		m_Bootloader = lpszOpenFile->GetPathName();//得到保存文件的路径
		WritePrivateProfileString(m_Session, m_BLKey, m_FWFile.GetBuffer(256), m_IniFileName);   //写入ini文件中相应字段
		UpdateData(FALSE);
	}

	delete lpszOpenFile;//释放分配的对话框
}


void CIAPDlg::OnBnClickedBtnState()					 //串口打印状态
{
	BYTE str[128];
	sprintf((char*)str, "$PC,8\r\n");
	m_SerialPort.WriteStringA((LPTSTR)str);
}

void CIAPDlg::OnBnClickedBtnRelay()						//擦除flash数据
{
	BYTE str[128];
	sprintf((char*)str, "$PC,22\r\n");
	m_SerialPort.WriteStringA((LPTSTR)str);
}


void CIAPDlg::OnBnClickedBtnClOffline()					//清空预约数据
{
	BYTE str[128];
	sprintf((char*)str, "$PC,14\r\n");
	m_SerialPort.WriteStringA((LPTSTR)str);
}

void CIAPDlg::OnBnClickedBtnClAll()			//清空所有数据
{
	BYTE str[128];
	sprintf((char*)str, "$PC,15\r\n");
	m_SerialPort.WriteStringA((LPTSTR)str);
}


void CIAPDlg::OnBnClickedBtnTime()			//同步时间
{
	BYTE str[128];
	time_t t;
	time(&t);
	sprintf((char*)str, "$PC,9,%d\r\n", t);
	m_SerialPort.WriteStringA((LPTSTR)str);
}

void CIAPDlg::OnBnClickedBtnBoot()					//更新bootlaoder
{
	// TODO: 在此添加控件通知处理程序代码
	BYTE str[128];
	UpdateData();
	if (m_Bootloader.GetLength()>0) {
		ReadFWFile(m_Bootloader);
	} else {
		MessageBox(_T("请先选择固件！"));
	}
	sprintf((char*)str, "$PC,16,%d\r\n", m_FWFileLen);
	m_SerialPort.WriteStringA((LPTSTR)str);
}




void CIAPDlg::OnBnClickedBtnChsn()				//设置SN号
{
	BYTE sn[64];
	TCHAR wSN[64];
	BYTE str[128];

	m_SN.GetWindowTextW((LPTSTR)wSN, 64);				
	WideCharToMultiByte(CP_ACP, 0, wSN, 64, (LPSTR)sn, 64, NULL, NULL);		
	memset(str, 0, 128);
	sprintf((char*)str, "$PC,99,%s\r\n", sn);			
	m_SerialPort.WriteStringA((LPTSTR)str);				
}









void CIAPDlg::OnBnClickedButtonJumpIap()			//IAP跳转
{
	BYTE str[128];
	sprintf((char*)str, "$PC,100\r\n");
	m_SerialPort.WriteStringA((LPTSTR)str);			
}





void CIAPDlg::OnBnClickedButtonSuperCardTime()		//设置特权卡使用时间
{
	BYTE SuperCardTime[64];
	TCHAR wSuperCardTime[64];
	BYTE str[128];

	m_EditSuperCardTime.GetWindowTextW((LPTSTR)wSuperCardTime, 64);				
	WideCharToMultiByte(CP_ACP, 0, wSuperCardTime, 64, (LPSTR)SuperCardTime, 64, NULL, NULL);		
	memset(str, 0, 128);
	sprintf((char*)str, "$PC,18,%s\r\n", SuperCardTime);			
	m_SerialPort.WriteStringA((LPTSTR)str);				
}


void CIAPDlg::OnBnClickedButtonLogOutTime()				//设置刷卡退出时间
{
	BYTE LogOutTime[64];
	TCHAR wLogOutTime[64];
	BYTE str[128];

	m_EditLogOutTime.GetWindowTextW((LPTSTR)wLogOutTime, 64);
	WideCharToMultiByte(CP_ACP, 0, wLogOutTime, 64, (LPSTR)LogOutTime, 64, NULL, NULL);		
	memset(str, 0, 128);
	sprintf((char*)str, "$PC,17,%s\r\n", LogOutTime);			
	m_SerialPort.WriteStringA((LPTSTR)str);				
}





void CIAPDlg::OnBnClickedButtonSetSuperCard()			//设置超级卡
{
	BYTE SuperCard[256];
	TCHAR wSuperCard[256];
	BYTE str[512];

	m_EditSetSuperCard.GetWindowTextW((LPTSTR)wSuperCard, 256);
	WideCharToMultiByte(CP_ACP, 0, wSuperCard, 256, (LPSTR)SuperCard, 256, NULL, NULL);		
	memset(str, 0, 512);
	sprintf((char*)str, "$PC,4,%s\r\n", SuperCard);			
	m_SerialPort.WriteStringA((LPTSTR)str);				
}




void CIAPDlg::OnBnClickedButtonDeleteSuperCard()			//删除指定的特权卡
{
	BYTE SuperCard[256];
	TCHAR wSuperCard[256];
	BYTE str[512];

	m_EditDeleteSuperCard.GetWindowTextW((LPTSTR)wSuperCard, 256);
	WideCharToMultiByte(CP_ACP, 0, wSuperCard, 256, (LPSTR)SuperCard, 256, NULL, NULL);		
	memset(str, 0, 512);
	sprintf((char*)str, "$PC,21,%s\r\n", SuperCard);			
	m_SerialPort.WriteStringA((LPTSTR)str);
}





void CIAPDlg::OnBnClickedButtonClearSuperCard()			//清除所有特权卡
{
	BYTE str[128];
	sprintf((char*)str, "$PC,20\r\n");
	m_SerialPort.WriteStringA((LPTSTR)str);
}


void CIAPDlg::OnBnClickedButtonSetDns()				//配置DNS
{
	BYTE DNS[64];
	TCHAR wDNS[64];
	BYTE str[128];

	m_EditSetDns.GetWindowTextW((LPTSTR)wDNS, 64);
	WideCharToMultiByte(CP_ACP, 0, wDNS, 64, (LPSTR)DNS, 64, NULL, NULL);		
	memset(str, 0, 128);
	sprintf((char*)str, "$PC,17,%s\r\n", DNS);		
	m_SerialPort.WriteStringA((LPTSTR)str);
}


void CIAPDlg::OnBnClickedButtonSetTcp()			//配置TCP
{
	BYTE TCP[128];
	TCHAR wTCP[128];
	BYTE str[256];

	m_EditSetTCP.GetWindowTextW((LPTSTR)wTCP, 128);
	WideCharToMultiByte(CP_ACP, 0, wTCP, 128, (LPSTR)TCP, 128, NULL, NULL);		
	memset(str, 0, 256);
	sprintf((char*)str, "$PC,7,%s\r\n", TCP);			
	m_SerialPort.WriteStringA((LPTSTR)str);
}


void CIAPDlg::OnBnClickedButtonSetIp()			//配置 静态IP
{
	BYTE IP[128];
	TCHAR wIP[128];
	BYTE str[256];

	m_EditSetIP.GetWindowTextW((LPTSTR)wIP, 128);
	WideCharToMultiByte(CP_ACP, 0, wIP, 128, (LPSTR)IP, 128, NULL, NULL);		
	memset(str, 0, 256);
	sprintf((char*)str, "$PC,23,%s\r\n", IP);			
	m_SerialPort.WriteStringA((LPTSTR)str);
}





void CIAPDlg::OnBnClickedButtonSetWifi()				//配置WIFI
{
	BYTE WIFI[128];
	TCHAR wWIFI[128];
	BYTE str[256];

	m_EditSetWIFI.GetWindowTextW((LPTSTR)wWIFI, 128);
	WideCharToMultiByte(CP_ACP, 0, wWIFI, 128, (LPSTR)WIFI, 128, NULL, NULL);		//转换字体
	memset(str, 0, 256);
	sprintf((char*)str, "$PC,6,%s\r\n", WIFI);			//打印成要发送的格式
	m_SerialPort.WriteStringA((LPTSTR)str);
}




void CIAPDlg::OnCbnSelchangeCombo2()
{

}


void CIAPDlg::OnBnClickedButtonClear()				//清除窗口信息
{
	BYTE WIFI[2]=" ";
	m_RecvInfo.SetWindowTextW((LPTSTR)WIFI);
}





void CIAPDlg::OnBnClickedButtonLogOff()				//修改蓝牙名称
{
	BYTE str[64];
	sprintf((char*)str, "$PC,16\r\n");
	m_SerialPort.WriteStringA((LPTSTR)str);
}
