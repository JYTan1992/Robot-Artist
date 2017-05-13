#include "stdafx.h"
using namespace std;
void cvRosenfeld(cv::Mat& src, cv::Mat& dst)
{

    //��ԭ�ز���ʱ��copy src��dst
    if(dst.data!=src.data)
    {
        src.copyTo(dst);
    }

    int i, j, n;
    int width, height;
    //֮���Լ�1���Ƿ��㴦��8���򣬷�ֹԽ��
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
        //���ĸ��ӵ������̣��ֱ��Ӧ�����ϣ��������ĸ��߽������
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
                    //���p���Ǳ����������Ϊ����߽�㣬����Ϊ���϶���������ѭ��
                    if(p[0]==0||p[dir[n]]>0) continue;
                    p2 = p[-step]>0?1:0;
                    p3 = p[-step+1]>0?1:0;
                    p4 = p[1]>0?1:0;
                    p5 = p[step+1]>0?1:0;
                    p6 = p[step]>0?1:0;
                    p7 = p[step-1]>0?1:0;
                    p8 = p[-1]>0?1:0;
                    p9 = p[-step-1]>0?1:0;
                    //8 simple�ж�
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
                    //�ж��Ƿ����ڽӵ�������,0,1�ֱ�����Ǹ�������Ͷ˵�
                    if(adjsum!=1&&adjsum!=0&&is8simple==1)
                    {
                        dst.at<uchar>(i,j) = 0; //����ɾ�����������õ�ǰ����Ϊ0
                        ifEnd = true;
                    }

                }
            }
        }

        //�Ѿ�û�п���ϸ���������ˣ����˳�����
        if(!ifEnd) break;
    }
	imshow("After",dst);
}