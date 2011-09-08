/*
   File name: fem_asse.c
   Date:      2011/01/31 18:27
   Author:    Jiri Brozovsky

   Copyright (C) 2011 Jiri Brozovsky

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

   General assessment routines (mainly for libfem.so)
*/

#include "fem.h"
#include "fem_data.h"
#include "fem_math.h"
#include "fem_sol.h"

extern double stress2D_J2(tVector *stress) ; /* from fem_ch2d */
extern double stress3D_J2(tVector *stress) ; /* from fem_chen */
extern int femPrinc3D(tVector *sx, tVector *s1);
extern int femGetPrincStress2D(tVector *sigma_x, tVector *sigma_1, double *phi);
extern double e001_lenght(long ePos);
extern double e007_length(long ePos);

/** Max. displacement test
 * @param disp_max maximum allowed displacement (x,y, or z)
 * @return 0 if passed, non-zero number (usually 1) if failed 
 */
long fem_asse_max_disp_simple(double disp_max)
{
  if (femVecMaxAbs(&u) >= fabs(disp_max)) {return(1);} /* failed */

  return(0);
}

/** Material point failure test - material 4: von Mises plasticity condition 
 * @param sigma stress vector
 * @param epsilon strain vector
 * @param mType type of material
 * @return 0 if passed, non-zero number (usually 1) if failed 
 */
long fem_asse_mat_vmis(tVector *sigma, tVector *epsilon, long ePos)
{
  double f_y, s_vmis ;

  f_y = femGetMPValPos(ePos, MAT_F_YC, 0) ;
  
  if (sigma->len > 3)
  {
    /* 3D case */
    s_vmis = sqrt( stress3D_J2(sigma) * 3.0 )  ;
  }
  else
  {
    /* 2D case */
    s_vmis = sqrt( stress2D_J2(sigma) * 3.0 ) ;
  }

  if ( s_vmis < fabs(f_y) ) { return(0); } /* passed */
  else                      { return(1); } /* failed */
  return(0);
}

/** Material point failure test - material 3: concrete (simplified)
 * @param sigma stress vector
 * @param epsilon strain vector
 * @param mType type of material
 * @return 0 if passed, non-zero number (usually 1) if failed 
 */
long fem_asse_mat_concr(tVector *sigma, tVector *epsilon, long ePos)
{
  double f_yt, f_ybc, val_t, val_bc, phi ;

  f_yt  = femGetMPValPos(ePos, MAT_F_YT,  0) ;
  f_ybc = femGetMPValPos(ePos, MAT_F_YBC, 0) ;
  
  if (sigma->len > 3)
  {
    /* 3D case */
    femPrinc3D(sigma, epsilon);
    val_bc = femVecGet(epsilon, 3);
  }
  else
  {
    /* 2D case */
    femGetPrincStress2D(sigma, epsilon, &phi); /* epsilon serves as principal stress vector! */
    val_bc = femVecGet(epsilon, 2);
  }

    val_t = femVecGet(epsilon, 1);

  if ( (val_t > fabs(f_yt)) &&(val_bc < (-1.0*fabs(f_ybc)) ) ) 
       { return(1); } /* failed */
  else { return(0); } /* passed */

  return(0);
}



/** Material point failure test wrapper
 * @param sigma stress vector
 * @param epsilon strain vector
 * @param mType type of material
 * @param ePos position of studied element
 * @return 0 if passed, non-zero number (usually 1) if failed 
 */
long fem_asse_mat_by_type(tVector *sigma, tVector *epsilon, long ePos)
{
  long mType ;

  mType = Mat[femGetMPTypePos(femGetEMPPos(ePos))].type ;

  switch (mType)
  {
    case 3: /* concrete */
      return ( fem_asse_mat_concr(sigma, epsilon, ePos) );
      break;
    case 4: /* von Mises steel */
      return ( fem_asse_mat_vmis(sigma, epsilon, ePos) );
      break;

    default:
      return(0); /* nothing to test */
      break;
  }

  return(0);
}

/** Tests elastic stability of links according to Eurocode 3 (one axis only):
  @param N  normal force - computed
  @param E  young modullus
  @param A  element area
  @param L  element lenght
  @param tH element height
  @param tF limit stress
  @param te initial excentricity
  @param tI moment of inertia
  @return status: 1..failed, 0..not failed
*/
long fem_steel_link_stability_simple(
  double NN,  /* normal force - computed */
  double E,  /* young modullus          */
  double A,  /* element area            */
  double L,  /* element lenght          */
  double tH, /* element height          */
  double tF, /* limit stress            */
  double te, /* initial excentricity    */
  double tI  /* moment of inertia       */
  )
{
  /* data for solution: */
  double ti; /* radius of garation      */
	double Rt, Rc, R1 ;
  double N ;

  /* computation of data */
  ti = sqrt(tI/A);

  N = (-1.0) * NN ;

  /* procedure: */
  R1 = 1.0 + (tF*pow(L/ti, 2)/(FEM_PI*FEM_PI*E)) + ((L*te*tH*0.5)/(ti*ti)) ;
  Rc = (R1 - sqrt(R1*R1 - tF*((4.0*pow(L/ti,2))/(FEM_PI*FEM_PI*E)))
      ) /
    (
     2.0*pow(L/ti,2)/(FEM_PI*FEM_PI*E*A)
    );
    
  Rt = tF ;

	if ((N) < 0.0)
	{
		if (fabs(N) > fabs(Rc)) { return(1); }
		else { return(0); }
	}
	else
	{
    if (fabs(N/A) > fabs(Rt)) 
	  {
		  return(1) ;
	  }
    else
    {
		  return(0);
    }
	}

	return(0);
}


/** Tests elastic stability of links according to Eurocode 3:
  @param ePos element position 
  @param eT   element type 
  @return status: 1..failed, 0..not failed
*/
long fem_test_steel_link_stability(long ePos, long eT)
{
  long   mType ;
  long   rv = 0 ;
  double N;  /* normal force - computed */
  double E;  /* young modullus          */
  double tF; /* limit stress            */
  double A;  /* element area            */
  double L;  /* element lenght          */
  
  double tHy; /* element height          */
  double tey; /* initial excentricity    */
  double tIy; /* moment of inertia       */

  double tHz; /* element height          */
  double tez; /* initial excentricity    */
  double tIz; /* moment of inertia       */

  if ((mType = Mat[femGetMPTypePos(femGetEMPPos(ePos))].type) != 4)
  {
    /* only type==4 (von Mises) can work here */
    return(0);
  }

  N  = femGetEResVal(ePos, RES_FX,0) ;
  E  = femGetMPValPos(ePos, MAT_EX, 0) ;
  tF = femGetMPValPos(ePos, MAT_F_YC, 0) ;
	A  = femGetRSValPos(ePos, RS_AREA, 0) ;

	tHy = femGetRSValPos(ePos, RS_HEIGHT, 0) ;
	tIy = femGetRSValPos(ePos, RS_INERTIA_Y, 0) ;
	tey = femGetRSValPos(ePos, RS_EXENTR_Y, 0) ;

  if (Elem[eT].dofs <= 2) /* 2 D link */
  {
    L   = e001_lenght(ePos) ;
    return(fem_steel_link_stability_simple(N,E,A,L,tHy,tF,tey,tIy));
  }
  else /* 3D link */
  {
    L   = e007_length(ePos) ;
    if ((rv=fem_steel_link_stability_simple(N,E,A,L,tHy,tF,tey,tIy)) != 0)
       { return(rv); }
    else
    {
	    tHz = femGetRSValPos(ePos, RS_WIDTH, 0) ;
	    tIz = femGetRSValPos(ePos, RS_INERTIA_Z, 0) ;
	    tez = femGetRSValPos(ePos, RS_EXENTR_Z, 0) ;
      return(fem_steel_link_stability_simple(N,E,A,L,tHz,tF,tez,tIz));
    }
  }
  
  return(0);
}


/** Checks failure condition in dependence of material and element type
 * @return 0 if not failed, 1 if failed
 */
long fem_asse_fail_cond(void)
{
  long    i, j, eT, dim, ips;
  static long  result = 0 ;
  tVector stress3 ;
  tVector stress6 ;
  tVector strain3 ;
  tVector strain6 ;

  femVecNull(&stress3) ;
  femVecNull(&stress6) ;
  femVecNull(&strain3) ;
  femVecNull(&strain6) ;
  
  if (femVecFullInit(&stress3, 3) != AF_OK) {goto memFree;}
  if (femVecFullInit(&stress6, 6) != AF_OK) {goto memFree;}

  if (femVecFullInit(&strain3, 3) != AF_OK) {goto memFree;}
  if (femVecFullInit(&strain6, 6) != AF_OK) {goto memFree;}

  for (i=0; i<eLen; i++)
  {
    eT = femGetETypePos(i) ;
    dim = Elem[eT].dim ;

      switch (dim)
      {
        case 1: /* link */
#if 0
          femVecPut(&stress3, 1, femGetEResVal(i, RES_SX, 0) );
          femVecPut(&stress3, 2, 0.0 );
          femVecPut(&stress3, 3, 0.0 );
          if ((result = fem_asse_mat_vmis(&stress3, &strain3, i)) != 0) {return(result);}

          ips = Elem[eT].res_rp ;
          for (j=1; j<=ips; j++)
          {
            femVecPut(&stress3, 1, femGetEResVal(i, RES_SX, j) );
            femVecPut(&stress3, 2, 0.0 );
            femVecPut(&stress3, 3, 0.0 );
            if ((result = fem_asse_mat_vmis(&stress3, &strain3, i)) != 0) {return(result);}
          }
#endif
          /* steel link stability: */
          if ((result = fem_test_steel_link_stability(i, eT)) != 0) 
          {
             return(result);
          }
          break ;
        case 2: /* plane */ 
          femVecPut(&stress3, 1, femGetEResVal(i, RES_SX, 0) );
          femVecPut(&stress3, 2, femGetEResVal(i, RES_SY, 0)  );
          femVecPut(&stress3, 3, femGetEResVal(i, RES_SXY, 0) );
          if ((result = fem_asse_mat_vmis(&stress3, &strain3, i)) != 0) {return(result);}

          ips = Elem[eT].res_rp ;
          for (j=1; j<=ips; j++)
          {
            femVecPut(&stress3, 1, femGetEResVal(i, RES_SX, j) );
            femVecPut(&stress3, 2, femGetEResVal(i, RES_SY, j)  );
            femVecPut(&stress3, 3, femGetEResVal(i, RES_SXY, j) );
            if ((result = fem_asse_mat_vmis(&stress3, &strain3, i)) != 0) {return(result);}
          }
          break ;
        case 3: /* 3D structure */
          /* TODO */
          femVecPut(&stress6, 1, femGetEResVal(i, RES_SX, 0) );
          femVecPut(&stress6, 2, femGetEResVal(i, RES_SY, 0)  );
          femVecPut(&stress6, 3, femGetEResVal(i, RES_SZ, 0)  );
          femVecPut(&stress6, 4, femGetEResVal(i, RES_SYZ, 0) );
          femVecPut(&stress6, 5, femGetEResVal(i, RES_SZX, 0) );
          femVecPut(&stress6, 6, femGetEResVal(i, RES_SXY, 0) );
          if ((result = fem_asse_mat_vmis(&stress6, &strain6, i)) != 0) {return(result);}

          ips = Elem[eT].res_rp ;
          for (j=1; j<=ips; j++)
          {
            femVecPut(&stress6, 1, femGetEResVal(i, RES_SX, j) );
            femVecPut(&stress6, 2, femGetEResVal(i, RES_SY, j)  );
            femVecPut(&stress6, 3, femGetEResVal(i, RES_SZ, j)  );
            femVecPut(&stress6, 4, femGetEResVal(i, RES_SYZ, j) );
            femVecPut(&stress6, 5, femGetEResVal(i, RES_SZX, j) );
            femVecPut(&stress6, 6, femGetEResVal(i, RES_SXY, j) );
            if ((result = fem_asse_mat_vmis(&stress6, &strain6, i)) != 0) {return(result);}
          }

          break ;
        case 4: /* slab/shell */ 
          /* TODO */
          break ;
        case 5: /* beam */
          /* TODO */
          break ;
        default: break ;
      }
  }

memFree:
  femVecFree(&stress3) ;
  femVecFree(&stress6) ;
  femVecFree(&strain3) ;
  femVecFree(&strain6) ;
  return (result);
}

/* end of fem_asse.c */
