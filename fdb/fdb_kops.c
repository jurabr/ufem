/*
   File name: fdb_kops.c
   Date:      Wed Apr 28 19:29:50 CEST 2004
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

	 FEM - keypoint copying, mirroring etc.
*/

#include "fdb_fem.h"
#include <math.h>

/* from fdb_nops.c: */
extern long   fdbCoordSysType ; /* coordinate system type - from "fdb_csys.c" */
extern double fdbDistTol ;  /* lenght to test entities as identical */
extern double fem_signum(double val);


int compute_k_coords(long kpoint_pos,
                     long copy_num, long mode, long dir, 
                     double dx, double dy, double dz,
                     double dx1, double dy1, double dz1,
                     double *ox, double *oy, double *oz)
{
  int rv = AF_OK ;
	double ddx, ddy, ddz ;
  double copy_val = 1 ;
  double r, phi, z ;

  if (mode == FDB_COPY)
  {
    copy_val = (double) (copy_num+1) ;

    if (fdbCoordSysType == FDB_CSYS_CART)
    {
	    ddx = copy_val*dx + ( fem_signum(dx1) * pow (copy_val*dx1, 2 ) ) ;
	    ddy = copy_val*dy + ( fem_signum(dy1) * pow (copy_val*dy1, 2 ) ) ;
	    ddz = copy_val*dz + ( fem_signum(dz1) * pow (copy_val*dz1, 2 ) ) ;

	    *ox = fdbInputGetDbl(KPOINT, KPOINT_X, kpoint_pos) + ddx ;
	    *oy = fdbInputGetDbl(KPOINT, KPOINT_Y, kpoint_pos) + ddy ;
	    *oz = fdbInputGetDbl(KPOINT, KPOINT_Z, kpoint_pos) + ddz ;
    }
    else
    {
      fdbCSysGetCylCoord(
	        fdbInputGetDbl(KPOINT, KPOINT_X, kpoint_pos),
	        fdbInputGetDbl(KPOINT, KPOINT_Y, kpoint_pos),
	        fdbInputGetDbl(KPOINT, KPOINT_Z, kpoint_pos),
          &r, &phi, &z);

      *ox = r ;
      *oy = (180.0*phi/FEM_PI)+(copy_val*dx) ;
      *oz = z ;
    }
  }
  else
  {
    if (dir == U_X)
    {
	    ddx = fdbInputGetDbl(KPOINT, KPOINT_X, kpoint_pos) ;
      *ox = dx + (dx - ddx) ;
    }
    else
    {
	    *ox = fdbInputGetDbl(KPOINT, KPOINT_X, kpoint_pos) ;
    }

    if (dir == U_Y)
    {
	    ddy = fdbInputGetDbl(KPOINT, KPOINT_Y, kpoint_pos) ;
      *oy = dy + (dy - ddy) ;
    }
    else
    {
	    *oy = fdbInputGetDbl(KPOINT, KPOINT_Y, kpoint_pos) ;
    }

    if (dir == U_Z)
    {
	    ddz = fdbInputGetDbl(KPOINT, KPOINT_Z, kpoint_pos) ;
      *oz = dz + (dz - ddz) ;
    }
    else
    {
	    *oz = fdbInputGetDbl(KPOINT, KPOINT_Z, kpoint_pos) ;
    }
  }

  return(rv);
}

/** Creates new kpoints by making copies of selected ones
 * @param num number of copies
 * @param dx distance in x direction
 * @param dy distance in y direction
 * @param dz distance in z direction
 * @return (status)
 */
int f_k_gen_1d(long num, long mode, long dir, double dx, double dy, double dz, 
    double dx1, double dy1, double dz1)
{
	int  rv = AF_OK ;
	long nkpoints ;
	long i,j ;
	double ox, oy, oz ;
	long count ;

	if ((nkpoints = fdbInputTabLenAll(KPOINT)) <= 0)
	{
		fprintf(msgout, "[E] %s!\n", _("No selected keypoints to be copied"));
		return(AF_ERR) ;
	}

	count = 0 ;

	for (i=0; i<num; i++)
	{

		for (j=0; j<nkpoints; j++)
		{
			if (fdbInputTestSelect(KPOINT, j) != AF_YES) {continue;}

      compute_k_coords(j,
                     i, mode, dir, 
                     dx, dy, dz,
                     dx1, dy1, dz1,
                     &ox, &oy, &oz) ;



			if ((rv=f_k_new_change(0, ox, oy, oz, NULL)) == AF_OK)
			{
				count++ ;
			}
			else
			{
				fprintf(msgout, "[E] %s!\n", _("Copying of kpoint failed"));
				return(rv);
			}
		}
	}


#ifdef RUN_VERBOSE
	fprintf(msgout, "[ ] %s: %li\n", _("Number of new kpoints"), count) ;
#endif
	return(rv) ;
}

/** Returns position of first kpoint at [x, y, z]
 * @param x wanted x coordinate
 * @param y wanted y coordinate
 * @param z wanted z coordinate
 * @param test returns AF_YES if found, AF_NO if not
 * @return kpoint position
 */
long found_suitable_kpoint(double x, double y, double z, double tol, int *test)
{
	long kpoint            = -1 ;
	long kpoint_not_so_far = -1 ;
	long i ;
	double nx, ny,nz, ndist ;

	*test = AF_YES ;

	for (i=0; i< fdbInputTabLenAll(KPOINT); i++)
	{
		/* unselected kpoints are ignored */
		if (fdbInputTestSelect(KPOINT,i) != AF_YES) {continue;}

		nx = fdbInputGetDbl(KPOINT, KPOINT_X, i) ;
		ny = fdbInputGetDbl(KPOINT, KPOINT_Y, i) ;
		nz = fdbInputGetDbl(KPOINT, KPOINT_Z, i) ;

		ndist = sqrt ( (x-nx)*(x-nx) + (y-ny)*(y-ny) + (z-nz)*(z-nz) ) ;

		if (ndist < fabs(tol))
		{
			kpoint_not_so_far = i ;
		}

		if (ndist == 0.0)
		{
			kpoint = i ;
			return(kpoint) ;
		}
	}
	
	kpoint = kpoint_not_so_far ;

	if (kpoint == -1)
	{
		*test = AF_NO ;
	}

	return(kpoint) ;
}

/** Creates new elements (and kpoints) by making copies of selected ones
 * @param num number of copies
 * @param dx distance in x direction
 * @param dy distance in y direction
 * @param dz distance in z direction
 * @return (status)
 */
int f_ent_k_gen_1d(long num, long mode, long dir, double dx, double dy, double dz, 
    double dx1, double dy1, double dz1)
{
	int    rv = AF_OK ;
	long   nelem ;
	long   i,j,k, jj ;
	double ox, oy, oz ;
	long   count, ncount, nncount ;
	long   ekpoints ;
	long  *ekpointlist = NULL ;
  long   en_div_len = 0 ;
  long   en_div[10] ;
  long   posn ; /*for "en_div" */
	long   npos ;
	long   e_id ;
	long   n_id ;
	int    test;
	double tol ;
	long   enttype, etype, rset,mat,egrp;

	tol = fdbDistTol ;

	if ((nelem = fdbInputTabLenAll(ENTITY)) <= 0)
	{
		fprintf(msgout, "[E] %s!\n", _("No selected kpoints to be copied"));
		return(AF_ERR) ;
	}

	count = 0 ;
	ncount = 0 ;
	nncount = 0 ;

	for (i=0; i<num; i++)
	{
		for (j=0; j<nelem; j++)
		{
			if (fdbInputTestSelect(ENTITY, j) != AF_YES) {continue;}

			e_id = fdbInputGetInt(ENTITY, ENTITY_ID, j ) ;

			if ((ekpoints = fdbInputGetInt(ENTITY, ENTITY_KPS, j)) < 1)
			{
				fprintf(msgout, "[E] %s: %li!\n", _("Invalid kpoint found"), e_id);
				return(AF_ERR_VAL);
			}

			if ((ekpointlist = femIntAlloc(ekpoints)) == NULL)
			{
				fprintf(msgout, "[E] %s!\n", _("Out of memory"));
				return(AF_ERR_MEM);
			}

      npos = fdbInputGetInt(ENTITY, ENTITY_KFROM, j);

			for (k=0; k<ekpoints; k++)
			{
				ekpointlist[k] = fdbInputGetInt(ENTKP, ENTKP_ENT, npos+k) ;
			}

			for (k=0; k<ekpoints; k++)
			{
        npos = fdbEntKpPos(j, k) ;
        
        compute_k_coords(npos,
                     i, mode, dir, 
                     dx, dy, dz,
                     dx1, dy1, dz1,
                     &ox, &oy, &oz) ;

				npos =  found_suitable_kpoint(ox, oy, oz, tol, &test) ;

				if (test == AF_YES)
				{
					/* existing kpoint */
				  n_id = fdbInputGetInt(KPOINT, KPOINT_ID, npos) ;
				}
				else
				{
					/* new kpoint */
					if ((rv=f_k_new_change(0, ox, oy, oz, NULL)) == AF_OK)
					{
						nncount++ ;
					}
					else
					{
						fprintf(msgout, "[E] %s!\n", _("Copying of keypoint failed"));
            femIntFree(ekpointlist) ;
						return(rv);
					}
					n_id = fdbInputFindMaxInt(KPOINT, KPOINT_ID) ;
				}

				ekpointlist[k] = n_id ;
			}

      /* NEW TESTING CODE:  ******************** */
#if 0
      for (k=0; k<(int)(ekpoints/2); k++)
      {
        npos = ekpointlist[k] ;
        ekpointlist[k] = ekpointlist[ekpoints-k] ;
        ekpointlist[ekpoints-k] = npos ;
      }
#endif
      /* END OF NEW TESTING CODE *************** */


			enttype = fdbInputGetInt(ENTITY, ENTITY_TYPE, j) ;
			etype   = fdbInputGetInt(ENTITY, ENTITY_ETYPE, j) ;
			rset    = fdbInputGetInt(ENTITY, ENTITY_RS, j) ;
			mat     = fdbInputGetInt(ENTITY, ENTITY_MAT, j) ;
			egrp    = fdbInputGetInt(ENTITY, ENTITY_SET, j) ;


      /* entity division: */
      if ((en_div_len=fdbInputCountInt(ENTDIV, ENTDIV_ENT, e_id, &posn)) < 1)
      {
        en_div_len = -1 ; /* nothing to do */
      }
      if (en_div_len > 10) { en_div_len = 10 ; } /* security check - fiield size */

      if (en_div_len > 0)
      {
        for (jj=0; jj< en_div_len; jj++)
        {
          en_div[jj] = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+jj) ;
        }
      }


      /** NEW ENTITY: */
			
			if ((rv=f_ent_new_change(0, enttype, etype, rset, mat,  egrp)) != AF_OK)
			{
				fprintf(msgout, "[E] %s!\n", _("Copying of entity failed (on whole element)"));
				femIntFree(ekpointlist) ;
				return(rv);
			}
			
			e_id = fdbInputFindMaxInt(ENTITY, ENTITY_ID) ;

			if ((rv=f_entkp_change(e_id, ekpointlist, ekpoints,enttype))  != AF_OK)
			{
				fprintf(msgout, "[E] %s!\n", _("Copying of entity failed (on kpoints)"));
				femIntFree(ekpointlist) ;
				return(rv);
			}

      if (en_div_len > 0) 
      { 
        f_entkp_div_change(e_id, en_div, en_div_len) ; 
      }

			count++ ;
			femIntFree(ekpointlist) ;
		}
	}

#ifdef RUN_VERBOSE
	fprintf(msgout, "[ ] %s: %li\n", _("Number of new keypoints"), nncount) ;
	fprintf(msgout, "[ ] %s: %li\n", _("Number of new entities"), count) ;
#endif

	return(rv) ;
}

/** Moves keypoints 
 * @param nPos keypoint position
 * @param dx x-move
 * @param dy y-move
 * @param dz z-move
 * @return status
 * */
int k_move_coord(long nPos, double dx, double dy, double dz)
{
  int rv = AF_OK ;

  if (fdbInputTestSelect(KPOINT,nPos) != AF_YES) {return(AF_OK);}

	fdbInputDblChangeVals(KPOINT, KPOINT_X, nPos, 1, &dx, AF_YES);
	fdbInputDblChangeVals(KPOINT, KPOINT_Y, nPos, 1, &dy, AF_YES);
	fdbInputDblChangeVals(KPOINT, KPOINT_Z, nPos, 1, &dz, AF_YES);

  return(rv);
}


/** Moves keypoints 
 * @param nPos keypoint position
 * @param dx x-move
 * @param dy y-move
 * @param dz z-move
 * @return status
 * */
int k_rotate_coord(long nPos, long plane, double angle, double x0, double y0, double z0)
{
  int rv = AF_OK ;
	double x,y,z, x1,y1,z1 ;
	double r, alpha ;

  if (fdbInputTestSelect(KPOINT,nPos) != AF_YES) {return(AF_OK);}

	x = fdbInputGetDbl(KPOINT, KPOINT_X, nPos) ;
	y = fdbInputGetDbl(KPOINT, KPOINT_Y, nPos) ;
	z = fdbInputGetDbl(KPOINT, KPOINT_Z, nPos) ;

	x1 = x ;
	y1 = y ;
	z1 = z ;

	switch (plane)
	{
		case 3: /* xy */
			r = sqrt(pow(y-y0, 2) + pow(x-x0, 2)) ;
			if (r < FEM_ZERO) {break;}
			if (fabs(x-x0) < FEM_ZERO/100)
			{
				if ((y-y0) > 0) { alpha = FEM_PI/2.0 ;	}
				else {alpha = -FEM_PI/2.0;}
			}
			else { alpha = atan( (y-y0) / (x-x0) ) ; }
			if ((fabs (alpha) < FEM_ZERO) && ((x-x0) < 0.0)) {alpha = FEM_PI;}

			x1 = x0 + r * cos(alpha + (FEM_PI*angle/180.0)) ;
			y1 = y0 + r * sin(alpha + (FEM_PI*angle/180.0)) ;
			z1 = z ;
			break ;
			
		case 2: /* xz */
			r = sqrt(pow(z-z0, 2) + pow(x-x0, 2)) ;
			if (r < FEM_ZERO) {break;}
			if (fabs(x-x0) < FEM_ZERO)
			{
				if ((z-z0) > 0) { alpha = FEM_PI/2.0 ;	}
				else {alpha = -FEM_PI/2.0;}
			}
			else { alpha = atan( (z-z0) / (x-x0) ) ; }
			if ((fabs (alpha) < FEM_ZERO) && ((x-x0) < 0.0)) {alpha = FEM_PI;}

			x1 = x0 + r * cos(alpha + (FEM_PI*angle/180.0)) ;
			y1 = y ;
			z1 = z0 + r * sin(alpha + (FEM_PI*angle/180.0)) ;
			break ;

		case 1: /* yz */
			r = sqrt(pow(z-z0, 2) + pow(y-y0, 2)) ;
			if (r < FEM_ZERO) {break;}
			if (fabs(y-y0) < FEM_ZERO)
			{
				if ((z-z0) > 0) { alpha = FEM_PI/2.0 ;	}
				else {alpha = -FEM_PI/2.0;}
			}
			else { alpha = atan( (z-z0) / (y-y0) ) ; }
			if ((fabs (alpha) < FEM_ZERO) && ((y-y0) < 0.0)) {alpha = FEM_PI;}

			x1 = x ;
			y1 = y0 + r * cos(alpha + (FEM_PI*angle/180.0)) ;
			z1 = z0 + r * sin(alpha + (FEM_PI*angle/180.0)) ;
			break ;
		default:
			rv = AF_ERR_VAL ;
			break;
	}

	fdbInputPutDbl(KPOINT, KPOINT_X, nPos,  x1);
	fdbInputPutDbl(KPOINT, KPOINT_Y, nPos,  y1);
	fdbInputPutDbl(KPOINT, KPOINT_Z, nPos,  z1);

  return(rv);
}



/* end of fdb_kops.c */
