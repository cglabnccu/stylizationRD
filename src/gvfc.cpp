/* NOTE:
 * 
 * You must have "mex" command working in your matlab.  In matlab, 
 * type "mex gvfc.c" to compile the code. See usage for details of 
 * calling this function.
 *
 * Replace GVF(...) with GVFC in the snakedeform.m. The speed is 
 * significantly faster than the Matlab version.
 */

/***************************************************************************
Copyright (c) 1996-1999 Chenyang Xu and Jerry Prince.

This software is copyrighted by Chenyang Xu and Jerry Prince.  The
following terms apply to all files associated with the software unless
explicitly disclaimed in individual files.

The authors hereby grant permission to use this software and its
documentation for any purpose, provided that existing copyright
notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant
permission to modify this software and its documentation for any
purpose, provided that such modifications are not distributed without
the explicit consent of the authors and that existing copyright
notices are retained in all copies.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
****************************************************************************/

//#include "mex.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gvfc.h"
/* 
 * [u,v] = GVFC(f, mu, ITER)
 * 
 * assuming dx = dy = 1, and dt = 1, 
 * need to figure out CFL
 *
 * Gradient Vector Flow
 *
 * Chenyang Xu and Jerry L. Prince 
 * Copyright (c) 1996-99 by Chenyang Xu and Jerry L. Prince
 * Image Analysis and Communications Lab, Johns Hopkins University
 *
 */

/* PG Macros */
#define PG_MAX(a,b)  (a>b? a: b)
#define PG_MIN(a,b)  (a<b? a: b)

typedef unsigned char  PGbyte;
typedef          char  PGchar;
typedef unsigned short PGushort;
typedef          short PGshort; 
typedef unsigned int   PGuint;
typedef          int   PGint;
typedef float          PGfloat;
typedef double         PGdouble;
typedef void           PGvoid;

/* Prints error message: modified to work in mex environment */
PGvoid pgError(char error_text[])
{
   char buf[255];
   
   sprintf(buf, "GVFC.MEX: %s", error_text);
   //mexErrMsgTxt(buf);
   
   return;
}


/* Allocates a matrix of doubles with range [nrl..nrh][ncl..nch] */
PGdouble **pgDmatrix(int nrl, int nrh, int ncl, int nch)
{
   int j;
   long bufsize,bufptr;
   PGdouble **m;
   
   bufsize = (nrh-nrl+1)*sizeof(PGdouble*)
      + (nrh-nrl+1)*(nch-ncl+1)*sizeof(PGdouble);
   
   m=(PGdouble **) malloc(bufsize);
   if (!m) pgError("allocation failure 1 in pgDmatrix()");
   m -= nrl;
   
   bufptr = ((long) (m+nrl)) + (nrh-nrl+1)*sizeof(PGdouble*);
   for(j=nrl;j<=nrh;j++) {
      m[j] = ((PGdouble *) (bufptr+(j-nrl)*(nch-ncl+1)*sizeof(PGdouble)));
      m[j] -= ncl;
   }
   
   return m;
}


/* Frees a matrix allocated by dmatrix */
PGvoid pgFreeDmatrix(PGdouble **m, int nrl, int nrh, int ncl, int nch)
     //PGdouble **m;
     //int nrl,nrh,ncl,nch;
{
   free((char*) (m+nrl));
   return;
}

inline PGdouble GX(int x, int y, int YN, const double *f)
{
	if(0)
		return 0.5 * (f[y + (x+1)*YN] - f[y + (x-1)*YN]); 
	else {
		PGdouble x1 = 0.25 * (f[y-1 + (x+1)*YN] - f[y-1 + (x-1)*YN]);
		PGdouble x2 = 0.5  * (f[y + (x+1)*YN]   - f[y + (x-1)*YN]);
		PGdouble x3 = 0.25 * (f[y+1 + (x+1)*YN] - f[y+1 + (x-1)*YN]);
		return 0.5*(x1 + x2 + x3);
	}

}

inline PGdouble GY(int x, int y, int YN, const double *f)
{	 
	 if(0)
		 return 0.5 * (f[y+1 + x*YN] - f[y-1 + x*YN]);
	 else {
		PGdouble x1 = 0.25 * (f[y+1 + (x-1)*YN] - f[y-1 + (x-1)*YN]);
		PGdouble x2 = 0.5  * (f[y+1 + x*YN]     - f[y-1 + x*YN]);
		PGdouble x3 = 0.25 * (f[y+1 + (x+1)*YN] - f[y-1 + (x+1)*YN]);

		return 0.5*(x1 + x2 + x3);
	 }
}

void GVFC(int YN, int XN, double *f, double *ou, double *ov, 
	  double mu, int ITER)
{
   double mag2, temp, tempx, tempy, fmax, fmin;
   int count, x, y, XN_1, XN_2, YN_1, YN_2;
   
   PGdouble **fx, **fy, **u, **v, **Lu, **Lv, **g, **c1, **c2, **b;
   
   /* define constants and create row-major double arrays */
   XN_1 = XN - 1;
   XN_2 = XN - 2;
   YN_1 = YN - 1;
   YN_2 = YN - 2;
   fx = pgDmatrix(0,YN_1,0,XN_1);
   fy = pgDmatrix(0,YN_1,0,XN_1);
   u  = pgDmatrix(0,YN_1,0,XN_1);
   v  = pgDmatrix(0,YN_1,0,XN_1);
   Lu = pgDmatrix(0,YN_1,0,XN_1);
   Lv = pgDmatrix(0,YN_1,0,XN_1);
   g  = pgDmatrix(0,YN_1,0,XN_1);
   c1 = pgDmatrix(0,YN_1,0,XN_1);
   c2 = pgDmatrix(0,YN_1,0,XN_1);
   b  = pgDmatrix(0,YN_1,0,XN_1);
   

   /************** I: Normalize the edge map to [0,1] **************/
   fmax = -1e10;
   fmin = 1e10;
   for (x=0; x<=YN*XN-1; x++) {
      fmax = PG_MAX(fmax,f[x]);
      fmin = PG_MIN(fmin,f[x]);
   }

   if (fmax==fmin) 
      pgError("Edge map is a constant image.");      

   for (x=0; x<=YN*XN-1; x++) 
      f[x] = (f[x]-fmin)/(fmax-fmin);

   /**************** II: Compute edge map gradient *****************/
   /* I.1: Neumann boundary condition: 
    *      zero normal derivative at boundary 
    */
   /* Deal with corners */
   fx[0][0] = fy[0][0] = fx[0][XN_1] = fy[0][XN_1] = 0;
   fx[YN_1][XN_1] = fy[YN_1][XN_1] = fx[YN_1][0] = fy[YN_1][0] = 0;

   /* Deal with left and right column */
   for (y=1; y<=YN_2; y++) {
      fx[y][0] = fx[y][XN_1] = 0;
      fy[y][0] = 0.5 * (f[y+1] - f[y-1]);
      fy[y][XN_1] = 0.5 * (f[y+1 + XN_1*YN] - f[y-1 + XN_1*YN]);
   }

   /* Deal with top and bottom row */
   for (x=1; x<=XN_2; x++) {
      fy[0][x] = fy[YN_1][x] = 0;
      fx[0][x] = 0.5 * (f[(x+1)*YN] - f[(x-1)*YN]);
      fx[YN_1][x] = 0.5 * (f[YN_1 + (x+1)*YN] - f[YN_1 + (x-1)*YN]);
   }
   
   /* I.2: Compute interior derivative using central difference */
   for (y=1; y<=YN_2; y++)
      for (x=1; x<=XN_2; x++) {
		/* NOTE: f is stored in column major */
		fx[y][x] = GX(x, y, YN, f);//0.5 * (f[y + (x+1)*YN] - f[y + (x-1)*YN]); 	 
		fy[y][x] = GY(x, y, YN, f);//0.5 * (f[y+1 + x*YN] - f[y-1 + x*YN]);
     }
   
   /******* III: Compute parameters and initializing arrays **********/
   temp = -1.0/(mu*mu);
   for (y=0; y<=YN_1; y++)
      for (x=0; x<=XN_1; x++) {
	 tempx = fx[y][x];
	 tempy = fy[y][x];
	 /* initial GVF vector */
	 u[y][x] = tempx;
	 v[y][x] = tempy;
	 /* gradient magnitude square */
	 mag2 = tempx*tempx + tempy*tempy; 
	 
	 g[y][x] = mu;
	 b[y][x] = mag2;

	 c1[y][x] = b[y][x] * tempx;
	 c2[y][x] = b[y][x] * tempy;
      }
   
   /* free memory of fx and fy */
   pgFreeDmatrix(fx,0,YN_1,0,XN_1);
   pgFreeDmatrix(fy,0,YN_1,0,XN_1);

   /************* Solve GVF = (u,v) iteratively ***************/
   for (count=1; count<=ITER; count++) {
      /* IV: Compute Laplace operator using Neuman condition */
      /* IV.1: Deal with corners */
      Lu[0][0] = (u[0][1] + u[1][0])*0.5 - u[0][0]; 
      Lv[0][0] = (v[0][1] + v[1][0])*0.5 - v[0][0];
      Lu[0][XN_1] = (u[0][XN_2] + u[1][XN_1])*0.5 - u[0][XN_1];
      Lv[0][XN_1] = (v[0][XN_2] + v[1][XN_1])*0.5 - v[0][XN_1];
      Lu[YN_1][0] = (u[YN_1][1] + u[YN_2][0])*0.5 - u[YN_1][0];
      Lv[YN_1][0] = (v[YN_1][1] + v[YN_2][0])*0.5 - v[YN_1][0];
      Lu[YN_1][XN_1] = (u[YN_1][XN_2] + u[YN_2][XN_1])*0.5 - u[YN_1][XN_1];
      Lv[YN_1][XN_1] = (v[YN_1][XN_2] + v[YN_2][XN_1])*0.5 - v[YN_1][XN_1];
      
      /* IV.2: Deal with left and right columns */
      for (y=1; y<=YN_2; y++) { 
	 Lu[y][0] = (2*u[y][1] + u[y-1][0] + u[y+1][0])*0.25 - u[y][0];
	 Lv[y][0] = (2*v[y][1] + v[y-1][0] + v[y+1][0])*0.25 - v[y][0];
	 Lu[y][XN_1] = (2*u[y][XN_2] + u[y-1][XN_1] 
			+ u[y+1][XN_1])*0.25 - u[y][XN_1];
	 Lv[y][XN_1] = (2*v[y][XN_2] + v[y-1][XN_1] 
			+ v[y+1][XN_1])*0.25 - v[y][XN_1];
      }
      
      /* IV.3: Deal with top and bottom rows */
      for (x=1; x<=XN_2; x++) { 
	 Lu[0][x] = (2*u[1][x] + u[0][x-1] + u[0][x+1])*0.25 - u[0][x];
	 Lv[0][x] = (2*v[1][x] + v[0][x-1] + v[0][x+1])*0.25 - v[0][x];
	 Lu[YN_1][x] = (2*u[YN_2][x] + u[YN_1][x-1] 
			+ u[YN_1][x+1])*0.25 - u[YN_1][x];
	 Lv[YN_1][x] = (2*v[YN_2][x] + v[YN_1][x-1] 
			+ v[YN_1][x+1])*0.25 - v[YN_1][x];
      }
      
      /* IV.4: Compute interior */
      for (y=1; y<=YN_2; y++)
	 for (x=1; x<=XN_2; x++) {
	    Lu[y][x] = (u[y][x-1] + u[y][x+1] 
			+ u[y-1][x] + u[y+1][x])*0.25 - u[y][x];
	    Lv[y][x] = (v[y][x-1] + v[y][x+1]
			+ v[y-1][x] + v[y+1][x])*0.25 - v[y][x];
	 }
      
      /******** V: Update GVF ************/
      for (y=0; y<=YN_1; y++)
	 for (x=0; x<=XN_1; x++) {
	    u[y][x] = (1- b[y][x])*u[y][x] + g[y][x]*Lu[y][x]*4 + c1[y][x];
	    v[y][x] = (1- b[y][x])*v[y][x] + g[y][x]*Lv[y][x]*4 + c2[y][x];
	 }
      
      /* print iteration number */
      printf("%5d",count);
      if (count%15 == 0)
	 printf("\n");
   }
   printf("\n");
   
   /* copy u,v to the output in column major order */
	for (y=0; y<=YN_1; y++)
		for (x=0; x<=XN_1; x++) {
			ou[x*YN+y] = u[y][x];
			ov[x*YN+y] = v[y][x];
	}

   /* free all the array memory */
   pgFreeDmatrix(u,0,YN_1,0,XN_1);
   pgFreeDmatrix(v,0,YN_1,0,XN_1);
   pgFreeDmatrix(Lu,0,YN_1,0,XN_1);
   pgFreeDmatrix(Lv,0,YN_1,0,XN_1);
   pgFreeDmatrix(g,0,YN_1,0,XN_1);
   pgFreeDmatrix(c1,0,YN_1,0,XN_1);
   pgFreeDmatrix(c2,0,YN_1,0,XN_1);
   pgFreeDmatrix(b,0,YN_1,0,XN_1);

   return;
}

void GVFC_GUIDE(int YN, int XN, double *f, double *ou, double *ov, 
	  double mu, int ITER, double gvec[2])
{
   double mag2, temp, tempx, tempy, fmax, fmin;
   int count, x, y, XN_1, XN_2, YN_1, YN_2;
   
   PGdouble **fx, **fy, **u, **v, **Lu, **Lv, **g, **c1, **c2, **b;
   
   /* define constants and create row-major double arrays */
   XN_1 = XN - 1;
   XN_2 = XN - 2;
   YN_1 = YN - 1;
   YN_2 = YN - 2;
   fx = pgDmatrix(0,YN_1,0,XN_1);
   fy = pgDmatrix(0,YN_1,0,XN_1);
   u  = pgDmatrix(0,YN_1,0,XN_1);
   v  = pgDmatrix(0,YN_1,0,XN_1);
   Lu = pgDmatrix(0,YN_1,0,XN_1);
   Lv = pgDmatrix(0,YN_1,0,XN_1);
   g  = pgDmatrix(0,YN_1,0,XN_1);
   c1 = pgDmatrix(0,YN_1,0,XN_1);
   c2 = pgDmatrix(0,YN_1,0,XN_1);
   b  = pgDmatrix(0,YN_1,0,XN_1);
   

   /************** I: Normalize the edge map to [0,1] **************/
   fmax = -1e10;
   fmin = 1e10;
   for (x=0; x<=YN*XN-1; x++) {
      fmax = PG_MAX(fmax,f[x]);
      fmin = PG_MIN(fmin,f[x]);
   }

   if (fmax==fmin) 
      pgError("Edge map is a constant image.");      

   for (x=0; x<=YN*XN-1; x++) 
      f[x] = (f[x]-fmin)/(fmax-fmin);

   /**************** II: Compute edge map gradient *****************/
   /* I.1: Neumann boundary condition: 
    *      zero normal derivative at boundary 
    */
   /* Deal with corners */
   fx[0][0] = fy[0][0] = fx[0][XN_1] = fy[0][XN_1] = 0;
   fx[YN_1][XN_1] = fy[YN_1][XN_1] = fx[YN_1][0] = fy[YN_1][0] = 0;

   /* Deal with left and right column */
   for (y=1; y<=YN_2; y++) {
      fx[y][0] = fx[y][XN_1] = 0;
      fy[y][0] = 0.5 * (f[y+1] - f[y-1]);
      fy[y][XN_1] = 0.5 * (f[y+1 + XN_1*YN] - f[y-1 + XN_1*YN]);
   }

   /* Deal with top and bottom row */
   for (x=1; x<=XN_2; x++) {
      fy[0][x] = fy[YN_1][x] = 0;
      fx[0][x] = 0.5 * (f[(x+1)*YN] - f[(x-1)*YN]);
      fx[YN_1][x] = 0.5 * (f[YN_1 + (x+1)*YN] - f[YN_1 + (x-1)*YN]);
   }
   
	/* I.2: Compute interior derivative using central difference */
	for (y=1; y<=YN_2; y++)
		for (x=1; x<=XN_2; x++) {
			/* NOTE: f is stored in column major */
			//fx[y][x] = 0.5 * (f[y + (x+1)*YN] - f[y + (x-1)*YN]); 	 
			//fy[y][x] = 0.5 * (f[y+1 + x*YN] - f[y-1 + x*YN]);

			PGdouble tempd[2];
			tempd[0] = GX(x, y, YN, f);//0.5 * (f[y + (x+1)*YN] - f[y + (x-1)*YN]); 	 
			tempd[1] = GY(x, y, YN, f);//0.5 * (f[y+1 + x*YN] - f[y-1 + x*YN]);

			//rotate
			PGdouble tempr[2] = {tempd[1], -tempd[0]};
			PGdouble dis = sqrt(tempr[0]*tempr[0] + tempr[1]*tempr[1]);		
			//if (dis == 0.0) {
			//	fx[y][x] = 0;
			//	fy[y][x] = 0;
			//	continue;
			//}
			tempr[0] /= dis;
			tempr[1] /= dis;
			PGdouble dotvalue = tempr[0]*gvec[0] + tempr[1]*gvec[1];

			if(dotvalue > 0 ) {
				fx[y][x] = tempd[1];
				fy[y][x] = -tempd[0];
			} else {
				fx[y][x] = -tempd[1];
				fy[y][x] = tempd[0];
			}
				//fx[y][x] = 1;
				//fy[y][x] = 0;
		}
   
   /******* III: Compute parameters and initializing arrays **********/
   temp = -1.0/(mu*mu);
   for (y=0; y<=YN_1; y++)
      for (x=0; x<=XN_1; x++) {
		tempx = fx[y][x];
		tempy = fy[y][x];
		/* initial GVF vector */
		u[y][x] = tempx;
		v[y][x] = tempy;
		/* gradient magnitude square */
		mag2 = tempx*tempx + tempy*tempy; 
		 
		g[y][x] = mu;
		b[y][x] = mag2;

		c1[y][x] = b[y][x] * tempx;
		c2[y][x] = b[y][x] * tempy;
      }
   
   /* free memory of fx and fy */
   pgFreeDmatrix(fx,0,YN_1,0,XN_1);
   pgFreeDmatrix(fy,0,YN_1,0,XN_1);

   /************* Solve GVF = (u,v) iteratively ***************/
   for (count=1; count<=ITER; count++) {
      /* IV: Compute Laplace operator using Neuman condition */
      /* IV.1: Deal with corners */
      Lu[0][0] = (u[0][1] + u[1][0])*0.5 - u[0][0]; 
      Lv[0][0] = (v[0][1] + v[1][0])*0.5 - v[0][0];
      Lu[0][XN_1] = (u[0][XN_2] + u[1][XN_1])*0.5 - u[0][XN_1];
      Lv[0][XN_1] = (v[0][XN_2] + v[1][XN_1])*0.5 - v[0][XN_1];
      Lu[YN_1][0] = (u[YN_1][1] + u[YN_2][0])*0.5 - u[YN_1][0];
      Lv[YN_1][0] = (v[YN_1][1] + v[YN_2][0])*0.5 - v[YN_1][0];
      Lu[YN_1][XN_1] = (u[YN_1][XN_2] + u[YN_2][XN_1])*0.5 - u[YN_1][XN_1];
      Lv[YN_1][XN_1] = (v[YN_1][XN_2] + v[YN_2][XN_1])*0.5 - v[YN_1][XN_1];
      
      /* IV.2: Deal with left and right columns */
      for (y=1; y<=YN_2; y++) { 
		Lu[y][0] = (2*u[y][1] + u[y-1][0] + u[y+1][0])*0.25 - u[y][0];
		Lv[y][0] = (2*v[y][1] + v[y-1][0] + v[y+1][0])*0.25 - v[y][0];
		Lu[y][XN_1] = (2*u[y][XN_2] + u[y-1][XN_1] 
				+ u[y+1][XN_1])*0.25 - u[y][XN_1];
		Lv[y][XN_1] = (2*v[y][XN_2] + v[y-1][XN_1] 
				+ v[y+1][XN_1])*0.25 - v[y][XN_1];
      }
      
      /* IV.3: Deal with top and bottom rows */
      for (x=1; x<=XN_2; x++) { 
		Lu[0][x] = (2*u[1][x] + u[0][x-1] + u[0][x+1])*0.25 - u[0][x];
		Lv[0][x] = (2*v[1][x] + v[0][x-1] + v[0][x+1])*0.25 - v[0][x];
		Lu[YN_1][x] = (2*u[YN_2][x] + u[YN_1][x-1] 
				+ u[YN_1][x+1])*0.25 - u[YN_1][x];
		Lv[YN_1][x] = (2*v[YN_2][x] + v[YN_1][x-1] 
				+ v[YN_1][x+1])*0.25 - v[YN_1][x];
      }
      
      /* IV.4: Compute interior */
      for (y=1; y<=YN_2; y++)
		for (x=1; x<=XN_2; x++) {
			Lu[y][x] = (u[y][x-1] + u[y][x+1] 
				+ u[y-1][x] + u[y+1][x])*0.25 - u[y][x];
			Lv[y][x] = (v[y][x-1] + v[y][x+1]
				+ v[y-1][x] + v[y+1][x])*0.25 - v[y][x];
		}
      
      /******** V: Update GVF ************/
      for (y=0; y<=YN_1; y++)
		for (x=0; x<=XN_1; x++) {
			u[y][x] = (1- b[y][x])*u[y][x] + g[y][x]*Lu[y][x]*4 + c1[y][x];
			v[y][x] = (1- b[y][x])*v[y][x] + g[y][x]*Lv[y][x]*4 + c2[y][x];
		}
      
      /* print iteration number */
      printf("%5d",count);
      if (count%15 == 0)
	 printf("\n");
   }
   printf("\n");
   
   /* copy u,v to the output in column major order */
	for (y=0; y<=YN_1; y++)
		for (x=0; x<=XN_1; x++) {
			ou[x*YN+y] = u[y][x];
			ov[x*YN+y] = v[y][x];
	}

   /* free all the array memory */
   pgFreeDmatrix(u,0,YN_1,0,XN_1);
   pgFreeDmatrix(v,0,YN_1,0,XN_1);
   pgFreeDmatrix(Lu,0,YN_1,0,XN_1);
   pgFreeDmatrix(Lv,0,YN_1,0,XN_1);
   pgFreeDmatrix(g,0,YN_1,0,XN_1);
   pgFreeDmatrix(c1,0,YN_1,0,XN_1);
   pgFreeDmatrix(c2,0,YN_1,0,XN_1);
   pgFreeDmatrix(b,0,YN_1,0,XN_1);

   return;
}

//spec:  0: no, 1:turn right, 2: turn left
void GVFC_SPECIFIC(int YN, int XN, double *f, double *ou, double *ov, 
	  double mu, int ITER, int *spec)
{
   double mag2, temp, tempx, tempy, fmax, fmin;
   int count, x, y, XN_1, XN_2, YN_1, YN_2;
   
   PGdouble **fx, **fy, **u, **v, **Lu, **Lv, **g, **c1, **c2, **b;
   
   /* define constants and create row-major double arrays */
   XN_1 = XN - 1;
   XN_2 = XN - 2;
   YN_1 = YN - 1;
   YN_2 = YN - 2;
   fx = pgDmatrix(0,YN_1,0,XN_1);
   fy = pgDmatrix(0,YN_1,0,XN_1);
   u  = pgDmatrix(0,YN_1,0,XN_1);
   v  = pgDmatrix(0,YN_1,0,XN_1);
   Lu = pgDmatrix(0,YN_1,0,XN_1);
   Lv = pgDmatrix(0,YN_1,0,XN_1);
   g  = pgDmatrix(0,YN_1,0,XN_1);
   c1 = pgDmatrix(0,YN_1,0,XN_1);
   c2 = pgDmatrix(0,YN_1,0,XN_1);
   b  = pgDmatrix(0,YN_1,0,XN_1);
   

   /************** I: Normalize the edge map to [0,1] **************/
   fmax = -1e10;
   fmin = 1e10;
   for (x=0; x<=YN*XN-1; x++) {
      fmax = PG_MAX(fmax,f[x]);
      fmin = PG_MIN(fmin,f[x]);
   }

   if (fmax==fmin) 
      pgError("Edge map is a constant image.");      

   for (x=0; x<=YN*XN-1; x++) 
      f[x] = (f[x]-fmin)/(fmax-fmin);

   /**************** II: Compute edge map gradient *****************/
   /* I.1: Neumann boundary condition: 
    *      zero normal derivative at boundary 
    */
   /* Deal with corners */
   fx[0][0] = fy[0][0] = fx[0][XN_1] = fy[0][XN_1] = 0;
   fx[YN_1][XN_1] = fy[YN_1][XN_1] = fx[YN_1][0] = fy[YN_1][0] = 0;

   /* Deal with left and right column */
   for (y=1; y<=YN_2; y++) {
      fx[y][0] = fx[y][XN_1] = 0;
      fy[y][0] = 0.5 * (f[y+1] - f[y-1]);
      fy[y][XN_1] = 0.5 * (f[y+1 + XN_1*YN] - f[y-1 + XN_1*YN]);
   }

   /* Deal with top and bottom row */
   for (x=1; x<=XN_2; x++) {
      fy[0][x] = fy[YN_1][x] = 0;
      fx[0][x] = 0.5 * (f[(x+1)*YN] - f[(x-1)*YN]);
      fx[YN_1][x] = 0.5 * (f[YN_1 + (x+1)*YN] - f[YN_1 + (x-1)*YN]);
   }
   
	/* I.2: Compute interior derivative using central difference */
	for (y=1; y<=YN_2; y++)
		for (x=1; x<=XN_2; x++) {
			/* NOTE: f is stored in column major */
			//fx[y][x] = 0.5 * (f[y + (x+1)*YN] - f[y + (x-1)*YN]); 	 
			//fy[y][x] = 0.5 * (f[y+1 + x*YN] - f[y-1 + x*YN]);

			PGdouble tempd[2];
			tempd[0] = GX(x, y, YN, f);//0.5 * (f[y + (x+1)*YN] - f[y + (x-1)*YN]); 	 
			tempd[1] = GY(x, y, YN, f);//0.5 * (f[y+1 + x*YN] - f[y-1 + x*YN]);


			switch(spec[y + x*YN]) {	//spec[y + x*YN]
				case 0:
					fx[y][x] = 0;
					fy[y][x] = 0;
					break;

				case 1:
					fx[y][x] = -tempd[1];
					fy[y][x] = tempd[0];
					break;
				
				case 2:
					fx[y][x] = tempd[1];
					fy[y][x] = -tempd[0];
					break;

				case 3:
					fx[y][x] = tempd[0];
					fy[y][x] = tempd[1];
					break;

				default:
					fx[y][x] = 0;
					fy[y][x] = 0;
					break;
			}
			
		}
   
   /******* III: Compute parameters and initializing arrays **********/
   temp = -1.0/(mu*mu);
   for (y=0; y<=YN_1; y++)
      for (x=0; x<=XN_1; x++) {
		tempx = fx[y][x];
		tempy = fy[y][x];
		/* initial GVF vector */
		u[y][x] = tempx;
		v[y][x] = tempy;
		/* gradient magnitude square */
		mag2 = tempx*tempx + tempy*tempy; 
		 
		g[y][x] = mu;
		b[y][x] = mag2;

		c1[y][x] = b[y][x] * tempx;
		c2[y][x] = b[y][x] * tempy;
      }
   
   /* free memory of fx and fy */
   pgFreeDmatrix(fx,0,YN_1,0,XN_1);
   pgFreeDmatrix(fy,0,YN_1,0,XN_1);

   /************* Solve GVF = (u,v) iteratively ***************/
   for (count=1; count<=ITER; count++) {
      /* IV: Compute Laplace operator using Neuman condition */
      /* IV.1: Deal with corners */
      Lu[0][0] = (u[0][1] + u[1][0])*0.5 - u[0][0]; 
      Lv[0][0] = (v[0][1] + v[1][0])*0.5 - v[0][0];
      Lu[0][XN_1] = (u[0][XN_2] + u[1][XN_1])*0.5 - u[0][XN_1];
      Lv[0][XN_1] = (v[0][XN_2] + v[1][XN_1])*0.5 - v[0][XN_1];
      Lu[YN_1][0] = (u[YN_1][1] + u[YN_2][0])*0.5 - u[YN_1][0];
      Lv[YN_1][0] = (v[YN_1][1] + v[YN_2][0])*0.5 - v[YN_1][0];
      Lu[YN_1][XN_1] = (u[YN_1][XN_2] + u[YN_2][XN_1])*0.5 - u[YN_1][XN_1];
      Lv[YN_1][XN_1] = (v[YN_1][XN_2] + v[YN_2][XN_1])*0.5 - v[YN_1][XN_1];
      
      /* IV.2: Deal with left and right columns */
      for (y=1; y<=YN_2; y++) { 
		Lu[y][0] = (2*u[y][1] + u[y-1][0] + u[y+1][0])*0.25 - u[y][0];
		Lv[y][0] = (2*v[y][1] + v[y-1][0] + v[y+1][0])*0.25 - v[y][0];
		Lu[y][XN_1] = (2*u[y][XN_2] + u[y-1][XN_1] 
				+ u[y+1][XN_1])*0.25 - u[y][XN_1];
		Lv[y][XN_1] = (2*v[y][XN_2] + v[y-1][XN_1] 
				+ v[y+1][XN_1])*0.25 - v[y][XN_1];
      }
      
      /* IV.3: Deal with top and bottom rows */
      for (x=1; x<=XN_2; x++) { 
		Lu[0][x] = (2*u[1][x] + u[0][x-1] + u[0][x+1])*0.25 - u[0][x];
		Lv[0][x] = (2*v[1][x] + v[0][x-1] + v[0][x+1])*0.25 - v[0][x];
		Lu[YN_1][x] = (2*u[YN_2][x] + u[YN_1][x-1] 
				+ u[YN_1][x+1])*0.25 - u[YN_1][x];
		Lv[YN_1][x] = (2*v[YN_2][x] + v[YN_1][x-1] 
				+ v[YN_1][x+1])*0.25 - v[YN_1][x];
      }
      
      /* IV.4: Compute interior */
      for (y=1; y<=YN_2; y++)
		for (x=1; x<=XN_2; x++) {
			Lu[y][x] = (u[y][x-1] + u[y][x+1] 
				+ u[y-1][x] + u[y+1][x])*0.25 - u[y][x];
			Lv[y][x] = (v[y][x-1] + v[y][x+1]
				+ v[y-1][x] + v[y+1][x])*0.25 - v[y][x];
		}
      
      /******** V: Update GVF ************/
      for (y=0; y<=YN_1; y++)
		for (x=0; x<=XN_1; x++) {
			u[y][x] = (1- b[y][x])*u[y][x] + g[y][x]*Lu[y][x]*4 + c1[y][x];
			v[y][x] = (1- b[y][x])*v[y][x] + g[y][x]*Lv[y][x]*4 + c2[y][x];
		}
      
      /* print iteration number */
      printf("%5d",count);
      if (count%15 == 0)
	 printf("\n");
   }
   printf("\n");
   
   /* copy u,v to the output in column major order */
	for (y=0; y<=YN_1; y++)
		for (x=0; x<=XN_1; x++) {
			ou[x*YN+y] = u[y][x];
			ov[x*YN+y] = v[y][x];
	}

   /* free all the array memory */
   pgFreeDmatrix(u,0,YN_1,0,XN_1);
   pgFreeDmatrix(v,0,YN_1,0,XN_1);
   pgFreeDmatrix(Lu,0,YN_1,0,XN_1);
   pgFreeDmatrix(Lv,0,YN_1,0,XN_1);
   pgFreeDmatrix(g,0,YN_1,0,XN_1);
   pgFreeDmatrix(c1,0,YN_1,0,XN_1);
   pgFreeDmatrix(c2,0,YN_1,0,XN_1);
   pgFreeDmatrix(b,0,YN_1,0,XN_1);

   return;
}


/*
void test( int nlhs, double *plhs[], 
		 int nrhs, const double *prhs[] )
		 //( int nlhs, mxArray *plhs[], 
		 //int nrhs, const mxArray *prhs[] )
{
   double  *f, *u, *v;
   int mrows, ncols;
   int m, n;
   double mu;
   int ITER;
   char buf[80];

   //mexUnlock();

   // Check for proper number of arguments 
   if ( nrhs < 1 || nrhs > 3) 
      pgError("GVFC.MEX: wrong number of input arguments.");
   if ( nlhs > 2 )
      pgError("GVFC.MEX: wrong number of output arguments.");
   
   // Obtain the dimension information of inputs
   mrows = mxGetM(prhs[0]);   ncols = mxGetN(prhs[0]);

   // Create matrix for the return argument
   plhs[0] = mxCreateDoubleMatrix(mrows, ncols, mxREAL);
   plhs[1] = mxCreateDoubleMatrix(mrows, ncols, mxREAL);
   
   // Assign pointers to each input and output
   f = (double *)mxGetPr(prhs[0]); 
   u = (double *)mxGetPr(plhs[0]);
   v = (double *)mxGetPr(plhs[1]);
   
   if (nrhs >= 2 && nrhs <= 3) {
      // The input must be a noncomplex scalar double.
      m = mxGetM(prhs[1]);
      n = mxGetN(prhs[1]);
      if( !mxIsDouble(prhs[1]) || mxIsComplex(prhs[1]) ||
	 !(m==1 && n==1) ) {
	 mexErrMsgTxt("Input mu must be a noncomplex scalar double.");
      }
      mu = mxGetScalar(prhs[1]);
   }

   if (nrhs == 3) {
      // The input must be an integer.
      m = mxGetM(prhs[2]);
      n = mxGetN(prhs[2]);
      if( !mxIsDouble(prhs[2]) || mxIsComplex(prhs[2]) ||
	 !(m==1 && n==1) ) {
	 mexErrMsgTxt("Input mu must be an integer.");
      }
      ITER = mxGetScalar(prhs[2]);
   }


   // set default mu and ITER 
   if (nrhs <= 1) 
	 mu = 0.2;
   if (nrhs <= 2)
      ITER = PG_MAX(mrows, ncols);
      
   // Call the distedt subroutine. 
   GVFC(mrows, ncols, f, u, v, mu, ITER);

   // done 
   return;
}
*/

