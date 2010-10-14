/*
   File name: fem_spnt.h
   Date:      2003/06/08 16:50
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

	 FEM Solver - routines to save results in selected points (for NRM etc.)

	 $Id: fem_spnt.h,v 1.1 2003/09/14 20:36:59 jirka Exp $
*/


#ifndef __FEM_SPNT_H__
#define __FEM_SPNT_H__


#ifndef _SMALL_FEM_CODE_

#include "fem.h"
#include "fem_dama.h"
#include "fem_data.h"
#include "fem_para.h"

#ifdef USE_MPI
#include "fem_mpi.h"
#endif

/** structure for saved values */
typedef struct 
{
	double mult ; /* load multiplier */
	double x ;    /* x displacement  */
	double y ;    /* y displacement  */
	double z ;    /* z displacement  */
	int    unused ;
} tSaPo;


extern char  *femSaPoFName;/* filename           */
extern long   femSaPoNode; /* node number        */
extern long   femSaPoLen ; /* lenght of femSaPo  */
extern long   femSaPoActual;/* actual item in femSaPo  */
extern tSaPo *femSaPo ; /* structure for saving of data in _ONE_ selected point  */

extern char  *femSumReactName ; /* filename for reactions */
extern long   femSumReactStdOut ; /* use stdout? */


extern int femSaPoAlloc(long NewLen, long OldLen) ;
extern void femSaPoFree(void) ;
extern int femSaPoInput(double mult, double x, double y, double z, long append, int unused) ;
extern int femSaPoSave(void) ;
extern void femSaPoDestroy(void) ;
extern int femSaPoFileName(char *fname) ;
extern int femSaPoSetNode(long from, long num) ;

extern int femGetSumReactInit(char  *fname);
extern int femGetSumReact(long step, long iter, double mult, char *fname, long TangentMatrix);
extern int femSumReactFileName(char *fname);

#endif /* end of _SMALL_FEM_CODE_ */

#endif

/* end of fem_spnt.h */
