
#include "stdafx.h"
#include "cvIndoor.h"
#include "cvIndoorDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
using namespace std;

#define white 255
#define black 0
#define gray 128

void CcvIndoorDlg::cvRosenfeld(cv::Mat& src, cv::Mat& dst)
{
	////////////////////////////////
	//IplImage * pImgPlanes[3] = {0,0,0};
	
	////////////////////////////////
	if(src.type()!=CV_8UC1)
	{

		//src.convertTo(src,CV_8UC1,1,0);
		MessageBox("只能处理二值或灰度图像");
		return;
	}


    //非原地操作时候，copy src到dst
    if(dst.data!=src.data)
    {
        src.copyTo(dst);
    }

    int i, j, n;
    int width, height;
    //之所以减1，是方便处理8邻域，防止越界
    width = src.cols -1;
    height = src.rows -1;
    int step = src.step;
    int  p2,p3,p4,p5,p6,p7,p8,p9;
    uchar* img;
    bool ifEnd;
    cv::Mat tmpimg;
    int dir[4] = {-step, step, 1, -1};

    while(1)
    {
        //分四个子迭代过程，分别对应北，南，东，西四个边界点的情况
        ifEnd = false;
        for(n =0; n < 4; n++)
        {
            dst.copyTo(tmpimg);
            img = tmpimg.data;
            for(i = 1; i < height; i++)
            {
                img += step;
                for(j =1; j<width; j++)
                {
                    uchar* p = img + j;
                    //如果p点是背景点或者且为方向边界点，依次为北南东西，继续循环
                    if(p[0]==0||p[dir[n]]>0) continue;
                    p2 = p[-step]>0?1:0;
                    p3 = p[-step+1]>0?1:0;
                    p4 = p[1]>0?1:0;
                    p5 = p[step+1]>0?1:0;
                    p6 = p[step]>0?1:0;
                    p7 = p[step-1]>0?1:0;
                    p8 = p[-1]>0?1:0;
                    p9 = p[-step-1]>0?1:0;
                    //8 simple判定
                    int is8simple = 1;
                    if(p2==0&&p6==0)
                    {
                        if((p9==1||p8==1||p7==1)&&(p3==1||p4==1||p5==1)) 
                            is8simple = 0;
                    }
                    if(p4==0&&p8==0)
                    {
                        if((p9==1||p2==1||p3==1)&&(p5==1||p6==1||p7==1)) 
                            is8simple = 0;
                    }
                    if(p8==0&&p2==0)
                    {
                        if(p9==1&&(p3==1||p4==1||p5==1||p6==1||p7==1))
                            is8simple = 0;
                    }
                    if(p4==0&&p2==0)
                    {
                        if(p3==1&&(p5==1||p6==1||p7==1||p8==1||p9==1))
                            is8simple = 0;
                    }
                    if(p8==0&&p6==0)
                    {
                        if(p7==1&&(p3==9||p2==1||p3==1||p4==1||p5==1))
                            is8simple = 0;
                    }
                    if(p4==0&&p6==0)
                    {
                        if(p5==1&&(p7==1||p8==1||p9==1||p2==1||p3==1))
                            is8simple = 0;
                    }
                    int adjsum;
                    adjsum = p2 + p3 + p4+ p5 + p6 + p7 + p8 + p9;
                    //判断是否是邻接点或孤立点,0,1分别对于那个孤立点和端点
                    if(adjsum!=1&&adjsum!=0&&is8simple==1)
                    {
                        dst.at<uchar>(i,j) = 0; //满足删除条件，设置当前像素为0
                        ifEnd = true;
                    }

                }
            }
        }

        //已经没有可以细化的像素了，则退出迭代
        if(!ifEnd) break;
    }
	imshow("After",dst);
}

int func_nc8(int *b)
    //端点的连通性检测
{
    int n_odd[4] = { 1, 3, 5, 7 };  //四邻域
    int i, j, sum, d[10];          

    for (i = 0; i <= 9; i++) {
        j = i;
        if (i == 9) j = 1;
        if (abs(*(b + j)) == 1)
        {
            d[i] = 1;
        } 
        else 
        {
            d[i] = 0;
        }
    }
    sum = 0;
    for (i = 0; i < 4; i++)
    {
        j = n_odd[i];
        sum = sum + d[j] - d[j] * d[j + 1] * d[j + 2];
    }
    return (sum);
}

void CcvIndoorDlg::cvHilditchThin(cv::Mat& src, cv::Mat& dst)
{
    if(src.type()!=CV_8UC1)
    {
        MessageBox("只能处理二值或灰度图像");
        return;
    }
    //非原地操作时候，copy src到dst
    if(dst.data!=src.data)
    {
        src.copyTo(dst);
    }

    //8邻域的偏移量
    int offset[9][2] = {{0,0},{1,0},{1,-1},{0,-1},{-1,-1},
    {-1,0},{-1,1},{0,1},{1,1} };
    //四邻域的偏移量
    int n_odd[4] = { 1, 3, 5, 7 };      
    int px, py;                        
    int b[9];                      //3*3格子的灰度信息
    int condition[6];              //1-6个条件是否满足
    int counter;                   //移去像素的数量
    int i, x, y, copy, sum;      

    uchar* img;
    int width, height;
    width = dst.cols;
    height = dst.rows;
    img = dst.data;
    int step = dst.step ;
    do
    {

        counter = 0;

        for (y = 0; y < height; y++)
        {

            for (x = 0; x < width; x++) 
            {

                //前面标记为删除的像素，我们置其相应邻域值为-1
                for (i = 0; i < 9; i++) 
                {
                    b[i] = 0;
                    px = x + offset[i][0];
                    py = y + offset[i][1];
                    if (px >= 0 && px < width &&    py >= 0 && py <height) 
                    {
                        // printf("%d\n", img[py*step+px]);
                        if (img[py*step+px] == white)
                        {
                            b[i] = 1;
                        } 
                        else if (img[py*step+px]  == gray) 
                        {
                            b[i] = -1;
                        }
                    }
                }
                for (i = 0; i < 6; i++)
                {
                    condition[i] = 0;
                }

                //条件1，是前景点
                if (b[0] == 1) condition[0] = 1;

                //条件2，是边界点
                sum = 0;
                for (i = 0; i < 4; i++) 
                {
                    sum = sum + 1 - abs(b[n_odd[i]]);
                }
                if (sum >= 1) condition[1] = 1;

                //条件3， 端点不能删除
                sum = 0;
                for (i = 1; i <= 8; i++)
                {
                    sum = sum + abs(b[i]);
                }
                if (sum >= 2) condition[2] = 1;

                //条件4， 孤立点不能删除
                sum = 0;
                for (i = 1; i <= 8; i++)
                {
                    if (b[i] == 1) sum++;
                }
                if (sum >= 1) condition[3] = 1;

                //条件5， 连通性检测
                if (func_nc8(b) == 1) condition[4] = 1;

                //条件6，宽度为2的骨架只能删除1边
                sum = 0;
                for (i = 1; i <= 8; i++)
                {
                    if (b[i] != -1) 
                    {
                        sum++;
                    } else 
                    {
                        copy = b[i];
                        b[i] = 0;
                        if (func_nc8(b) == 1) sum++;
                        b[i] = copy;
                    }
                }
                if (sum == 8) condition[5] = 1;

                if (condition[0] && condition[1] && condition[2] &&condition[3] && condition[4] && condition[5])
                {
                    img[y*step+x] = gray; //可以删除，置位gray，gray是删除标记，但该信息对后面像素的判断有用
                    counter++;
                    
                }
            } 
        }

        if (counter != 0)
        {
            for (y = 0; y < height; y++)
            {
                for (x = 0; x < width; x++)
                {
                    if (img[y*step+x] == gray)
                        img[y*step+x] = black;

                }
            }
        }

    }while (counter != 0);
}