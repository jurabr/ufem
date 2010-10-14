/*
   File name: fem_e014.c
   Date:      2005/06/03 18:10
   Author:    Jiri Brozovsky

   Copyright (C) 2005 Jiri Brozovsky

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.
  
   FEM solver: 3D beam
*/

#include "fem_elem.h"


#ifndef _SMALL_FEM_CODE_

int e014_local_stiff_matrix(long ePos, 
                            long Mode, 
                            double Lx, 
                            double Ex, 
                            double Gx, 
                            double Ax, 
                            double Ix, 
                            double Iy, 
                            double Iz, 
                            tMatrix *k_0)
{
  int    rv = AF_OK ;
  long   i,j ;
  
  femMatSetZero(k_0);

  /* X - torsion: */
  femMatPut(k_0, 4,4, ( (Gx*Ix) / Lx ) ) ;
  femMatPut(k_0, 10,10, ( (Gx*Ix) / Lx ) ) ;

  femMatPut(k_0, 4,10, ( -(Gx*Ix) / Lx ) ) ;
  femMatPut(k_0, 10,4, ( -(Gx*Ix) / Lx ) ) ;

  /* X - normal: */
  femMatPut(k_0, 1,1, ( (Ex*Ax) / Lx ) ) ;
  femMatPut(k_0, 7,7, ( (Ex*Ax) / Lx ) ) ;

  femMatPut(k_0, 1,7, ( -(Ex*Ax) / Lx ) ) ;
  femMatPut(k_0, 1,7, ( -(Ex*Ax) / Lx ) ) ;

  /* Y - bending */
	femMatPut(k_0, 2, 2,  (12.0 * Ex*Iy) / (Lx*Lx*Lx)  );
	femMatPut(k_0, 2, 6,  (-6.0 * Ex*Iy) / (Lx*Lx)  );
	femMatPut(k_0, 2, 8,  (-12.0 * Ex*Iy) / (Lx*Lx*Lx) );
	femMatPut(k_0, 2, 12,  (-6.0 * Ex*Iy) / (Lx*Lx)  );

	femMatPut(k_0, 6, 6,  (4.0 * Ex*Iy) / Lx       );
	femMatPut(k_0, 6, 8,  (6.0 * Ex*Iy) / (Lx*Lx)     );
	femMatPut(k_0, 6, 12,  (2.0 * Ex*Iy) / Lx   );

	femMatPut(k_0, 8, 8,  (12.0 * Ex*Iy) / (Lx*Lx*Lx)  );
	femMatPut(k_0, 8, 12,  (6.0 * Ex*Iy) / (Lx*Lx) );

	femMatPut(k_0, 12, 12,  (4.0 * Ex*Iy) / Lx       );

  /* Z - bending */
	femMatPut(k_0, 3, 3,  (12.0 * Ex*Iz) / (Lx*Lx*Lx)  );
	femMatPut(k_0, 3, 5,  (-6.0 * Ex*Iz) / (Lx*Lx)  );
	femMatPut(k_0, 3, 9,  (-12.0 * Ex*Iz) / (Lx*Lx*Lx) );
	femMatPut(k_0, 3, 11, (-6.0 * Ex*Iz) / (Lx*Lx)  );

	femMatPut(k_0, 5, 5,  (4.0 * Ex*Iz) / Lx       );
	femMatPut(k_0, 5, 9,  (6.0 * Ex*Iz) / (Lx*Lx)     );
	femMatPut(k_0, 5, 11, (2.0 * Ex*Iz) / Lx   );

	femMatPut(k_0, 9, 9,  (12.0 * Ex*Iz) / (Lx*Lx*Lx)  );
	femMatPut(k_0, 9, 11, (6.0 * Ex*Iz) / (Lx*Lx) );

	femMatPut(k_0, 11, 11, (4.0 * Ex*Iz) / Lx       );
 
  for (i=2; i<=12; i++)
  {
    for (j=1; j<=i; j++)
    {
      femMatPut( k_0, i, j, femMatGet(k_0, j, i) ) ;
    }
  }



  femMatPrn(k_0,"K0");

  return(rv);
}

#if 0
int e014_T_mat2(
    double xa, 
    double ya, 
    double za, 
    double xb, 
    double yb, 
    double zb, 
    tMatrix *T, 
    tMatrix *TT)
{
  long i, j ;
  double a[4],b[4],c[4]  ;
  double xc, yc, zc ;
  double xba, xca ;
  double yba, yca ;
  double zba, zca ;
  double A, B, C, D ;
  double L ;

  L = sqrt ( pow(xa-xb,2) + pow(ya-yb,2) + pow(za-zb,2) ) ;

  xba = xb - xa ;
  yba = yb - ya ;
  zba = zb - za ;

  /* "c" point: */
  if (fabs(xa-xb) < FEM_ZERO)
  {
    /* collumn in x-y plane */
    xc = xa - L ;
    yc = ya ;
    zc = za ;
  }
  else
  {
    /* normal */
    xc = 0.5*(xa+xb) ;
    yc = 0.5*(ya+yb)-L ;
    zc = 0.5*(za+zb) ;
  }

  xca = xc - xa ;
  yca = yc - ya ;
  zca = zc - za ;

  A = yca * zba - yba * zca ;
  B = xba * zca - xca * zba ;
  C = xca * yba - xba * yca ;

  D = sqrt ( A*A + B*B + C*C ) ;

  a[1] = xba / L ;
  b[1] = yba / L ;
  c[1] = zba / L ;

  a[2] = A / D ;
  b[2] = B / D ;
  c[2] = C / D ;

  a[3] = b[1]*c[2] - b[2]*c[1] ;
  b[3] = a[2]*c[1] - a[1]*c[2] ;
  c[3] = a[1]*b[2] - b[1]*a[2] ;

  for (i=0; i<4; i++)
  {
    for (j=1; j<=3; j++)
    {
      femMatPut(T, 1+i*3, j+i*3,  a[j] ) ;
      femMatPut(T, 2+i*3, j+i*3,  b[j] ) ;
      femMatPut(T, 3+i*3, j+i*3,  c[j] ) ;
    }
  }

	femMatTran(T, TT) ;

  return(AF_OK);
}
#else
int e014_T_mat2(
    double xa, 
    double ya, 
    double za, 
    double xb, 
    double yb, 
    double zb, 
    tMatrix *T, 
    tMatrix *TT)
{
  tMatrix Ta,Tb,Tc ;
  double  am, a1, cd, sd, cs, ss;
  long    i, j, k, m, n ;
  int     rv = AF_OK ;

  femMatNull(&Ta);
  femMatNull(&Tb);
  femMatNull(&Tc);

  if (femFullMatInit(&Ta, 3,3) != AF_OK){rv = AF_ERR_MEM; goto memFree;}
  if (femFullMatInit(&Tb, 3,3) != AF_OK){rv = AF_ERR_MEM; goto memFree;}
  if (femFullMatInit(&Tc, 3,3) != AF_OK){rv = AF_ERR_MEM; goto memFree;}

  femMatSetZero(T);
  femMatSetZero(TT);


  am = sqrt (pow(xb-xa,2)+pow(yb-ya,2)+pow(zb-za,2)) ;
  a1 = sqrt (pow(xb-xa,2)+pow(yb-ya,2));

  if (fabs(a1) > FEM_ZERO)
  {
    cd = (xb-xa) / a1 ;
    sd = (yb-ya) / a1 ;
  }
  else
  {
    cd = 1.0 ;
    sd = 0.0 ;
  }

  femMatPut(&Ta, 1,1,  cd );
  femMatPut(&Ta, 2,1, -sd );
  femMatPut(&Ta, 1,2,  sd );
  femMatPut(&Ta, 2,2,  cd );
  femMatPut(&Ta, 3,3, 1.0 );

  cs = a1 / am ;
  ss = (zb-za) / am ;

  femMatPut(&Tb, 1,1,  cs );
  femMatPut(&Tb, 1,3,  ss );
  femMatPut(&Tb, 2,2, 1.0 );
  femMatPut(&Tb, 3,1, -ss );
  femMatPut(&Tb, 3,3,  cs );

  femMatMatMult(&Tb, &Ta, &Tc) ;
  femMatSetZero(&Ta);

  femMatPut(&Ta, 1,1,  1.0 );
  femMatPut(&Ta, 2,2,   cd );
  femMatPut(&Ta, 2,3,   sd );
  femMatPut(&Ta, 3,2,  -sd );
  femMatPut(&Ta, 3,3,   cd );

  femMatSetZero(&Tb);
  femMatMatMult(&Ta, &Tc, &Tb) ;

  for (i=1;i<=4; i++)
  {
    for (j=1; j<=3; j++)
    {
      m = (i-1)*3+j;
      for (k=1; k<=3; k++)
      {
        n = (i-1)*3+k ;
        femMatPut(T, m,n, femMatGet(&Tb, j, k)) ;
      }
    }
  }

	femMatTran(T, TT) ;

memFree:
  femMatFree(&Ta);
  femMatFree(&Tb);
  femMatFree(&Tc);
  return(rv);
}
#endif

int e014_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
	int     rv = AF_OK ;
  double  Lx,Ex,Gx,Ax,pr,Ix,Iy,Iz ;
  double  x1,y1,z1,x2,y2,z2;
  long    klx1,kly1,klz1,klx2,kly2,klz2 ;
  tMatrix k_0 ;
  tMatrix T ;
  tMatrix TT ;
  tMatrix Tk_0 ;
  tVector u_e ;
  tVector F_0 ;

	femMatNull(&k_0);
	femMatNull(&T);
	femMatNull(&TT);
	femMatNull(&Tk_0);

	if ((rv=femFullMatInit(&k_0,12,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&Tk_0,12,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&T,12,12)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&TT,12,12)) != AF_OK) { goto memFree; }

	/* geometric properties: */
	Ax   = femGetRSValPos(ePos, RS_AREA, 0) ;
	Ix   = femGetRSValPos(ePos, RS_INERTIA_X, 0) ;
	Iy   = femGetRSValPos(ePos, RS_INERTIA_Y, 0) ;
	Iz   = femGetRSValPos(ePos, RS_INERTIA_Z, 0) ;

	klx1 = femGetRSValPos(ePos, RS_STF_X_1, 0) ;
	kly1 = femGetRSValPos(ePos, RS_STF_Y_1, 0) ;
	klz1 = femGetRSValPos(ePos, RS_STF_Z_1, 0) ;

	klx2 = femGetRSValPos(ePos, RS_STF_X_2, 0) ;
	kly2 = femGetRSValPos(ePos, RS_STF_Y_2, 0) ;
	klz2 = femGetRSValPos(ePos, RS_STF_Z_2, 0) ;

  /* material properties: */
	Ex = femGetMPValPos(ePos, MAT_EX,  0) ;
	pr = femGetMPValPos(ePos, MAT_NU,  0) ;
	Gx = Ex / (2.0 * (1.0 + pr)) ;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
  z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));

  Lx = sqrt( pow(x1-x2,2) + pow(y1-y2,2) + pow(z1-z2,2) );

	if (Lx <= 0 )
	{
#ifdef RUN_VERBOSE
		fprintf(msgout, "[E] %s!\n", _("Zero element lenght"));
#endif
		rv = AF_ERR_VAL;
    goto memFree ;
	}

  e014_local_stiff_matrix(ePos, Mode, 
                            Lx, Ex, Gx, Ax, 
                            Ix, Iy, Iz, 
                            &k_0);

  femMatPrn(&k_0,"K_0");

  if ((rv=e014_T_mat2(x1, y1, z1, x2, y2, z2, &T, &TT)) != AF_OK)
  {
    goto memFree;
  }

  femMatPrn(&T,"T");

	femMatMatMult(&TT, &k_0, &Tk_0);
	femMatMatMult(&Tk_0, &T, K_e);

  femMatPrn(K_e,"K_e");

	/* ====================================================================== */
	if (Mode == AF_YES) /* result, F_e, Fr_e */
  {
    femVecNull(&F_0);
		femVecFullInit(&F_0,12);
		femVecNull(&u_e);
		femVecFullInit(&u_e,12);

    femLocUtoU_e(&u, ePos, &u_e);
		femMatVecMult(K_e, &u_e, F_e) ; /* global */

    femMatVecMult(&T, F_e, &F_0) ; /* local */

		femVecSetZero(Fr_e); /* residual forces ;-) */

    if (femTangentMatrix == AF_YES)
	  {
	    femAddEResVal(ePos, RES_FX, 0, femVecGet(&F_0,1));
	    femAddEResVal(ePos, RES_FY, 0, femVecGet(&F_0,2));
	    femAddEResVal(ePos, RES_FZ, 0, femVecGet(&F_0,3));
	    femAddEResVal(ePos, RES_MX, 0, femVecGet(&F_0,4));
	    femAddEResVal(ePos, RES_MY, 0, femVecGet(&F_0,5));
	    femAddEResVal(ePos, RES_MZ, 0, femVecGet(&F_0,6));

	    femAddEResVal(ePos, RES_FX, 1, femVecGet(&F_0,7));
	    femAddEResVal(ePos, RES_FY, 1, femVecGet(&F_0,8));
	    femAddEResVal(ePos, RES_FZ, 1, femVecGet(&F_0,9));
	    femAddEResVal(ePos, RES_MX, 1, femVecGet(&F_0,10));
	    femAddEResVal(ePos, RES_MY, 1, femVecGet(&F_0,11));
	    femAddEResVal(ePos, RES_MZ, 1, femVecGet(&F_0,12));
		}
		else
		{
	    femPutEResVal(ePos, RES_FX, 0, femVecGet(&F_0,1));
	    femPutEResVal(ePos, RES_FY, 0, femVecGet(&F_0,2));
	    femPutEResVal(ePos, RES_FZ, 0, femVecGet(&F_0,3));
	    femPutEResVal(ePos, RES_MX, 0, femVecGet(&F_0,4));
	    femPutEResVal(ePos, RES_MY, 0, femVecGet(&F_0,5));
	    femPutEResVal(ePos, RES_MZ, 0, femVecGet(&F_0,6));

	    femPutEResVal(ePos, RES_FX, 1, femVecGet(&F_0,7));
	    femPutEResVal(ePos, RES_FY, 1, femVecGet(&F_0,8));
	    femPutEResVal(ePos, RES_FZ, 1, femVecGet(&F_0,9));
	    femPutEResVal(ePos, RES_MX, 1, femVecGet(&F_0,10));
	    femPutEResVal(ePos, RES_MY, 1, femVecGet(&F_0,11));
	    femPutEResVal(ePos, RES_MZ, 1, femVecGet(&F_0,12));
		}

		femVecFree(&u_e);
		femVecFree(&F_0);
  }

memFree:
  femMatFree( &k_0 );
  femMatFree( &T );
  femMatFree( &TT );
  femMatFree( &Tk_0 );
  return(rv);
}

int e014_mass(long ePos, tMatrix *M_e)
{
	int    rv = AF_OK ;

	return(rv);
}


long e014_rvals(long ePos)
{
	return(12);
}

int e014_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}

int e014_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	*x = 0.0 ;
	*y = 0.0 ;
	*z = 0.0 ;

	switch(point)
	{
		case 0:
  			*x = femGetNCoordPosX(femGetENodePos(ePos,0));
  			*y = femGetNCoordPosY(femGetENodePos(ePos,0));
  			*z = femGetNCoordPosZ(femGetENodePos(ePos,0));
				break;
		case 1:
  			*x = femGetNCoordPosX(femGetENodePos(ePos,1));
  			*y = femGetNCoordPosY(femGetENodePos(ePos,1));
  			*z = femGetNCoordPosZ(femGetENodePos(ePos,1));
				break;
	}
	return(AF_OK);
}

int addElem_014(void)
{
	int rv = AF_OK;
	static long type    = 14 ;
	static long nodes   = 2 ;
	static long dofs    = 6 ;
	static long ndof[6] = {U_X,U_Y,U_Z,ROT_X,ROT_Y,ROT_Z} ;
	static long rs      = 4 ;
	static long real[4] = {
              RS_AREA, RS_INERTIA_X, RS_INERTIA_Y, RS_INERTIA_Z } ;
	static long rs_rp      = 0 ;
	static long *real_rp = NULL ;
	static long res      = 6 ;
	static long nres[6]    = {RES_FX,RES_FY,RES_FZ,RES_MX,RES_MY,RES_MZ};
	static long res_rp   = 6 ;
	static long nres_rp[6] = {RES_FX,RES_FY,RES_FZ,RES_MX,RES_MY,RES_MZ};

	if (type != femAddElem(type)) {return(AF_ERR_VAL);}
	Elem[type].nodes = nodes ;
	Elem[type].dofs = dofs ;
	Elem[type].ndof = ndof ;
	Elem[type].rs = rs ;
	Elem[type].real = real ;
	Elem[type].rs_rp = rs_rp ;
	Elem[type].real_rp = real_rp ;
	Elem[type].res = res ;
	Elem[type].nres = nres ;
	Elem[type].res_rp = res_rp ;
	Elem[type].nres_rp = nres_rp ;

	Elem[type].stiff = e014_stiff;
	Elem[type].mass  = e014_mass;
	Elem[type].rvals = e014_rvals;
	Elem[type].eload = e014_eload;
	Elem[type].res_p_loc = e014_res_p_loc;
	Elem[type].res_node = e000_res_node;
	return(rv);
}

#endif
/* end of fem_e014.c */
