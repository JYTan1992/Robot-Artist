#include "stdafx.h"
#include "cvIndoor.h"
#include "cvIndoorDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <iostream>
#include <string>
#include "FGCamera.h"
#include "errstat.h"
using namespace cv;
using namespace std;



#if 0
void CcvIndoorDlg::OnBnClickedCamera()
{
	CFGCamera Camera;
//	UINT32 Result;				// �洢����ͷ����ͼƬ�Ĵ�С����ɫ��֡��
	FGNODEINFO NodeInfo[3];
	UINT32 NodeCnt;
	FGFRAME Frame;
	int option = 0;
	
	// TODO: �ڴ���ӿؼ�֪ͨ����������;
	UINT32 format; // �洢����ͷ����ͼƬ�Ĵ�С����ɫ��֡��;
	IplImage *image = cvCreateImage(cvSize(1280,960),IPL_DEPTH_8U,3); //OPENCV���´洢ͼƬ�Ľṹ�����;
	UINT Result = FGInitModule(NULL); //��ʼ������ͷ;
	if(Result == FCE_NOERROR)
		Result = FGGetNodeList(NodeInfo,3,&NodeCnt); // Get list of connected nodes
	if(Result == FCE_NOERROR && NodeCnt)
		Result = Camera.Connect(&NodeInfo[option].Guid); // Connect with node
	if(Result==FCE_NOERROR) //����Ϊ������ͷ������;
	{
		format=MAKEDCAMFORMAT(RES_1280_960,CM_RGB8,FR_1_875);//��������ͷ����ͼƬ�Ĵ�С��;
		//��ɫ��֡��;
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
	CWnd *pWnd=GetDlgItem(IDC_BUTTON_BROWSE);
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

	CvSize ori_size;//����ǰ�ĳߴ�;
		ori_size.height = 960;
		ori_size.width = 1280;
	CvSize dst_size;//������ĳߴ�;
		dst_size.height = rc.Height();
		dst_size.width = rc.Width();

	IplImage *show1 = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);
	IplImage* contour = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//��ԭʼͼ����ȡԲ;
	IplImage* GRAY = cvCreateImage(ori_size, IPL_DEPTH_8U, 1);//�Ҷ�	;
	IplImage *final = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//ԭͼ��С����ͼ;
	IplImage *show_final = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);//��ʾ��С����ͼ;

	CvSeq * circles = 0;
	CvMemStorage * storage = cvCreateMemStorage(0);

	CString instruction;
	float* p;
	char c;

	while(1)//////////////////while(1)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	{
	
	if(Result==FCE_NOERROR)
	{
		Camera.GetFrame(&Frame);
		Result=Camera.PutFrame(&Frame); //ͼ����;
	
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
	cvSmooth(image, contour, CV_GAUSSIAN, 3, 0);//ƽ��ͼ�񡣶�ͼ����к˴�СΪ3��3�ĸ�˹���;
	cvCvtColor(contour, GRAY, CV_BGR2GRAY);//ת�Ҷ�;

	circles = cvHoughCircles(GRAY, storage, CV_HOUGH_GRADIENT, 2, GRAY->height/15, 200, 100, 10, 50);
	instruction.Format("There are %d circles!\n",circles->total);
	//MessageBox(instruction);
	for(i=0; i < circles->total; i++ )
	{
		p = (float*)cvGetSeqElem(circles, i);//��������ָ����Ԫ��ָ��
		//cout<<"Բ��x="<<cvRound(p[0])<<"y="<<cvRound(p[1]);
		if(cvRound(p[0]) >=260 && cvRound(p[1]) >= 120 &&cvRound(p[1]) <=840 && cvRound(p[2]) >= 12){
		  cvCircle( final, cvPoint(cvRound(p[0]),cvRound(p[1])), 3, CV_RGB(0,255,0), -1, 8, 0 );
		  //Ŀ��ͼ��Բ�ġ��뾶����ɫ����ϸ���������ࡢ���ű�������//CV_RGB(0,255,0)���̵�; 
		  cvCircle( final, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );
		  //CV_RGB(255,0,0)�Ǻ���;
		}
	}


	cvResize(final, show1, CV_INTER_LINEAR );//��scale������ֵ;
	
	cvShowImage("INPUT",show1); //��ʾͼƬ;
	cvSaveImage("image6.bmp",image); //��ʾͼƬ;
	c = cvWaitKey(0);
	
	
	}// end of if 

	}// end of while(1)
	

	
}
#endif
#if 0
#include <list>
#include <vector>
#include <algorithm>
//��ͨ�������Խṹ;
typedef struct tagMarkRegion
{
std::list<POINT> MarkPointList;//���б�;
RECT rect;
}MarkRegion;
 
//����MarkMap �ṹ��������ŵȼ۶�;
typedef struct tagEqualMark
{   int MarkValue1;    //���ֵ;
	int MarkValue2;    //���ֵ;
} EqualMark;
 
//����MarkMapping �ṹ��������ű��ӳ���ϵ;
typedef struct tagMarkMapping
{    int nOriginalMark;   //��һ��ɨ��ı��;
int nMappingMark;  //�ȼ�����֮���Ӧ���;
} MarkMapping;
 
/*
����˵��������ͨ���;
����˵����I����ʾͼ������ָ��;
           ImageWidth,��ʾͼ���;
           ImageHeight����ʾͼ���;
           off����ʾƫ����;
           nFlag,��ʾָ�����;
           iColorType����ʾ��ɫ���ͣ����ڵ㣬�׵㣩;
           markInfo����ʾ��ͨ����������;Ϣ;
����ֵ����ͨ��������int����;
*/

int FillAreaFlag33(LPINT I,int ImageWidth,int ImageHeight,long off,int nFlag,int iColorType,MarkRegion &markInfo)
{
     bool bNew;
     RECT rect;
     int m,n,i,j,k,nDot=1,offset,offtemp,yMin;
     int dxy[8],x,y;
     dxy[0]=-ImageWidth-1;     dxy[1]=-ImageWidth;                  dxy[2]=-ImageWidth+1;
     dxy[3]=-1;                                                                                     dxy[4]=1;
     dxy[5]=ImageWidth-1;       dxy[6]=ImageWidth;          dxy[7]=ImageWidth+1;
     rect.left=65535; rect.right=-1;
     rect.bottom=65535;   rect.top=-1;
     markInfo.MarkPointList.clear();
     POINT ptTmp;
     if(I[off]==iColorType && I[off]!=nFlag)//�ڵ�ͬʱδ����ǵ����;
     {
         I[off]=nFlag;
         x=off%ImageWidth;
         y=off/ImageWidth;
         ptTmp.x = x;
         ptTmp.y = y;
         markInfo.MarkPointList.push_back(ptTmp);
         if(x<rect.left)
              rect.left=x;
         if(x>rect.right)
              rect.right=x;
         if(y<rect.bottom)
              rect.bottom=y;
         if(y>rect.top)
              rect.top=y;
     }
     else
     {
         return 0;
     }

     for(i=y; i<ImageHeight; i++)
     {
         bNew=false;
         yMin=i;
         for(j=0; j<ImageWidth; j++)
         {
              offset=i*ImageWidth+j;
              if(I[offset]==nFlag)
              {
                   for(k=0; k<8; k++)//����������;
                   {
                       if(i==0 && k<=2)
                            continue;
                       if(i==ImageHeight-1 && k>=5)
                            continue;
                       if(j==0 && (k==0 || k==3 || k==5))
                            continue;
                       if(j==ImageWidth-1 && (k==2 || k==4 || k==7))
                            continue;
                       offtemp=offset+dxy[k];
                       if(I[offtemp]==iColorType && I[offtemp]!=nFlag)
                       {
                            I[offtemp]=nFlag;
                            nDot++;
                            m=offtemp/ImageWidth;
                            n=offtemp%ImageWidth;
                            ptTmp.x = n;
                            ptTmp.y = m;
                            markInfo.MarkPointList.push_back(ptTmp);
                            if(n < rect.left)
                                 rect.left=n;
                            if(n > rect.right)
                                 rect.right=n;
                            if(m < rect.bottom)
                                 rect.bottom=m;
                            if(m > rect.top)
                                 rect.top=m;
                            y=offtemp/ImageWidth;
                            if(y<=yMin)
                            {
                                 yMin=y;
                                 if(!bNew)
                                     bNew=true;
                            }
                       }
                   }
              }
         }
         if(bNew)
         {
              i=yMin-1;
         }
     }
     markInfo.rect.left = rect.left;
     markInfo.rect.right = rect.right;
     markInfo.rect.top = rect.top;
     markInfo.rect.bottom = rect.bottom;
     return nDot;
}
#endif
	//  Author:  www.icvpr.com  
	//  Blog  :  http://blog.csdn.net/icvpr 
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <stack>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


	void icvprCcaBySeedFill(const cv::Mat& _binImg, cv::Mat& _lableImg)
{
	// connected component analysis (4-component)
	// use seed filling algorithm
	// 1. begin with a foreground pixel and push its foreground neighbors into a stack;
	// 2. pop the top pixel on the stack and label it with the same label until the stack is empty
	// 
	// foreground pixel: _binImg(x,y) = 1
	// background pixel: _binImg(x,y) = 0


	if (_binImg.empty() ||
		_binImg.type() != CV_8UC1)
	{
		return ;
	}

	_lableImg.release() ;
	_binImg.convertTo(_lableImg, CV_32SC1) ;

	int label = 1 ;  // start by 2

	int rows = _binImg.rows - 1 ;
	int cols = _binImg.cols - 1 ;
	for (int i = 1; i < rows-1; i++)
	{
		int* data= _lableImg.ptr<int>(i) ;
		for (int j = 1; j < cols-1; j++)
		{
			if (data[j] == 1)
			{
				std::stack<std::pair<int,int>> neighborPixels ;   
				neighborPixels.push(std::pair<int,int>(i,j)) ;     // pixel position: <i,j>
				++label ;  // begin with a new label
				while (!neighborPixels.empty())
				{
					// get the top pixel on the stack and label it with the same label
					std::pair<int,int> curPixel = neighborPixels.top() ;
					int curX = curPixel.first ;
					int curY = curPixel.second ;
					_lableImg.at<int>(curX, curY) = label ;

					// pop the top pixel
					neighborPixels.pop() ;

					// push the 4-neighbors (foreground pixels)
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
				}		
			}
		}
	}
}
