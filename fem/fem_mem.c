/*
   File name: fem_mem.c
   Date:      2003/04/07 10:16
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

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

	FEM Solver - memory handling
  $Id: fem_mem.c,v 1.5 2004/07/06 21:03:44 jirka Exp $
*/

#include "fem_mem.h"

FILE *msgout    = NULL ;

/* 1D fields ---------------------------------------------------------- */

/** allocates and returns 1D int field  (NULL if failed) 
 * @param length length of field
 * @returns field (or NULL)
 */
long *femIntAlloc(long length)
{
	long *field = NULL;
	long  i;

	if (length < 1) { return(NULL); }

	if ((field = (long *) malloc(length * sizeof(long))) == NULL)
	{ 
		return(NULL);  
	}
	else                                                    
	{ 
		for (i = 0; i < length; i++) { field[i] = 0; }
		return(field); 
	}
}

/** removes memory from int field 
 * @param field  field to be freed
 * @returns state value
 */
int femIntFree(long *field)
{
	free(field);
	field = NULL;
	return(AF_OK);
}


/** allocates and returns 1D double field  (NULL if failed) 
 * @param length length of field
 * @returns field (or NULL)
 */
double *femDblAlloc(long length)
{
	double *field = NULL;
	long  i;

	if (length < 1) { return(NULL); }

	if ((field = (double *) malloc(length * sizeof(double))) == NULL)
	{ 
		return(NULL);  
	}
	else                                                    
	{ 
		for (i = 0; i < length; i++) { field[i] = 0.0; }
		return(field); 
	}
}

/* removes memory from dbl field 
 * @param field  field to be freed
 * @returns state value
 */
int femDblFree(double *field)
{
	free(field);
	field = NULL;
	return(AF_OK);
}

/* end of fem_mem.c */
