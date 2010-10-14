/*
   File name: fdb_test.c
   Date:      2004/12/13 20:56
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

   Database for FEM - testing of  data validity

   $Id: fdb_test.c,v 1.3 2005/07/11 17:57:02 jirka Exp $
*/

#include "fdb_fem.h"

/* Some more tests should be here:
 * - if real sets are related to element type
 * - is some element types are defined
 * - if elements have nonempty nodes
 * .....
 */

/** Tests if all materials have at least some nonzero data
 * @return status
 */
int fdb_test_mat(void)
{
  int rv = AF_OK ;
  long i, j ;
  long pos, len ;
  long nonzero = AF_NO ;

#ifdef RUN_VERBOSE
  if (fdbInputTabLenAll(MAT) < 1)
  {
    fprintf(msgout, "[E] %s!\n", _("No materials at all"));
    return(rv) ;
  }

  for (i=0; i<fdbInputTabLenAll(MAT); i++)
  {
    if ((len=fdbInputCountIntAll(MVAL, MVAL_MAT,
						fdbInputGetInt(MAT, MAT_ID, i),
						&pos)) <= 0)
    {
      rv = AF_ERR_VAL ;
      fprintf(msgout, "[E] %s: %li!\n", _("Undefined material"),
          fdbInputGetInt(MAT, MAT_ID, i));
      return(rv) ;
    }

    nonzero = AF_NO ;
    for (j=pos ; j<(pos+len); j++)
    {
      if (fdbInputGetDbl(MVAL, MVAL_VAL, j) != 0.0)
      {
        nonzero = AF_YES ;
      }
    }

    if (nonzero != AF_YES)
    {
      rv = AF_ERR_VAL ;
      fprintf(msgout, "[W] %s: %li!\n", _("Empty data for material"),
          fdbInputGetInt(MAT, MAT_ID, i));
    }
  }

#endif
  return(rv);
}

/** Tests if all real sets have at least some nonzero data
 * @return status
 */
int fdb_test_rset(void)
{
  int rv = AF_OK ;
  long i, j ;
  long pos, len ;
  long nonzero = AF_NO ;

#ifdef RUN_VERBOSE
  for (i=0; i<fdbInputTabLenAll(RSET); i++)
  {
    if ((len=fdbInputCountIntAll(RVAL, RVAL_RSET, 
						fdbInputGetInt(RSET, RSET_ID, i),
						&pos)) <= 0)
    {
      rv = AF_ERR_VAL ;
      fprintf(msgout, "[E] %s: %li!\n", _("Undefined real set"),
          fdbInputGetInt(RSET, RSET_ID, i));
      return(rv) ;
    }

    nonzero = AF_NO ;
    for (j=pos ; j<(pos+len); j++)
    {
      if (fdbInputGetDbl(RVAL, RVAL_VAL, j) != 0.0)
      {
        nonzero = AF_YES ;
      }
    }

    if (nonzero != AF_YES)
    {
      rv = AF_ERR_VAL ;
      fprintf(msgout, "[W] %s: %li!\n", _("Empty data for real set"),
          fdbInputGetInt(RSET, RSET_ID, i));
    }
  }

#endif
  return(rv);
}

/** Tests if all real sets have at least some nonzero data
 * @return status
 */
int fdb_test_n_e_d(void)
{
  int rv = AF_OK ;
  long i, j ;
  long num ;

#ifdef RUN_VERBOSE
  if ((num=fdbInputTabLenSel(NODE)) < 2)
	{
    rv = AF_ERR_VAL ;
    fprintf(msgout, "[E] %s: %li!\n", _("Insuficient number of nodes"), num);
		return(rv);
	}

	if ((num=fdbInputTabLenSel(ELEM)) < 1)
	{
    rv = AF_ERR_VAL ;
    fprintf(msgout, "[E] %s: %li!\n", _("Insuficient number of elements"), num);
		return(rv);
	}
	
	if ((num=fdbInputTabLenSel(NDISP)) < 3)
	{
    rv = AF_ERR_VAL ;
    fprintf(msgout, "[E] %s: %li!\n", _("Insuficient number of boundary conditions"), num);
		return(rv);
	}

	if ((num=fdbInputTabLenAll(NLOAD)+fdbInputTabLenSel(ELOAD)+fdbInputTabLenSel(GRAV)) < 1)
	{
    fprintf(msgout, "[w] %s.\n", _("No loads found"));
	}

  femOneDirSupp = 0 ;

  for (i=0; i<fdbInputTabLenAll(NDISP); i++)
  {
    j = fdbInputGetInt(NDISP, NDISP_TYPE, i);
    if ((j >=13) && (j <=36)) 
    {
      femOneDirSupp = 1 ; 
      break ;
    }
  }

#endif
  return(rv);
}

int fdb_test_data(void)
{
	int rv1 = AF_OK ;
	int rv  = AF_OK ;

  if ((rv1=fdb_test_mat()) != AF_OK) { rv = rv1 ;}
  if ((rv1=fdb_test_rset()) != AF_OK) { rv = rv1 ;}
  if ((rv1=fdb_test_n_e_d()) != AF_OK) { rv = rv1 ;}

	return(rv);
}


/* end of fdb_test.c */
