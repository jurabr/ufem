/*
   File name: fem_elem.c
   Date:      2003/04/09 11:44
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

	 FEM Solver - element definition

  $Id: fem_elem.c,v 1.6 2003/08/02 18:37:52 jirka Exp $
*/

/* Included headers: */
#include "fem_elem.h"
#include "fem_dama.h"


/* definitions of data structures */
tElem *Elem = NULL;  /* elements */
long   elemLen = 0;  /* number of elements */

/** Adds new (empty) element structure
 * @param type element type number
 * @return element type number (or value >= 0 on error)
 */
long femAddElem(long type)
{
	tElem *tmpElem = NULL;
	int    i;

	if (type != (elemLen)) 
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: %li!\n",_("Incorrect element type number"),type);
#endif
		return(AF_ERR_VAL);
	}

	if((tmpElem = (tElem *)malloc((elemLen+1)*sizeof(tElem))) == NULL)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n",_("Out of memory during element type preparing"));
#endif
		return(AF_ERR_MEM);
	}

	if (elemLen > 0)
	{
		for (i=0; i<elemLen; i++)
		{
			tmpElem[i] = Elem[i] ;
		}
	}
	free (Elem); Elem = NULL;
	Elem = tmpElem;
	tmpElem = NULL;
	Elem[elemLen].type = type;

	Elem[elemLen].nodes = 0;
	Elem[elemLen].dofs = 0;
	Elem[elemLen].ndof = NULL;
	Elem[elemLen].rs = 0;
	Elem[elemLen].real = NULL;
	Elem[elemLen].rs_rp = 0;
	Elem[elemLen].real_rp = NULL;

	Elem[elemLen].res = 0;
	Elem[elemLen].nres = NULL;
	Elem[elemLen].res_rp = 0;
	Elem[elemLen].nres_rp = NULL;

	Elem[elemLen].stiff = NULL ;
	Elem[elemLen].mass  = NULL ;
	Elem[elemLen].rvals = NULL ;
	Elem[elemLen].eload = NULL ;

	elemLen++;
	return(Elem[elemLen-1].type);
}

/** Computes number of element internal (integration) points
 * param ePos element position
 */
long femElemIPoints(long ePos)
{
	long num = 0 ;
	long eT ;

	eT = femGetETypePos(ePos); 

	if (Elem[eT].res_rp  > 0)
	{
		num = (Elem[eT].rvals(ePos) - Elem[eT].res ) / Elem[eT].res_rp ;
	}

	if (Elem[eT].res > 0) 
	{
		num++ ;
	}
	
	return(num);
}
