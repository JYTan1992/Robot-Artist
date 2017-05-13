
// cvIndoorDlg.h : 头文件
//

#pragma once

#include <opencv2\opencv.hpp>
#include "afxcmn.h"
// CcvIndoorDlg 对话框
class CcvIndoorDlg : public CDialogEx
{
// 构造
public:
	CcvIndoorDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CVINDOOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数;
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonBrowse();
	void OnBnClickedCamera();
	void periLooks();
	// 找到的轮廓数目;
	CString m_contours_in;
	afx_msg void OnBnClickedButtonSobel();
	IplImage * m_img;
	afx_msg void OnBnClickedButtonLaplace();
	void invColor(IplImage *img);
	IplImage *diffImg(IplImage *src, IplImage *dst);//src-dst;
	int blackNum(IplImage *img);
	CvPoint OctAdj(IplImage *img,CvPoint p);
	afx_msg void OnBnClickedButtonOctAdj();
	afx_msg void OnBnClickedButtonZhangsuenBest();
	CString m_filepath;
	afx_msg void OnBnClickedButtonChoi();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonRsfd();
	void cvRosenfeld(cv::Mat& src, cv::Mat& dst);
	void cvHilditchThin(cv::Mat& src, cv::Mat& dst);
	afx_msg void OnBnClickedHilditch();
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnNMCustomdrawSlider(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl m_slider;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCanny();
	int m_int;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedTrack();
	afx_msg void OnBnClickedPainttrack();
//	int m_threshold;
	int m_threshold;
};
//void cvRosenfeld(cv::Mat& src, cv::Mat& dst);