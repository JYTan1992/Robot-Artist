
// cvIndoorDlg.cpp : ʵ���ļ�;
//

#include "stdafx.h"
#include "cvIndoor.h"
#include "cvIndoorDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>

#include <iostream>
#include <string>

#include "zhangsuenbest.h"
#include "choi.h"
#include "PaintTrack.h"
using namespace cv;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IplImage* Bord;
CcvIndoorDlg *pDlg;

double ZA[2]={0},ZC=0;
void setZ(double x1,double y1,double z1,double x2,double y2,double z2,double x3,double y3,double z3){
	ZA[0]=((z2-z1)*(y3-y1)-(z3-z1)*(y2-y1))/((x2-x1)*(y3-y1)-(x3-x1)*(y2-y1));
	ZA[1]=((z2-z1)*(x3-x1)-(z3-z1)*(x2-x1))/((y2-y1)*(x3-x1)-(y3-y1)*(x2-x1));
	ZC=z1-ZA[0]*x1-ZA[1]*y1;
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������;
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��;

// ʵ��;
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


// CcvIndoorDlg �Ի���;




CcvIndoorDlg::CcvIndoorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CcvIndoorDlg::IDD, pParent)
	, m_contours_in(_T(""))
	, m_threshold(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_threshold = 0;
}

void CcvIndoorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_contours_in);
	DDX_Control(pDX, IDC_SLIDER, m_slider);
	DDX_Text(pDX, IDC_THRESHOLD, m_threshold);
	DDV_MinMaxInt(pDX, m_threshold, 1, 100);
}

BEGIN_MESSAGE_MAP(CcvIndoorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CcvIndoorDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CcvIndoorDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_SOBEL, &CcvIndoorDlg::OnBnClickedButtonSobel)
	ON_BN_CLICKED(IDC_BUTTON_LAPLACE, &CcvIndoorDlg::OnBnClickedButtonLaplace)
	ON_BN_CLICKED(IDC_BUTTON_OCT_ADJ, &CcvIndoorDlg::OnBnClickedButtonOctAdj)
	ON_BN_CLICKED(IDC_BUTTON_ZHANGSUEN_BEST, &CcvIndoorDlg::OnBnClickedButtonZhangsuenBest)
	ON_BN_CLICKED(IDC_BUTTON_CHOI, &CcvIndoorDlg::OnBnClickedButtonChoi)
	ON_BN_CLICKED(IDC_BUTTON_RSFD, &CcvIndoorDlg::OnBnClickedButtonRsfd)
	ON_BN_CLICKED(IDC_Hilditch, &CcvIndoorDlg::OnBnClickedHilditch)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER, &CcvIndoorDlg::OnNMCustomdrawSlider)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CANNY, &CcvIndoorDlg::OnBnClickedCanny)
	ON_BN_CLICKED(IDC_PAINTTRACK, &CcvIndoorDlg::OnBnClickedPainttrack)
END_MESSAGE_MAP()


// CcvIndoorDlg ��Ϣ�������;

BOOL CcvIndoorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�;;

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�;
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�;
	//  ִ�д˲���;
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��;
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��;

	// TODO: �ڴ���Ӷ���ĳ�ʼ������;
	m_slider.SetRange(1,100);//���û�����Χ
	m_slider.SetTicFreq(1);
	m_slider.SetPos(1);   
    // �ڱ༭������ʾ20   
    SetDlgItemInt(IDC_EDIT1,1);
	m_img = NULL;
	m_int=1;
	pDlg=this;
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE;
}

void CcvIndoorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CcvIndoorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���;
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CcvIndoorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CcvIndoorDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	periLooks();

	#if 0
	//int i = 0;
	int mode = CV_RETR_CCOMP;	//��ȡ������ģʽ;
	int contours_num = 0;
	CvScalar external_color;
	CvScalar hole_color;
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contour = 0;				//�洢��ȡ����������ָ��;
	IplImage * pImg = NULL;
	IplImage * pContourImg = NULL;	//��ʾ��ȡ������ͼ��;

	IplImage *src;
	//CString numofcontours;
	src = cvLoadImage("btcr.jpg",-1);
	pImg = cvCreateImage(cvGetSize(src),src->depth,1);
	pContourImg = cvCreateImage(cvGetSize(pImg),IPL_DEPTH_8U,3);
	cvCvtColor(src,pImg,CV_BGR2GRAY);	//תΪ�Ҷ�ͼ��;
	
	
;
	//תΪ��ֵͼ��;
	cvThreshold(pImg,pImg,128.0f,255,CV_THRESH_BINARY);
	//cvFindContours������������;
	mode = CV_RETR_LIST;
	contours_num = cvFindContours(pImg,storage,&contour,sizeof(CvContour),mode,CV_CHAIN_APPROX_NONE);
	m_contours_in.Format("��������%d",contours_num);
	UpdateData(FALSE);
	for (; contour!=0; contour=contour->h_next)
	{
		hole_color = CV_RGB(rand()&255,rand()&255,rand()&255);
		external_color = CV_RGB(rand()&255,rand()&255,rand()&255);
		cvDrawContours(pContourImg,contour,external_color,hole_color,1,2,8);

	}
	//MessageBox()
	cvNamedWindow("src", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("contour", CV_WINDOW_AUTOSIZE);
	cvShowImage("src",src);
	cvShowImage("contour",pContourImg);
		cvWaitKey(0);
	//cvDestroyWindow("cpi.jpg");
		cvReleaseImage(&src);
		cvReleaseImage(&pImg);
		cvReleaseImage(&pContourImg);
		cvReleaseMemStorage(&storage);
#endif
	#if 0
Mat img = imread("cpi.jpg");
	     if(img.empty())
		     {
			         MessageBox("ERROR");
			         CDialogEx::OnOK();
			     }
	    imshow("��Ǯ����",img);
	     waitKey();
#endif

	//CDialogEx::OnOK();
}


void CcvIndoorDlg::OnBnClickedButtonBrowse()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	LPCTSTR lpszFilter="(*.bmp;*.jpg)|*.bmp;*.jpg|�κ��ļ�|*.*||";
	CFileDialog dlg(TRUE,lpszFilter,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,lpszFilter,NULL);
	
	//CString m_filepath;

	IplImage * img = NULL;
	if(dlg.DoModal()==IDOK)
	{
		m_filepath=dlg.GetPathName();
		std::string imgpath = m_filepath.GetBuffer(0);
		m_filepath.ReleaseBuffer();
		///////////////////////////////////
		m_img = cvLoadImage(m_filepath,-1);
		cvNamedWindow("origin",1);
		cvShowImage("origin",m_img);
		cvWaitKey(0);
		cvDestroyWindow("origin");
		cvReleaseImage(&m_img);
		///////////////////////////////////
		#if 0
Mat img = imread(imgpath);
		if(img.empty())
		{
			MessageBox("ERROR");
			CDialogEx::OnOK();
		}
		imshow(imgpath,img);
		waitKey();
#endif
		
	}
	//cvHoughCircles
}


void CcvIndoorDlg::periLooks()
{
	IplImage * pImg = NULL;
	IplImage * pCannyImg = NULL;	//��ʾ��ȡ������ͼ��;
	pImg = cvLoadImage("btcr.jpg",-1);
	pCannyImg = cvCreateImage(cvGetSize(pImg),IPL_DEPTH_8U,1);
	cvCanny(pImg,pCannyImg,50,150,3);
	cvNamedWindow("canny",1);
	cvShowImage("canny",pCannyImg);
	cvWaitKey(0);
	cvDestroyWindow("canny");
	cvReleaseImage(&pCannyImg);
}




void CcvIndoorDlg::OnBnClickedButtonSobel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	IplImage * pImage = NULL;
	IplImage * pImgSobelGray = NULL;
	IplImage * pImg8u = NULL;
	IplImage * pImg8uSmooth = NULL;
	IplImage * pImgColor = NULL;
	IplImage * pImgSobelColor = NULL;
	IplImage * pImgPlanes[3] = {0,0,0};
	CvSize uniSize;//, colorSize;
	int blacks=0;
	pImage = cvCloneImage(m_img);
	uniSize = cvGetSize(pImage);
	//colorSize = cvSize(pImage->width,pImage->height);

	pImg8u = cvCreateImage(uniSize,IPL_DEPTH_8U,1);
	pImg8uSmooth = cvCreateImage(uniSize,IPL_DEPTH_8U,1);
	//�ԻҶ�ͼ�����Sobelת��;
	//����ɫͼ��Ϊ�Ҷ�ͼ��;
	cvCvtColor(pImage,pImg8u,CV_BGR2GRAY);
	//��ͼ����и�˹�˲�;
	cvSmooth(pImg8u,pImg8uSmooth,CV_GAUSSIAN,3,0,0);
	//����һ���µ�ͼ���ڴ�����ͼ��Ԫ�ص�λ�����Ϊ;
	//IPL_DEPTH_16S;�з���16λ����;
	//��Ϊ cvSobel����Ҫ��Ŀ��ͼ�������16bitͼ��;
	pImgSobelGray = cvCreateImage(uniSize,IPL_DEPTH_16S,1);
	//����һ��x�����ͼ���֣��ɸ�����Ҫ���ò���;
	cvSobel(pImg8uSmooth,pImgSobelGray,0,1,3);
	//��ͼ���ʽ��ת��������������ʾ;
	cvConvertScaleAbs(pImgSobelGray,pImg8u,1,0);
	cvNamedWindow("Sobel Gray Image",CV_WINDOW_NORMAL);
	invColor(pImg8u);//��ɫ
	cvThreshold(pImg8u,pImg8u,225.0f,255,CV_THRESH_BINARY);//��ֵ��;
	invColor(pImg8u);
	cvShowImage("Sobel Gray Image",pImg8u);
	blacks = blackNum(pImg8u);
	CString infofo;
	infofo.Format("%f percent",double(blacks)*100.0/double(1280*960));
	
	//MessageBox()
	//�Բ�ɫͼ�����Sobel�任;
	//��������ͼ���ڴ����Դ洢RGBͨ��;
	//����λ�����ΪIPL_DEPTH_8U;
	int channel;
	for (channel=0; channel<3; ++channel)
	{
		pImgPlanes[channel] = cvCreateImage(uniSize,8,1);
	}
	//����һ���µ�ͼ���ڴ���������λ�����Ϊ;
	//IPL_DEPTH_16S�з���16λ����;
	pImgSobelColor = cvCreateImage(uniSize,IPL_DEPTH_16S,1);
	//Ҫ�����ͼ����16λ�з��ŵ�;
	pImgColor = cvCreateImage(uniSize,8,3);
	//����ɫͼ��ֳ�3����ͨ��ͼ��;
	cvSplit(pImage,pImgPlanes[0],pImgPlanes[1],pImgPlanes[2],0);
	for (channel=0; channel<3; ++channel)
	{	//�ֱ��ÿ��ͨ��ͼ�����Sobel�任;
		cvSobel(pImgPlanes[channel],pImgSobelColor,0,1,3);
		//ת��Ϊ8λ��ͼ��;
		cvConvertScaleAbs(pImgSobelColor,pImgPlanes[channel],1,0);
	}
	//������ͨ��ͼ����кϲ�;
	cvMerge(pImgPlanes[0],pImgPlanes[1],pImgPlanes[2],0,pImgColor);
	cvNamedWindow("Sobel Color Image",CV_WINDOW_NORMAL);
	invColor(pImgColor);	//��ɫ
	cvThreshold(pImgColor,pImgColor,225.0f,255,CV_THRESH_BINARY);//��ֵ��;
	cvShowImage("Sobel Color Image",pImgColor);
	MessageBox(infofo);
	cvWaitKey(0);
	cvDestroyWindow("Sobel Gray Image");
	cvDestroyWindow("Sobel Color Image");
	cvReleaseImage(&pImage);
	cvReleaseImage(&pImgSobelGray);
	cvReleaseImage(&pImgSobelColor);
	cvReleaseImage(&pImg8u);
	cvReleaseImage(&pImg8uSmooth);
}



void CcvIndoorDlg::OnBnClickedButtonLaplace()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	IplImage * pImage = NULL;
	IplImage * pImgLaplace = NULL;
	IplImage * pImg8u = NULL;
	IplImage * pImg8uSmooth = NULL;
	IplImage * pImgColor = NULL;
	IplImage * pImgLaplaceColor = NULL;
	IplImage * pImgPlanes[3] = {0,0,0};
	CvSize uniSize;//, colorSize;
	pImage = cvCloneImage(m_img);
	uniSize = cvGetSize(pImage);
	//������ԭʼͼ��һ����ͼ���ڴ���������λ���;
	//��Ϊ�޷���8λ����IPL_DEPTH_8U;

	pImg8u = cvCreateImage(uniSize,IPL_DEPTH_8U,1);
	pImg8uSmooth = cvCreateImage(uniSize,IPL_DEPTH_8U,1);
	//�ԻҶ�ͼ�����Laplaceת��;
	//����ɫͼ��Ϊ�Ҷ�ͼ��;
	cvCvtColor(pImage,pImg8u,CV_BGR2GRAY);
	//��ͼ����и�˹�˲�;
	cvSmooth(pImg8u,pImg8uSmooth,CV_GAUSSIAN,3,0,0);
	//����һ���µ�ͼ���ڴ�����ͼ��Ԫ�ص�λ�����Ϊ;
	//IPL_DEPTH_16S;�з���16λ����;
	//��Ϊ cvSobel����Ҫ��Ŀ��ͼ�������16bitͼ��;
	pImgLaplace = cvCreateImage(uniSize,IPL_DEPTH_16S,1);
	//����һ��x�����ͼ���֣��ɸ�����Ҫ���ò���;
	cvLaplace(pImg8uSmooth,pImgLaplace,3);
	//��ͼ���ʽ��ת��������������ʾ;
	cvConvertScaleAbs(pImgLaplace,pImg8u,1,0);
	cvNamedWindow("Laplace Gray Image",1);
	invColor(pImg8u);	//��ɫ;
	cvThreshold(pImg8u,pImg8u,200.0f,255,CV_THRESH_BINARY);//��ֵ��;
	cvSmooth(pImg8u,pImg8u,CV_GAUSSIAN,3,0,0);	//��˹�˲�;
	cvShowImage("Laplace Gray Image",pImg8u);
	
	//�Բ�ɫͼ�����Laplace�任;
	//��������ͼ���ڴ����Դ洢RGBͨ��;
	//����λ�����ΪIPL_DEPTH_8U;
	int channel;
	for (channel=0; channel<3; ++channel)
	{
		pImgPlanes[channel] = cvCreateImage(uniSize,8,1);
	}
	//����һ���µ�ͼ���ڴ���������λ�����Ϊ;
	//IPL_DEPTH_16S�з���16λ����;
	pImgLaplaceColor = cvCreateImage(uniSize,IPL_DEPTH_16S,1);
	//Ҫ�����ͼ����16λ�з��ŵ�;
	pImgColor = cvCreateImage(uniSize,8,3);
	//����ɫͼ��ֳ�3����ͨ��ͼ��;
	cvSplit(pImage,pImgPlanes[0],pImgPlanes[1],pImgPlanes[2],0);
	for (channel=0; channel<3; ++channel)
	{	//�ֱ��ÿ��ͨ��ͼ�����Sobel�任;
		cvLaplace(pImgPlanes[channel],pImgLaplaceColor,3);
		//ת��Ϊ8λ��ͼ��;
		cvConvertScaleAbs(pImgLaplaceColor,pImgPlanes[channel],1,0);
	}
	//������ͨ��ͼ����кϲ�;
	cvMerge(pImgPlanes[0],pImgPlanes[1],pImgPlanes[2],0,pImgColor);

	cvNamedWindow("Laplace Color Image",1);
	invColor(pImgColor);	//��ɫ;
	cvThreshold(pImgColor,pImgColor,128.0f,255,CV_THRESH_BINARY);//��ֵ��;
	cvSmooth(pImgColor,pImgColor,CV_GAUSSIAN,3,0,0);	//��˹�˲�;
	cvShowImage("Laplace Color Image",pImgColor);
	cvWaitKey(0);
	cvDestroyWindow("Laplace Gray Image");
	cvDestroyWindow("Laplace Color Image");
	cvReleaseImage(&pImage);
	cvReleaseImage(&pImgLaplace);
	cvReleaseImage(&pImgLaplaceColor);
	cvReleaseImage(&pImg8u);
	cvReleaseImage(&pImg8uSmooth);
}

void CcvIndoorDlg::invColor( IplImage *img )
{
	int i,j,k;
	int channels = img->nChannels;
	uchar *data = (uchar *) img->imageData;
	CvSize uniSize = cvGetSize(img);
	int step = img->widthStep; 
	for (i=0;i<uniSize.height;i++)  
	{  
		for (j=0;j<uniSize.width;j++)  
		{  
			for (k=0;k<channels;k++)  
			{  
				data[i*step+j*channels+k] = 255-data[i*step+j*channels+k];  
			}  
		}  
	}  
}

IplImage * CcvIndoorDlg::diffImg( IplImage *src, IplImage *dst )
{
	IplImage *res = NULL;
	return NULL;
}

int CcvIndoorDlg::blackNum( IplImage *img )
{
	int blacks=0;
	bool isblack = false;
	int i,j,k;
	int channels = img->nChannels;
	uchar *data = (uchar *) img->imageData;
	CvSize uniSize = cvGetSize(img);
	int step = img->widthStep; 
	for (i=0;i<uniSize.height;i++)  
	{  
		for (j=0;j<uniSize.width;j++)  
		{  
			for (k=0;k<channels;k++)  
			{  
				isblack = false;
				if(data[i*step+j*channels+k] != 0 )
				{
					break;
				}
				else
				{
					isblack = true;
				}
			}
			if (isblack)
			{
				blacks++;
			}
		}  
	}  
	return blacks;
}


void CcvIndoorDlg::OnBnClickedButtonOctAdj()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;

}
#if 0
CvPoint CcvIndoorDlg::OctAdj( IplImage *img,CvPoint p )
{
	CvPoint np;
	np = cvPoint(0,0);
	uchar *data = (uchar *) img->imageData;
	Mat

	if (_lableImg.at<int>(curX, curY-1) == 1)
	{// left pixel
		neighborPixels.push(std::pair<int,int>(curX, curY-1)) ;
	}
	if (_lableImg.at<int>(curX, curY+1) == 1)
	{// right pixel
		neighborPixels.push(std::pair<int,int>(curX, curY+1)) ;
	}
	if (_lableImg.at<int>(curX-1, curY) == 1)
	{// up pixel
		neighborPixels.push(std::pair<int,int>(curX-1, curY)) ;
	}
	if (_lableImg.at<int>(curX+1, curY) == 1)
	{// down pixel
		neighborPixels.push(std::pair<int,int>(curX+1, curY)) ;
	}
	return np;
}
#endif

void CcvIndoorDlg::OnBnClickedButtonZhangsuenBest()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	IMAGE data, im;
	int i,j;
	//char filename[128];
	CString temp_str;
	//printf ("Thinning. Please enter the input image file name.\n");
	//scanf ("%s", filename);
	//m_filepath;
	char *filename =  (LPSTR)(LPCTSTR)m_filepath;
	data = get_image (filename);
	if (data == NULL)
	{
		//MessageBox()
		temp_str.Format ("Bad input file '%s'\n", filename);
		//MessageBox(temp_str);
		exit(1);
	}
	display_image (data);

	im = newimage (data->info->nr+2, data->info->nc+2);
	for (i=0; i<data->info->nr; i++)
		for (j=0; j<data->info->nc; j++)
			im->data[i+1][j+1] = data->data[i][j];
	for (i=0; i<im->info->nr; i++) 
	{
		im->data[i][0] = 1;
		im->data[i][im->info->nc-1] = 1;
	}
	for (j=0; j<im->info->nc; j++)
	{
		im->data[0][j] = 1;
		im->data[im->info->nr-1][j] = 1;
	}

	/* Pre_process */
	pre_smooth (im);
	aae (im);

	/* Thin */
	thnz (im);

	for (i=0; i<data->info->nr; i++)
		for (j=0; j<data->info->nc; j++)
			data->data[i][j] = im->data[i+1][j+1];

	display_image (data);
	save_image (data, "zsout.jpg");
}


void CcvIndoorDlg::OnBnClickedButtonChoi()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	IplImage *data, *im, *d1, *d2;
	int i,j, **dy, **dx, nr, nc, Px, Py, Qix[9], Qiy[9], Qx, Qy, more=0;
	char *filename =  (LPSTR)(LPCTSTR)m_filepath;
	//char filename[128] = "H:\\AIPCV\\CH5\\5r.pbm";
	int flag = 0,jj,k,g,ii;
	CvScalar ss;
	int RADIUS = 120;
	float a1, a2;
	FILE *out;
	CString str_tmp;

	//printf ("Thinning. Please enter the input image file name.\n");
//	scanf ("%s", filename);

	data = cvLoadImage (filename, 1);
	if (data == NULL)
	{
		str_tmp.Format("Bad input file '%s'\n", filename);
		AfxMessageBox(str_tmp);
		exit(1);
	}
	//cvNamedWindow ("Input Image", 1);
	//cvShowImage ("Input Image", data);
	//cvWaitKey (0);

// Threshold. 0 are object, 255 everything else.
	thresh (data);

	d1 = cvCloneImage(data);		 // Copy
	nr = data->height; nc = data->width;
	boundary (d1); // Initialize the image - boundary is 0, object is 255, others are 1
	dy = makearray (nr, nc);
	dx = makearray (nr, nc);


//	Compute the signed sequential Euclidean distance (8SSED) map.
	ssed8 (d1, data, dy, dx);
/*	out = fopen ("H:\\AIPCV\\CH5\\distance", "w");
	if (out)
	{
		for (i=0; i<nr; i++)
		{
			for (j=0; j<nc; j++)
			{
				fprintf (out, "(%d,%d) ", dx[i][j], dy[i][j]);
			}
			fprintf (out, "\n");
		}
	}
	fclose (out); */

    cvShowImage ("Input Image", d1);
	cvWaitKey (0);

	d2 = cvCloneImage (d1);
	ss.val[1] = 128;
	ss.val[0] = 128;

//	Set P to be the first pixel. This will be (Py, Px)
// do for all object pixels.
	Px = Py = -1;
	for (i=0; i<nr; i++)
	{
		for (j=0; j<nc; j++)
		{
			if ( !objectPixel(data, i, j)) continue;
			if (Boundary(d1, i, j) ) continue;
			Px = j; Py = i; 

// Set Q=DM(P) to be the nearest contour point of the  pixel P according to the 
// distance map where DM(.) represents the 8SSED map.
			Qx = Px + dx[Py][Px]; 
			Qy = Py + dy[Py][Px];
			nearestContour (Py, Px, dx, dy, &Qy, &Qx);

			if ( (Qx >= nc) || (Qy >= nr) ) continue;

			if (!objectPixel (data, Qy, Qx)) 
			{	
				//str_tmp.Format
				str_tmp.Format ("Warning: From (%d,%d) + (%d, %d) the result Q=(%d,%d) is not object.\n", 
				Py, Px, dx[Py][Px], dy[Py][Px], Qy, Qx);
				//MessageBox(str_tmp);
			}
			if (!Boundary(d1, Qy, Qx))
			{	
				str_tmp.Format  ("Warning: From (%d,%d) + (%d, %d) the result Q=(%d,%d) is not boundary.\n", 
					 Py, Px, dx[Py][Px], dy[Py][Px], Qy, Qx);
				//MessageBox(str_tmp);
			}
// Find the nearest contour points of its 8 neighbors Pi
//           Qi = DM(Pi) + (xi; yi)
// where (xi; yi) is the relative position of the neighborhood i with respect to 
			nearestContour (Py-1, Px,   dx, dy, &(Qiy[0]), &(Qix[0]));
			nearestContour (Py-1, Px-1, dx, dy, &(Qiy[1]), &(Qix[1]));
			nearestContour (Py,   Px-1, dx, dy, &(Qiy[2]), &(Qix[2]));
			nearestContour (Py+1, Px-1, dx, dy, &(Qiy[3]), &(Qix[3]));
			nearestContour (Py+1, Px,   dx, dy, &(Qiy[4]), &(Qix[4]));
			nearestContour (Py+1, Px+1, dx, dy, &(Qiy[5]), &(Qix[5]));
			nearestContour (Py,   Px+1, dx, dy, &(Qiy[6]), &(Qix[6]));
			nearestContour (Py-1, Px+1, dx, dy, &(Qiy[7]), &(Qix[7]));

// the pixel P and i = 1,2,. . . ,8 is the index of the neighborhoods.
// The eight point pairs are formed by the nearest contour points of both the pixel P 
// and its 8-neighbors, which are denoted as (Q;Qi)
//  where i = 1,2,. . . ,8.
			for (ii=0; ii<8; ii++)
			{

// Apply the connectivity criterion:
//	D*D =|Qi?Q|>=r (r=2)  
				if (norm(Qiy[ii]-Qy, Qix[ii]-Qx) < RADIUS) continue;		// Nope
			
//			and |Qi|*|Qi| - |Q|*|Q| <= max(X(Qi-Q),Y(Qi-Q))
				jj = norm(Qiy[ii], Qix[ii])*norm(Qiy[ii], Qix[ii]) - norm(Qx, Qy)*norm(Qx,Qy);
				k = Qix[ii]-Qx;  if (k<0) k = -k;
				g = Qiy[ii]-Qy;  if (g<0) g = -g;
				if (k<g) k = g;
				if (jj > k) continue;

//where X(p) and X(p) represent the x and y coordinates of the point.

				flag = 1;
				
				str_tmp.Format("Skeletal at (%d, %d)\n", Py, Px);
				a1 = angle2pt ((float)Py, (float)Px, (float)Qy, (float)Qx);
				a2 = angle2pt ((float)Py, (float)Px, (float)Qiy[ii], (float)Qix[ii]);
				//MessageBox(str_tmp);
				str_tmp.Format ("Skeletal point at (%d,%d): P-Q is %f  Qi-Q is %f\n", 
					Px, Py, angle2pt ((float)Py, (float)Px, (float)Qy, (float)Qx),
					angle2pt ((float)Py, (float)Px, (float)Qiy[ii], (float)Qix[ii]));
				//MessageBox(str_tmp);
				a1 = a1 - a2;
				if (a1 < 0.0f) a1 = -a1;
				if (a1 < 90.0)
				{
					str_tmp.Format ("This point fails.  (%f)\n", a1);
					//AfxMessageBox(str_tmp);
					continue;
				}
				cvSet2D (d1, Py, Px, ss);
				break;
			}

// If one of the point pairs, (Q,Qi), satisfies the connectivity
//  criterion, then the pixel P is a skeleton point.
		}

	} while (more);
    cvShowImage ("Input Image", d1);
	cvWaitKey (0);
}


void CcvIndoorDlg::OnBnClickedButtonRsfd()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	Mat srcimg = m_img;
	if (srcimg.type()!=CV_8UC1)
	{
		CvSize uniSize;//, colorSize;
		IplImage *pImage = cvCloneImage(m_img);
		IplImage *pImg8u;
		uniSize = cvGetSize(pImage);
		pImg8u = cvCreateImage(uniSize,IPL_DEPTH_8U,1);
		cvCvtColor(pImage,pImg8u,CV_BGR2GRAY);
		srcimg = pImg8u;
	}
	//imshow("srcimg",srcimg);
	//waitKey(0);
	Mat dstimg;
	medianBlur(srcimg,srcimg,5); 
	imshow("�˲�",srcimg);
	cvRosenfeld(srcimg,dstimg);

}


void CcvIndoorDlg::OnBnClickedHilditch()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Mat srcimg = Bord;
	if (srcimg.type()!=CV_8UC1)
	{
		CvSize uniSize;//, colorSize;
		IplImage *pImage = cvCloneImage(m_img);
		IplImage *pImg8u;
		uniSize = cvGetSize(pImage);
		pImg8u = cvCreateImage(uniSize,IPL_DEPTH_8U,1);
		cvCvtColor(pImage,pImg8u,CV_BGR2GRAY);
		srcimg = pImg8u;
	}
	Mat dstimg;
	/*medianBlur(srcimg,srcimg,5); 
	Mat element = getStructuringElement(MORPH_RECT, Size(5,5));
	//������̬ѧ����
	morphologyEx(srcimg,srcimg, MORPH_OPEN, element);
	imshow("�˲�",srcimg);*/
	cvHilditchThin(srcimg,dstimg);
	imshow("result",dstimg);
}



void CcvIndoorDlg::OnNMCustomdrawSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}


void CcvIndoorDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CSliderCtrl   *pSlidCtrl=(CSliderCtrl*)GetDlgItem(IDC_SLIDER);
    m_int=pSlidCtrl->GetPos();//ȡ�õ�ǰλ��ֵ  
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
	SetDlgItemInt(IDC_EDIT1,m_int);
}


void CcvIndoorDlg::OnBnClickedCanny()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	IplImage *scr,*CannyImg;	
	scr=CannyImg=cvCreateImage(cvGetSize(m_img), IPL_DEPTH_8U, 1);
	cvCvtColor(m_img,scr,CV_BGR2GRAY);
	cvCanny(scr,CannyImg,m_int,m_int * 3, 3);
	Bord=CannyImg;
	cvShowImage("Border",Bord);
}


void CcvIndoorDlg::OnBnClickedPainttrack()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	PaintTrack T(Bord,m_int,m_threshold);
	T.startPaint();
}
