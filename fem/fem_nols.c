/*
   File name: fem_nols.c
   Date:      2006/11/05 16:02
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

	 Simple nonlocal-like material model procedures (for clay bricks and so)
*/

#include "fem_elem.h"
#include "fem_mat.h"

#ifndef _SMALL_FEM_CODE_

long *nols_brick_type  = NULL ;
long *nols_brick_brck   = NULL ;

/** 
 */ 
long nols_brick_near_2d(long mat_id, long *br_list, long br_list_max)
{
	static long num = 0;

	return(num);
}



#endif /* SMALL_FEM_CODE */

/* end of fem_nols.c */
