
// CoordinateGetterDlg.h : 头文件
//

#pragma once
#include "cv.h"
#include "highgui.h"
#include <cxcore.h>
#include "PaintTrack.h"
#include "afxcmn.h"

// CCoordinateGetterDlg 对话框;
class CCoordinateGetterDlg : public CDialogEx
{
// 构造;
public:
	CCoordinateGetterDlg(CWnd* pParent = NULL);	// 标准构造函数;
	BOOL ROBOT_INIT();	//开机器人;
	BOOL ROBOT_END();   //关机器人;
	void ROBOT_MOVE(double xx,double yy,int zz);	//动机器人;
	void ROBOT_VALVE(); //开关气阀;
// 对话框数据;
	enum { IDD = IDD_COORDINATEGETTER_DIALOG };

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
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	// 输入XYZ
	CString m_XYZ_I;
	// 姿态角
	CString m_showAngles;
	afx_msg void OnBnClickedButtonRead();
	// 显示直角坐标;
	CString m_showXYZ;
	afx_msg void OnBnClickedButtonOpenEyes();
	double m_XX_in;
	double m_YY_in;
	afx_msg void OnBnClickedButtonBrowse();
	IplImage * m_img;

	afx_msg void OnBnClickedButtonCanny();
	int m_canny_threshold;
	int m_draw_step;

	IplImage* Bord;
	afx_msg void OnBnClickedButtonTrack();
	CString m_filepath;
//
	//PaintTrack m_pt;
	double m_lt_x;
	double m_lt_y;
	double m_rt_x;
	double m_rt_y;
	double m_rd_x;
	double m_rd_y;
	afx_msg void OnBnClickedButtonOff();
	afx_msg void OnEnChangeThreshold();
	int m_threshold;
	// 给定的Z
	double m_z_ref;
	BOOL m_check;
	afx_msg void OnEnChangeEditZRef();
	CSpinButtonCtrl m_spin;
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeEdit2();
};
