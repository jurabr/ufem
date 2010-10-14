/*
   File name: fdb_deps.c
   Date:      2003/08/09 15:07
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

	 FEM Database - testing of data dependencies

	 $Id: fdb_deps.c,v 1.5 2004/12/29 19:03:31 jirka Exp $
*/

#include "fdb.h"
#include "fdb_fem.h"

/* Tests if something depends on this value
 * @param val value to be tested 
 * @param tested_tab table to find dependency
 * @param tested_col column to find dependency
 * @return AF_YES (depends) or AF_NO (no dependencies)
 */
int fdbInputTestDep(long val, long tested_tab, long tested_col)
{
	long pos ;

	/* obviously, it's slow - but it is secure, too: */
	if (fdbInputCountIntAll(tested_tab, tested_col, val, &pos) > 0)
	{
		return(AF_YES);
	}
	else
	{
		return(AF_NO);
	}
}

/* end of fdb_deps.c */
