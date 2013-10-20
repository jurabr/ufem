/*
   File name: fem_e020.c
   Date:      2011/07/08 12:33
   Author:    Jiri Brozovsky

   Copyright (C) 2013 Jiri Brozovsky

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

	 FEM Solver - element 020 (isoparametric thermal plane) definition
*/

#include "fem_elem.h"

#ifndef _SMALL_FEM_CODE_

extern void mirrorK(tMatrix *Ke);
extern int e005_volume(long ePos, double *vol);
extern int e005_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e000_res_node(long ePos, long nPos, long type, double *val);

int e020_stiff(long ePos, long Mode, tMatrix *K_e, tVector *Fe, tVector *Fre)
{
	int     rv = AF_OK;
  int     i, j, ii, jj ;
	double  thick, kxx ;
  long    ipos ;
  double  x, y, weight_x, weight_y, detj, mult ;
  double  sign[4][2] ;
	tVector u_e;
  tMatrix D ;
  tMatrix B ;
  tMatrix Bt ;
  tMatrix BtD ;
  tMatrix BtDB ;
  tMatrix N ;
  tMatrix G ;
  tMatrix xyz ;
  double gauss =  0.577350269189626 ;

  sign[0][0] = -1 ; sign[1][1] = -1 ;
  sign[1][0] = -1 ; sign[1][1] = +1 ;
  sign[2][0] = +1 ; sign[1][1] = -1 ;
  sign[3][0] = +1 ; sign[1][1] = +1 ;

  /* vectors and matrices: */
  femVecNull(&u_e);
	femMatNull(&D);
	femMatNull(&B);
	femMatNull(&Bt);
	femMatNull(&BtD);
	femMatNull(&BtDB);
	femMatNull(&N);
	femMatNull(&G);
	femMatNull(&xyz);

	if (Mode == AF_YES)
	{
	  if ((rv = femVecFullInit(&u_e, 4)) != AF_OK) {goto memFree;}
  	femLocUtoU_e(&u, ePos, &u_e);
  }

	if ((rv = femFullMatInit(&D, 2,2)) != AF_OK) {goto memFree;}
	if ((rv=femFullMatInit(&B,2,4)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&Bt,4,2)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&BtD,4,2)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&BtDB,4,4)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&N,2,4)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&G,2,2)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&xyz,4,2)) != AF_OK) { goto memFree; }

	thick = femGetRSValPos(ePos, RS_HEIGHT, 0) ;

  /* D matrix: */
  kxx = femGetMPValPos(ePos, MAT_KXX, 0) ;
  femMatPut(&D, 1,1, kxx );
  femMatPut(&D, 2,2, kxx );


  /* real coordinates: */
  for (i=1; i<=4; i++)
  {
    femMatPut(&xyz,i,1, femGetNCoordPosX(femGetENodePos(ePos,i-1)) );
    femMatPut(&xyz,i,2, femGetNCoordPosY(femGetENodePos(ePos,i-1)) );
  }

  /* numerical integration: */
  ipos = -1 ;

  for (i=1; i<=2; i++)
  {
    for (j=1; j<=2; j++)
    {
        ipos++ ;
        
        /* int. point coordinates: */
        
        x = gauss * sign[ipos][0] ;
        y = gauss * sign[ipos][1] ;

        weight_x = 1.0 ;
        weight_y = 1.0 ;

        /* N matrix (derivatives) */
        femMatSetZero(&N);
        femMatPut(&N, 1,1,  y - 1.0);
        femMatPut(&N, 1,2, -y + 1.0);
        femMatPut(&N, 1,3,  y + 1.0);
        femMatPut(&N, 1,4, -y - 1.0);

        femMatPut(&N, 2,1,  x - 1.0);
        femMatPut(&N, 2,2, -x - 1.0);
        femMatPut(&N, 2,3,  x + 1.0);
        femMatPut(&N, 2,4, -x + 1.0);

        femMatPrn(&N,"N matrix");
        femMatPrn(&xyz,"xyz matrix");

        femValMatMultSelf(0.25, &N);


        /* G matrix (Jac inversion): */
        femMatSetZero(&G);

        for (ii=1; ii<=4; ii++)
        {
          femMatAdd(&G,2,2, femMatGet(&N,1,ii) * femMatGet(&xyz,ii,1));
          femMatAdd(&G,1,1, femMatGet(&N,2,ii) * femMatGet(&xyz,ii,2));
          femMatAdd(&G,1,2, -1.0*femMatGet(&N,1,ii) * femMatGet(&xyz,ii,2));
          femMatAdd(&G,2,1, -1.0*femMatGet(&N,2,ii) * femMatGet(&xyz,ii,1));
        }

        femMatPrn(&G,"G");

        detj = femMatGet(&G,1,1)*femMatGet(&G,2,2)-(femMatGet(&D,1,2)*femMatGet(&D,2,1));
        if (detj <= FEM_ZERO)
        {
#ifdef RUN_VERBOSE
          fprintf(msgout,"[E] %s: %li!\n",_("Invalid Jacobi matrix in element"),eID[ePos]);
#endif
          rv = AF_ERR_VAL; goto memFree;
        }
        femValMatMultSelf(1.0/detj, &G);

        /* integration multiplier */
				mult = detj * weight_x * weight_y * thick ;

        femMatMatMult(&G,&N, &B);
        femMatTran(&B, &Bt);

        femMatMatMult(&Bt,&D, &BtD);
        femMatMatMult(&BtD, &B, &BtDB);
        femValMatMultSelf(mult, &BtDB);

        for (ii=1; ii<=4; ii++)
        {
          for (jj=1; jj<=4; jj++)
          {
            femMatAdd(K_e,ii,jj, mult*femMatGet(&BtDB,ii,jj));
          }
        }
    }
  }

	if (Mode == AF_YES)
	{
    for (jj=1; jj<=4; jj++);
    {
		  if (femTangentMatrix == AF_YES)
	     { femAddEResVal(ePos, RES_TEMP,  jj-1, femVecGet(&u_e,jj) ); }
	    else
	     { femPutEResVal(ePos, RES_TEMP,  jj-1, femVecGet(&u_e,jj) ); }
    }
    femVecPrn(&u_e,"U_E"); 
	}
		
	femMatVecMult(K_e, &u_e, Fe) ;
	femVecSetZero(Fre);
	
memFree:
  if (Mode == AF_YES) { femVecFree(&u_e); }

	femMatFree(&D);
	femMatFree(&B);
	femMatFree(&Bt);
	femMatFree(&BtD);
	femMatFree(&BtDB);
	femMatFree(&N);
	femMatFree(&G);
	femMatFree(&xyz);

	return(rv);
}

int e020_mass(long ePos, tMatrix *M_e) { return(AF_OK); }

long e020_rvals(long ePos) { return(4); }

int e020_eload(long ePos, long mode, tVector *F_e) { return(AF_OK); }


int addElem_020(void)
{
	int rv = AF_OK;
	static long type     = 20 ;
	static long dim      = 0 ;
	static long nodes    = 4 ;
	static long dofs     = 1 ;
	static long ndof[1]  = {TEMP} ;
	static long rs       = 1 ;
	static long real[1]  = {RS_HEIGHT} ;
	static long rs_rp    = 0 ;
	static long *real_rp = NULL ;
	static long res      = 0 ;
	static long *nres    = NULL ;
	static long res_rp   = 1 ;
	static long nres_rp[1] = { RES_TEMP} ;

	if (type != femAddElem(type)) {return(AF_ERR_VAL);}
	Elem[type].dim = dim ;
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

	Elem[type].stiff = e020_stiff;
	Elem[type].mass  = e020_mass;
	Elem[type].rvals = e020_rvals;
	Elem[type].eload = e020_eload;
	Elem[type].res_p_loc = e005_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e005_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e002.c */
