#ifndef _GVFC_H_
#define _GVFC_H_

//% Read in the 64x64 U-shape image
//  [I,map] = rawread('../images/U64.pgm');  
//  
//% Compute its edge map
//  disp(' Compute edge map ...');
//  f = 1 - I/255; 

//% Compute the GVF of the edge map f
//  disp(' Compute GVF ...');
//  [u,v] = GVF(f, 0.2, 80); 
//  disp(' Nomalizing the GVF external force ...');
//  mag = sqrt(u.*u+v.*v);
//  px = u./(mag+1e-10); py = v./(mag+1e-10); 

void GVFC(int YN, int XN, double *f, double *ou, double *ov, 
	  double mu, int ITER);

void GVFC_GUIDE(int YN, int XN, double *f, double *ou, double *ov, 
	  double mu, int ITER, double gvec[2]);

void GVFC_SPECIFIC(int YN, int XN, double *f, double *ou, double *ov, 
	  double mu, int ITER, int *spec);

#endif
