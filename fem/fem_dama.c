/*
   File name: fem_dama.c
   Date:      2003/04/07 10:03
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

	 FEM Solver - data manipulations

  $Id: fem_dama.c,v 1.8 2004/11/11 21:39:23 jirka Exp $
*/

#include "fem_dama.h"
#include "fem_data.h"
#include "fem_mem.h"

/** Sets all data fields to NULL or 0 (solution parameters are untouched)
 * @return AF_OK in all cases ;-)
 */
int femDataNull(void)
{
  nLen  = 0; /* number of nodes                */
  eLen  = 0; /* number of elemnts              */
  eLenL = 0; /* lenght of element nodes field  */
 rsLen  = 0; /* number or real sets            */
 rsLenL = 0; /* number of real value field     */
 mpLen  = 0; /* number of materials            */
 mpLenL = 0; /* number of materials values     */
 nlLen  = 0; /* number of nodal loads/b.c.s    */
 elLen  = 0; /* number of element loads        */
 elLenL = 0; /* number of element loads values */



/* NODES */
  nID = NULL ; /* node number  */
  n_x = NULL ; /* x coordinate */
  n_y = NULL ; /* y coordinate */
  n_z = NULL ; /* z coordinate */


/* ELEMENTS */
  eID     = NULL ; /* element number               */
  eType   = NULL ; /* element type number          */
  eRS     = NULL ; /* real set data position       */
  eMP     = NULL ; /* material data position       */
  eFrom   = NULL ; /* starting point in eNodesL    */

  eNodesL = NULL;  /* list of nodes          */


/* REAL SETS */
  rsID     = NULL ; /* real set number              */
  rsType   = NULL ; /* element type number          */
  rsFrom   = NULL ; /* starting point in rsValL     */

  rsValL   = NULL ; /* real set values              */


/* MATERIAL PROPERTIES */
  mpID     = NULL ; /* material number              */
  mpType   = NULL ; /* material type number         */
  mpFrom   = NULL ; /* starting point in mpValL     */

  mpValL   = NULL ; /* real set values              */


/* NODAL LOADS AND BOUNDARY CONDITIONS  */
  nlNode = NULL ;   /* node to be used (position) */
  nlType = NULL ;   /* type of load               */
  nlDir  = NULL ;   /* direction of load          */
  nlVal  = NULL ;   /* value of load              */


/* ELEMENT LOADS */
  elElem = NULL ;   /* node to be used (position) */
  elType = NULL ;   /* type of load               */
  elFrom = NULL ;   /* starting point in elValL   */

  elValL = NULL ;   /* values of load             */


/* GRAVITATION */

  grDir = 0 ; /* direction (none=0, x=1, y=3, z=3)      */
  grVal = 0 ; /* acceleration value (9.81, for example) */

	return(AF_OK);
}


/** Frees all data fields (solution parameters are untouched)
 * @return AF_OK in all cases ;-)
 */
int femDataFree(void)
{
  nLen  = 0; /* number of nodes                */
  eLen  = 0; /* number of elemnts              */
  eLenL = 0; /* lenght of element nodes field  */
 rsLen  = 0; /* number or real sets            */
 rsLenL = 0; /* number of real value field     */
 mpLen  = 0; /* number of materials            */
 mpLenL = 0; /* number of materials values     */
 nlLen  = 0; /* number of nodal loads/b.c.s    */
 elLen  = 0; /* number of element loads        */
 elLenL = 0; /* number of element loads values */



/* NODES */
  femIntFree(nID ) ; /* node number  */
  femDblFree(n_x ) ; /* x coordinate */
  femDblFree(n_y ) ; /* y coordinate */
  femDblFree(n_z ) ; /* z coordinate */


/* ELEMENTS */
  femIntFree(eID     ) ; /* element number               */
  femIntFree(eType   ) ; /* element type number          */
  femIntFree(eRS     ) ; /* real set data position       */
  femIntFree(eMP     ) ; /* material data position       */
  femIntFree(eFrom   ) ; /* starting point in eNodesL    */

  femIntFree(eNodesL );  /* list of nodes          */


/* REAL SETS */
  femIntFree(rsID     ) ; /* real set number              */
  femIntFree(rsType   ) ; /* element type number          */
  femIntFree(rsFrom   ) ; /* starting point in rsValL     */

  femDblFree(rsValL   ) ; /* real set values              */


/* MATERIAL PROPERTIES */
  femIntFree(mpID     ) ; /* material number              */
  femIntFree(mpType   ) ; /* material type number         */
  femIntFree(mpFrom   ) ; /* starting point in mpValL     */

  femDblFree(mpValL   ) ; /* real set values              */


/* NODAL LOADS AND BOUNDARY CONDITIONS  */
  femIntFree(nlNode ) ;   /* node to be used (position) */
  femIntFree(nlType ) ;   /* type of load               */
  femIntFree(nlDir  ) ;   /* direction of load          */
  femDblFree(nlVal  ) ;   /* value of load              */


/* ELEMENT LOADS */
  femIntFree(elElem ) ;   /* node to be used (position) */
  femIntFree(elType ) ;   /* type of load               */
  femIntFree(elFrom ) ;   /* starting point in elValL   */

  femDblFree(elValL ) ;   /* values of load             */


/* GRAVITATION */
  grDir = 0 ; /* direction (none=0, x=1, y=3, z=3)      */
  grVal = 0 ; /* acceleration value (9.81, for example) */


	return(AF_OK);
}

/** NULLs result data fields (solution parameters are untouched)
 * @return AF_OK in all cases ;-)
 */
int femResNull(void)
{
	int rv = AF_OK;

  resRLen = 0; /* number of nodes with reactions */
  resELen = 0; /* number of element results      */

/* RESULTS  - reaction */
  resRnode = NULL ; /* node with reaction(s)     */
  resRdof  = NULL ;
  resRval  = NULL ; 
  resRval0 = NULL ;  

/* RESULTS ON ELEMENTS */
  resEFrom = NULL ;  /* number of results on element */
  resEVal  = NULL ;  /* results on element (values)  */
	return(rv);
}

/** Frees result data fields (solution parameters are untouched)
 * @return AF_OK in all cases ;-)
 */
int femResFree(void)
{
	int rv = AF_OK;

  resRLen = 0; /* number of nodes with reactions */
  resELen = 0; /* number of element results      */


/* RESULTS  - reaction */
  femIntFree(resRnode ) ; /* node with reaction(s)     */
  femIntFree(resRdof  ) ; /* node with reaction(s)     */
  femDblFree(resRval  ) ;
  femDblFree(resRval0 ) ; 

/* RESULTS ON ELEMENTS */
  femIntFree(resEFrom ) ;  /* number of results on element */
  femDblFree(resEVal ) ;  /* results on element (values)  */
	return(rv);
}

/* *************************************************************
 * *** Backup files for results ********************************
 * ************************************************************* */

/** NULLs result data fields (solution parameters are untouched)
 * @return AF_OK in all cases ;-)
 */
int femBackResNull(void)
{
	int rv = AF_OK;

  resBackRLen = 0; /* number of nodes with reactions */
  resBackELen = 0; /* number of element results      */

/* RESULTS  - reaction */
  resBackRnode = NULL ; /* node with reaction(s)     */
  resBackRdof  = NULL ;
  resBackRval  = NULL ; 
  resBackRval0 = NULL ;  

/* RESULTS ON ELEMENTS */
  resBackEFrom = NULL ;  /* number of results on element */
  resBackEVal  = NULL ;  /* results on element (values)  */
	return(rv);
}

/** Frees result data fields (solution parameters are untouched)
 * @return AF_OK in all cases ;-)
 */
int femBackResFree(void)
{
	int  rv = AF_OK;

  resBackRLen = 0; /* number of nodes with reactions */
  resBackELen = 0; /* number of element results      */

/* RESULTS  - reaction */
  femIntFree(resBackRnode ) ; /* node with reaction(s)     */
  femIntFree(resBackRdof  ) ; /* node with reaction(s)     */
  femDblFree(resBackRval  ) ;
  femDblFree(resBackRval0 ) ; 

/* RESULTS ON ELEMENTS */
  femIntFree(resBackEFrom ) ;  /* number of results on element */
  femDblFree(resBackEVal ) ;  /* results on element (values)  */
	return(rv);
}

/** Gets backup result date from the original ones
 * @return AF_OK in all cases ;-)
 */
int femBackResGet(void)
{
	int rv = AF_OK;
	long i ;

	if ((resBackRLen > 0) || (resBackELen > 0))
	{
		femBackResFree(); /* previously used */
	}
	else
	{
		femBackResNull(); /* used first time or after free() */
	}

  resBackRLen = resRLen; /* number of nodes with reactions */
  resBackELen = resELen; /* number of element results      */

	if ((resBackRnode=femIntAlloc(resBackRLen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
	if ((resBackRdof =femIntAlloc(resBackRLen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
	if ((resBackRval0=femDblAlloc(resBackRLen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
	if ((resBackRval =femDblAlloc(resBackRLen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}

	if ((resBackEFrom=femIntAlloc(eLen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
	if ((resBackEVal=femDblAlloc(resBackELen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}

	for (i=0; i<resBackRLen; i++)
	{
		resBackRnode[i] = resRnode[i] ;
		resBackRdof[i]  = resRdof[i] ;
		resBackRval0[i] = resRval0[i] ;
		resBackRval[i]  = resRval[i] ;
	}

	for (i=0; i<eLen; i++)
	{
		resBackEFrom[i] = resEFrom[i] ;
	}

	for (i=0; i<resBackELen; i++)
	{
		resBackEVal[i] = resEVal[i] ;
	}
	
	return(rv);

memFree:
	femBackResFree();
	return(rv);
}

/** Put backup result date from the original 
 * @return AF_OK in all cases ;-)
 */
int femBackResPut(void)
{
	int rv = AF_OK;
	long i ;

	if ((resBackRLen <= 0) && (resBackELen <= 0))
	{
#ifdef RUN_VERBOSE
		fprintf(msgout, "[E] Backup results ar not available!\n");
#endif
		return(AF_ERR);
	}

	for (i=0; i<resBackRLen; i++)
	{
		resRnode[i] = resBackRnode[i] ;
		resRdof[i]  = resBackRdof[i] ;
		resRval0[i] = resBackRval0[i] ;
		resRval[i]  = resBackRval[i] ;
	}

	for (i=0; i<eLen; i++)
	{
		resEFrom[i] = resBackEFrom[i] ;
	}

	for (i=0; i<resBackELen; i++)
	{
		resEVal[i] = resBackEVal[i] ;
	}
	
	return(rv);
}


/* *********************************************************
 * Replacement for macros (getting of values from real sets and materials)
 * *********************************************************
 */

/** Returnes value of real constant 
 @param  ePos element position
 @param  valType type of value (see predefined constants)
 @param  repNum repeat number (0 for non-repeating)
 @return requested value (0 if not found)
 */
double femGetRSValPos(long ePos, long valType, long repNum)
{
	long eT = 0 ;
	long valPos = 0 ;

	eT = femGetETypePos(ePos); 
	if (eT < 0) {return(0);}

	valPos = femGetERSPos(ePos);
	if (valPos < 0) {return(0);}

	return(
			  femGetRepVal(
					valPos, 
					valType, 
					repNum, 
					rsValL, 
					rsLenL, 
					rsFrom, 
					rsLen, 
					Elem[eT].real, 
					Elem[eT].rs, 
					Elem[eT].real_rp, 
					Elem[eT].rs_rp 
					)
			);
}

/** Returnes value of material property 
 @param  ePos element position
 @param  valType type of value (see predefined constants)
 @param  repNum repeat number (0 for non-repeating)
 @return requested value (0 if not found)
 */
double femGetMPValPos(long ePos, long valType, long repNum)
{
	long mT = 0 ;
	long valPos = 0 ;

	/* note: this is VERY DIFFERENT from RS handling!! */
	mT = femGetEMPPos(ePos); 
	if (mT < 0) {return(0);}

	valPos = femGetMPTypePos(mT) ;
	if (valPos < 0) {return(0);}

	return( femGetRepVal (	
	 							mT, 
								valType, 
								repNum, 
								mpValL, 
								mpLenL, 
								mpFrom, 
								mpLen, 
								Mat[valPos].val, 
								Mat[valPos].num, 
								Mat[valPos].val_rp, 
								Mat[valPos].num_rp
								)
			);
}

/** Returnes SYSTEM id of the material type 
 * @param  element position
 * @return material id
 * */
long femGetSystemMatType(long ePos)
{
  long pos, type;

  pos = femGetEMPPos(ePos) ;
  if ((pos < 0) || (pos > mpLen)) {return(0);}

  type = femGetMPTypePos(pos) ;

  return(Mat[type].type) ;
}

/** Returnes number of repeating values of real constant 
 @param  ePos element position
 @return number of repeating data sets
 */
long femGetRSRepNumPos(long ePos)
{
	long eT = 0 ;
	long valPos = 0 ;

	eT = femGetETypePos(ePos); 
	if (eT < 0) {return(0);}

	valPos = femGetERSPos(ePos);
	if (valPos < 0) {return(0);}

	return(
			  femGetRepNum(
					valPos, 
					rsValL, 
					rsLenL, 
					rsFrom, 
					rsLen, 
					Elem[eT].real, 
					Elem[eT].rs, 
					Elem[eT].real_rp, 
					Elem[eT].rs_rp 
					)
			);
}

/** Returnes number of repeating sets of material
 @param  ePos element position
 @return number of sets
 */
long femGetMPRepNumPos(long ePos)
{
	long mT = 0 ;
	long valPos = 0 ;

	/* note: this is VERY DIFFERENT from RS handling!! */
	mT = femGetEMPPos(ePos); 
	if (mT < 0) {return(0);}

	valPos = femGetMPTypePos(mT) ;
	if (valPos < 0) {return(0);}

	return( femGetRepNum (	
	 							mT, 
								mpValL, 
								mpLenL, 
								mpFrom, 
								mpLen, 
								Mat[valPos].val, 
								Mat[valPos].num, 
								Mat[valPos].val_rp, 
								Mat[valPos].num_rp
								)
			);
}

/* end of fem_dama.c */
