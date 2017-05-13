#pragma once
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <stdlib.h>

//#define DEBUG 1

struct node_ {
	double angle;
	int r, c;
	struct node *opp, *prev, *next;
};
typedef struct node_ * NODE_;
void thresh (IplImage *p);
int ** makearray (int rows, int cols);
void boundary (IplImage *p);
void dump (IplImage *p);
int zabs (int a);
int Boundary (IplImage *a, int i, int j);
int objectPixel (IplImage *a, int i, int j);
int dcomp (int **dx, int ** dy, int ** old1, int ** new1, int nr, int nc);
void ssed8 (IplImage *p, IplImage *data, int **dy, int **dx);
void nearestContour (int I, int J, int **dx, int **dy, int *ci, int *cj);
int norm (int a, int b);
double angle2pt (float r1, float c1, float r2, float c2);