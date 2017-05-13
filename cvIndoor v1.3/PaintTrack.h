#pragma once
#include <opencv2/core/core.hpp>  
#include<opencv2/highgui/highgui.hpp>
#include "math.h"
#include <queue>
using namespace cv;  
using namespace std;

#define PI 3.14159265

class Pos
{
public:
	double x;
	double y;
	Pos(int x1=0,int y1=0){x=x1,y=y1;};
	Pos operator + (Pos& p1){int x2=x+p1.x,y2=y+p1.y;Pos p2(x2,y2);return p2;};
	bool operator == (Pos& p1){if(p1.x==x&&p1.y==y) return true;return false;}
	bool operator != (Pos& p1){if(p1.x==x&&p1.y==y) return false;return true;}
	Point p(){Point t(x,y);return t;}
};


class PaintTrack
{
private:
	IplImage* ori;
	int step;
	int threshold;
	int** zone;
	Pos sp;
	IplImage* track;
	IplImage* sketch;
	void clean();
	queue<Pos> cand;
	Pos dir[8];
	void findTrack(Pos p,float s,int l,queue<Pos>& q);
	void pushCand(Pos n);
	void paintNode(queue<Pos>& q);
	double A[2][2];
	double C[2];
	void coorT(Pos lt,Pos rt,Pos rb);
public:
	PaintTrack(IplImage* m,int s=3,int t=3);
	bool safe(Pos p);
	void startPaint();
};

int getIt(IplImage* m,Pos p);
void setIt(IplImage* m,Pos p,int t);