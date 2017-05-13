/* Basic Zhang & Suen's thinnning algorithm */

#include "stdio.h"
#include "cv.h"
#include "highgui.h"

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

long seed = 132531;
typedef struct image * IMAGE;

#if defined (MAX)
int    PBM_SE_ORIGIN_COL=0, PBM_SE_ORIGIN_ROW=0;
char **arg;
int maxargs;
#else
extern int PBM_SE_ORIGIN_COL, PBM_SE_ORIGIN_ROW;
#endif


#define DEBUG 1

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
double angle_2pt (int r1, int  c1, int  r2, int  c2);
int range (IMAGE im, int i, int j);
void print_se (IMAGE p);
IMAGE Input_PBM (char *fn);
IMAGE Output_PBM (IMAGE image, char *filename);
void get_num_pbm (FILE *f, char *b, int *bi, int *res);
void pbm_getln (FILE *f, char *b);
void pbm_param (char *s);
struct image  *newimage (int nr, int nc);
void freeimage (struct image  *z);
void sys_abort (int val, char *mess);
void CopyVarImage (IMAGE *a, IMAGE *b);
void Display (IMAGE x);
float ** f2d (int nr, int nc);
void srand32 (long k);
double drand32 ();
IplImage *toOpenCV (IMAGE x);
IMAGE fromOpenCV (IplImage *x);
void display_image (IMAGE x);
void save_image (IMAGE x, char *name);
IMAGE get_image (char *name);

/*      Check that a pixel index is in range. Return TRUE(1) if so.     */
int range (IMAGE im, int i, int j)
{
	if ((i<0) || (i>=im->info->nr)) return 0;
	if ((j<0) || (j>=im->info->nc)) return 0;
	return 1;
}

struct image  *newimage (int nr, int nc)
{
	struct image  *x;                /* New image */
	int i;
	unsigned char *p;

	if (nr < 0 || nc < 0) {
		printf ("Error: Bad image size (%d,%d)\n", nr, nc);
		return 0;
	}

/*      Allocate the image structure    */
	x = (struct image  *) malloc( sizeof (struct image) );
	if (!x) {
		printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}

/*      Allocate and initialize the header      */

	x->info = (struct header *)malloc( sizeof(struct header) );
	if (!(x->info)) 
	{
		printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}
	x->info->nr = nr;       x->info->nc = nc;
	x->info->oi = x->info->oj = 0;

/*      Allocate the pixel array        */

	x->data = (unsigned char **)malloc(sizeof(unsigned char *)*nr); 

/* Pointers to rows */
	if (!(x->data)) 
	{
		printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}

	x->data[0] = (unsigned char *)malloc (nr*nc);
	p = x->data[0];
	if (x->data[0]==0)
	  {
		printf ("Out of storage. Newimage  \n");
		exit(1);
	  }

	for (i=1; i<nr; i++) 
	{
	  x->data[i] = (p+i*nc);
	}

	return x;
}

void freeimage (struct image  *z)
{
/*      Free the storage associated with the image Z    */

	if (z != 0) 
	{
	   free (z->data[0]);
	   free (z->info);
	   free (z->data);
	   free (z);
	}
}

void sys_abort (int val, char *mess)
{
	fprintf (stderr, "**** System library ABORT %d: %s ****\n", 
			val, mess);
	exit (2);
}

void copy (IMAGE *a, IMAGE b)
{
	CopyVarImage (a, &b);
}

void CopyVarImage (IMAGE *a, IMAGE *b)
{
	int i,j;

	if (a == b) return;
	if (*a) freeimage (*a);
	*a = newimage ((*b)->info->nr, (*b)->info->nc);
	if (*a == 0) sys_abort (0, "No more storage.\n");

	for (i=0; i<(*b)->info->nr; i++)
	  for (j=0; j< (*b)->info->nc; j++)
	    (*a)->data[i][j] = (*b)->data[i][j];
	(*a)->info->oi = (*b)->info->oi;
	(*a)->info->oj = (*b)->info->oj;
}

float ** f2d (int nr, int nc)
{
	float **x;
	int i;

	x = (float **)calloc ( nr, sizeof (float *) );
	if (x == 0)
	{
	  fprintf (stderr, "Out of storage: F2D.\n");
	  exit (1);
	}

	for (i=0; i<nr; i++)
	{  
	  x[i] = (float *) calloc ( nc, sizeof (float)  );
	  if (x[i] == 0)
	  {
	    fprintf (stderr, "Out of storage: F2D %d.\n", i);
	    exit (1);
	  }
	}
	return x;
}

/* Small system random number generator */
double drand32 ()
{
	static long a=16807L, m=2147483647L,
		    q=127773L, r = 2836L;
	long lo, hi, test;

	hi = seed / q;
	lo = seed % q;
	test = a*lo -r*hi;
	if (test>0) seed = test;
	else seed = test + m;

	return (double)seed/(double)m;
}

void srand32 (long k)
{
	seed = k;
}

IplImage *toOpenCV (IMAGE x)
{
	IplImage *img;
	int i=0, j=0;
	CvScalar s;
	
	img = cvCreateImage(cvSize(x->info->nc, x->info->nr),8, 1);
	for (i=0; i<x->info->nr; i++)
	{
		for (j=0; j<x->info->nc; j++)
		{
			s.val[0] = x->data[i][j];
			cvSet2D (img, i,j,s);
		}
	}
	return img;
}

IMAGE fromOpenCV (IplImage *x)
{
	IMAGE img;
	int color=0, i=0;
	int k=0, j=0;
	CvScalar s;
	
	if ((x->depth==IPL_DEPTH_8U) &&(x->nChannels==1))								// 1 Pixel (grey) image
		img = newimage (x->height, x->width);
	else if ((x->depth==8) && (x->nChannels==3)) //Color
	{
		color = 1;
		img = newimage (x->height, x->width);
	}
	else return 0;

	for (i=0; i<x->height; i++)
	{
		for (j=0; j<x->width; j++)
		{
			s = cvGet2D (x, i, j);
			if (color) 
			  k = (unsigned char)((s.val[0] + s.val[1] + s.val[2])/3);
			else k = (unsigned char)(s.val[0]);
			img->data[i][j] = k;
		}
	}
	return img;
}

/* Display an image on th escreen */
void display_image (IMAGE x)
{
	IplImage *image = 0;
	char wn[20];
	int i;

	image = toOpenCV (x);
	if (image <= 0) return;

	for (i=0; i<19; i++) wn[i] = (char)((drand32()*26) + 'a');
	wn[19] = '\0';
	cvNamedWindow( wn, CV_WINDOW_AUTOSIZE );
	cvShowImage( wn, image );
	cvWaitKey(0);
	cvReleaseImage( &image );
}

void save_image (IMAGE x, char *name)
{
	IplImage *image = 0;

	image = toOpenCV (x);
	if (image <0) return;

	cvSaveImage( name, image );
	cvReleaseImage( &image );
}

IMAGE get_image (char *name)
{
	IMAGE x=0;
	IplImage *image = 0;

	image = cvLoadImage(name, 0);
	if (image <= 0) return 0;
	x = fromOpenCV (image);
	cvReleaseImage( &image );
	return x;
}

void thnz (IMAGE im)
{
	int i,j,k, again=1;
	IMAGE tmp;

	tmp = newimage (im->info->nr, im->info->nc);

/* BLACK = 1, WHITE = 0. */
	for (i=0; i<im->info->nr; i++)
	  for (j=0; j<im->info->nc; j++)
	  {
	    if (im->data[i][j] > 0) im->data[i][j] = 0;
	      else im->data[i][j] = 1;
	    tmp->data[i][j] = 0;
	  }

/* Mark and delete */
	while (again)
	{
	  again = 0;

/* Second sub-iteration */
	for (i=1; i<im->info->nr-1; i++)
	  for (j=1; j<im->info->nc-1; j++)
	  {
	    if (im->data[i][j] != 1) continue;
	    k = nays8(im, i, j);
	    if ((k >= 2 && k <= 6) && Connectivity(im, i,j)==1)
	    {
	      if (im->data[i][j+1]*im->data[i-1][j]*im->data[i][j-1] == 0 &&
		  im->data[i-1][j]*im->data[i+1][j]*im->data[i][j-1] == 0)
		{
		  tmp->data[i][j] = 1;
		  again = 1;
		}
	    }
	  }

	  Delete (im, tmp);
	  if (again == 0) break;

/* First sub-iteration */
	for (i=1; i<im->info->nr-1; i++)
	  for (j=1; j<im->info->nc-1; j++)
	  {
	    if (im->data[i][j] != 1) continue;
	    k = nays8(im, i, j);
	    if ((k >= 2 && k <= 6) && Connectivity(im, i,j)==1)
	    {
	      if (im->data[i-1][j]*im->data[i][j+1]*im->data[i+1][j] == 0 &&
		  im->data[i][j+1]*im->data[i+1][j]*im->data[i][j-1] == 0)
		{
		  tmp->data[i][j] = 1;
		  again = 1;
		}
	    }
	  }

	  Delete (im, tmp);
	}

/* Restore levels */
	for (i=1; i<im->info->nr-1; i++)
	  for (j=1; j<im->info->nc-1; j++)
	  if (im->data[i][j] > 0) im->data[i][j] = 0;
	    else im->data[i][j] = 255;

	freeimage (tmp);
}

void Delete (IMAGE im, IMAGE tmp)
{
	int i,j;

/* Delete pixels that are marked  */
	for (i=1; i<im->info->nr-1; i++)
	  for (j=1; j<im->info->nc-1; j++)
	    if (tmp->data[i][j])
	    {
	        im->data[i][j] = 0;
		tmp->data[i][j] = 0;
	    }
}

int nays8 (IMAGE im, int r, int c)
{
	int i,j,k=0;

	for (i=r-1; i<=r+1; i++)
	  for (j=c-1; j<=c+1; j++)
	    if (i!=r || c!=j)
	      if (im->data[i][j] >= 1) k++;
	return k;
}

int Connectivity (IMAGE im, int r, int c)
{
	int N=0;

	if (im->data[r][c+1]   >= 1 && im->data[r-1][c+1] == 0) N++;
	if (im->data[r-1][c+1] >= 1 && im->data[r-1][c]   == 0) N++;
	if (im->data[r-1][c]   >= 1 && im->data[r-1][c-1] == 0) N++;
	if (im->data[r-1][c-1] >= 1 && im->data[r][c-1]   == 0) N++;
	if (im->data[r][c-1]   >= 1 && im->data[r+1][c-1] == 0) N++;
	if (im->data[r+1][c-1] >= 1 && im->data[r+1][c]   == 0) N++;
	if (im->data[r+1][c]   >= 1 && im->data[r+1][c+1] == 0) N++;
	if (im->data[r+1][c+1] >= 1 && im->data[r][c+1]   == 0) N++;

	return N;
}

double angle_2pt (int r1, int  c1, int  r2, int  c2)
{
/*      Compute the angle between two points. (r1,c1) is the origin
	specified as row, column, and (r2,c2) is the second point.
	Result is between 0-360 degrees, where 0 is horizontal right. */

	double x, dr, dc, conv;

	conv = 180.0/3.1415926535;
	dr = (double)(r2-r1); dc = (double)(c2-c1);

/*      Compute the raw angle based of Drow, Dcolumn            */
	if (dr==0 && dc == 0) x = 0.0;
	else if (dc == 0) x = 90.0;
	else {
	        x = fabs(atan (dr/dc));
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

/* void getnodes ()
{
	FILE *f;

	f = fopen ("c:\\temp\\nodes", "r");

	fscanf (f, "%d", &n);
	for (i=0; i<n; i++)
	{
	  fscanf (f, "%d", &r1);
	  fscanf (f, "%d", &c1);
	  fscanf (f, "%d", &r2);
	  fscanf (f, "%d", &c2);

	  x = newnode (r1, c1, r2, c2);
} */

	void main (int argc, char *argv[])
{
	IMAGE data, im;
	int i,j;
	char filename[128];

	printf ("Thinning. Please enter the input image file name.\n");
	scanf ("%s", filename);

	data = get_image (filename);
	if (data == NULL)
	{
		printf ("Bad input file '%s'\n", filename);
		exit(1);
	}
	display_image (data);
	
	im = newimage (data->info->nr+2, data->info->nc+2);
	for (i=0; i<data->info->nr; i++)
	  for (j=0; j<data->info->nc; j++)
	    im->data[i+1][j+1] = data->data[i][j];
	for (i=0; i<im->info->nr; i++) 
	{
	  im->data[i][0] = 1;
	  im->data[i][im->info->nc-1] = 1;
	}
	for (j=0; j<im->info->nc; j++)
	{
	  im->data[0][j] = 1;
	  im->data[im->info->nr-1][j] = 1;
	}

	thnz (im);

	for (i=0; i<data->info->nr; i++)
           for (j=0; j<data->info->nc; j++)
	      data->data[i][j] = im->data[i+1][j+1];

	display_image (data);
	save_image (data, "martinez.jpg");
}
