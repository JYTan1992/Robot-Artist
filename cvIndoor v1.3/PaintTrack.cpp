#include "StdAfx.h"    
#include "cvIndoor.h"
#include "cvIndoorDlg.h"
extern CcvIndoorDlg *pDlg;
using namespace std;  

PaintTrack::PaintTrack(IplImage* m,int s,int t)
{
	ori=m;
	step=s;
	threshold=t;
	sp=Pos(0,0);
	zone = new int*[step*2+1]; 
    for(int i=0;i<step*2+1;i++) zone[i] = new int[step*2+1];
	int x[8]={-1,0,1,1,1,0,-1,-1},y[8]={1,1,1,0,-1,-1,-1,0};
	for(int i=0;i<8;i++) dir[i]=Pos(x[i],y[i]);
};

bool PaintTrack::safe(Pos p){
	if(p.x>0&&p.x<cvGetDimSize(ori,1)&&p.y>0&&p.y<cvGetDimSize(ori,0)) return true;
	return false;
}

void PaintTrack::clean(){
	int i,j;
	for(i=0;i<step*2+1;i++)
	{
		for(j=0;j<step*2+1;j++) zone[i][j]=0;
	}
}

int getIt(IplImage* m,Pos p){
	return cvGet2D(m,p.y,p.x).val[0];
}

void setIt(IplImage* m,Pos p,int t){
	cvSet2D(m,p.y,p.x,cvScalar(t));
}

void PaintTrack::startPaint(){	//绘图主函数
	//清空 
	clean();
	track=cvCreateImage(cvGetSize(ori),IPL_DEPTH_8U,1);
	cvSet(track,cvScalar(255));
	sketch=cvCreateImage(cvGetSize(ori),IPL_DEPTH_8U,1);
	cvSet(sketch,cvScalar(255));

	queue<Pos> Node;
	while(sp.y!=cvGetDimSize(ori,0)) 
	{
		//扫描
		if(cand.empty())
		{
			for(;sp.y<cvGetDimSize(ori,0);sp.y++)
			{
				for(;sp.x<cvGetDimSize(ori,1);sp.x++)
				{					
					if(getIt(ori,sp)==255&&getIt(track,sp)!=0)
					{
						//开始查找轨迹
						Node.swap(queue<Pos>()) ;
						findTrack(sp,-3*PI/4,0,Node);
						paintNode(Node);
						break;
					}					
				}
				
				if(sp.x!=cvGetDimSize(ori,1)) break;
				else sp.x=0;
			}
		}
		else
		{
			Pos p=cand.front();
			cand.pop();						
			if (getIt(track,p)!=0)				
			{
				Node.swap(queue<Pos>()) ;
				findTrack(p,-3*PI/4,0,Node);					
				paintNode(Node);
			}
		}
	}
	cvShowImage("轨迹",track);
	cvShowImage("result",sketch);
}

void PaintTrack::findTrack(Pos p,float s,int l,queue<Pos>& q){
	clean();

	int i,j,k,m;
	queue<Pos> list;
	queue<Pos> al;
	Pos t(-1,-1);//下一个目标点	

	q.push(p);
	setIt(track,p,0); //标记为已搜索过
	list.push(p);
	for(i=0;i<step&&t.x==-1;i++)
	{
		int n=list.size();
		for(j=0;j<n&&t.x==-1;j++)
		{
			Pos c=list.front();
			list.pop();
			int dir4[4]={0,0,0,0};
			for(m=0;m<4;m++)
			{
				k=2*m+1;
				Pos w=c+dir[k];
				if(safe(w)&&getIt(ori,w)==255&&zone[int(w.y-p.y)+step][int(w.x-p.x)+step]==0&&w!=p)
				{
					zone[int(w.y-p.y)+step][int(w.x-p.x)+step]=k+1;
					dir4[m]=1;
					if(getIt(track,w)!=0)	
					{
						list.push(w);
					}
					else if(i!=0)
					{
						//确保图形闭合
						al.push(w); 
					}
				}
			}
			for(m=0;m<4;m++)
			{
				if(dir4[m]==0&&dir4[(m+3)%4]==0)
				{
					k=2*m;
					Pos w=c+dir[k];
					if(safe(w)&&getIt(ori,w)==255&&zone[int(w.y-p.y)+step][int(w.x-p.x)+step]==0&&w!=p)
					{
						zone[int(w.y-p.y)+step][int(w.x-p.x)+step]=k+1;
						dir4[m]=1;
						if(getIt(track,w)!=0)	
						{
							list.push(w);
						}
						else if(i!=0)
						{
							//确保图形闭合
							al.push(w); 
						}
				}
				}
			}
		}
		
	}

	if(list.empty())
	{
		if(al.empty())
		{
			//全部标黑
			for(i=0;i<step*2+1;i++)
			{
				for(j=0;j<step*2+1;j++)
				{
					if(zone[i][j]!=0)
					{
						setIt(track,Pos(j+p.x-step,i+p.y-step),0);
					}
				}
			}
			return;
		}
		else
		{
			int d=0;
			while(!al.empty())
			{
				Pos tmp=al.front();
				al.pop();
				int d1=abs(tmp.x-p.x)+abs(tmp.y-p.y);
				if(d1>d)
				{
					t=tmp;
					d=d1;
				}
			}
		}
	}
	else
	{
		int d=100;
		while(!list.empty())
		{
			Pos tmp=list.front();
			list.pop();
			int d1=abs(atan2(float(tmp.y-p.y),float(tmp.x-p.x))-s);//斜率差
			if(d1<d)
			{
				t=tmp;
				d=d1;
			}
		}
	}
	
	//从t点反推路径
	Pos n=t;
	while(n!=p)
	{
		 setIt(track,n,0);
		 int r=(zone[int(n.y-p.y)+step][int(n.x-p.x)+step]+3)%8;//反方向
		 Pos n1=n+dir[r];
		 setIt(track,n1,0);
		 if(n!=t)
		 {
			 pushCand(n);
		 }
		 n=n1;
	}
	pushCand(p);
	
	findTrack(t,atan2(float(t.y-p.y),float(t.x-p.x)),l+1,q);
}

void PaintTrack::pushCand(Pos n){
	Pos n2;
	for(int i=0;i<8;i++)
	{
		 n2=n+dir[i];
		if(safe(n2)&&getIt(ori,n2)==255&&getIt(track,n2)!=0)
		{
			setIt(track,n2,128);
			cand.push(n2);
		}
	}
}

void PaintTrack::coorT(Pos lt,Pos rt,Pos rb){
	double Width=sqrt((lt.x-rt.x)*(lt.x-rt.x)+(lt.y-rt.y)*(lt.y-rt.y));
	double Height=sqrt((rb.x-rt.x)*(rb.x-rt.x)+(rb.y-rt.y)*(rb.y-rt.y));

	
	if(double(cvGetDimSize(ori,1)/cvGetDimSize(ori,0))<(Width/Height))
	{
		double w=double(Height/cvGetDimSize(ori,0)*cvGetDimSize(ori,1));
		C[0]=lt.x+(rt.x-lt.x)/Width*((Width-w)/2);
		C[1]=lt.y+(rt.y-lt.y)/Width*((Width-w)/2);
		Width=w;
	}
	else
	{
		double h=double(Width/cvGetDimSize(ori,1)*cvGetDimSize(ori,0));
		C[0]=lt.x+(rb.x-rt.x)/Height*((Height-h)/2);
		C[1]=lt.y+(rb.y-rt.y)/Height*((Height-h)/2);
		Height=h;
	}

	A[0][0]=(rt.x-lt.x)/double(cvGetDimSize(ori,1));
	A[0][1]=(rt.y-lt.y)/double(cvGetDimSize(ori,1));
	A[1][0]=(rb.x-rt.x)/double(cvGetDimSize(ori,0));
	A[1][1]=(rb.y-rt.y)/double(cvGetDimSize(ori,0));
}

void PaintTrack::paintNode(queue<Pos>& Node){
	if(Node.size()>threshold)
	{
		Pos p=Node.front();
		Node.pop();
		while(!Node.empty())
		{
			Pos p1=Node.front();
			Node.pop();
			cvLine(sketch,p.p(),p1.p(),Scalar(0),1);
			p=p1;
		}
		cvShowImage("result",sketch);
//		pDlg->MessageBox("Dang!");
	}	
}