/*
   File name: fdb_cres.c
   Date:      2005/02/12 12:40
   Author:    Jiri Brozovsky

   Copyright (C) 2005 Jiri Brozovsky

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.

	 Database for FEM: results computed after solution

   $Id: fdb_cres.c,v 1.3 2005/02/16 19:41:14 jirka Exp $
*/

#include "fdb_res.h"
#include "fdb_edef.h"
#include "cint.h"

extern double fdbResElemGetVal0(long res_pos, long etype, long type, long set);

long fdbResComputed = AF_NO ; /* if ==AF_YES then data are computed */
long fdbResCompLen  = 10 ;
long fdbResCompList[10] = {
                        RES_S1, RES_S2, RES_S3, RES_TMAX,
                        RES_SM1, RES_SM2, RES_SMSMAX,
                        RES_SMXDIM, RES_SMYDIM, RES_SVMIS
                        } ;

/** Tests if "restype" is a computed result 
 * @param restype result type to be tested
 * @return AF_YES (have to be computed), AF_NO (usual result)
 */
int fdbResComp_IsComp(long restype)
{
  int i ;

  for (i=0; i<fdbResCompLen; i++)
  {
    if (restype == fdbResCompList[i])
    {
      return(AF_YES) ;
    }
  }
  return(AF_NO) ;
}

/* principal stresses/moments -- 2D: **********  */
void comp_s12ab_2D(double sx, double sy, double txy, double *a, double *b)
{
  *a = 0.5*(sx+sy) ;
  *b = 0.5*sqrt( pow((sy-sx),2) + 4.0*txy*txy) ;
}

double comp_s1_2D(double sx, double sy, double txy)
{
  double s1,s2 ;
  double a, b ;

  comp_s12ab_2D(sx, sy, txy, &a, &b);

  s1 = a + b ;
  s2 = a - b ;

  if (s1 > s2) { return(s1); }
  else         { return(s2); }
}

double comp_s2_2D(double sx, double sy, double txy)
{
  double s1,s2 ;
  double a, b ;

  comp_s12ab_2D(sx, sy, txy, &a, &b);

  s1 = a + b ;
  s2 = a - b ;

  if (s1 > s2) { return(s2); }
  else         { return(s1); }
}

double comp_tmax_2D(double sx, double sy, double txy)
{
  double a, b ;

  comp_s12ab_2D(sx, sy, txy, &a, &b);

  return( 0.5*((a+b) - (a-b)) );
}

double fdb_sgn(double a)
{
  if (a < 0.0) { return(-1.0); }
  else         { return( 1.0); }
}

double comp_dim_m_2D(double mx, double mxy)
{
  return(mx + fdb_sgn(mx)*fabs(mxy));
}


/* principal stresses -- 3D: **********  */

/** sorts 3 double value s1>=s2>=s3 */
void femSort3dMaxMin(double *s1, double *s2, double *s3)
{
  double val = 0.0 ;
  double a[3];
  int posmax = 0 ;
  int posmid = 0;
  int posmin = 0;
  int i;

  if ( (*s1 == *s2) && (*s2 == *s3) ) {return;}

  a[0] = *s1 ;
  a[1] = *s2 ;
  a[2] = *s3 ;

  val = a[0] ;
  posmax = 0 ;
  for (i=1; i<3; i++) { if (a[i] >= val) {val = a[i]; posmax = i;} }

  val = a[0] ;
  posmin = 0 ;
  for (i=1; i<3; i++) { if (a[i] <= val) {val = a[i]; posmin = i;} }

  for (i=0; i<3; i++)
  {
    if ((i != posmax) && (i != posmin)) 
    {
      posmid = i ; 
      break ;
    }
  }

  *s1 = a[posmax] ;
  *s2 = a[posmid] ;
  *s3 = a[posmin] ;
}

int femPrinc3D(
    double s_x,
    double s_y,
    double s_z,
    double s_yz,
    double s_zx,
    double s_xy,
    double *s_1,
    double *s_2,
    double *s_3
    )
{
  double a,b,c ; /* s^3 - a*s^2 - b*s - c = 0 */
  double Q, R, Q3, R2, sQ,a3,  ang ;


	if ( sqrt(s_x*s_x+s_y*s_y+s_z*s_z+s_yz*s_yz+s_zx*s_zx+s_xy*s_xy) <= FEM_ZERO) {return(AF_OK);}

  /* cubic equation coefficicents */
  a = (-1.0)*(s_x + s_y + s_z) ;
  b = s_x*s_y + s_y*s_z + s_z*s_x - s_xy*s_xy - s_yz*s_yz - s_zx*s_zx ;
  c = (-1.0)*(s_x*s_y*s_z+2*s_xy*s_yz*s_zx - (s_zx*s_y*s_zx+s_xy*s_xy*s_z+s_x*s_yz*s_yz)) ;

  /* root finding by Francois Viete: De emendatione, 1615 */
  Q = (a*a - 3.0*b) / 9.0 ;
  R = (2.0*a*a*a - 9.0*a*b + 27.0*c) / 54.0 ;

  Q3 = Q*Q*Q ;
	R2 = R*R ;

  if ((R2) <= (Q3))
  {
    ang = acos(R / sqrt(Q3)) ;

    sQ = sqrt(Q) ;
    a3 = a/3.0 ;

    *s_1 = -2.0*sQ*cos(ang/3.0)                - a3 ;
    *s_2 = -2.0*sQ*cos((ang+(2.0*FEM_PI))/3.0) - a3 ;
    *s_3 = -2.0*sQ*cos((ang-(2.0*FEM_PI))/3.0) - a3 ;
  }
  else
  {
		/* If only one root is real - impossible here!!!!! */
		if ((R2) > Q3)
		{
			*s_1 = -1.0 * 
					( 
						pow( (sqrt(R*R-Q3)+fabs(R)),(1.0/3.0) ) +
						Q / pow( (sqrt(R*R-Q3)+fabs(R)),(1.0/3.0) )
					);

			if (R >= 0) { /* do nothing */ }
			else { *s_1 *= (-1.0); }

			*s_2 = *s_1 ;
			*s_3 = *s_1 ;

#ifdef RUN_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Impossible stresses found - principal stresses will be invalid"));
#endif
		}
		else
		{
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s: Q3=%e, R2=%e!\n",_("Complex principal values found"),Q3,R*R);
#endif
      *s_1 = 0 ;
      *s_2 = 0 ;
      *s_3 = 0 ;
      return(AF_ERR_VAL);
		}
  }

  /* s_1 > s_2 > s_3 */
  femSort3dMaxMin(s_1, s_2, s_3) ;

  return(AF_OK);
}

double comp_s1_3D(
    double s_x,
    double s_y,
    double s_z,
    double s_yz,
    double s_zx,
    double s_xy
    )
{
  static double s_1 ;
  static double s_2 ;
  static double s_3 ;

  femPrinc3D(
    s_x, s_y, s_z,
    s_yz, s_zx, s_xy,
    &s_1, &s_2, &s_3
    );

  return(s_1);
}

double comp_s2_3D(
    double s_x,
    double s_y,
    double s_z,
    double s_yz,
    double s_zx,
    double s_xy
    )
{
  static double s_1 ;
  static double s_2 ;
  static double s_3 ;

  femPrinc3D(
    s_x, s_y, s_z,
    s_yz, s_zx, s_xy,
    &s_1, &s_2, &s_3
    );

  return(s_2);
}

double comp_s3_3D(
    double s_x,
    double s_y,
    double s_z,
    double s_yz,
    double s_zx,
    double s_xy
    )
{
  static double s_1 ;
  static double s_2 ;
  static double s_3 ;

  femPrinc3D(
    s_x, s_y, s_z,
    s_yz, s_zx, s_xy,
    &s_1, &s_2, &s_3
    );

  return(s_3);
}

/* ---------------------------------- */

/** Tests if element of type "etype" has result called "test_val" (RES_SYZ or so)
 * @param er_pos element position in result data structure
 * @param etype element type ID (1 == LINK1 etc.)
 * @param test_val result type to be found in element
 * @return AF_YES if found, AF_NO if not
 */
int fdbResComp_Sel2D3D(long er_pos, long etype, long test_val)
{
  int j, num, rnum, rrep ;

  num  = fdbElementType[etype].res ;
  rnum = fdbElementType[etype].res_rp ;
  rrep = fdbResElemNumRepSets(er_pos, etype);

    for (j=0; j<num; j++)
    {
      if (fdbElementType[etype].nres[j] == test_val)
      {
        return(AF_YES);
      }
    }

    if (rrep > 0)
    {
      for (j=0; j<rnum; j++)
      {
        if (fdbElementType[etype].nres_rp[j] == test_val)
        {
          return(AF_YES);
        }
      }
    }

  return(AF_NO);
}

double fdbResComp_GetPrincStress(long er_pos, long i_rep, long etype, long stress_type)
{
  double s_x,s_y,s_z, s_xy,s_yz,s_zx ;

  /* can these data be computed here? */
  if (fdbResComp_Sel2D3D(er_pos, etype, RES_SXY) != AF_YES){return(AF_OK);}

  s_x  = fdbResElemGetVal0(er_pos, etype, RES_SX,  i_rep);
  s_y  = fdbResElemGetVal0(er_pos, etype, RES_SY,  i_rep);
  s_xy = fdbResElemGetVal0(er_pos, etype, RES_SXY, i_rep);

  if (fdbResComp_Sel2D3D(er_pos, etype, RES_SYZ) == AF_YES)
  {
    /* 3D */
    s_z  = fdbResElemGetVal0(er_pos, etype, RES_SZ,  i_rep);
    s_yz = fdbResElemGetVal0(er_pos, etype, RES_SYZ, i_rep);
    s_zx = fdbResElemGetVal0(er_pos, etype, RES_SZX, i_rep);

    switch (stress_type)
    {
      case RES_S1: return(comp_s1_3D(s_x,s_y,s_z,s_yz,s_zx,s_xy)); break ;
      case RES_S2: return(comp_s2_3D(s_x,s_y,s_z,s_yz,s_zx,s_xy)); break ;
      case RES_S3: return(comp_s3_3D(s_x,s_y,s_z,s_yz,s_zx,s_xy)); break ;
    }
  }
  else
  {
    /* 2D */
    switch (stress_type)
    {
      case RES_S1:   return(comp_s1_2D(s_x, s_y, s_xy)); break ;
      case RES_S2:   return(comp_s2_2D(s_x, s_y, s_xy));  break ;
      case RES_TMAX: return(comp_tmax_2D(s_x, s_y, s_xy)); break ;
    }
  }

  return(0.0);
}

double fdbResComp_GetPrincSlabMom(long er_pos, long i_rep, long etype, long stress_type)
{
  double sm_x,sm_y,sm_xy;

  /* can these data be computed here? */
  if (fdbResComp_Sel2D3D(er_pos, etype, RES_S_MX) != AF_YES){return(AF_OK);}

  sm_x  = fdbResElemGetVal0(er_pos, etype, RES_S_MX,  i_rep);
  sm_y  = fdbResElemGetVal0(er_pos, etype, RES_S_MY,  i_rep);
  sm_xy = fdbResElemGetVal0(er_pos, etype, RES_S_MXY, i_rep);

  
  /* 2D */
  switch (stress_type)
  {
    case RES_SM1:    return(comp_s1_2D(sm_x, sm_y, sm_xy)); break ;
    case RES_SM2:    return(comp_s2_2D(sm_x, sm_y, sm_xy));  break ;
    case RES_SMSMAX: return(comp_tmax_2D(sm_x, sm_y, sm_xy)); break ;
    case RES_SMXDIM: return(comp_dim_m_2D(sm_x, sm_xy)); break ;
    case RES_SMYDIM: return(comp_dim_m_2D(sm_y, sm_xy)); break ;
  }

  return(0.0);
}

double fdbResComp_vonMisesStress(long er_pos, long i_rep, long etype, long stress_type)
{
  double s_x,s_y,s_z, s_xy,s_yz,s_zx ;
  double s_1 = 0.0 ;
  double s_2 = 0.0 ;
  double s_3 = 0.0 ;

  /* can these data be computed here? */
  if (fdbResComp_Sel2D3D(er_pos, etype, RES_SXY) != AF_YES){return(0.0);}

  s_x  = fdbResElemGetVal0(er_pos, etype, RES_SX,  i_rep);
  s_y  = fdbResElemGetVal0(er_pos, etype, RES_SY,  i_rep);
  s_xy = fdbResElemGetVal0(er_pos, etype, RES_SXY, i_rep);

  if (fdbResComp_Sel2D3D(er_pos, etype, RES_SYZ) == AF_YES)
  {
    /* 3D */
    s_z  = fdbResElemGetVal0(er_pos, etype, RES_SZ,  i_rep);
    s_yz = fdbResElemGetVal0(er_pos, etype, RES_SYZ, i_rep);
    s_zx = fdbResElemGetVal0(er_pos, etype, RES_SZX, i_rep);

    s_1 = comp_s1_3D(s_x,s_y,s_z,s_yz,s_zx,s_xy) ;
    s_2 = comp_s2_3D(s_x,s_y,s_z,s_yz,s_zx,s_xy) ;
    s_3 = comp_s3_3D(s_x,s_y,s_z,s_yz,s_zx,s_xy) ;
  }
  else
  {
    /* 2D */
    s_1 = comp_s1_2D(s_x, s_y, s_xy) ;
    s_2 = comp_s2_2D(s_x, s_y, s_xy) ;
  }

  /* verified: the same result as in Jirasek pp. 88 */
  return(sqrt(0.5*(pow(s_1-s_2,2)+pow(s_2-s_3,2)+pow(s_3-s_1,2))));
}

/** Computes the result:
 * @param er_pos result position
 * @param i_rep repeating number (0=non-repeating, 1,2,..=repeating)
 * @param etype type of element (fdbElementType)
 * @param stress_type type of result (not only stress)
 */
double fdbResComp_Val(long er_pos, long i_rep, long etype, long stress_type)
{
  switch(stress_type)
  {
    /* principal stresses */
    case RES_S1:
    case RES_S2:
    case RES_S3:
    case RES_TMAX:
      return( fdbResComp_GetPrincStress(er_pos, i_rep, etype, stress_type));
      break ;

    /* principal/design slab moments */
    case RES_SM1:
    case RES_SM2:
    case RES_SMSMAX: 
    case RES_SMXDIM: 
    case RES_SMYDIM:
      return( fdbResComp_GetPrincSlabMom(er_pos, i_rep, etype, stress_type));
      break ;

    /* von Mises stress */
    case RES_SVMIS:
      return(fdbResComp_vonMisesStress(er_pos,i_rep,etype,stress_type));
      break ;
  }

  return(0.0);
}

/* end of fdb_cres.c */
