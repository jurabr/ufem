/*
   File name: fdb_csys.c
   Date:      2004/01/23 09:22
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

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

	 Database for FEM: coordinate systems

	 $Id: fdb_csys.c,v 1.4 2004/04/28 19:18:41 jirka Exp $
*/

#include <math.h>
#include "fdb.h"

/* Coordinate System Type */
long   fdbCoordSysType = FDB_CSYS_CART ;

/* Center of the coordinate systems position */
double fdbCoordSysX    = 0.0 ;
double fdbCoordSysY    = 0.0 ;
double fdbCoordSysZ    = 0.0 ;

/** Sets default properties of the coordinate system */
void fdbCoodSysReset(void)
{
	fdbCoordSysType = FDB_CSYS_CART ;

	fdbCoordSysX    = 0.0 ;
	fdbCoordSysY    = 0.0 ;
	fdbCoordSysZ    = 0.0 ;
}

/** Sets properties of the coordinate system
 * @param type type of coord. system
 * @param x
 * @param y
 * @param z
 * @return status
 * */
int fdbCoodSysSet(long type, double x, double y, double z)
{
	fdbCoordSysType = type ;

	fdbCoordSysX    = x ;
	fdbCoordSysY    = y ;
	fdbCoordSysZ    = z ;

  return(AF_OK);
}

/** Computes cartesian coordinates from given data 
 * @param x   given 1st coordinate
 * @param y   given 2nd coordinate
 * @param z   given 3rd coordinate
 * @param n_x cartesian x (result)
 * @param n_y cartesian y (result)
 * @param n_z cartesian z (result)
 * @return    status
 */
int fdbCSysGetXYZ(double    x, double    y, double    z, 
		              double *n_x, double *n_y, double *n_z)
{
	switch (fdbCoordSysType)
	{
		case FDB_CSYS_CYL_XY:  
													 *n_x = x * cos (FEM_PI*y/180.0) + fdbCoordSysX ;
													 *n_y = x * sin (FEM_PI*y/180.0) + fdbCoordSysY ;
													 *n_z = z + fdbCoordSysZ ;
													 break ;
		case FDB_CSYS_CYL_YZ:  
													 *n_y = x * cos (FEM_PI*y/180.0) + fdbCoordSysY ;
													 *n_z = x * sin (FEM_PI*y/180.0) + fdbCoordSysZ ;
													 *n_x = z + fdbCoordSysX ;
													 break ;
		case FDB_CSYS_CYL_ZX:  
													 *n_z = x * cos (FEM_PI*y/180.0) + fdbCoordSysZ ;
													 *n_x = x * sin (FEM_PI*y/180.0) + fdbCoordSysX ;
													 *n_y = z + fdbCoordSysY ;
													 break ;

		case FDB_CSYS_CART: 
		default: 
													 *n_x = fdbCoordSysX + x ;
													 *n_y = fdbCoordSysY + y ;
													 *n_z = fdbCoordSysZ + z ;
													 break ;
	}
	return(AF_OK);
}

/** Computes cartesian coordinates from given data (angles are in radians!)
 * @param x   cartesian x
 * @param y   cartesian y
 * @param z   cartesian z
 * @param n_x 1st coordinate (result)
 * @param n_y 2nd coordinate (result)
 * @param n_z 3rd coordinate (result)
 * @return    status
 */
int fdbCSysGetCylCoord(double    x, double    y, double    z, 
		              		 double *n_x, double *n_y, double *n_z)
{
	switch (fdbCoordSysType)
	{
		case FDB_CSYS_CYL_XY:  
													 *n_x = sqrt ((x*x)+(y*y)) ;

													 if ( x == 0.0 )
													 {
														 if (y >= 0.0) { *n_y = FEM_PI / 2.0; }
														 else { *n_y = FEM_PI * (3.0 / 2.0) ; }
													 }
													 else { *n_y = atan(y/x) ; }

													 *n_z = z ;
													 break ;
		case FDB_CSYS_CYL_YZ:  
													 *n_x = sqrt ((z*z)+(y*y)) ;
													 
													 if ( y == 0.0 )
													 {
														 if (z >= 0.0) { *n_y = FEM_PI / 2.0; }
														 else { *n_y = FEM_PI * (3.0 / 2.0) ; }
													 }
													 else { *n_y = atan(z/y) ; }

													 *n_z = x ;
													 break ;
		case FDB_CSYS_CYL_ZX:  
													 *n_x = sqrt ((x*x)+(z*z)) ;

													 if ( z == 0.0 )
													 {
														 if (x >= 0.0) { *n_y = FEM_PI / 2.0; }
														 else { *n_y = FEM_PI * (3.0 / 2.0) ; }
													 }
													 else { *n_y = atan(x/z) ; }

													 *n_z = y ;
													 break ;

		case FDB_CSYS_CART: 
		default: 
													 *n_x = x ;
													 *n_y = y ;
													 *n_z = z ;
													 break ;
	}
	return(AF_OK);
}

/** Computes cartesian coordinates from given data (angles are in DEG!)
 * @param x   cartesian x
 * @param y   cartesian y
 * @param z   cartesian z
 * @param n_x 1st coordinate (result)
 * @param n_y 2nd coordinate (result)
 * @param n_z 3rd coordinate (result)
 * @return    status
 */
int fdbCSysGetCylCoordDEG(double    x, double    y, double    z, 
		              		 double *n_x, double *n_y, double *n_z)
{
  int rv = AF_OK ;

  rv = fdbCSysGetCylCoord(x, y, z, n_x, n_y, n_z) ;

	if (fdbCoordSysType != FDB_CSYS_CART)
	{
    *n_y = 180.0 * (*n_y) / FEM_PI ;
  }

  return(rv);
}


/** Returnes string specification of the active coordinate system */
char *fdbCSysTypeStr(void)
{
  switch ( fdbCoordSysType )
  {
    case FDB_CSYS_CYL_XY: return("CS: CYL XY");
                          break ;
    case FDB_CSYS_CYL_YZ: return("CS: CYL YZ");
                          break ;
    case FDB_CSYS_CYL_ZX: return("CS: CYL ZX");
                          break ;
    case FDB_CSYS_CART: 
    default:              return("CS: CART");
                          break;
  }
}

/** Prints specification and data of the active coordinate system to
 * output*/
void fdbCSysTypeList(FILE *fw)
{
  fprintf(fw,"[I] %s: ",_("Coordinate system"));

  switch ( fdbCoordSysType )
  {
    case FDB_CSYS_CYL_XY: fprintf(fw, "%s xy [%f, %f, %f]\n", 
                              _("cyllindric"),
                              fdbCoordSysX, fdbCoordSysY, fdbCoordSysZ);
                          break ;
    case FDB_CSYS_CYL_YZ: fprintf(fw, "%s yz [%f, %f, %f]\n", 
                              _("cyllindric"),
                              fdbCoordSysX, fdbCoordSysY, fdbCoordSysZ);
                          break ;
    case FDB_CSYS_CYL_ZX: fprintf(fw, "%s zx [%f, %f, %f]\n", 
                              _("cyllindric"),
                              fdbCoordSysX, fdbCoordSysY, fdbCoordSysZ);
                          break ;
    case FDB_CSYS_CART: 
    default:              fprintf(fw, "%s [%f, %f, %f]\n", 
                              _("cartesian"),
                              fdbCoordSysX, fdbCoordSysY, fdbCoordSysZ);
                          break;
  }
}

/** Prints specification and data of the active coordinate system to
 * output*/
int fdbCSysTypePrn(FILE *fw)
{
  int rv = AF_OK ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 4, _("Coordinate system"));
  
  fdbPrnTableHeaderItem(fw, 0, _("Type")) ;
  fdbPrnTableHeaderItem(fw, 1, _("X")) ;
  fdbPrnTableHeaderItem(fw, 2, _("Y")) ;
  fdbPrnTableHeaderItem(fw, 3, _("Z")) ;

  switch ( fdbCoordSysType )
  {
    case FDB_CSYS_CYL_XY: 
                          fdbPrnTableItemStr(fw,0,
                              _("cyllindric"));
                          break ;
    case FDB_CSYS_CYL_YZ: 
                          fdbPrnTableItemStr(fw,0,
                              _("cyllindric"));
                          break ;
    case FDB_CSYS_CYL_ZX: 
                          fdbPrnTableItemStr(fw,0,
                              _("cyllindric"));
                          break ;
    case FDB_CSYS_CART: 
    default:              
                          fdbPrnTableItemStr(fw,0,
                              _("cartesian"));
                          break;
  }

  fdbPrnTableItemDbl(fw,1,fdbCoordSysX);
  fdbPrnTableItemDbl(fw,2,fdbCoordSysY);
  fdbPrnTableItemDbl(fw,3,fdbCoordSysZ);

  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/* end of fdb_csys.c */
