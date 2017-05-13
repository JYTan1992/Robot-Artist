
#include "stdafx.h"
int t00, t01, t11, t01s;
long seed = 132531;
using namespace std;
struct image  *newimage (int nr, int nc)
{
	struct image  *x;                /* New image */
	int i;
	unsigned char *p;

	if (nr < 0 || nc < 0) {
		//printf ("Error: Bad image size (%d,%d)\n", nr, nc);
		return 0;
	}

/*      Allocate the image structure    */
	x = (struct image  *) malloc( sizeof (struct image) );
	if (!x) {
		//printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}

/*      Allocate and initialize the header      */

	x->info = (struct header *)malloc( sizeof(struct header) );
	if (!(x->info)) 
	{
		//printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}
	x->info->nr = nr;       x->info->nc = nc;
	x->info->oi = x->info->oj = 0;

/*      Allocate the pixel array        */

	x->data = (unsigned char **)malloc(sizeof(unsigned char *)*nr); 

/* Pointers to rows */
	if (!(x->data)) 
	{
		//printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}

	x->data[0] = (unsigned char *)malloc (nr*nc);
	p = x->data[0];
	if (x->data[0]==0)
	  {
		//printf ("Out of storage. Newimage  \n");
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

/*	Zhang-Suen with Holt's staircase removal */
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

/* Post_process */
        stair (im, tmp, NORTH);
        Delete (im, tmp);
        stair (im, tmp, SOUTH);
        Delete (im, tmp);

/* Restore levels */
	for (i=1; i<im->info->nr-1; i++)
	  for (j=1; j<im->info->nc-1; j++)
	  if (im->data[i][j] > 0) im->data[i][j] = 0;
	    else im->data[i][j] = 255;

	freeimage (tmp);
}

/*	Delete any pixel in IM corresponding to a 1 in TMP	*/
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

/*	Number of neighboring 1 pixels	*/
int nays8 (IMAGE im, int r, int c)
{
	int i,j,k=0;

	for (i=r-1; i<=r+1; i++)
	  for (j=c-1; j<=c+1; j++)
	    if (i!=r || c!=j)
	      if (im->data[i][j] >= 1) k++;
	return k;
}

/*	Number of neighboring 0 pixels	*/
int snays (IMAGE im, int r, int c)
{
        int i,j,k=0;

	for (i=r-1; i<=r+1; i++)
	  for (j=c-1; j<=c+1; j++)
	    if (i!=r || c!=j)
	      if (im->data[i][j] == 0) k++;
	return k;
}

/*	Connectivity by counting black-white transitions on the boundary	*/
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

/*	Stentiford's boundary smoothing method		*/
void pre_smooth (IMAGE im)
{
	int i,j;

	for (i=0; i<im->info->nr; i++)
	  for (j=0; j<im->info->nc; j++)
	    if (im->data[i][j] == 0)
		if (snays(im, i, j) <= 2 && Yokoi (im, i, j)<2)
		  im->data[i][j] = 2;

	for (i=0; i<im->info->nr; i++)
	  for (j=0; j<im->info->nc; j++)
	    if (im->data[i][j] == 2) im->data[i][j] = 1;
}

/*	Stentiford's Acute Angle Emphasis	*/
void aae (IMAGE im)
{
	int i,j, again = 0, k;

	again = 0;
	for (k=5; k>= 1; k-=2)
	{
	  for (i=2; i<im->info->nr-2; i++)
	    for (j=2; j<im->info->nc-2; j++)
	      if (im->data[i][j] == 0)
		match_du (im, i, j, k);

	  for (i=2; i<im->info->nr-2; i++)
	    for (j=2; j<im->info->nc-2; j++)
	    if (im->data[i][j] == 2)
	    {
		again = 1;
		im->data[i][j] = 1;
	    }

	  if (again == 0) break;
	} 
}

/*	Template matches for acute angle emphasis	*/
void match_du (IMAGE im, int r, int c, int k)
{

/* D1 */
	if (im->data[r-2][c-2] == 0 && im->data[r-2][c-1] == 0 &&
	    im->data[r-2][c]   == 1 && im->data[r-2][c+1] == 0 &&
	    im->data[r-2][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 1 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 0 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r+2][c-1] == 0 &&
	    im->data[r+2][c]   == 0 && im->data[r+2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* D2 */
	if (k >= 2)
	if (im->data[r-2][c-2] == 0 && im->data[r-2][c-1] == 1 &&
	    im->data[r-2][c]   == 1 && im->data[r-2][c+1] == 0 &&
	    im->data[r-2][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 1 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 0 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r+2][c-1] == 0 &&
	    im->data[r+2][c]   == 0 && im->data[r+2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* D3 */
	if (k>=3)
	if (im->data[r-2][c-2] == 0 && im->data[r-2][c-1] == 0 &&
	    im->data[r-2][c]   == 1 && im->data[r-2][c+1] == 1 &&
	    im->data[r-2][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 1 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 0 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r+2][c-1] == 0 &&
	    im->data[r+2][c]   == 0 && im->data[r+2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* D4 */
	if (k>=4)
	if (im->data[r-2][c-2] == 0 && im->data[r-2][c-1] == 1 &&
	    im->data[r-2][c]   == 1 && im->data[r-2][c+1] == 0 &&
	    im->data[r-2][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 1 &&
	    im->data[r-1][c]   == 1 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 0 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r+2][c-1] == 0 &&
	    im->data[r+2][c]   == 0 && im->data[r+2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* D5 */
	if (k>=5)
	if (im->data[r-2][c-2] == 0 && im->data[r-2][c-1] == 0 &&
	    im->data[r-2][c]   == 1 && im->data[r-2][c+1] == 1 &&
	    im->data[r-2][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 1 && im->data[r-1][c+1] == 1 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 0 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r+2][c-1] == 0 &&
	    im->data[r+2][c]   == 0 && im->data[r+2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* U1 */
	if (im->data[r+2][c-2] == 0 && im->data[r+2][c-1] == 0 &&
	    im->data[r+2][c]   == 1 && im->data[r+2][c+1] == 0 &&
	    im->data[r+2][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 1 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 0 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 0 && im->data[r-1][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* U2 */
	if (k>=2)
	if (im->data[r+2][c-2] == 0 && im->data[r+2][c-1] == 1 &&
	    im->data[r+2][c]   == 1 && im->data[r+2][c+1] == 0 &&
	    im->data[r+2][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 1 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 0 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r-2][c-1] == 0 &&
	    im->data[r-2][c]   == 0 && im->data[r-2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* U3 */
	if (k>=3)
	if (im->data[r+2][c-2] == 0 && im->data[r+2][c-1] == 0 &&
	    im->data[r+2][c]   == 1 && im->data[r+2][c+1] == 1 &&
	    im->data[r+2][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 1 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 0 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r-2][c-1] == 0 &&
	    im->data[r-2][c]   == 0 && im->data[r-2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* U4 */
	if (k>=4)
	if (im->data[r+2][c-2] == 0 && im->data[r+2][c-1] == 1 &&
	    im->data[r+2][c]   == 1 && im->data[r+2][c+1] == 0 &&
	    im->data[r+2][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 1 &&
	    im->data[r+1][c]   == 1 && im->data[r+1][c+1] == 0 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 0 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r-2][c-1] == 0 &&
	    im->data[r-2][c]   == 0 && im->data[r-2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}

/* U5 */
	if (k>=5)
	if (im->data[r+2][c-2] == 0 && im->data[r+2][c-1] == 0 &&
	    im->data[r+2][c]   == 1 && im->data[r+2][c+1] == 1 &&
	    im->data[r+2][c+2] == 0 &&
	    im->data[r+1][c-2] == 0 && im->data[r+1][c-1] == 0 &&
	    im->data[r+1][c]   == 1 && im->data[r+1][c+1] == 1 &&
	    im->data[r+1][c+2] == 0 &&
	    im->data[r][c-2] == 0 && im->data[r][c-1] == 0 &&
	    im->data[r][c]   == 0 && im->data[r][c+1] == 0 &&
	    im->data[r][c+2] == 0 &&
	    im->data[r-1][c-2] == 0 && im->data[r-1][c-1] == 0 &&
	    im->data[r-1][c]   == 0 && im->data[r-1][c+1] == 0 &&
	    im->data[r-1][c+2] == 0 &&
	    im->data[r-2][c-1] == 0 &&
	    im->data[r-2][c]   == 0 && im->data[r-2][c+1] == 0 )
	{
		im->data[r][c] = 2;
		return;
	}
}

/*	Yokoi's connectivity measure	*/
int Yokoi (IMAGE im, int r, int c)
{
	int N[9];
	int i,k, i1, i2;

	N[0] = im->data[r][c]      != 0;
	N[1] = im->data[r][c+1]    != 0;
	N[2] = im->data[r-1][c+1]  != 0;
	N[3] = im->data[r-1][c]    != 0;
	N[4] = im->data[r-1][c-1]  != 0;
	N[5] = im->data[r][c-1]    != 0;
	N[6] = im->data[r+1][c-1]  != 0;
	N[7] = im->data[r+1][c]    != 0;
	N[8] = im->data[r+1][c+1]  != 0;

	k = 0;
	for (i=1; i<=7; i+=2)
	{
	  i1 = i+1; if (i1 > 8) i1 -= 8;
	  i2 = i+2; if (i2 > 8) i2 -= 8;
	  k += (N[i] - N[i]*N[i1]*N[i2]);
	}

	return k;
}

/*	Holt's staircase removal stuff	*/
void check (int v1, int v2, int v3)
{
	if (!v2 && (!v1 || !v3)) t00 = TRUE;
	if ( v2 && ( v1 ||  v3)) t11 = TRUE;
	if ( (!v1 && v2) || (!v2 && v3) )
	{
		t01s = t01;
		t01  = TRUE;
	}
}

int edge (IMAGE im, int r, int c)
{
	if (im->data[r][c] == 0) return 0;
	t00 = t01 = t01s = t11 = FALSE;

/* CHECK(vNW, vN, vNE) */
	check (im->data[r-1][c-1], im->data[r-1][c], im->data[r-1][c+1]);

/* CHECK (vNE, vE, vSE) */
	check (im->data[r-1][c+1], im->data[r][c+1], im->data[r+1][c+1]);

/* CHECK (vSE, vS, vSW) */
	check (im->data[r+1][c+1], im->data[r+1][c], im->data[r+1][c-1]);

/* CHECK (vSW, vW, vNW) */
	check (im->data[r+1][c-1], im->data[r][c-1], im->data[r-1][c-1]);

	return t00 && t11 && !t01s;
}

void stair (IMAGE im, IMAGE tmp, int dir)
{
	int i,j;
	int N, S, E, W, NE, NW, SE, SW, C;

	if (dir == NORTH)
	for (i=1; i<im->info->nr-1; i++)
	  for (j=1; j<im->info->nc-1; j++)
	  {
	   NW = im->data[i-1][j-1]; N = im->data[i-1][j]; NE = im->data[i-1][j+1];
	   W = im->data[i][j-1]; C = im->data[i][j]; E = im->data[i][j+1];
	   SW = im->data[i+1][j-1]; S = im->data[i+1][j]; SE = im->data[i+1][j+1];

	   if (dir == NORTH)
	   {
	     if (C && !(N && 
		      ((E && !NE && !SW && (!W || !S)) || 
		       (W && !NW && !SE && (!E || !S)) )) )
	       tmp->data[i][j] = 0;		/* Survives */
	     else
	       tmp->data[i][j] = 1;
	   } else if (dir == SOUTH)
	   {
	     if (C && !(S && 
		      ((E && !SE && !NW && (!W || !N)) || 
		       (W && !SW && !NE && (!E || !N)) )) )
	       tmp->data[i][j] = 0;		/* Survives */
	     else
	       tmp->data[i][j] = 1;
	   }
	  }
}

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

/* Pre_process */
	pre_smooth (im);
	aae (im);

/* Thin */
	thnz (im);

	for (i=0; i<data->info->nr; i++)
           for (j=0; j<data->info->nc; j++)
	      data->data[i][j] = im->data[i+1][j+1];

	display_image (data);
	save_image (data, "zsout.jpg");
}
