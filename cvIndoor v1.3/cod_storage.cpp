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
//	UINT32 Result;				// 存储摄像头拍摄图片的大小、颜色、帧速
	FGNODEINFO NodeInfo[3];
	UINT32 NodeCnt;
	FGFRAME Frame;
	int option = 0;
	
	// TODO: 在此添加控件通知处理程序代码;
	UINT32 format; // 存储摄像头拍摄图片的大小、颜色、帧速;
	IplImage *image = cvCreateImage(cvSize(1280,960),IPL_DEPTH_8U,3); //OPENCV创新存储图片的结构体变量;
	UINT Result = FGInitModule(NULL); //初始化摄像头;
	if(Result == FCE_NOERROR)
		Result = FGGetNodeList(NodeInfo,3,&NodeCnt); // Get list of connected nodes
	if(Result == FCE_NOERROR && NodeCnt)
		Result = Camera.Connect(&NodeInfo[option].Guid); // Connect with node
	if(Result==FCE_NOERROR) //以下为对摄像头的设置;
	{
		format=MAKEDCAMFORMAT(RES_1280_960,CM_RGB8,FR_1_875);//设置摄像头拍摄图片的大小、;
		//颜色、帧速;
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

	CvSize ori_size;//调整前的尺寸;
		ori_size.height = 960;
		ori_size.width = 1280;
	CvSize dst_size;//调整后的尺寸;
		dst_size.height = rc.Height();
		dst_size.width = rc.Width();

	IplImage *show1 = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);
	IplImage* contour = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//从原始图象提取圆;
	IplImage* GRAY = cvCreateImage(ori_size, IPL_DEPTH_8U, 1);//灰度	;
	IplImage *final = cvCreateImage(ori_size, IPL_DEPTH_8U, 3);//原图大小最终图;
	IplImage *show_final = cvCreateImage(dst_size, IPL_DEPTH_8U, 3);//显示大小最终图;

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
		Result=Camera.PutFrame(&Frame); //图像传输;
	
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
	cvSmooth(image, contour, CV_GAUSSIAN, 3, 0);//平滑图像。对图像进行核大小为3×3的高斯卷积;
	cvCvtColor(contour, GRAY, CV_BGR2GRAY);//转灰度;

	circles = cvHoughCircles(GRAY, storage, CV_HOUGH_GRADIENT, 2, GRAY->height/15, 200, 100, 10, 50);
	instruction.Format("There are %d circles!\n",circles->total);
	//MessageBox(instruction);
	for(i=0; i < circles->total; i++ )
	{
		p = (float*)cvGetSeqElem(circles, i);//返回索引指定的元素指针
		//cout<<"圆心x="<<cvRound(p[0])<<"y="<<cvRound(p[1]);
		if(cvRound(p[0]) >=260 && cvRound(p[1]) >= 120 &&cvRound(p[1]) <=840 && cvRound(p[2]) >= 12){
		  cvCircle( final, cvPoint(cvRound(p[0]),cvRound(p[1])), 3, CV_RGB(0,255,0), -1, 8, 0 );
		  //目标图像、圆心、半径、颜色、粗细、线条种类、缩放比例数据//CV_RGB(0,255,0)是绿点; 
		  cvCircle( final, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );
		  //CV_RGB(255,0,0)是红线;
		}
	}


	cvResize(final, show1, CV_INTER_LINEAR );//按scale放缩赋值;
	
	cvShowImage("INPUT",show1); //显示图片;
	cvSaveImage("image6.bmp",image); //显示图片;
	c = cvWaitKey(0);
	
	
	}// end of if 

	}// end of while(1)
	

	
}
#endif
#if 0
#include <list>
#include <vector>
#include <algorithm>
//连通区域属性结构;
typedef struct tagMarkRegion
{
std::list<POINT> MarkPointList;//点列表;
RECT rect;
}MarkRegion;
 
//定义MarkMap 结构，用来存放等价对;
typedef struct tagEqualMark
{   int MarkValue1;    //标记值;
	int MarkValue2;    //标记值;
} EqualMark;
 
//定义MarkMapping 结构，用来存放标记映射关系;
typedef struct tagMarkMapping
{    int nOriginalMark;   //第一次扫描的标记;
int nMappingMark;  //等价整理之后对应标记;
} MarkMapping;
 
/*
功能说明：八连通标记;
参数说明：I，表示图像数据指针;
           ImageWidth,表示图像宽;
           ImageHeight，表示图像高;
           off，表示偏移量;
           nFlag,表示指定标记;
           iColorType，表示颜色类型，（黑点，白点）;
           markInfo，表示连通区域属性信;息;
返回值：连通点数量，int类型;
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
     if(I[off]==iColorType && I[off]!=nFlag)//黑点同时未被标记的情况;
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
                   for(k=0; k<8; k++)//八邻域搜索;
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
