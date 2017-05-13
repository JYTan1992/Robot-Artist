//#pragma once
#ifndef ZSB
#define ZSB

#include "stdio.h"
#include "cv.h"
#include "highgui.h"
//#include "lib.h"
#include <math.h>

#define TRUE 1
#define FALSE 0
#define NORTH 1
#define SOUTH 3
#define REMOVE_STAIR 1





/* The image header data structure      */
struct header 
{
	int nr, nc;             /* Rows and columns in the image */
	int oi, oj;             /* Origin */
};

/*      The IMAGE data structure        */
struct image 
{
	struct header *info;            /* Pointer to header */
	unsigned char **data;           /* Pixel values */
};

#define SQRT2 1.414213562
#define BLACK 0
#define WHITE 1


typedef struct image * IMAGE;

#if defined (MAX)
int    PBM_SE_ORIGIN_COL=0, PBM_SE_ORIGIN_ROW=0;
char **arg;
int maxargs;
#else
extern int PBM_SE_ORIGIN_COL, PBM_SE_ORIGIN_ROW;
#endif


//#define DEBUG 1

struct node {
	double angle;
	int r, c;
	struct node *opp, *prev, *next;
};
typedef struct node * NODE;

void thnz (IMAGE im);
int nays8 (IMAGE im, int r, int c);
int Connectivity (IMAGE im, int r, int c);
void Delete (IMAGE im, IMAGE tmp);
void print_se (IMAGE p);
struct image  *newimage (int nr, int nc);
void freeimage (struct image  *z);

void CopyVarImage (IMAGE *a, IMAGE *b);
void Display (IMAGE x);

void srand32 (long k);
double drand32 ();
IplImage *toOpenCV (IMAGE x);
IMAGE fromOpenCV (IplImage *x);
void display_image (IMAGE x);
void save_image (IMAGE x, char *name);
IMAGE get_image (char *name);
void thnz (IMAGE im);
void check (int v1, int v2, int v3);
int edge (IMAGE im, int r, int c);
void stair (IMAGE im, IMAGE tmp, int dir);
int Yokoi (IMAGE im, int r, int c);
void pre_smooth (IMAGE im);
void match_du (IMAGE im, int r, int c, int k);
void aae (IMAGE image);
int snays (IMAGE im, int r, int c);
#endif