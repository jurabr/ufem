/*
   File name: fem_e001.c
   Date:      2003/04/09 13:11
   Author:    Jiri Brozovsky

   Copyright (C) 2003 

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

	FEM Solver - Element 001 (2D link)
*/

#include "fem_elem.h"
#include "fem_pl1d.h"

extern tVector u;

extern double sb1d_get_Rt_tension(long ePos,
    double E0, 
    double smax, 
    double Gf, 
    double L, 
    double epsilon); /* from fem_sb1d.c  - material 008 */


int e001_geom_matrix(long ePos, long Mode, double L, tMatrix *K_s)
{
	int rv = AF_OK ;
  double val = 0.0 ;

  val = 1.0 / L ;

	femMatSetZero(K_s);

	femMatPut(K_s, 2, 2, val ) ;
	femMatPut(K_s, 2, 4, (-1.0)*val ) ;

	femMatPut(K_s, 4, 2, (-1.0)*val ) ;
	femMatPut(K_s, 4, 4, val ) ;

	femMatPrn(K_s,"K_s");

	return(rv);
}

void FillTlink2d(double c, double s, tMatrix *T)
{
	int i,j;

	for (i=1; i<=4; i++)
	{
		for (j=1; j<=4; j++)
		{
			femMatPut(T, i,j, 0.0 ) ;
		}
	}

	femMatPut(T, 1,1,  c) ;
	femMatPut(T, 1,2,  s) ;
	femMatPut(T, 2,1, -s) ;
	femMatPut(T, 2,2,  c) ;

	femMatPut(T, 3,3,  c) ;
	femMatPut(T, 3,4,  s) ;
	femMatPut(T, 4,3, -s) ;
	femMatPut(T, 4,4,  c) ;
	
}

int e001_local_stiff_matrix(long ePos, long Mode, double A, double B, double D,  tMatrix *K_e)
{
  int rv = AF_OK ;

  /* element stifness matrix */
	femMatPut(K_e, 1,1,  A) ;  
	femMatPut(K_e, 1,2,  B) ;
	femMatPut(K_e, 1,3, -A) ;
	femMatPut(K_e, 1,4, -B) ;

	femMatPut(K_e, 2,1,  B) ;
	femMatPut(K_e, 2,2,  D) ;
	femMatPut(K_e, 2,3, -B) ;
	femMatPut(K_e, 2,4, -D) ;

	femMatPut(K_e, 3,1, -A) ;
	femMatPut(K_e, 3,2, -B) ;
	femMatPut(K_e, 3,3,  A) ;
	femMatPut(K_e, 3,4,  B) ;

	femMatPut(K_e, 4,1, -B) ;
	femMatPut(K_e, 4,2, -D) ;
	femMatPut(K_e, 4,3,  B) ;
	femMatPut(K_e, 4,4,  D) ;

  return(rv);
}


int e001_stiff(long ePos, long Mode, tMatrix *K_e, tVector *F_e, tVector *Fr_e)
{
	tMatrix T;  /* transformation matrix */
	tMatrix T_T;  /* transformation matrix */
	tMatrix k_0;  /* transformation matrix */
	tMatrix Tk_1;  /* transformation matrix */
	tMatrix k_1;  /* transformation matrix */
	tVector F_0;
	tVector u_e;
  tVector F_1;
  tVector F_nl;
  tVector u_0;
	double c,s, A,B,D, x1,y1,x2,y2,dx,dy, L, Nas;
	double Ax = 0;  /* area */
	double Ex = 0;  /* area */
	double Ex0 = 0;  /* area */
	double Fx, sx, ex; /* results */
	double Fx1 ;
  double N ;
  double f0_Norm, f1_Norm ;
  double ft, Gf ;
  long   i ;
  long   mtype = 0 ;

  mtype = femGetMPTypePos(femGetEMPPos(ePos))  ;

	femMatNull(&T);
	femFullMatInit(&T,4,4);

  if (fem2ndOrder == AF_YES)
  {
	  femMatNull(&k_0);
	  femMatNull(&k_1);
	  femMatNull(&Tk_1);

	  femFullMatInit(&k_0,4,4);
	  femFullMatInit(&k_1,4,4);
	  femFullMatInit(&Tk_1,4,4);
  }

	femVecNull(&F_1);
	femVecFullInit(&F_1,4);
	femMatNull(&T_T);
	femFullMatInit(&T_T,4,4);

  if ((mtype == 4)||(mtype == 8))
  {
	  femVecNull(&F_nl);
	  femVecFullInit(&F_nl,4);
  }

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));
	
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"E[%li]: x1 = %f, y1 = %f, x2 = %f y2 = %f\n",
			eID[ePos], x1,y1,x2,y2);
#endif

	dx = x2 - x1;
	dy = y2 - y1;
	L  = sqrt((dy*dy) + (dx*dx));

	c = dx/L;
	s = dy/L;

	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
  
  if (mtype == 4)
  {
    Ex = femGetEResVal(ePos, RES_ECR1, 0) ;
    if (Ex <= FEM_ZERO) { Ex = femGetMPValPos(ePos, MAT_EX, 0) ; }
  }
  else
  {
	  Ex = femGetMPValPos(ePos, MAT_EX, 0) ;
  }


#ifdef DEVEL_VERBOSE
	fprintf(msgout,"Ax = %e, Ex = %e, L=%f c=%f s=%f\n",Ax,Ex,L,c,s);
  fprintf(msgout, "Ex (m=4): %e\n", Ex);
#endif
	
	Nas = (Ex * Ax)/L;

	A = Nas * c * c;
	B = Nas * c * s;
	D = Nas * s * s;

  femMatSetZero(K_e) ;

  FillTlink2d(c, s, &T) ;
	femMatTran(&T, &T_T) ;

  
  if (fem2ndOrder == AF_YES)
  {
    e001_local_stiff_matrix(ePos, Mode, A, B, D, &k_0);

    e001_geom_matrix(ePos, Mode, L, &k_1);
	  femMatMatMult(&T_T, &k_1, &Tk_1);
    femMatSetZero(&k_1);
	  femMatMatMult(&Tk_1, &T, &k_1);

    N = femGetEResVal(ePos, RES_FX, 0);

    femMatLinComb(1.0, &k_0, (-1.0)*N, &k_1, K_e);
  }
  else
  {
    e001_local_stiff_matrix(ePos, Mode, A, B, D,  K_e);
  }

	if (Mode == AF_YES) /* result, F_e, Fr_e */
	{
		femVecNull(&F_0);
		femVecFullInit(&F_0,4);
		femVecNull(&u_e);
		femVecFullInit(&u_e,4);
  
    
    if ((fem2ndOrder == AF_YES)||(mtype == 4))
    {
		  femVecNull(&u_0);
		  femVecFullInit(&u_0,4);
    }

    femLocUtoU_e(&u, ePos, &u_e);
    
		femMatVecMult(K_e, &u_e, F_e); /* global c.s. */

 		FillTlink2d(c, s, &T);

		femVecSetZero(Fr_e); /* residual forces ;-) */

    if (fem2ndOrder == AF_YES)
    {
		  femMatVecMult(&T, &u_e, &u_0) ; /* local */

      f0_Norm = femVecNorm(F_e) ;
      femVecClone(F_e, &F_1);
      femVecClone(F_e, &F_0);

      femMatSetZero(&k_0) ;

      /* ------------------------------------------ */
      e001_local_stiff_matrix(ePos, Mode, A, B, D, &k_0);

      e001_geom_matrix(ePos, Mode, L, &k_1);
      femMatMatMult(&T_T, &k_1, &Tk_1);
      femMatSetZero(&k_1);
	    femMatMatMult(&Tk_1, &T, &k_1);
      /* ------------------------------------------ */

      for (i=0; i<10e4; i++)
      {
        femMatSetZero(K_e);

	      femMatVecMult(&T, &F_1, F_e);
        N = femVecGet(F_e,1) ;
        femMatLinComb(1.0, &k_0, (-1.0*N), &k_1, K_e);

        femVecSetZero(&F_1);
		    femMatVecMult(K_e, &u_e, &F_1) ;

        f1_Norm = femVecNorm(&F_1) ;

        if (fabs(f0_Norm - f1_Norm) < 1e-4) { break ; }
        f0_Norm = f1_Norm ;
      }


      femVecLinComb(1.0, &F_0, -1.0, &F_1, Fr_e) ;
      femVecClone(&F_1, F_e);

    }

	  femMatVecMult(&T, F_e, &F_0);   /* local c.s.  */

		Fx = femVecGet(&F_0, 1) * (-1.0) ;
		sx = Fx / Ax ;
		ex = sx/Ex ;

#ifdef DEVEL_VERBOSE
    fprintf(msgout,"ex = %e\n", ex);
#endif

    if (mtype == 4) 
    {
      Ex0 = femGetEResVal(ePos, RES_ECR1, 0) ;
      if (Ex0 <= FEM_ZERO) { Ex0 = femGetMPValPos(ePos, MAT_EX, 0) ; }

#ifdef DEVEL_VERBOSE
    fprintf(msgout,"ex = %e | EX0 = %e\n", ex, Ex0);
#endif
      Ex = fem_pl1d_E(
		  ePos, 
		  Ex0, 
		  ex, 
		  femGetEResVal(ePos, RES_EX, 0), 
		  femGetEResVal(ePos, RES_SX, 0) 
		  );
#ifdef DEVEL_VERBOSE
	    fprintf(msgout,"Ex = %e <- %e \n", Ex, Ex0);
#endif
    }
    else
    {
	    Ex = femGetMPValPos(ePos, MAT_EX, 0) ;
    }

    if (mtype == 4)
    {
	    Nas = (Ex * Ax)/L;

	    A = Nas * c * c;
	    B = Nas * c * s;
	    D = Nas * s * s;

		  femMatVecMult(K_e, &u_e, F_e); /* global c.s. */

      femMatSetZero(K_e) ;
      e001_local_stiff_matrix(ePos, Mode, A, B, D, K_e);

		  femMatVecMult(K_e, &u_e, &F_nl); /* global c.s. */

      femVecLinComb(1.0, F_e, -1.0, &F_nl, &F_1) ;
      femVecAddVec(Fr_e, +1.0,  &F_1);
      femVecClone(&F_nl, F_e);
    }


#if 1
    if (mtype == 8) /* concrete - for testing ONLY (does not work) */
    {
	    Ex0 = femGetMPValPos(ePos, MAT_EX, 0) ;
      ft = femGetMPValPos(ePos, MAT_STEN, 0) ;

			/* TODO: compute proper ex here:*/
#if 0
			ex = sqrt(pow(femVecGet(&u_e,4),2) + pow(femVecGet(&u_e,3),2))/L;
printf("EX = %e / L=%e XXXXXXXXXXXXXXXXXXXXXXXXXXXXx (%e,%e)\n",ex, L, femVecGet(&u_e,3),femVecGet(&u_e,4) );
#endif

      if ((ex +femGetEResVal(ePos,RES_EX,0)) > (ft/Ex0))
      {
        Gf = femGetMPValPos(ePos, MAT_GF, 0) ;

				if (ex + femGetEResVal(ePos, RES_EX, 0) > 0.0)
				{
        	Ex = sb1d_get_Rt_tension(ePos, Ex0, ft, Gf, sqrt(Ax),
            	ex + femGetEResVal(ePos, RES_EX, 0)) ;
					if (Ex > Ex0) {Ex = Ex0;}
					if (Ex < FEM_ZERO) {Ex = FEM_ZERO;}
				}
				else
				{
					Ex = Ex0 ;
				}


        Fx = (femGetEResVal(ePos, RES_EX, 0) + ex ) * Ex * Ax - 
             femGetEResVal(ePos, RES_FX,0) ;
		    sx = Fx / Ax ; 
        femVecSetZero(&F_1);
        femVecPut(&F_1,1, -Fx) ;
        femVecPut(&F_1,3,  Fx) ;
	      femMatVecMult(&T_T, &F_1, F_e);


        Fx1 = (femGetEResVal(ePos, RES_EX, 0) + ex ) * Ex0 * Ax - 
             femGetEResVal(ePos, RES_FX,0) ;


printf("Fx1=%e Fx=%e\n", Fx1, Fx);

        Fx1 = (Fx1  - Fx) ;

printf("dFxr=%e \n", Fx1);

        femVecSetZero(Fr_e);
        femVecSetZero(&F_1);
        femVecPut(&F_1,1, -Fx1) ;
        femVecPut(&F_1,3,  Fx1) ;
	      femMatVecMult(&T_T, &F_1, &F_nl);
        femVecAddVec(Fr_e, +1.0,  &F_nl);

femMatPrn(&T_T, "T_T");
femVecPrn(&F_1,"F1");
femVecPrn(&F_nl,"FNL");
femVecPrn(Fr_e,"FrE");
      }
      else
      {
        Ex = Ex0 ;
      }
    }
#endif


	  if (femTangentMatrix == AF_YES)
	  {
	    femAddEResVal(ePos, RES_FX, 0, Fx);
	    femAddEResVal(ePos, RES_SX, 0, sx);
	    femAddEResVal(ePos, RES_EX, 0, ex);
	  }
	  else
	  {
	    femPutEResVal(ePos, RES_FX, 0, Fx);
	    femPutEResVal(ePos, RES_SX, 0, sx);
	    femPutEResVal(ePos, RES_EX, 0, ex);
	  }

	  femPutEResVal(ePos, RES_ECR1, 0, Ex);
		

		femVecFree(&F_0);
		femVecFree(&u_e);

    if ((fem2ndOrder == AF_YES) || (mtype == 4))
    { 
      femVecFree(&u_0); 
    }
	}

  if (fem2ndOrder == AF_YES)
  {
	  femMatFree(&k_0);
	  femMatFree(&k_1);
	  femMatFree(&Tk_1);
  }

  if ((mtype == 4)||(mtype == 8))
  {
    femVecFree(&F_nl); 
  }
	
	femMatFree(&T);
	femMatFree(&T_T);
  femVecFree(&F_1); 

#ifdef DEVEL_VERBOSE
  femVecPrn(F_e, "F");
  femVecPrn(Fr_e, "FR");
  femMatPrn(K_e, "K");
#endif
	return(AF_OK);
}

double e001_lenght(long ePos)
{
	double x1,y1,x2,y2,dx,dy;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));

	dx = x2 - x1;
	dy = y2 - y1;

	return( sqrt((dy*dy) + (dx*dx)));
}

int e001_mass(long ePos, tMatrix *M_e)
{
	int rv = AF_OK;
	double Ax, ro, mass, L;

	L = e001_lenght(ePos) ;

	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
	ro = femGetMPValPos(ePos, MAT_DENS, 0) ;

	mass = Ax * L * ro ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"Ax = %f, ro = %f, L=%f mass=%f\n",Ax,ro,L,mass);
#endif

	femMatSetZero(M_e);

	femMatPut(M_e, 1,1, (mass /2.0) ) ;
	femMatPut(M_e, 2,2, (mass /2.0) ) ;
	femMatPut(M_e, 3,3, (mass /2.0) ) ;
	femMatPut(M_e, 4,4, (mass /2.0) ) ;

	return(rv);
}

int e001_volume(long ePos, double *val)
{
	int rv = AF_OK;
	double Ax ;
	double L;

	*val = 0 ;

	L = e001_lenght(ePos) ;
	Ax = femGetRSValPos(ePos, RS_AREA, 0) ;
	*val = Ax * L ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"Ax = %f, L=%f mass=%f\n",Ax, L, *val);
#endif

	return(rv);
}

long e001_rvals(long ePos)
{
	return(4);
}

int e001_eload(long ePos, long mode, tVector *F_e)
{
	return(AF_OK);
}


int e001_res_p_loc(long ePos, long point, double *x, double *y, double *z)
{
	double x1,x2,y1,y2 ;

  x1 = femGetNCoordPosX(femGetENodePos(ePos,0));
  y1 = femGetNCoordPosY(femGetENodePos(ePos,0));
  x2 = femGetNCoordPosX(femGetENodePos(ePos,1));
  y2 = femGetNCoordPosY(femGetENodePos(ePos,1));

	*x = 0.5*(x1+x2) ;
	*y = 0.5*(y1+y2) ;

	*z = 0 ;

	return(AF_OK);
}

int e001_res_node(long ePos, long nPos, long type, double *val)
{
  switch (type)
  {
    case RES_FX: *val = femGetEResVal(ePos, RES_FX, 0) ;
      return(AF_OK); break ;
    case RES_SX:  *val = femGetEResVal(ePos, RES_SX, 0) ;
      return(AF_OK); break ;
    case RES_EX:  *val = femGetEResVal(ePos, RES_EX, 0) ;
      return(AF_OK); break ;
    default: return(AF_ERR); break ;
  }
  return(AF_ERR);
}

int addElem_001(void)
{
	int rv = AF_OK;
	static long type    = 1 ;
	static long dim     = 1 ;
	static long nodes   = 2 ;
	static long dofs    = 2 ;
	static long ndof[2] = {U_X,U_Y} ;
	static long rs      = 1 ;
	static long real[1] = {RS_AREA} ;
	static long rs_rp      = 0 ;
	static long *real_rp = NULL ;
	static long res      = 4 ;
	static long nres[4] = {RES_FX,RES_SX,RES_EX,RES_ECR1} ;
	static long res_rp      = 0 ;
	static long *nres_rp = NULL ;

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

	Elem[type].stiff = e001_stiff;
	Elem[type].mass  = e001_mass;
	Elem[type].rvals = e001_rvals;
	Elem[type].eload = e001_eload;
	Elem[type].res_p_loc = e001_res_p_loc;
	Elem[type].res_node = e001_res_node;
	Elem[type].volume = e001_volume;
	Elem[type].therm = e000_therm;
	return(rv);
}

/* end of fem_e001.c */
