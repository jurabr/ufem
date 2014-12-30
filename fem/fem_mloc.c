/*
   File name: fem_mloc.c
   Date:      2003/04/13 13:10
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

	 FEM Solver - localization of element matrices and vectors

	 $Id: fem_mloc.c,v 1.13 2004/07/08 22:46:43 jirka Exp $
*/

#include "fem_sol.h"

#ifdef USE_MPI
#include "fem_mpi.h"
extern tVector u_big ;
#endif

extern long  nDOFAct  ; /* total number of ACTIVE DOFs in structure (== size of "K" matrix) */
extern long  nDOFlen  ; /* lenght of nDOFfld                        */
extern long *nDOFfld  ; /* description of DOFs in nodes             */
extern long *K_rows   ; /* numbers of items in "K" rows K_rows[nDOFAct] */
extern tMatrix K      ; /* "K" matrix */

#ifdef _USE_THREADS_
extern pthread_mutex_t *mutexK  ; /* mutexes for K  */
extern pthread_mutex_t *mutexF  ; /* mutexes for F  */
extern pthread_mutex_t *mutexFr ; /* mutexes for Fr */
#endif

/** Returnes row/collumn (e.g. 1..n) number in "K" matrix
 * @param node_pos position of node
 * @param dof_id DOF TYPE (for example U_X)
 * @return position (0 on error)
 */
long femKpos(long node_pos, long dof)
{
	static long pos = 0;
	long  pos_ind;

	pos_ind = (node_pos*KNOWN_DOFS) + (dof-1) ;

	if ((pos_ind < nDOFlen)&&(pos_ind >= 0))
	{
	  pos = nDOFfld[pos_ind];
	}
	else
	{
		/* ERROR ! */
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("K-index out of boundaries"));
#endif
#ifdef DEVEL
		exit(AF_ERR_SIZ);
#endif
    return(0);
	}

#ifdef DEVEL_VERBOSE
printf ("[ ] pos = %li (index %li, n=%li, dof=%li) \n", pos, pos_ind, node_pos, dof);
#endif
	
	return(pos);
}

/** Computes number of "K" rows */
int femKhit(void)
{
	long i,j,k;
	long pos;
	long node_pos;

#ifdef DEVEL
	if (K_rows == NULL) {return(AF_ERR_MEM);}
#endif

	for (i=0; i<eLen; i++)
	{
		for (j=0; j<Elem[eType[i]].nodes; j++)
		{
			node_pos = femGetENodePos(i,j);
			for (k=0; k<Elem[eType[i]].dofs; k++)
			{
				if ((pos=femKpos(node_pos, Elem[eType[i]].ndof[k])) < 1)
#if 0
				   { return(AF_ERR_VAL); }
#else
        continue ; /* workaround for femFastBC */
#endif
#ifdef DEVEL_VERBOSE
#if 0
				fprintf(msgout,"pos %li (id=%li, %li) \n",pos,nID[node_pos], Elem[eType[i]].ndof[k]);				
#endif
#endif
				K_rows[pos-1] += Elem[eType[i]].nodes*Elem[eType[i]].dofs;
			}
		}
	}
	return(AF_OK);
}



/** Fills element localization vector */
int femFillLocVec(long ePos, long *Vec, long vLen)
{
	int i,j;
	long eT = 0;
	long pos = 0;
	long node_pos = 0;

	eT = eType[ePos];
#ifdef DEVEL
	if ((Elem[eT].dofs*Elem[eT].nodes) != vLen) { return(AF_ERR_VAL); }
#endif

	for (i=0; i<Elem[eT].nodes; i++)
	{
		node_pos = femGetENodePos(ePos, i);
		for (j=0; j<Elem[eT].dofs; j++)
		{
			if ((Vec[pos] = femKpos(node_pos, Elem[eT].ndof[j])) < 1)
		  { 
#if 0
        return(AF_ERR_VAL); 
#else
        Vec[pos] = 0.0 ;
#endif
      }
		  pos++;
		}
	}
#ifdef DEVEL_VERBOSE
#if 0
	for (i=0; i<vLen; i++) { fprintf(msgout," %li ",Vec[i]); } fprintf(msgout,"|| \n");
#endif
#endif
	return(AF_OK);
}


/** localizes items from "K_e" to "K"
 * @param K_e matrix
 * @param ePos element position
 * @return state value
 */
int femLocKM_e(tMatrix *K_e, tMatrix *K, long ePos)
{
	int rv = AF_OK;
	long i,j;
	long *lFld = NULL;
	long  lFldLen = 0;
#ifdef USE_MPI
	long row;
#endif

	lFldLen =  Elem[eType[ePos]].dofs*Elem[eType[ePos]].nodes ;
#ifdef DEVEL
	if (lFldLen != K_e->rows) {return(AF_ERR_SIZ);}
#endif
	if ((lFld = femIntAlloc(lFldLen)) == NULL ) {return(AF_ERR_MEM);}

	if ((rv=femFillLocVec(ePos, lFld, lFldLen)) != AF_OK)
		 { goto memFree; }

	for (i=1; i<=lFldLen; i++)
	{
#ifdef _USE_THREADS_
    if (lFld[i-1] != 0.0) /* <- femFastBC workaround */
    {
      if (mutexK != NULL) { pthread_mutex_lock(&mutexK[lFld[i-1]-1]) ; }
    }
#endif
		for (j=1; j<=lFldLen; j++)
		{
      if ((lFld[i-1]*lFld[j-1]) != 0.0) /* <- femFastBC workaround */
      {
#ifndef USE_MPI
			/* standard way: */
			  rv = femMatAdd(K, lFld[i-1], lFld[j-1], femMatGet(K_e,i,j));
#else
			/* MPI modification: */
			row = lFld[i-1] - (ppDOFfrom-1) ;
			if ((row < 1) || (row > ppDOFlen)) {continue;}

			rv = femMatAdd(K, row, lFld[j-1], femMatGet(K_e,i,j));
#endif
      }

#ifdef DEVEL
			if (rv != AF_OK) {goto memFree;}
#endif
		}
#ifdef _USE_THREADS_
    if (mutexK != NULL) { pthread_mutex_unlock(&mutexK[lFld[i-1]-1]) ; }
#endif
	}

memFree:
	femIntFree(lFld);
	return(rv);
}


/** localizes items from "F_e" to "F"
 * @param F_e vector
 * @param ePos element position
 * @param isF AF_YES if F is F, AF_NO for Fr (used for mutexes only ;-)
 * @return state value
 */
int femLocF_e(tVector *F_e, long ePos, tVector *F, long isF)
{
	int rv = AF_OK;
	long i;
	long *lFld = NULL;
	long  lFldLen = 0;
#ifdef USE_MPI
	long row;
#endif

	lFldLen =  Elem[eType[ePos]].dofs*Elem[eType[ePos]].nodes ;
#ifdef DEVEL
	if (lFldLen != F_e->rows) {return(AF_ERR_SIZ);}
#endif
	if ((lFld = femIntAlloc(lFldLen)) == NULL ) {return(AF_ERR_MEM);}

	if ((rv=femFillLocVec(ePos, lFld, lFldLen)) != AF_OK)
		 { goto memFree; }

	for (i=1; i<=lFldLen; i++)
	{
#ifndef USE_MPI
#ifdef _USE_THREADS_
    if (isF == AF_YES)
      { if (mutexF != NULL) { pthread_mutex_lock(&mutexF[lFld[i-1]-1]) ; } }
    else
      { if (mutexFr != NULL) { pthread_mutex_lock(&mutexFr[lFld[i-1]-1]) ; } }
#endif

    if (lFld[i-1] != 0.0) /* <- femFastBC workaround */
    {
		  rv = femVecAdd(F, lFld[i-1], femVecGet(F_e,i));

#ifdef _USE_THREADS_
    if (isF == AF_YES)
      { if (mutexF != NULL) { pthread_mutex_unlock(&mutexF[lFld[i-1]-1]) ; } }
    else
      { if (mutexFr != NULL) { pthread_mutex_unlock(&mutexFr[lFld[i-1]-1]) ; } }
#endif
#else
    if (lFld[i-1] != 0.0) /* <- femFastBC workaround */
    {
		row = lFld[i-1] - (ppDOFfrom-1) ;
		if ((row < 1) || (row > ppDOFlen)) {continue;}

		rv = femVecAdd(F, lFld[i-1] - (ppDOFfrom-1), femVecGet(F_e,i));
    }
#endif
    }
#ifdef DEVEL
		if (rv != AF_OK) {goto memFree;}
#endif
	}

memFree:
	femIntFree(lFld);
	return(rv);
}


/** localizes items from "U" to "U_e"
 * @param F_e vector
 * @param ePos element position
 * @return state value
 */
int femLocUtoU_e(tVector *U, long ePos, tVector *U_e)
{
	int rv = AF_OK;
	long i;
	long *lFld = NULL;
	long  lFldLen = 0;
#ifdef USE_MPI
	/*long row;*/
#endif

	lFldLen =  Elem[eType[ePos]].dofs*Elem[eType[ePos]].nodes ;
#ifdef DEVEL
	if (lFldLen != U_e->rows) {return(AF_ERR_SIZ);}
#endif
	if ((lFld = femIntAlloc(lFldLen)) == NULL ) {return(AF_ERR_MEM);}

	if ((rv=femFillLocVec(ePos, lFld, lFldLen)) != AF_OK)
		 { goto memFree; }

	for (i=1; i<=lFldLen; i++)
	{
    if (lFld[i-1] != 0.0) /* <- femFastBC workaround */
    {
#ifndef USE_MPI
		rv = femVecAdd(U_e,i, femVecGet(U, lFld[i-1]));
#else
		/*
		row = lFld[i-1] - (ppDOFfrom-1) ;
		if ((row < 1) || (row > ppDOFlen)) {continue;}

		rv = femVecAdd(U_e,i, femVecGet(U, lFld[i-1] - (ppDOFfrom-1) ));
		*/
		rv = femVecAdd(U_e,i, femVecGet(&u_big, lFld[i-1]));
#endif
    }
    else { femVecAdd(U_e,i, 0.0); }

#ifdef DEVEL
		if (rv != AF_OK) {goto memFree;}
#endif
	}

memFree:
	femIntFree(lFld);
	return(rv);
}


/* end of fem_mloc.c */
