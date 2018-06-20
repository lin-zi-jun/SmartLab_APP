
// IAPDlg.h : 头文件
//

#pragma once
#include "Serial.h"
#include "afxwin.h"

// CIAPDlg 对话框
class CIAPDlg : public CDialogEx
{
// 构造
public:
	CIAPDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_IAP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnBrowse();
	void ReadFWFile(CString filePath);
	CString m_FWFile;
	BYTE *m_FWBuf;
	DWORD m_FWFileLen;
	DWORD m_FWFilePos;
	DWORD m_FWPkg;
	CString m_IniFileName;
	CString m_Session;
	CString m_FWKey;
	CString m_BLKey;
	CString m_COMKey;
	CString m_BaudKey;
	afx_msg void OnClose();


	// 打开串口按钮
	CButton m_BtnOpenCommPort;
	int GetDevCommInfo();
	//关闭串口
	void CloseComm();
	//打开串口
	afx_msg void OnBnClickedBtnOpenComm();
	//串口名
	CString m_strCommName;
	CString m_ComName;
	CString m_Baudrate;
	//a获取串口名
	CString GetCommName();
	// 串口类变量
	CSerial m_SerialPort;
	BYTE m_COMBuf[1024];
	BOOL Contains(BYTE *buf, BYTE key, DWORD len);
	int GetComNum(CString ComName);
	void CleanDlState(void);

	afx_msg LRESULT OnRevFrame(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnBrowse2();
	CString m_Bootloader;
	CComboBox m_CommboBoxCommName;
	CComboBox m_ComboBaudrate;
	CEdit m_RecvInfo;
	BOOL AppendTextToRich(TCHAR *text);
	afx_msg void OnBnClickedBtnChsn();
	CEdit m_SN;
	CButton m_BtnChSN;
	CEdit m_RemainTime;
	afx_msg void OnBnClickedBtnState();
	CButton m_BtnState;
	afx_msg void OnBnClickedBtnRelay();
	afx_msg void OnBnClickedBtnClOffline();
	afx_msg void OnBnClickedBtnClAll();
	afx_msg void OnBnClickedBtnTime();
	CButton m_BtnRelay;
	CButton m_BtnTime;
	CButton m_BtnClOffline;
	CButton m_BtnClAll;
	void ChBtnState(BOOL state);
	afx_msg void OnCbnDropdownCombo1();
	afx_msg void OnBnClickedBtnDlfw();
	CButton m_BtnDL;
	CStatic m_DLState;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnBoot();
	CButton m_BtnBoot;
	afx_msg void OnEnChangeEditSn();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnBnClickedCheckDns();

	CButton m_BtnJumpIap;
	afx_msg void OnBnClickedButtonJumpIap();

	CButton m_BtnSuperCardTime;
	CEdit m_EditSuperCardTime;
	afx_msg void OnEnChangeEditSuperCardTime();
	afx_msg void OnBnClickedButtonSuperCardTime();

	CButton m_BtnLogOutTime;
	CEdit m_EditLogOutTime;
	afx_msg void OnBnClickedButtonLogOutTime();
	afx_msg void OnEnChangeEditLogOutTime();

	CButton m_BtnSetSuperCard;
	CEdit m_EditSetSuperCard;

	CButton m_BtnClearSuperCard;
	CButton m_BtntDeleteSuperCard;
	CEdit m_EditDeleteSuperCard;

	afx_msg void OnBnClickedButtonSetSuperCard();
	afx_msg void OnEnChangeEditSetSuperCard();
	afx_msg void OnBnClickedButtonDeleteSuperCard();
	afx_msg void OnEnChangeEditDeleteSuperCard();
	afx_msg void OnBnClickedButtonClearSuperCard();

	CButton m_BtnSetDns;
	CButton m_BtnSetIP;
	CButton m_BtntSetTCP;
	CEdit m_EditSetDns;
	CEdit m_EditSetIP;
	CEdit m_EditSetTCP;

	afx_msg void OnBnClickedButtonSetDns();
	afx_msg void OnBnClickedButtonSetTcp();
	afx_msg void OnBnClickedButtonSetIp();
	afx_msg void OnEnChangeEditSetIp();
	afx_msg void OnEnChangeEditSetTcp();
	afx_msg void OnEnChangeEditSetDns();

	CButton m_BtntSetWIFI;
	CEdit m_EditSetWIFI;
	afx_msg void OnBnClickedButtonSetWifi();
	afx_msg void OnEnChangeEditSetWifi();
	afx_msg void OnCbnSelchangeCombo2();

	CButton m_BtntClear;
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnEnChangeEdit3();

	CButton m_BtntLogOff;
	afx_msg void OnBnClickedButtonLogOff();
};
