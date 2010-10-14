/*
   File name: fdb_fdef.c
   Date:      2003/08/22 13:52
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

	 FEM Database - default input values

	 $Id: fdb_fdef.c,v 1.6 2005/02/12 18:34:50 jirka Exp $
*/

#include "fdb_fem.h"
#include "fdb_edef.h"

long fdbInputDefET   = 0 ; /* default element type (for ELEM) */
long fdbInputDefRS   = 0 ; /* default real set (for ELEM)     */
long fdbInputDefMAT  = 0 ; /* default material (for ELEM)     */
long fdbInputLoadSet = 1 ; /* default load cast  */
long fdbInputDefDiv  = 10; /* default division of geom. entities  */
long fdbInputDefGEnt = 0 ; /* default type of geom. entity  */

/** Sets default element type
	* @param set_to value to be used (0 for current value)
	* @return value
 */
long fdbSetInputDefET(long set_to)
{
	static long value  = 0;

	if (set_to > 0) { fdbInputDefET = set_to ; }

	if (fdbInputDefET <= 0)
	{
		fdbInputDefET = fdbInputFindMaxInt(ETYPE, ETYPE_ID) ;
		if (fdbInputDefET <= 0)
		{
			fdbInputDefET = 0 ;
			/* ERROR - NOTHING IS DEFINED !!! */
		}
	}

	/* setting final value */
	value = fdbInputDefET ;
	return(value);
}

/** Sets default real set
	* @param set_to value to be used (0 for current value)
	* @return value
 */
long fdbSetInputDefRS(long set_to)
{
	static long value  = 0;

	if (set_to > 0) { fdbInputDefRS = set_to ; }

	if (fdbInputDefRS <= 0)
	{
		fdbInputDefRS = fdbInputFindMaxInt(RSET, RSET_ID) ;
		if (fdbInputDefRS <= 0)
		{
			fdbInputDefRS = 0 ;
			/* ERROR - NOTHING IS DEFINED !!! */
		}
	}

	/* setting final value */
	value = fdbInputDefRS ;
	return(value);
}

/** Sets default material
	* @param set_to value to be used (0 for current value)
	* @return value
 */
long fdbSetInputDefMAT(long set_to)
{
	static long value  = 0;

	if (set_to > 0) { fdbInputDefMAT = set_to ; }

	if (fdbInputDefMAT <= 0)
	{
		fdbInputDefMAT = fdbInputFindMaxInt(MAT, MAT_ID) ;
		if (fdbInputDefMAT <= 0)
		{
			fdbInputDefMAT = 0 ;
			/* ERROR - NOTHING IS DEFINED !!! */
		}
	}

	/* setting final value */
	value = fdbInputDefMAT ;
	return(value);
}

/** Sets default set for finite elements
	* @param set_to value to be used (0 for current value)
	* @return value
 */
long fdbSetInputDefEset(long set_to)
{
	static long value  = 0;

	if (set_to > 0) { fdbInputLoadSet = set_to ; }

  /* probably incorrect:
	if (fdbInputLoadSet <= 0) { fdbInputLoadSet = 1 ; }
  */

	/* setting final value */
	value = fdbInputLoadSet ;
	return(value);
}

/** Sets default set for finite elements
	* @param set_to value to be used (0 for current value)
	* @return value
 */
long fdbSetInputLoadSet(long set_to)
{
	static long value  = 0;

	if (set_to > 0) { fdbInputLoadSet = set_to ; }

  /* probably incorrect:
	if (fdbInputLoadSet <= 0) { fdbInputLoadSet = 1 ; }
  */

	/* setting final value */
	value = fdbInputLoadSet ;
	return(value);
}

/** Sets default set for finite elements
	* @param set_to value to be used (0 for current value)
	* @return value
 */
long fdbSetInputDefDiv(long set_to)
{
	static long value  = 0;

	if (set_to > 0) { fdbInputDefDiv = set_to ; }

	if (fdbInputDefDiv <= 0) { fdbInputLoadSet = 1 ; }

	/* setting final value */
	value = fdbInputDefDiv ;
	return(value);
}

/** Sets default type of geometry entity
	* @param set_to value to be used (0 for current value)
	* @return value
 */
long fdbSetInputDefGEnt(long set_to)
{
	static long value  = 0;
  long tmp = 0 ;
  long len = 0 ;

	if (set_to > 0) { fdbInputDefGEnt = set_to ; }

	if (fdbInputDefGEnt <= 0)
	{
    if ((len=fdbInputTabLenAll(ETYPE)) > 0)
    {
      tmp = fdbInputGetInt(ETYPE, ETYPE_TYPE, len-1) ;

      if (tmp < fdbElementTypeLen)
      {
		    fdbInputDefGEnt =  fdbElementType[tmp].getype ;
      }
      else
      {
        fdbInputDefGEnt = 0 ;
      }
    }
    else
    {
      fdbInputDefGEnt = 0 ;
    }

		if (fdbInputDefGEnt <= 0)
		{
			fdbInputDefGEnt = 1 ;
			/* ERROR - NOTHING IS DEFINED (line by default) !!! */
		}
	}

	/* setting final value */
	value = fdbInputDefGEnt ;
	return(value);
}

/* end of fdb_fdef.c */
