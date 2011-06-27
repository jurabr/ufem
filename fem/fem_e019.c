/*
   File name: fem_e019.c
   Date:      Mon Jun 27 19:20:04 CEST 2011
   Author:    Jiri Brozovsky

   Copyright (C) 2011  Jiri Brozovsky

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

	 FEM Solver - Element 019 (3D - thermal tetrahedron)
*/

#include "fem_elem.h"
#include "fem_pl3d.h"

#ifndef _SMALL_FEM_CODE_
extern tVector u;

extern int e004_res_p_loc(long ePos, long point, double *x, double *y, double *z);
extern int e004_volume(long ePos, double *vol);

int e019_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
	int rv = AF_OK ;
	tMatrix D;      /* 6x6   */
	tMatrix S;      /* 12x12 */
	tMatrix St;     /* 12x12 */
	tMatrix B;      /* 6x12  */
	tMatrix Bt;     /* 12x6  */
  tMatrix StBt;   /* 12x6  */
  tMatrix StBtD;  /* 12x6  */
  tMatrix BS;     /* 6x12  */
  tVector u_e ;
	double  volume ;
	double x1,y1,x2,y2,x3,y3,x4,y4,z1,z2,z3,z4;
	double a1,b1,c1,a2,b2,c2,a3,b3,c3 ;
  double kxx, mult ;
	long   eT, mT;

	eT = femGetETypePos(ePos); 
	mT = femGetEMPPos(ePos); 

  kxx = femGetMPValPos(ePos, MAT_KXX, 0) ;

  femMatSetZero(K_e);

	femMatNull(&D);
	femMatNull(&S);
	femMatNull(&St);
	femMatNull(&B);
	femMatNull(&Bt);
	femMatNull(&StBt);
	femMatNull(&StBtD);
	femMatNull(&BS);

	if ((rv=femFullMatInit(&D,3,3)) != AF_OK) { goto memFree; }
  if ((rv=femFullMatInit(&S,4,4)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&St,4,4)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&B,3,4)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&Bt,4,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&StBt,4,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&StBtD,4,3)) != AF_OK) { goto memFree; }
	if ((rv=femFullMatInit(&BS,3,4)) != AF_OK) { goto memFree; }

  if (Mode == AF_YES)
  {
		femVecNull(&u_e);

		if ((femVecFullInit(&u_e,12)) != AF_OK) { goto memFree; }
  }

  /* coordinates */
  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  z1 = femGetNCoordPosZ(femGetENodePos(ePos,0));

  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
  z2 = femGetNCoordPosZ(femGetENodePos(ePos,1));

  x3 = femGetNCoordPosX(femGetENodePos(ePos,2));
  y3 = femGetNCoordPosY(femGetENodePos(ePos,2));
  z3 = femGetNCoordPosZ(femGetENodePos(ePos,2));

  x4 = femGetNCoordPosX(femGetENodePos(ePos,3));
  y4 = femGetNCoordPosY(femGetENodePos(ePos,3));
  z4 = femGetNCoordPosZ(femGetENodePos(ePos,3));
	
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"E[%li]: x1=%f y1=%f z1=%f, x2=%f y2=%f z2=%f. x3=%f y3=%f z3=%f x4=%f y4=%f z4=%f\n",
			eID[ePos],  x1,y1,z1,x2,y2,z2, x3,y3,z3,x4,y4,z4 );
#endif

	a1 = x2 - x1 ;
	a2 = y2 - y1 ;
	a3 = z2 - z1 ;

	b1 = x3 - x1 ;
	b2 = y3 - y1 ;
	b3 = z3 - z1 ;

	c1 = x4 - x1 ;
	c2 = y4 - y1 ;
	c3 = z4 - z1 ;

	/* "D" creation: */
  femMatPut(&D, 1,1, kxx);
  femMatPut(&D, 2,2, kxx);
  femMatPut(&D, 3,3, kxx);

	/* "B" matrix: */
	femMatPut(&B, 1,1,  1.0) ;  
	femMatPut(&B, 2,2,  1.0) ;  
	femMatPut(&B, 3,3, 1.0) ;  

	femMatTran(&B, &Bt);


	/* "S" matrix: */
	femMatPut(&S, 1,4,  1.0) ;  
	femMatPut(&S, 2,4,  1.0) ;  
	femMatPut(&S, 3,4,  1.0) ;  
	femMatPut(&S, 4,4,  1.0) ;  

	femMatPut(&S, 1,1,  x1) ;  
	femMatPut(&S, 1,2,  y1) ;  
	femMatPut(&S, 1,3,  z1) ;  

	femMatPut(&S, 2,1,  x2) ;  
	femMatPut(&S, 2,2,  y2) ;  
	femMatPut(&S, 2,3,  z2) ;  

	femMatPut(&S, 3,1,  x3) ;  
	femMatPut(&S, 3,2,  y3) ;  
	femMatPut(&S, 3,3,  z3) ;  

	femMatPut(&S, 4,1,  x4) ;  
	femMatPut(&S, 4,2,  y4) ;  
	femMatPut(&S, 4,3,  z4) ;  


  /* hope this works (inversion of "S"): */
  if (femLUinverse(&S) != AF_OK)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout, "[E] %s!\n", _("Inversion of S failed"));
#endif
  }

  femMatTran(&S, &St);

	femMatMatMult(&St, &Bt, &StBt);
	femMatMatMult(&StBt, &D, &StBtD);
	femMatMatMult(&B, &S, &BS);
	
  /* element stiffness matrix: */
  femMatMatMult(&StBtD, &BS, K_e);
	volume = ( (a1*b2*c3 + c1*a2*b3 + a3*b1*c2) - (c1*b2*a3 + c3*b1*a2 + a1*b3*c2) ) / (6.0) ;
	femValMatMultSelf(volume, K_e);


	/* ====================================================================== */
	if (Mode == AF_YES) /* result, F_e, Fr_e */
	{
    /* obtaining of element displacements */
    femLocUtoU_e(&u, ePos, &u_e);

	  femVecPrn(&u,"u global:");
	  femVecPrn(&u_e,"u local:");

    mult = (
        femVecGet(&u_e, 1) + femVecGet(&u_e, 2) 
        + femVecGet(&u_e, 3) + femVecGet(&u_e, 4) 
        ) / 4.0 ;

		if (femTangentMatrix == AF_YES)
	     { femAddEResVal(ePos, RES_TEMP,  0, mult); }
	  else
	     { femPutEResVal(ePos, RES_TEMP,  0, mult); }

		femMatVecMult(K_e, &u_e, F_e) ;
		femVecSetZero(Fr_e); 
	}
	
memFree:
	femMatFree(&D);
	femMatFree(&S);
	femMatFree(&St);
	femMatFree(&B);
	femMatFree(&Bt);
	femMatFree(&StBt);
	femMatFree(&StBtD);
	femMatFree(&BS);

  if (Mode == AF_YES)
  {
		femVecFree(&u_e);
  }

	return(AF_OK);
}

int e019_mass(long ePos, tMatrix *M_e) { return(AF_OK); }

long e019_rvals(long ePos) { return(1); }

int e019_eload(long ePos, long mode, tVector *F_e) { return(AF_OK); }



int addElem_019(void)
{
	int rv = AF_OK;
	static long type    = 19 ;
	static long nodes   = 4 ;
	static long dofs    = 1 ;
	static long ndof[1] = {TEMP} ;
	static long rs      = 0 ;
	static long *real = NULL ;
	static long rs_rp      = 0 ;
	static long *real_rp = NULL ;
	static long res      = 1 ;
	static long nres[1]    = { RES_TEMP };
	static long res_rp   = 0 ;
	static long *nres_rp = NULL;

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

	Elem[type].stiff = e019_stiff;
	Elem[type].mass  = e019_mass;
	Elem[type].rvals = e019_rvals;
	Elem[type].eload = e019_eload;
	Elem[type].res_p_loc = e004_res_p_loc;
	Elem[type].res_node = e000_res_node;
	Elem[type].volume = e004_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

#endif
/* end of fem_e019.c */
