
// CoordinateGetterDlg.cpp : 实现文件;
//
#include "motocom.h"
#include "stdafx.h"
#include "CoordinateGetter.h"
#include "CoordinateGetterDlg.h"
#include "afxdialogex.h"
#include "FGCamera.h"
#include "math.h"//MYADD,支持sqrt;
#include "cv.h"
#include "highgui.h"
#include <cxcore.h>
#include "PaintTrack.h"
#include <sstream>
#include <fstream>

const double disterr = 10.0f;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//对于外部变量的声明;
#if 1

 int nCid;	//进程的标识符号;
 char *path;	//路径;
char *framename;//标识坐标系的类型;
 WORD rconf;	   //区分多解问题的标识;
 double x2,y2,z2;//零件在拼装平台上的中心点坐标;
 double p[12];//机器人相于腰关节坐标系的位姿(前6位有效);
 double p2[12] = {0};
 CCoordinateGetterDlg* pDlg;
 /*FGNODEINFO *NodeInfo;	
 UINT32 NodeCnt;
 CFGCamera Camera;
 FGFRAME Frame;*/
double ZA[2]={0},ZC=0;
int m_zz_low=0;
int m_zz_high=1;
void setZ(double x1,double y1,double z1,double x2,double y2,double z2,double x3,double y3,double z3){
	ZA[0]=((z2-z1)*(y3-y1)-(z3-z1)*(y2-y1))/((x2-x1)*(y3-y1)-(x3-x1)*(y2-y1));
	ZA[1]=((z2-z1)*(x3-x1)-(z3-z1)*(x2-x1))/((y2-y1)*(x3-x1)-(y3-y1)*(x2-x1));
	ZC=z1-ZA[0]*x1-ZA[1]*y1;
}
double Z(double x,double y){return ZA[0]*x+ZA[1]*y+ZC;}
#endif
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框;



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


// CCoordinateGetterDlg 对话框




CCoordinateGetterDlg::CCoordinateGetterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCoordinateGetterDlg::IDD, pParent)
	, m_XYZ_I(_T(""))
	, m_showAngles(_T(""))
	, m_showXYZ(_T(""))
	, m_XX_in(0)
	, m_YY_in(0)
	, m_canny_threshold(0)
	, m_draw_step(0)
	, m_lt_x(0)
	, m_lt_y(0)
	, m_rt_x(0)
	, m_rt_y(0)
	, m_rd_x(0)
	, m_rd_y(0)
	, m_threshold(1)
	, m_z_ref(0)
	, m_check(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCoordinateGetterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SHOWXYZ, m_showXYZ);
	DDX_Text(pDX, IDC_EDIT_CANNY_TSHD, m_canny_threshold);
	DDX_Text(pDX, IDC_EDIT_DRAW_STEP, m_draw_step);
	DDX_Text(pDX, IDC_THRESHOLD, m_threshold);
	DDV_MinMaxInt(pDX, m_threshold, 1, 1000000);
	DDX_Text(pDX, IDC_EDIT_Z_REF, m_z_ref);
	DDX_Check(pDX, IDC_CHECK1, m_check);
	DDX_Control(pDX, IDC_SPIN1, m_spin);
}

BEGIN_MESSAGE_MAP(CCoordinateGetterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CCoordinateGetterDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CCoordinateGetterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_READ, &CCoordinateGetterDlg::OnBnClickedButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_EYES, &CCoordinateGetterDlg::OnBnClickedButtonOpenEyes)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CCoordinateGetterDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_CANNY, &CCoordinateGetterDlg::OnBnClickedButtonCanny)
	ON_BN_CLICKED(IDC_BUTTON_TRACK, &CCoordinateGetterDlg::OnBnClickedButtonTrack)
	ON_BN_CLICKED(IDC_BUTTON_OFF, &CCoordinateGetterDlg::OnBnClickedButtonOff)
	//ON_EN_CHANGE(IDC_THRESHOLD, &CCoordinateGetterDlg::OnEnChangeThreshold)
	ON_EN_CHANGE(IDC_EDIT_Z_REF, &CCoordinateGetterDlg::OnEnChangeEditZRef)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CCoordinateGetterDlg::OnDeltaposSpin1)
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CCoordinateGetterDlg 消息处理程序

BOOL CCoordinateGetterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动;
	//  执行此操作;
	SetIcon(m_hIcon, TRUE);			// 设置大图标;
	SetIcon(m_hIcon, FALSE);		// 设置小图标;

	// TODO: 在此添加额外的初始化代码;
	//ROBOT_INIT();

	pDlg = this;	//主对话框指针;
	CRect rc;
	CWnd *pWnd=GetDlgItem(IDC_STATIC3);
	pWnd->GetClientRect(&rc);
	cvNamedWindow("INPUT",1);
	cvResizeWindow("INPUT",rc.Width(),rc.Height());
	HWND hWnd=(HWND) cvGetWindowHandle("INPUT");
	HWND hParent=::GetParent(hWnd);
	::SetParent(hWnd,pWnd->m_hWnd);
	::ShowWindow(hParent,SW_HIDE);
	m_spin.SetBuddy(GetDlgItem(IDC_EDIT_Z_REF));
	m_spin.SetRange(-1000,1000);
	m_spin.SetBase(1);
	m_spin.SetPos(0);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCoordinateGetterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCoordinateGetterDlg::OnPaint()
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
HCURSOR CCoordinateGetterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CCoordinateGetterDlg::ROBOT_INIT()
{
	BOOL suc = FALSE; //MYADD，表示程序初始化是否成功;
	int result; //接收函数的返回值
	CString temp;
	nCid=BscOpen(path,16); //打开连接
	if(nCid==0) //打开成功
	{
		HWND hWnd;
		hWnd=this->m_hWnd; //取得窗口句柄
		result=BscSetEther(nCid,"192.168.255.1",0,hWnd); //设置以太网
		if(result==1) //设置以太网成功
		{
			BscConnect(nCid); //连接以太网
			if(result!=1)
			{
				temp="连接失败！";
				suc = FALSE;// MYADD
			}
			else
			{
				temp="连接成功！";
				suc = TRUE;// MYADD
			}
		}
		else
		{
			temp="以太网连接错误";
			suc = FALSE;// MYADD
		}
	}
	else
	{
		temp="以太网设置错误";
		suc = FALSE;// MYADD
	}
	if(BscServoOn(nCid)==0) //打开伺服电机
	{
		temp="伺服打开成功";
		suc = TRUE;// MYADD
	}
	else
	{
		temp="伺服打开失败";
		suc = FALSE;// MYADD
	}
	MessageBox(temp); //MYADD
	//*p2 = {0,0,0,0,0,0,0,0,0,0,0,0};
	x2 = 250;
	y2 = 180;
	z2 = 100;
	return suc;
}

BOOL CCoordinateGetterDlg::ROBOT_END()
{
	BOOL fin = FALSE; //MYADD,标志是否成功关闭;
	CString temp;
	int result=BscServoOff(nCid); //断开伺服电机
	if(result==0)
	{
		temp="伺服关闭成功";
		fin = TRUE; // MYADD
	}
	else
	{
		temp="伺服关闭失败";
		fin = FALSE; // MYADD
	}
	BscDisConnect(nCid); //断开连接
	if(BscClose(nCid)==0)
	{
		temp="连接关闭成功";
		fin = TRUE; // MYADD
	}
	else
	{
		temp="连接关闭失败";
		fin = FALSE;	//MYADD
	}
	MessageBox(temp); //MYADD
	return fin;
}

void CCoordinateGetterDlg::ROBOT_MOVE(double xx,double yy,int zz)
{
	//UpdateData(TRUE);	
	framename="BASE"; //选择腰关节坐标系(即直角坐标系);
	rconf=0;
	short toolno=0; //工具编号，一般情况下设为0;
	short speed= 12; //机器人移动速度占关节最大速度的百分比;
	double dist; //位置误差;
	static double p1[12]; //实时读取末端位置和角度;
	p[0]=xx; //目标位置关节角;//x2
	p[1]=yy;					//y2
	p[2] = Z(xx,yy)+m_z_ref;
	if(zz) p[2]+=50.0f;
	for(int jj=3; jj<12; ++jj)
	{
		p[jj] = p2[jj];
	}
	/*
	p[2]=0;					//z2
	p[3]=0;	//CRD
	p[4]=0;
	p[5]=0;
	p[6]=0;
	p[7]=0;
	p[8]=0;
	p[9]=0;
	p[10]=0;
	p[11]=0;
	*/
	BscMovj(nCid,speed,framename,rconf,toolno,p);
	while(1) //确保移动命令执行完毕;
	{
		BscIsLoc(nCid,0,&rconf,p1); //读取直角坐标;
		dist=sqrt((p[0]-p1[0])*(p[0]-p1[0])+(p[1]-p1[1])*(p[1]-p1[1])+(p[2]-p1[2])*(p[2]-p1[2]));
		if(dist<=disterr)
			break;
	}
	//MessageBox("");
	//m_showAngles.Format("%d,%d,%d,%d,%d,%d",p1[6],p1[7],p1[8],p1[9],p1[10],p1[11]);
	//UpdateData(FALSE);
}

void CCoordinateGetterDlg::ROBOT_VALVE()
{
	BscSelectJob(nCid,"OPEN.jbi"); //在NX100示教编程器中编好的打开气阀程序
	Sleep(500);
	BscStartJob(nCid); //打开气阀;
	Sleep(500);
	BscSelectJob(nCid,"CLOSE.jbi"); //在NX100示教编程器中编好的关闭气阀程序
	BscStartJob(nCid);
}



void CCoordinateGetterDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//ROBOT_END();
	CDialogEx::OnCancel();
}


void CCoordinateGetterDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码;
	ROBOT_INIT();
	//CDialogEx::OnOK();
}



void CCoordinateGetterDlg::OnBnClickedButtonRead()
{
	// TODO: 在此添加控件通知处理程序代码;
	
	//static int p33[12]={0}
	BscIsLoc(nCid,1,&rconf,p2); //读取关节角;
	m_showAngles.Format("%f,%f,%f,%f,%f,%f",p2[0],p2[1],p2[2],p2[3],p2[4],p2[5]);
	
	BscIsLoc(nCid,0,&rconf,p2); //读取直角坐标;
	//m_zz_low = p2[2];			//设定的“低Z坐标”
	//m_zz_high = m_zz_low + 50.0f;//设定的“高Z坐标”
	m_showXYZ.Format("%f,%f,%f,%f,%f,%f",p2[0],p2[1],p2[2],p2[3],p2[4],p2[5]);
	UpdateData(FALSE);
}


void CCoordinateGetterDlg::OnBnClickedButtonOpenEyes()
{
	CFGCamera Camera;
//	UINT32 Result;				// 存储摄像头拍摄图片的大小、颜色、帧速
	FGNODEINFO NodeInfo[3];
	UINT32 NodeCnt;
	FGFRAME Frame;
	int option = 0;
	//IplImage *image=cvCreateImage(cvSize(1200,800),IPL_DEPTH_8U,3); // OPENCV创新存储图片的结构体变量
	// TODO: 在此添加控件通知处理程序代码
	UINT32 format; // 存储摄像头拍摄图片的大小、颜色、帧速
	IplImage *image = cvCreateImage(cvSize(1280,960),IPL_DEPTH_8U,3); //OPENCV创新存储图片的结构体变量
	UINT Result = FGInitModule(NULL); //初始化摄像头
	if(Result == FCE_NOERROR)
		Result = FGGetNodeList(NodeInfo,3,&NodeCnt); // Get list of connected nodes
	if(Result == FCE_NOERROR && NodeCnt)
		Result = Camera.Connect(&NodeInfo[option].Guid); // Connect with node
	if(Result==FCE_NOERROR) //以下为对摄像头的设置
	{
		format=MAKEDCAMFORMAT(RES_1280_960,CM_RGB8,FR_1_875);//设置摄像头拍摄图片的大小、
		//颜色、帧速
		Camera.SetParameter(FGP_IMAGEFORMAT,format);
		Camera.SetParameter(FGP_PHYSPEED,400);
		Camera.SetParameter(FGP_FRAMEBUFFERCOUNT,16);
		Camera.SetParameter(FGP_BRIGHTNESS,PVAL_AUTO);
		Camera.SetParameter(FGP_SHARPNESS,PVAL_AUTO);
		Camera.SetParameter(FGP_GAIN,PVAL_AUTO);
		Camera.SetParameter(FGP_GAMMA,PVAL_AUTO);
		Camera.SetParameter(FGP_SHUTTER,PVAL_AUTO);
		Camera.SetParameter(FGP_WHITEBALCB,PVAL_AUTO);
		Camera.SetParameter(FGP_WHITEBALCR,PVAL_AUTO);
	}
	if(Result == FCE_NOERROR)
		Result = Camera.OpenCapture();// Start DMA logic
	if(Result == FCE_NOERROR)
		Result = Camera.StartDevice(); // Start image device
	
	CRect rc;
	CWnd *pWnd=GetDlgItem(IDC_STATIC3);
	pWnd->GetClientRect(&rc);

	UINT8 *pBuf;
	UINT8 *pBGR;
	UINT8 *pR,*pB,*pG0,*pG1;
	UINT32 i,j;
		struct pdst
		{
			UINT8 B;
			UINT8 G;
			UINT8 R;
		}*pDst;

	CvSize ori_size;//调整前的尺寸
		ori_size.height = 960;
		ori_size.width = 1280;
	CvSize dst_size;//调整后的尺寸
		dst_size.height = rc.Height();
		dst_size.width = rc.Width();

	IplImage *show1 = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);
	IplImage* contour = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//从原始图象提取圆
	IplImage* GRAY = cvCreateImage(ori_size, IPL_DEPTH_8U, 1);//灰度	
	IplImage *final = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//原图大小最终图
	IplImage *show_final = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);//显示大小最终图

	CvSeq * circles = 0;
	CvMemStorage * storage = cvCreateMemStorage(0);

	CString instruction;
//	float* p;
	char c;

	while(1)//////////////////while(1)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	{
	
	if(Result==FCE_NOERROR)
	{
		Camera.GetFrame(&Frame);
		Result=Camera.PutFrame(&Frame); //图像传输
	
		UINT32 XSize=1280;UINT32 YSize=960;
		pBuf=(unsigned char *)Frame.pData;
		pBGR=(unsigned char *)image->imageData;
		
	pDst=(struct pdst*)pBGR;
	for(i=0;i<YSize-1;i++)
	{
		if(i&1)
		{
			pG1=pBuf+i*XSize;
			pB=pG1+1;
			pR=pG1+XSize;

			pG0=pR+1;
		}
		else
		{
			pR=pBuf+i*XSize;
			pG0=pR+1;
			pG1=pR+XSize;
			pB=pG1+1;
		}

	// Go through all pixels
		for(j=0;j<XSize-1;j++)
		{
			pDst->B=*pB;
			pDst->G=(UINT8)((*pG0+*pG1)/2);
			pDst->R=*pR;
			pDst++;
			if(j&1)
			{
				pB+=2;
				pG0+=2;
			}
			else
			{
				pR+=2;
				pG1+=2;
			}
		}
		pDst->B=0;
		pDst->R=0;
		pDst++;

	}

	
   
	cvCopy(image, final);
	cvSmooth(image, contour, CV_GAUSSIAN, 3, 0);//平滑图像。对图像进行核大小为3×3的高斯卷积
	cvCvtColor(contour, GRAY, CV_BGR2GRAY);//转灰度
	
#if 0
	circles = cvHoughCircles(GRAY, storage, CV_HOUGH_GRADIENT, 2, GRAY->height/15, 200, 100, 10, 50);
	
	instruction.Format("There are %d circles!\n",circles->total);
	GetDlgItem(IDC_STATIC2)->SetWindowText(instruction);
	for(i=0; i < circles->total; i++ )
	{
		p = (float*)cvGetSeqElem(circles, i);//返回索引指定的元素指针
		//cout<<"圆心x="<<cvRound(p[0])<<"y="<<cvRound(p[1]);
		if(cvRound(p[0]) >=260 && cvRound(p[1]) >= 120 &&cvRound(p[1]) <=840 && cvRound(p[2]) >= 12){
		  cvCircle( final, cvPoint(cvRound(p[0]),cvRound(p[1])), 3, CV_RGB(0,255,0), -1, 8, 0 );
		  //目标图像、圆心、半径、颜色、粗细、线条种类、缩放比例数据//CV_RGB(0,255,0)是绿点	 
		  cvCircle( final, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );
		  //CV_RGB(255,0,0)是红线
		}
	}

#endif
	cvResize(final, show1, CV_INTER_LINEAR );//按scale放缩赋值
	
	cvShowImage("INPUT",show1); //显示图片
	cvSaveImage("image6.bmp",image); //显示图片
	c = cvWaitKey(0);

	//cvNamedWindow( "当前图像2", 1); //产生用于显示图片的窗口
	//	
	//cvShowImage("当前图像2",show1); //显示图片
	//MessageBox(_T("ehkghs"));
	//SetTimer(1,10,NULL);

	//cvDestroyWindow("当前图像2"); //销毁窗口
	
	}

	}
#if 0
		CFGCamera Camera;
//	UINT32 Result;				// 存储摄像头拍摄图片的大小、颜色、帧速
	FGNODEINFO NodeInfo[3];
	UINT32 NodeCnt;
	FGFRAME Frame;
	int option = 0;
	//IplImage *image=cvCreateImage(cvSize(1200,800),IPL_DEPTH_8U,3); // OPENCV创新存储图片的结构体变量
	// TODO: 在此添加控件通知处理程序代码
	UINT32 format; // 存储摄像头拍摄图片的大小、颜色、帧速
	IplImage *image = cvCreateImage(cvSize(1280,960),IPL_DEPTH_8U,3); //OPENCV创新存储图片的结构体变量
	UINT Result = FGInitModule(NULL); //初始化摄像头
	if(Result == FCE_NOERROR)
		Result = FGGetNodeList(NodeInfo,3,&NodeCnt); // Get list of connected nodes
	if(Result == FCE_NOERROR && NodeCnt)
		Result = Camera.Connect(&NodeInfo[option].Guid); // Connect with node
	if(Result==FCE_NOERROR) //以下为对摄像头的设置
	{
		format=MAKEDCAMFORMAT(RES_1280_960,CM_RGB8,FR_1_875);//设置摄像头拍摄图片的大小、
		//颜色、帧速
		Camera.SetParameter(FGP_IMAGEFORMAT,format);
		Camera.SetParameter(FGP_PHYSPEED,400);
		Camera.SetParameter(FGP_FRAMEBUFFERCOUNT,16);
		Camera.SetParameter(FGP_BRIGHTNESS,PVAL_AUTO);
		Camera.SetParameter(FGP_SHARPNESS,PVAL_AUTO);
		Camera.SetParameter(FGP_GAIN,PVAL_AUTO);
		Camera.SetParameter(FGP_GAMMA,PVAL_AUTO);
		Camera.SetParameter(FGP_SHUTTER,PVAL_AUTO);
		Camera.SetParameter(FGP_WHITEBALCB,PVAL_AUTO);
		Camera.SetParameter(FGP_WHITEBALCR,PVAL_AUTO);
	}
	if(Result == FCE_NOERROR)
		Result = Camera.OpenCapture();// Start DMA logic
	if(Result == FCE_NOERROR)
		Result = Camera.StartDevice(); // Start image device
	
	CRect rc;
	CWnd *pWnd=GetDlgItem(IDC_EDIT_XYZ_I);
	pWnd->GetClientRect(&rc);

	UINT8 *pBuf;
	UINT8 *pBGR;
	UINT8 *pR,*pB,*pG0,*pG1;
	UINT32 i,j;
		struct pdst
		{
			UINT8 B;
			UINT8 G;
			UINT8 R;
		}*pDst;

	CvSize ori_size;//调整前的尺寸
		ori_size.height = 960;
		ori_size.width = 1280;
	CvSize dst_size;//调整后的尺寸
		dst_size.height = rc.Height();
		dst_size.width = rc.Width();

	IplImage *show1 = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);
	IplImage* contour = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//从原始图象提取圆
	IplImage* GRAY = cvCreateImage(ori_size, IPL_DEPTH_8U, 1);//灰度	
	IplImage *final = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//原图大小最终图
	IplImage *show_final = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);//显示大小最终图

	CvSeq * circles = 0;
	CvMemStorage * storage = cvCreateMemStorage(0);

	CString instruction;
	float* p;
	char c;

		
	if(Result==FCE_NOERROR)
	{
		Camera.GetFrame(&Frame);
		Result=Camera.PutFrame(&Frame); //图像传输
	
		UINT32 XSize=1280;UINT32 YSize=960;
		pBuf=(unsigned char *)Frame.pData;
		pBGR=(unsigned char *)image->imageData;
		
	pDst=(struct pdst*)pBGR;
	for(i=0;i<YSize-1;i++)
	{
		if(i&1)
		{
			pG1=pBuf+i*XSize;
			pB=pG1+1;
			pR=pG1+XSize;

			pG0=pR+1;
		}
		else
		{
			pR=pBuf+i*XSize;
			pG0=pR+1;
			pG1=pR+XSize;
			pB=pG1+1;
		}

	// Go through all pixels
		for(j=0;j<XSize-1;j++)
		{
			pDst->B=*pB;
			pDst->G=(UINT8)((*pG0+*pG1)/2);
			pDst->R=*pR;
			pDst++;
			if(j&1)
			{
				pB+=2;
				pG0+=2;
			}
			else
			{
				pR+=2;
				pG1+=2;
			}
		}
		pDst->B=0;
		pDst->R=0;
		pDst++;

	}

	}
    memcpy(image->imageData, Frame.pData, 1280*960);

	cvNamedWindow( "当前图像", 1 ); //产生用于显示图片的窗口	
	cvShowImage("当前图像",image); //显示图片
	cvSaveImage("picture.bmp",image); //存储图片
    c=cvWaitKey(0); //等待按键，0表示无限等待
	cvDestroyWindow("当前图像"); //销毁窗口
#endif
}


void CCoordinateGetterDlg::OnBnClickedButtonBrowse()
{
	// TODO: 在此添加控件通知处理程序代码;
	LPCTSTR lpszFilter="(*.bmp;*.jpg)|*.bmp;*.jpg|任何文件|*.*||";
	CFileDialog dlg(TRUE,lpszFilter,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,lpszFilter,NULL);

	

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
	}
}


void CCoordinateGetterDlg::OnBnClickedButtonCanny()
{
	// TODO: 在此添加控件通知处理程序代码;
	UpdateData(TRUE);
	IplImage *scr,*CannyImg;
	CannyImg=cvCreateImage(cvGetSize(m_img), IPL_DEPTH_8U, 1);
	scr=cvCreateImage(cvGetSize(m_img), IPL_DEPTH_8U, 1);
	cvCvtColor(m_img,scr,CV_BGR2GRAY);
	cvCanny(scr,CannyImg,m_canny_threshold,m_canny_threshold * 3, 3);
	Bord=CannyImg;
	cvNamedWindow("Border",1);
	cvShowImage("Border",Bord);
	cvWaitKey(0);
	//cvDestroyWindow("Border");
	//cvReleaseImage(&m_img);
}


void CCoordinateGetterDlg::OnBnClickedButtonTrack()
{
	// TODO: 在此添加控件通知处理程序代码;
	//OnBnClickedButtonRead();
	UpdateData(TRUE);

	ifstream file1("F:\coor.txt");
	if(!file1)
	{
		MessageBox("coor.txt not found!");
		return;
	}
	ostringstream tmp;
	tmp<<file1.rdbuf();
	CString longer = tmp.str().c_str();
	longer.Replace(',',' ');
	std::string streamy = longer.GetBuffer(0);
	longer.ReleaseBuffer();	
	istringstream stream(streamy);
	double coors[12];
	// coors[0]~coors[8]:  3 X-Y-Z coordinates
	// coors[9]~coors[11]:  position angles
	for(int i=0;i<12;i++)
	{
		stream >> coors[i];
	}
	setZ(coors[0],coors[1],coors[2],coors[3],coors[4],coors[5],coors[6],coors[7],coors[8]);
	Pos lt,rt,rb;
	if(!m_check)
	{
		lt.x=coors[0];
		lt.y=coors[1];
		rt.x=coors[3];
		rt.y=coors[4];
		rb.x=coors[6];
		rb.y=coors[7];
	}
	else
	{
		lt.x=coors[0]+coors[6]-coors[3];
		lt.y=coors[1]+coors[7]-coors[4];
		rt.x=coors[0];
		rt.y=coors[1];
		rb.x=coors[3];
		rb.y=coors[4];
	}

	p2[3]=coors[9];p2[4]=coors[10];p2[5]=coors[11];

	ROBOT_MOVE(lt.x,lt.y,m_zz_high);

	PaintTrack T(Bord,m_draw_step,m_threshold);
	T.coorT(lt,rt,rb);
	T.startPaint();
}


void CCoordinateGetterDlg::OnBnClickedButtonOff()
{
	// TODO: 在此添加控件通知处理程序代码
	ROBOT_END();
}



void CCoordinateGetterDlg::OnEnChangeEditZRef()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
///	m_z_ref+=0.2*double(m_spin.GetPos());
//	m_zz_low=m_z_ref;
//	m_zz_high=m_zz_low+50.0f;
}


void CCoordinateGetterDlg::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString z1;
	if(pNMUpDown-> iDelta == 1) // 如果此值为1 , 说明点击了Spin的往下箭头 
	{  
		m_z_ref -= 0.2;			
		z1.Format("%f",m_z_ref);
		SetDlgItemText(IDC_EDIT_Z_REF,z1);	          
	} 
	else if(pNMUpDown-> iDelta == -1) // 如果此值为-1 , 说明点击了Spin的往上箭头 
	{ 
		m_z_ref += 0.2;			
		z1.Format("%f",m_z_ref);
		SetDlgItemText(IDC_EDIT_Z_REF,z1);	        
	}
	*pResult = 0;

}


void CCoordinateGetterDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
#if 0
	if(nSBCode==SB_ENDSCROLL) return;
	if(pScrollBar->GetDlgCtrlID()==IDC_SPIN1)
	{
		CSpinButtonCtrl* pSpin=(CSpinButtonCtrl)pScrollBar;
		CString value;
		value.Format((double)pSpin->GetPos()/5);
		pSpin->GetBuddy()->SetWindowTextA(value);
	}
#endif
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}



