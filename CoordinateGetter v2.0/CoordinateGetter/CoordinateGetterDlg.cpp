
// CoordinateGetterDlg.cpp : ʵ���ļ�;
//
#include "motocom.h"
#include "stdafx.h"
#include "CoordinateGetter.h"
#include "CoordinateGetterDlg.h"
#include "afxdialogex.h"
#include "FGCamera.h"
#include "math.h"//MYADD,֧��sqrt;
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
//�����ⲿ����������;
#if 1

 int nCid;	//���̵ı�ʶ����;
 char *path;	//·��;
char *framename;//��ʶ����ϵ������;
 WORD rconf;	   //���ֶ������ı�ʶ;
 double x2,y2,z2;//�����ƴװƽ̨�ϵ����ĵ�����;
 double p[12];//�������������ؽ�����ϵ��λ��(ǰ6λ��Ч);
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
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���;



class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CCoordinateGetterDlg �Ի���




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


// CCoordinateGetterDlg ��Ϣ�������

BOOL CCoordinateGetterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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
	//ROBOT_INIT();

	pDlg = this;	//���Ի���ָ��;
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
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCoordinateGetterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
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
HCURSOR CCoordinateGetterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CCoordinateGetterDlg::ROBOT_INIT()
{
	BOOL suc = FALSE; //MYADD����ʾ�����ʼ���Ƿ�ɹ�;
	int result; //���պ����ķ���ֵ
	CString temp;
	nCid=BscOpen(path,16); //������
	if(nCid==0) //�򿪳ɹ�
	{
		HWND hWnd;
		hWnd=this->m_hWnd; //ȡ�ô��ھ��
		result=BscSetEther(nCid,"192.168.255.1",0,hWnd); //������̫��
		if(result==1) //������̫���ɹ�
		{
			BscConnect(nCid); //������̫��
			if(result!=1)
			{
				temp="����ʧ�ܣ�";
				suc = FALSE;// MYADD
			}
			else
			{
				temp="���ӳɹ���";
				suc = TRUE;// MYADD
			}
		}
		else
		{
			temp="��̫�����Ӵ���";
			suc = FALSE;// MYADD
		}
	}
	else
	{
		temp="��̫�����ô���";
		suc = FALSE;// MYADD
	}
	if(BscServoOn(nCid)==0) //���ŷ����
	{
		temp="�ŷ��򿪳ɹ�";
		suc = TRUE;// MYADD
	}
	else
	{
		temp="�ŷ���ʧ��";
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
	BOOL fin = FALSE; //MYADD,��־�Ƿ�ɹ��ر�;
	CString temp;
	int result=BscServoOff(nCid); //�Ͽ��ŷ����
	if(result==0)
	{
		temp="�ŷ��رճɹ�";
		fin = TRUE; // MYADD
	}
	else
	{
		temp="�ŷ��ر�ʧ��";
		fin = FALSE; // MYADD
	}
	BscDisConnect(nCid); //�Ͽ�����
	if(BscClose(nCid)==0)
	{
		temp="���ӹرճɹ�";
		fin = TRUE; // MYADD
	}
	else
	{
		temp="���ӹر�ʧ��";
		fin = FALSE;	//MYADD
	}
	MessageBox(temp); //MYADD
	return fin;
}

void CCoordinateGetterDlg::ROBOT_MOVE(double xx,double yy,int zz)
{
	//UpdateData(TRUE);	
	framename="BASE"; //ѡ�����ؽ�����ϵ(��ֱ������ϵ);
	rconf=0;
	short toolno=0; //���߱�ţ�һ���������Ϊ0;
	short speed= 12; //�������ƶ��ٶ�ռ�ؽ�����ٶȵİٷֱ�;
	double dist; //λ�����;
	static double p1[12]; //ʵʱ��ȡĩ��λ�úͽǶ�;
	p[0]=xx; //Ŀ��λ�ùؽڽ�;//x2
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
	while(1) //ȷ���ƶ�����ִ�����;
	{
		BscIsLoc(nCid,0,&rconf,p1); //��ȡֱ������;
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
	BscSelectJob(nCid,"OPEN.jbi"); //��NX100ʾ�̱�����б�õĴ���������
	Sleep(500);
	BscStartJob(nCid); //������;
	Sleep(500);
	BscSelectJob(nCid,"CLOSE.jbi"); //��NX100ʾ�̱�����б�õĹر���������
	BscStartJob(nCid);
}



void CCoordinateGetterDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//ROBOT_END();
	CDialogEx::OnCancel();
}


void CCoordinateGetterDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	ROBOT_INIT();
	//CDialogEx::OnOK();
}



void CCoordinateGetterDlg::OnBnClickedButtonRead()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	
	//static int p33[12]={0}
	BscIsLoc(nCid,1,&rconf,p2); //��ȡ�ؽڽ�;
	m_showAngles.Format("%f,%f,%f,%f,%f,%f",p2[0],p2[1],p2[2],p2[3],p2[4],p2[5]);
	
	BscIsLoc(nCid,0,&rconf,p2); //��ȡֱ������;
	//m_zz_low = p2[2];			//�趨�ġ���Z���ꡱ
	//m_zz_high = m_zz_low + 50.0f;//�趨�ġ���Z���ꡱ
	m_showXYZ.Format("%f,%f,%f,%f,%f,%f",p2[0],p2[1],p2[2],p2[3],p2[4],p2[5]);
	UpdateData(FALSE);
}


void CCoordinateGetterDlg::OnBnClickedButtonOpenEyes()
{
	CFGCamera Camera;
//	UINT32 Result;				// �洢����ͷ����ͼƬ�Ĵ�С����ɫ��֡��
	FGNODEINFO NodeInfo[3];
	UINT32 NodeCnt;
	FGFRAME Frame;
	int option = 0;
	//IplImage *image=cvCreateImage(cvSize(1200,800),IPL_DEPTH_8U,3); // OPENCV���´洢ͼƬ�Ľṹ�����
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UINT32 format; // �洢����ͷ����ͼƬ�Ĵ�С����ɫ��֡��
	IplImage *image = cvCreateImage(cvSize(1280,960),IPL_DEPTH_8U,3); //OPENCV���´洢ͼƬ�Ľṹ�����
	UINT Result = FGInitModule(NULL); //��ʼ������ͷ
	if(Result == FCE_NOERROR)
		Result = FGGetNodeList(NodeInfo,3,&NodeCnt); // Get list of connected nodes
	if(Result == FCE_NOERROR && NodeCnt)
		Result = Camera.Connect(&NodeInfo[option].Guid); // Connect with node
	if(Result==FCE_NOERROR) //����Ϊ������ͷ������
	{
		format=MAKEDCAMFORMAT(RES_1280_960,CM_RGB8,FR_1_875);//��������ͷ����ͼƬ�Ĵ�С��
		//��ɫ��֡��
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

	CvSize ori_size;//����ǰ�ĳߴ�
		ori_size.height = 960;
		ori_size.width = 1280;
	CvSize dst_size;//������ĳߴ�
		dst_size.height = rc.Height();
		dst_size.width = rc.Width();

	IplImage *show1 = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);
	IplImage* contour = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//��ԭʼͼ����ȡԲ
	IplImage* GRAY = cvCreateImage(ori_size, IPL_DEPTH_8U, 1);//�Ҷ�	
	IplImage *final = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//ԭͼ��С����ͼ
	IplImage *show_final = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);//��ʾ��С����ͼ

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
		Result=Camera.PutFrame(&Frame); //ͼ����
	
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
	cvSmooth(image, contour, CV_GAUSSIAN, 3, 0);//ƽ��ͼ�񡣶�ͼ����к˴�СΪ3��3�ĸ�˹���
	cvCvtColor(contour, GRAY, CV_BGR2GRAY);//ת�Ҷ�
	
#if 0
	circles = cvHoughCircles(GRAY, storage, CV_HOUGH_GRADIENT, 2, GRAY->height/15, 200, 100, 10, 50);
	
	instruction.Format("There are %d circles!\n",circles->total);
	GetDlgItem(IDC_STATIC2)->SetWindowText(instruction);
	for(i=0; i < circles->total; i++ )
	{
		p = (float*)cvGetSeqElem(circles, i);//��������ָ����Ԫ��ָ��
		//cout<<"Բ��x="<<cvRound(p[0])<<"y="<<cvRound(p[1]);
		if(cvRound(p[0]) >=260 && cvRound(p[1]) >= 120 &&cvRound(p[1]) <=840 && cvRound(p[2]) >= 12){
		  cvCircle( final, cvPoint(cvRound(p[0]),cvRound(p[1])), 3, CV_RGB(0,255,0), -1, 8, 0 );
		  //Ŀ��ͼ��Բ�ġ��뾶����ɫ����ϸ���������ࡢ���ű�������//CV_RGB(0,255,0)���̵�	 
		  cvCircle( final, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );
		  //CV_RGB(255,0,0)�Ǻ���
		}
	}

#endif
	cvResize(final, show1, CV_INTER_LINEAR );//��scale������ֵ
	
	cvShowImage("INPUT",show1); //��ʾͼƬ
	cvSaveImage("image6.bmp",image); //��ʾͼƬ
	c = cvWaitKey(0);

	//cvNamedWindow( "��ǰͼ��2", 1); //����������ʾͼƬ�Ĵ���
	//	
	//cvShowImage("��ǰͼ��2",show1); //��ʾͼƬ
	//MessageBox(_T("ehkghs"));
	//SetTimer(1,10,NULL);

	//cvDestroyWindow("��ǰͼ��2"); //���ٴ���
	
	}

	}
#if 0
		CFGCamera Camera;
//	UINT32 Result;				// �洢����ͷ����ͼƬ�Ĵ�С����ɫ��֡��
	FGNODEINFO NodeInfo[3];
	UINT32 NodeCnt;
	FGFRAME Frame;
	int option = 0;
	//IplImage *image=cvCreateImage(cvSize(1200,800),IPL_DEPTH_8U,3); // OPENCV���´洢ͼƬ�Ľṹ�����
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UINT32 format; // �洢����ͷ����ͼƬ�Ĵ�С����ɫ��֡��
	IplImage *image = cvCreateImage(cvSize(1280,960),IPL_DEPTH_8U,3); //OPENCV���´洢ͼƬ�Ľṹ�����
	UINT Result = FGInitModule(NULL); //��ʼ������ͷ
	if(Result == FCE_NOERROR)
		Result = FGGetNodeList(NodeInfo,3,&NodeCnt); // Get list of connected nodes
	if(Result == FCE_NOERROR && NodeCnt)
		Result = Camera.Connect(&NodeInfo[option].Guid); // Connect with node
	if(Result==FCE_NOERROR) //����Ϊ������ͷ������
	{
		format=MAKEDCAMFORMAT(RES_1280_960,CM_RGB8,FR_1_875);//��������ͷ����ͼƬ�Ĵ�С��
		//��ɫ��֡��
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

	CvSize ori_size;//����ǰ�ĳߴ�
		ori_size.height = 960;
		ori_size.width = 1280;
	CvSize dst_size;//������ĳߴ�
		dst_size.height = rc.Height();
		dst_size.width = rc.Width();

	IplImage *show1 = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);
	IplImage* contour = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//��ԭʼͼ����ȡԲ
	IplImage* GRAY = cvCreateImage(ori_size, IPL_DEPTH_8U, 1);//�Ҷ�	
	IplImage *final = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//ԭͼ��С����ͼ
	IplImage *show_final = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);//��ʾ��С����ͼ

	CvSeq * circles = 0;
	CvMemStorage * storage = cvCreateMemStorage(0);

	CString instruction;
	float* p;
	char c;

		
	if(Result==FCE_NOERROR)
	{
		Camera.GetFrame(&Frame);
		Result=Camera.PutFrame(&Frame); //ͼ����
	
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

	cvNamedWindow( "��ǰͼ��", 1 ); //����������ʾͼƬ�Ĵ���	
	cvShowImage("��ǰͼ��",image); //��ʾͼƬ
	cvSaveImage("picture.bmp",image); //�洢ͼƬ
    c=cvWaitKey(0); //�ȴ�������0��ʾ���޵ȴ�
	cvDestroyWindow("��ǰͼ��"); //���ٴ���
#endif
}


void CCoordinateGetterDlg::OnBnClickedButtonBrowse()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	LPCTSTR lpszFilter="(*.bmp;*.jpg)|*.bmp;*.jpg|�κ��ļ�|*.*||";
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ROBOT_END();
}



void CCoordinateGetterDlg::OnEnChangeEditZRef()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
///	m_z_ref+=0.2*double(m_spin.GetPos());
//	m_zz_low=m_z_ref;
//	m_zz_high=m_zz_low+50.0f;
}


void CCoordinateGetterDlg::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString z1;
	if(pNMUpDown-> iDelta == 1) // �����ֵΪ1 , ˵�������Spin�����¼�ͷ 
	{  
		m_z_ref -= 0.2;			
		z1.Format("%f",m_z_ref);
		SetDlgItemText(IDC_EDIT_Z_REF,z1);	          
	} 
	else if(pNMUpDown-> iDelta == -1) // �����ֵΪ-1 , ˵�������Spin�����ϼ�ͷ 
	{ 
		m_z_ref += 0.2;			
		z1.Format("%f",m_z_ref);
		SetDlgItemText(IDC_EDIT_Z_REF,z1);	        
	}
	*pResult = 0;

}


void CCoordinateGetterDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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



