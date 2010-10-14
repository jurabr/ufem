/*
   File name: fem_elem.c
   Date:      2003/04/16 20:38
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

  $Id: fem_mat.c,v 1.4 2004/01/10 21:09:29 jirka Exp $
*/

/* Included headers: */
#include "fem_mat.h"


/* definitions of data structures */
tMat *Mat = NULL;  /* elements */
long  matLen = 0;  /* number of elements */

/** Adds new (empty) element structure
 * @param type element type number
 * @return element type number (or value >= 0 on error)
 */
long femAddMat(long type)
{
	tMat *tmpMat = NULL;
	int    i;

	if (type != (matLen)) 
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: %li!\n",_("Incorrect material type number"),type);
#endif
		return(AF_ERR_VAL);
	}

	if((tmpMat = (tMat *)malloc((matLen+1)*sizeof(tMat))) == NULL)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n",_("Out of memory during material type preparing"));
#endif
		return(AF_ERR_MEM);
	}

	if (matLen > 0)
	{
		for (i=0; i<matLen; i++)
		{
			tmpMat[i] = Mat[i] ;
		}
	}
	free (Mat); Mat = NULL;
	Mat = tmpMat;
	tmpMat = NULL;
	Mat[matLen].type = type;

	Mat[matLen].num = 0;
	Mat[matLen].val = NULL;
	Mat[matLen].num_rp = 0;
	Mat[matLen].val_rp = NULL;

#if 1
	Mat[matLen].test = NULL ;
#endif

	matLen++;
	return(Mat[matLen-1].type);
}

/* end of fem_mat.c */
