//#include "choi.h"
#include "stdafx.h"
///* Choi thinning using the outline */
#if 0
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <stdlib.h>

//#define DEBUG 1

struct node {
	double angle;
	int r, c;
	struct node *opp, *prev, *next;
};
typedef struct node * NODE;
#endif
void thresh (IplImage *p)
{
	int i,j;
	CvScalar s, t;

	t.val[0] = t.val[1] = t.val[2] = 255;
	for (i=0; i<p->height; i++)
		for (j=0; j<p->width; j++)
		{
			s = cvGet2D (p, i, j);
			if (s.val[0] != 0) cvSet2D (p, i, j, t);
		}
}

int ** makearray (int rows, int cols)
{
	int *ptr, **a;
	int i;

	ptr = (int *)malloc ( sizeof(int) * rows*cols );
	a = (int **) malloc (sizeof(int *)*rows);
	*a = ptr;
	for (i=1; i<rows; i++)
		a[i] = &(ptr[i*cols]);
	return a;
}

void boundary (IplImage *p)
{
	int i,j;
	CvScalar r, s, t;

	r.val[0] = 1;
	for (i=1; i<p->height-1; i++)
		for (j=1; j<p->width-1; j++)
		{
			s = cvGet2D (p, i, j);
			if (s.val[0] == 0)
			{
				t = cvGet2D (p, i-1, j-1);
				if (t.val[0] > 1) { cvSet2D (p, i, j, r); continue; }
				t = cvGet2D (p, i-1, j);
				if (t.val[0] > 1) { cvSet2D (p, i, j, r); continue; }
				t = cvGet2D (p, i-1, j+1);
				if (t.val[0] > 1) { cvSet2D (p, i, j, r); continue; }

				t = cvGet2D (p, i, j-1);
				if (t.val[0] > 1) { cvSet2D (p, i, j, r); continue; }
				t = cvGet2D (p, i, j+1);
				if (t.val[0] > 1) { cvSet2D (p, i, j, r); continue; }

				t = cvGet2D (p, i+1, j-1);
				if (t.val[0] > 1) { cvSet2D (p, i, j, r); continue; }
				t = cvGet2D (p, i+1, j);
				if (t.val[0] > 1) { cvSet2D (p, i, j, r); continue; }
				t = cvGet2D (p, i+1, j+1);
			}
		}

	t.val[0] = t.val[1]= t.val[2]= 255; 
	r.val[0] = r.val[1] = r.val[2] = 0;
	for (i=1; i<p->height-1; i++)
		for (j=1; j<p->width-1; j++)
		{
			s = cvGet2D (p, i,j);
			if (s.val[0] == 1) cvSet2D (p, i,j,r);
				else cvSet2D (p, i,j, t);
		}
}

void dump (IplImage *p)
{
	int i,j;
	CvScalar s;
	char c;

	for (i=1; i<24; i++)
	{
		for (j=1; j<24; j++)
		{
			s = cvGet2D (p, i, j);
			printf ("%3d ", (int)s.val[1]);
		}
		printf ("\n");
	}
	printf ("===================================================\n\n");
	scanf ("%c", &c);
}

int zabs (int a)
{
	if (a >= 0) return a;
	else return -a;
}

int Boundary (IplImage *a, int i, int j)
{
	CvScalar s;

	s = cvGet2D (a, i, j);
	if (s.val[0] == 0) return 1;
	return 0;
}

int objectPixel (IplImage *a, int i, int j)
{
	CvScalar s;

	s = cvGet2D ( a, i, j );
	if (s.val[0] == 0) return 1;
	return 0;
}

int dcomp (int **dx, int ** dy, int ** old1, int ** new1, int nr, int nc)
{
	int i,j,d=0, a;

	for (i=0; i<nr; i++)
		for (j=0; j<nc; j++)
			if ( (dx[i][j]>=0 && dx[i][j]<1000) && (dy[i][j]>=0 && dy[i][j]<1000) )
			{
				a = zabs(dx[i][j]) + zabs(dy[i][j]);
				if (old1[i][j] != a) d++;
				old1[i][j] = new1[i][j];
				new1[i][j] = a;
			}
			return d;
}

void ssed8 (IplImage *p, IplImage *data, int **dy, int **dx)
{
	int i,j, jj, val=0, height, width, x, y;
	int change = 1, count=0, d, mind, m1, m2;
	CvScalar r, s, t;
	int BACKGROUND = -100, BOUNDARY=0, OBJECT=1000, UNKNOWN=254;
	float rtz;
	int **old1, **new1;

	height = p->height;   width = p->width;
	old1 = makearray (height, width);
	new1 = makearray (height, width);

// Initialize distance arrays
	for (i=0; i<height; i++)
		for (j=0; j<width; j++)
		{
			old1[i][j] = 0;
			new1[i][j] = 0;
		}

// Initiaize
		for (i=0; i<height; i++)
			for (j=0; j<width; j++)
				if (Boundary (p, i, j))			// Boundary pixel
				{
					dx[i][j] = 0; dy[i][j] = 0;
				} else if (objectPixel (data, i, j))	// Object pixel
				{
					dx[i][j] = 1000; dy[i][j] = 1000;
				} else
				{
					dx[i][j] = BACKGROUND; dy[i][j] = BACKGROUND;
				}
				printf ("In 8 distance point A\n");
//	dumpa (dx, dy, height, width);

// Repeat until no change

	while (change)
	{
		change = 0;

// Scan the images top down, left to right
		for (i=1; i<height-2; i++)
			for (j=1; j<width-2; j++)
				if ((dx[i][j] <1000)&&(dx[i][j]>BACKGROUND))
				{
					m1 = (int)(zabs(dx[i][j]) + zabs(dy[i][j]));

					if (objectPixel (data, i-1, j))
					{
					 if ((dx[i-1][j] == 1000))		// Up
					 {
						dx[i-1][j] = dx[i][j];  dy[i-1][j] = dy[i][j] + 1; change += 1;
					 } else if (dx[i-1][j] > BACKGROUND)
					 {
						m1 = zabs(dx[i][j]-1) + zabs(dy[i][j]+1);
						m2 = (int)(zabs(dx[i-1][j]) + zabs(dy[i-1][j]));
						if (m1 < m2)
						{
							if ( (dx[i][j-1] != dx[i][j]) || (dy[i][j-1] != dy[i][j]+1)) change += 1;
							dx[i-1][j] = dx[i][j]; 
							dy[i-1][j] = dy[i][j] + 1;
						}
					 }
//					 dumpb (dx, dy, height, width, i,j, "Up");
					}

					if (objectPixel (data, i+1,j))
					{
					 if ((dx[i+1][j] == 1000))		// Down
					 {
						dx[i+1][j] = dx[i][j];  dy[i+1][j] = dy[i][j] - 1; 
						change += 1;
					 } else if (dx[i+1][j] > BACKGROUND)
					 {
						m1 = zabs(dx[i][j]-1) + zabs(dy[i][j]-1);
						m2 = (int)(zabs(dx[i+1][j]) + zabs(dy[i+1][j]));
						if (m1 < m2)
						{
							if ( (dx[i][j-1] != dx[i][j]) || (dy[i][j-1] != dy[i][j]-1)) change += 1;
							dx[i+1][j] = dx[i][j]; 
							dy[i+1][j] = dy[i][j] - 1;
						}
					 }
//					dumpb (dx, dy, height, width, i,j, "Down");
					}

					if (objectPixel(data, i, j+1))
					{
					 if ((dx[i][j+1] == 1000))		// Right
					 {
						dx[i][j+1] = dx[i][j] - 1;  dy[i][j+1] = dy[i][j]; 
						change += 1;
					 } else if (dx[i-1][j] > BACKGROUND)
					 {
						m1 = zabs(dx[i][j]-1) + zabs(dy[i][j]);
						m2 = (int)(zabs(dx[i][j+1]) + zabs(dy[i][j+1]));
						if (m1 < m2)
						{
							if ( (dx[i][j-1] != dx[i][j]-1) || (dy[i][j-1] != dy[i][j])) change += 1;
							dx[i][j+1] = dx[i][j]-1; 
							dy[i][j+1] = dy[i][j];
						}
					 }
//					 dumpb (dx, dy, height, width, i,j, "Right");
					}

					if(objectPixel(data, i, j-1))
					{
					 if ((dx[i][j-1] == 1000))		// Left
					 {
						dx[i][j-1] = dx[i][j]+1;  dy[i][j-1] = dy[i][j]; 
						change += 1;
					 } else if (dx[i][j-1] > BACKGROUND)
					 {
 						m1 = zabs(dx[i][j]+1) + zabs(dy[i][j]);
						m2 = (int)(zabs(dx[i][j-1]) + zabs(dy[i][j-1]));
						if (m1 < m2)
						{
							if ( (dx[i][j-1] != dx[i][j]+1) || (dy[i][j-1] != dy[i][j])) change += 1;
							dx[i][j-1] = dx[i][j]+1; 
							dy[i][j-1] = dy[i][j];
						}
					 }
					}
//					 dumpb (dx, dy, height, width, i,j, "Left");
				}
		change = dcomp (dx, dy, old1, new1, height, width);
	}
	free (old1[0]); free(new1[0]);
	free(old1);     free(new1);
}

void nearestContour (int I, int J, int **dx, int **dy, int *ci, int *cj)
{
	*ci = I + dy[I][J];
	*cj = J + dx[I][J];
}

int norm (int a, int b)
{
	return a*a + b*b;
}

double angle2pt (float r1, float c1, float r2, float c2)
{
/*      Compute the angle between two points. (r1,c1) is the origin
	specified as row, column, and (r2,c2) is the second point.
	Result is between 0-360 degrees, where 0 is horizontal right. */

	double atan(), fabs();
	double x, dr, dc, conv;

	conv = 180.0/3.1415926535;
	dr = (double)(r2-r1); dc = (double)(c2-c1);

/*      Compute the raw angle based of Drow, Dcolumn            */
	if (dr==0 && dc == 0) x = 0.0;
	else if (dc == 0) x = 90.0;
	else {
		x= abs(atan2(dr,dc));
		//x = fabs(atan (double(dr/dc)));
		x = x * conv;
	}

/*      Adjust the angle according to the quadrant              */
	if (dr <= 0) {                  /* upper 2 quadrants */
	  if (dc < 0) x = 180.0 - x;    /* Left quadrant */
	} else if (dr > 0) {            /* Lower 2 quadrants */
	  if (dc < 0) x = x + 180.0;    /* Left quadrant */
	  else x = 360.0-x;             /* Right quadrant */
	}

	return x;
}

#if 0
int main (int argc, char *argv[])
{
	IplImage *data, *im, *d1, *d2;
	int i,j, **dy, **dx, nr, nc, Px, Py, Qix[9], Qiy[9], Qx, Qy, more=0;
	char filename[128] = "H:\\AIPCV\\CH5\\5r.pbm";
	int flag = 0,jj,k,g,ii;
	CvScalar ss;
	int RADIUS = 120;
	float a1, a2;
	FILE *out;

	printf ("Thinning. Please enter the input image file name.\n");
//	scanf ("%s", filename);

	data = cvLoadImage (filename, 1);
	if (data == NULL)
	{
		printf ("Bad input file '%s'\n", filename);
		exit(1);
	}
	cvNamedWindow ("Input Image", 1);
	cvShowImage ("Input Image", data);
	cvWaitKey (0);

// Threshold. 0 are object, 255 everything else.
	thresh (data);

	d1 = cvCloneImage(data);		 // Copy
	nr = data->height; nc = data->width;
	boundary (d1); // Initialize the image - boundary is 0, object is 255, others are 1
	dy = makearray (nr, nc);
	dx = makearray (nr, nc);


//	Compute the signed sequential Euclidean distance (8SSED) map.
	ssed8 (d1, data, dy, dx);
/*	out = fopen ("H:\\AIPCV\\CH5\\distance", "w");
	if (out)
	{
		for (i=0; i<nr; i++)
		{
			for (j=0; j<nc; j++)
			{
				fprintf (out, "(%d,%d) ", dx[i][j], dy[i][j]);
			}
			fprintf (out, "\n");
		}
	}
	fclose (out); */

    cvShowImage ("Input Image", d1);
	cvWaitKey (0);

	d2 = cvCloneImage (d1);
	ss.val[1] = 128;
	ss.val[0] = 128;

//	Set P to be the first pixel. This will be (Py, Px)
// do for all object pixels.
	Px = Py = -1;
	for (i=0; i<nr; i++)
	{
		for (j=0; j<nc; j++)
		{
			if ( !objectPixel(data, i, j)) continue;
			if (Boundary(d1, i, j) ) continue;
			Px = j; Py = i; 

// Set Q=DM(P) to be the nearest contour point of the  pixel P according to the 
// distance map where DM(.) represents the 8SSED map.
			Qx = Px + dx[Py][Px]; 
			Qy = Py + dy[Py][Px];
			nearestContour (Py, Px, dx, dy, &Qy, &Qx);

			if ( (Qx >= nc) || (Qy >= nr) ) continue;

			if (!objectPixel (data, Qy, Qx)) 
				printf ("Warning: From (%d,%d) + (%d, %d) the result Q=(%d,%d) is not object.\n", 
					Py, Px, dx[Py][Px], dy[Py][Px], Qy, Qx);
			if (!Boundary(d1, Qy, Qx))
				printf ("Warning: From (%d,%d) + (%d, %d) the result Q=(%d,%d) is not boundary.\n", 
					 Py, Px, dx[Py][Px], dy[Py][Px], Qy, Qx);

// Find the nearest contour points of its 8 neighbors Pi
//           Qi = DM(Pi) + (xi; yi)
// where (xi; yi) is the relative position of the neighborhood i with respect to 
			nearestContour (Py-1, Px,   dx, dy, &(Qiy[0]), &(Qix[0]));
			nearestContour (Py-1, Px-1, dx, dy, &(Qiy[1]), &(Qix[1]));
			nearestContour (Py,   Px-1, dx, dy, &(Qiy[2]), &(Qix[2]));
			nearestContour (Py+1, Px-1, dx, dy, &(Qiy[3]), &(Qix[3]));
			nearestContour (Py+1, Px,   dx, dy, &(Qiy[4]), &(Qix[4]));
			nearestContour (Py+1, Px+1, dx, dy, &(Qiy[5]), &(Qix[5]));
			nearestContour (Py,   Px+1, dx, dy, &(Qiy[6]), &(Qix[6]));
			nearestContour (Py-1, Px+1, dx, dy, &(Qiy[7]), &(Qix[7]));

// the pixel P and i = 1,2,. . . ,8 is the index of the neighborhoods.
// The eight point pairs are formed by the nearest contour points of both the pixel P 
// and its 8-neighbors, which are denoted as (Q;Qi)
//  where i = 1,2,. . . ,8.
			for (ii=0; ii<8; ii++)
			{

// Apply the connectivity criterion:
//	D*D =|Qi?Q|>=r (r=2)  
				if (norm(Qiy[ii]-Qy, Qix[ii]-Qx) < RADIUS) continue;		// Nope
			
//			and |Qi|*|Qi| - |Q|*|Q| <= max(X(Qi-Q),Y(Qi-Q))
				jj = norm(Qiy[ii], Qix[ii])*norm(Qiy[ii], Qix[ii]) - norm(Qx, Qy)*norm(Qx,Qy);
				k = Qix[ii]-Qx;  if (k<0) k = -k;
				g = Qiy[ii]-Qy;  if (g<0) g = -g;
				if (k<g) k = g;
				if (jj > k) continue;

//where X(p) and X(p) represent the x and y coordinates of the point.

				flag = 1;
				printf ("Skeletal at (%d, %d)\n", Py, Px);
				a1 = angle2pt ((float)Py, (float)Px, (float)Qy, (float)Qx);
				a2 = angle2pt ((float)Py, (float)Px, (float)Qiy[ii], (float)Qix[ii]);
				printf ("Skeletal point at (%d,%d): P-Q is %f  Qi-Q is %f\n", 
					Px, Py, angle2pt ((float)Py, (float)Px, (float)Qy, (float)Qx),
					angle2pt ((float)Py, (float)Px, (float)Qiy[ii], (float)Qix[ii]));
				a1 = a1 - a2;
				if (a1 < 0.0f) a1 = -a1;
				if (a1 < 90.0)
				{
					printf ("This point fails.  (%f)\n", a1);
					continue;
				}
				cvSet2D (d1, Py, Px, ss);
				break;
			}

// If one of the point pairs, (Q,Qi), satisfies the connectivity
//  criterion, then the pixel P is a skeleton point.
		}

	} while (more);
    cvShowImage ("Input Image", d1);
	cvWaitKey (0);
	return 0;
}
#endif
