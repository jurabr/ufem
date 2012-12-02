/*
   File name: fem_mpi.c
   Date:      2003/04/24 12:32
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

	 FEM Solver - MPI Communication code

	 $Id: fem_mpi.c,v 1.26 2005/07/11 17:56:16 jirka Exp $
*/

#include "fem_mpi.h"

#ifdef USE_MPI

/* from "fem_sol.c": */
extern int fem_dofs(void);
extern void fem_sol_null(void);
extern int fem_sol_free(void);
extern int fem_fill_K(long mode);
extern int fem_add_loads(long step);
extern int fem_add_disps(long disp_mode, long step);
extern double femMultNRM(long incr_type, int i, int n);

extern double femVecNormMPI(tVector *a);

extern long  nDOFAct;    /* total number of ACTIVE DOFs in structure (== size of "K" matrix) */
extern long *K_rows ; /* numbers of items in "K" rows K_rows[nDOFAct] */

extern tMatrix K; /* structure stiffness matrix    */
extern tVector F; /* structure load vector         */
extern tVector Fr;/* unballanced forces vector     */
extern tVector u; /* structure displacement vector */

extern long  nDOFlen  ; /* lenght of nDOFfld                        */
extern long *nDOFfld  ; /* description of DOFs in nodes             */

/* Prototypes: */

int femSolveNRM_MPI(long incr_type);
int femMPI_SendResults(double mult);

int ppRank = 0 ;
int ppSize = 0 ;

long  ppDOFfrom = 0;
long  ppDOFto = 0;
long  ppDOFlen = 0;

long  ppNfrom = 0 ;
long  ppNnum = 0 ;

tVector u_big; /* Full size displacement vector */

/** Gets process number and number of processes for parallel computations
 * @param rank process number [0..N-1]
 * @param size number of processes [N]
 */
void femMPIwhoami(int *rank, int *size)
{
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
  MPI_Comm_size(MPI_COMM_WORLD, size);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"process %i from %i\n", *rank, *size);
#endif
}

void ppSync(void)
{
	MPI_Barrier(MPI_COMM_WORLD);
}


/** Sends solution control data to all processes
 * @return  status
 */
int ppBcastSolData(void)
{
	int           rv = AF_OK ;
	static long   fldI[7];
	static double fldF[3];

	if (ppRank == 0)
	{
		fldI[0] = solID           ;   /* "time node" identifier                       */
		fldI[1] = solDepOn        ;   /* dependency on prefious "time node"           */
		fldI[2] = solLinSol       ;   /* linear eq. sys. solve                        */
		fldI[3] = solNoLinS       ;   /* non-linear solution method                   */
		fldI[4] = solNoLinSteps   ;   /* number of steps in non-linear solution       */
		fldI[5] = solLinStepMax   ;   /* max. number of steps in linear solution      */
		fldI[6] = solNoLinStepMax ;   /* max. number of steps in non-linear solution  */

		fldF[0] = solLinPrec      ;   /* linear precission                            */
		fldF[1] = solNoLinPre     ;   /* nonlinear precission                         */
		fldF[2] = solNoLinBig     ;   /* nonlinear max. residual size                 */

	}

	MPI_Bcast(fldI,7,MPI_LONG,0,MPI_COMM_WORLD);
	MPI_Bcast(fldF,3,MPI_DOUBLE,0,MPI_COMM_WORLD);
	ppSync();

	if (ppRank > 0)
	{
		solID           = fldI[0];
		solDepOn        = fldI[1];
		solLinSol       = fldI[2];
		solNoLinS       = fldI[3];
		solNoLinSteps   = fldI[4];
		solLinStepMax   = fldI[5];
		solNoLinStepMax = fldI[6];

		solLinPrec      = fldF[0];
		solNoLinPre     = fldF[1];
		solNoLinBig     = fldF[2];
	}

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[%i] ID=%li Dep=%li LinS=%li NoLinS=%li LinP=%f NoLP=%f NoLPB=%f\n",
			ppRank,
		solID           ,
		solDepOn        ,
		solLinSol       ,
		solNoLinS       ,
		solLinPrec      ,
		solNoLinPre     ,
		solNoLinBig     
			);
#endif
	return(rv);
}


/** Sends lenghts of input data fields to all processes
 * @return  status
 */
int ppBcastLens(void)
{
	int rv = AF_OK ;
	static long fldI[10];

	if (ppRank == 0)
	{
		fldI[0] =  nLen  ; /* number of nodes                */
		fldI[1] =  eLen  ; /* number of elements             */
		fldI[2] =  eLenL ; /* lenght of element nodes field  */
		fldI[3] = rsLen  ; /* number or real sets            */
		fldI[4] = rsLenL ; /* number of real value field     */
		fldI[5] = mpLen  ; /* number of materials            */
		fldI[6] = mpLenL ; /* number of materials values     */
		fldI[7] = nlLen  ; /* number of nodal loads/b.c.s    */
		fldI[8] = elLen  ; /* number of element loads        */
		fldI[9] = elLenL ; /* number of element loads values */
	}

	MPI_Bcast(fldI,10,MPI_LONG,0,MPI_COMM_WORLD);
	ppSync();

	if (ppRank > 0)
	{
		nLen   = fldI[0];
		eLen   = fldI[1];
		eLenL  = fldI[2];
		rsLen  = fldI[3];
		rsLenL = fldI[4];
		mpLen  = fldI[5];
		mpLenL = fldI[6];
		nlLen  = fldI[7];
		elLen  = fldI[8];
		elLenL = fldI[9];
	}

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[%i] lens: %li %li %li %li %li %li %li %li %li %li\n",
			ppRank,
			nLen, eLen, eLenL, rsLen, rsLenL, mpLen, 
			mpLenL, nlLen, elLen, elLenL );
#endif

	return(rv);
}

/** Sends properties to all processes
 * @return  status
 */
int ppBcastData(void)
{
	int rv = AF_OK ;
	static long *hugeInt = NULL ;
	static double           *hugeDbl = NULL ;
	int   sizeI = 0;
	int   sizeD = 0;
	long   posI = 0 ;
	long   posD = 0 ;
	long i;

	ppSync(); /* TODO WHY TWO TIMES !?!?!?!? */
	ppSync();

		if (rsLen > 0)
		{
			sizeI +=  (3*rsLen)  ;
			sizeD +=    rsLenL ;
		}

		sizeI += (3*mpLen) ;
		sizeD += mpLenL ;

		sizeI += nLen ;
		sizeD += (3*nLen) ;

		sizeI += ((5*eLen) + eLenL) ;

		sizeI += (3*nlLen) ;
		sizeD +=   nlLen ;

		if (elLen > 0)
		{
			sizeI += (4*elLen) ;
			sizeD +=   elLenL ;
		}

		/* gravitation: */
		sizeI++;
		sizeD++;

#ifdef DEVEL_VERBOSE
fprintf(msgout,"[%i] Sizes: sizeI = %i, sizeD = %i\n", ppRank, sizeI, sizeD);
#endif

		if ((hugeInt = (signed long int *) malloc(sizeI*sizeof(signed long int))) == NULL)
		{
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[E] %s %i!\n", _("Out of memory for process"), ppRank);
#endif
			exit(AF_ERR_MEM);
		}

		for (i=0; i< sizeI; i++) { hugeInt[i] = 0 ; }

		if ((hugeDbl = (double *) malloc(sizeD*sizeof(double))) == NULL)
		{
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[E] %s %i!\n", _("Out of memory for process"), ppRank);
#endif
			exit(AF_ERR_MEM);
		}

		for (i=0; i< sizeD; i++) { hugeDbl[i] = 0.0 ; }


	if (ppRank == 0) /* data moving */
	{
	/* ************************************************** */
		posI = 0;
		posD = 0 ;

		/* real sets */
		if (rsLen > 0)
		{
			ppDataCpyInt(hugeInt, rsID, rsLen, posI); posI+= rsLen;
			ppDataCpyInt(hugeInt, rsType, rsLen, posI); posI+= rsLen;
			ppDataCpyInt(hugeInt, rsFrom, rsLen, posI); posI+= rsLen;

			ppDataCpyDbl(hugeDbl, rsValL, rsLenL, posD); posD+= rsLenL;
		}


    ppDataCpyInt(hugeInt, mpID, mpLen, posI); posI+= mpLen;
		ppDataCpyInt(hugeInt, mpType, mpLen, posI); posI+= mpLen;
		ppDataCpyInt(hugeInt, mpFrom, mpLen, posI); posI+= mpLen;

		ppDataCpyDbl(hugeDbl, mpValL, mpLenL, posD); posD+= mpLenL;


    ppDataCpyInt(hugeInt, nID, nLen, posI); posI+= nLen;
		
		ppDataCpyDbl(hugeDbl, n_x, nLen, posD); posD+= nLen;
		ppDataCpyDbl(hugeDbl, n_y, nLen, posD); posD+= nLen;
		ppDataCpyDbl(hugeDbl, n_z, nLen, posD); posD+= nLen;



    ppDataCpyInt(hugeInt, eID, eLen, posI); posI+= eLen;
    ppDataCpyInt(hugeInt, eType, eLen, posI); posI+= eLen;
    ppDataCpyInt(hugeInt, eRS, eLen, posI); posI+= eLen;
    ppDataCpyInt(hugeInt, eMP, eLen, posI); posI+= eLen;
    ppDataCpyInt(hugeInt, eFrom, eLen, posI); posI+= eLen;

    ppDataCpyInt(hugeInt, eNodesL, eLenL, posI); posI+= eLenL;



    ppDataCpyInt(hugeInt, nlNode, nlLen, posI); posI+= nlLen;
    ppDataCpyInt(hugeInt, nlType, nlLen, posI); posI+= nlLen;
    ppDataCpyInt(hugeInt, nlDir, nlLen, posI); posI+= nlLen;

		ppDataCpyDbl(hugeDbl, nlVal, nlLen, posD); posD+= nlLen;

		if (elLen > 0)
		{
    	ppDataCpyInt(hugeInt, elElem, elLen, posI); posI+= elLen;
    	ppDataCpyInt(hugeInt, elType, elLen, posI); posI+= elLen;
    	/*ppDataCpyInt(hugeInt, elDir, elLen, posI); posI+= elLen;*/
    	ppDataCpyInt(hugeInt, elFrom, elLen, posI); posI+= elLen;
			
			ppDataCpyDbl(hugeDbl, elValL, elLenL, posD); posD+= elLenL;
		}

		hugeInt[posI] = grDir ;
		hugeDbl[posD] = grVal ;
	}
	else
	{
		/* real sets */
		if (rsLen > 0)
		{
			if ((rsID   = femIntAlloc(rsLen)) == NULL) {exit(AF_ERR_MEM);}
			if ((rsType = femIntAlloc(rsLen)) == NULL) {exit(AF_ERR_MEM);}
			if ((rsFrom = femIntAlloc(rsLen)) == NULL) {exit(AF_ERR_MEM);}

			if ((rsValL = femDblAlloc(rsLenL)) == NULL) {exit(AF_ERR_MEM);}
		}


		if ((mpID   = femIntAlloc(mpLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((mpType = femIntAlloc(mpLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((mpFrom = femIntAlloc(mpLen)) == NULL) {exit(AF_ERR_MEM);}

		if ((mpValL = femDblAlloc(mpLenL)) == NULL) {exit(AF_ERR_MEM);}


		if ((nID = femIntAlloc(nLen)) == NULL) {exit(AF_ERR_MEM);}

		if ((n_x = femDblAlloc(nLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((n_y = femDblAlloc(nLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((n_z = femDblAlloc(nLen)) == NULL) {exit(AF_ERR_MEM);}


		if ((eID   = femIntAlloc(eLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((eType = femIntAlloc(eLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((eRS   = femIntAlloc(eLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((eMP   = femIntAlloc(eLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((eFrom = femIntAlloc(eLen)) == NULL) {exit(AF_ERR_MEM);}

		if ((eNodesL = femIntAlloc(eLenL)) == NULL) {exit(AF_ERR_MEM);}


#ifdef DEVEL_VERBOSE
fprintf(msgout,"[%i] NLlen = %li ELlen = %li ELlenL = %li\n", ppRank, nlLen,elLen, elLenL );
#endif

		if ((nlNode = femIntAlloc(nlLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((nlType = femIntAlloc(nlLen)) == NULL) {exit(AF_ERR_MEM);}
		if ((nlDir  = femIntAlloc(nlLen)) == NULL) {exit(AF_ERR_MEM);}

		if ((nlVal  = femDblAlloc(nlLen)) == NULL) {exit(AF_ERR_MEM);}

		if (elLen > 0)
		{
			if ((elElem = femIntAlloc(elLen)) == NULL) {exit(AF_ERR_MEM);}
			if ((elType = femIntAlloc(elLen)) == NULL) {exit(AF_ERR_MEM);}
			/*if ((elDir  = femIntAlloc(elLen)) == NULL) * {exit(AF_ERR_MEM);}*/
			if ((elFrom = femIntAlloc(elLen)) == NULL) {exit(AF_ERR_MEM);}

			if ((elValL = femDblAlloc(elLenL)) == NULL) {exit(AF_ERR_MEM);}
		}
	}

	ppSync();
	MPI_Bcast(hugeInt,sizeI,MPI_LONG,0,MPI_COMM_WORLD);
	ppSync();
	MPI_Bcast(hugeDbl,sizeD,MPI_DOUBLE,0,MPI_COMM_WORLD);
	ppSync();

	if (ppRank > 0)
	{
		posI = 0;
		posD = 0 ;

		/* real sets */
		if (rsLen > 0)
		{
			ppDataCpyIntBack(hugeInt, rsID, rsLen, posI); posI+= rsLen;
			ppDataCpyIntBack(hugeInt, rsType, rsLen, posI); posI+= rsLen;
			ppDataCpyIntBack(hugeInt, rsFrom, rsLen, posI); posI+= rsLen;

			ppDataCpyDblBack(hugeDbl, rsValL, rsLenL, posD); posD+= rsLenL;
		}


    ppDataCpyIntBack(hugeInt, mpID, mpLen, posI); posI+= mpLen;
		ppDataCpyIntBack(hugeInt, mpType, mpLen, posI); posI+= mpLen;
		ppDataCpyIntBack(hugeInt, mpFrom, mpLen, posI); posI+= mpLen;

		ppDataCpyDblBack(hugeDbl, mpValL, mpLenL, posD); posD+= mpLenL;


    ppDataCpyIntBack(hugeInt, nID, nLen, posI); posI+= nLen;
		
		ppDataCpyDblBack(hugeDbl, n_x, nLen, posD); posD+= nLen;
		ppDataCpyDblBack(hugeDbl, n_y, nLen, posD); posD+= nLen;
		ppDataCpyDblBack(hugeDbl, n_z, nLen, posD); posD+= nLen;



    ppDataCpyIntBack(hugeInt, eID, eLen, posI); posI+= eLen;
    ppDataCpyIntBack(hugeInt, eType, eLen, posI); posI+= eLen;
    ppDataCpyIntBack(hugeInt, eRS, eLen, posI); posI+= eLen;
    ppDataCpyIntBack(hugeInt, eMP, eLen, posI); posI+= eLen;
    ppDataCpyIntBack(hugeInt, eFrom, eLen, posI); posI+= eLen;

    ppDataCpyIntBack(hugeInt, eNodesL, eLenL, posI); posI+= eLenL;



    ppDataCpyIntBack(hugeInt, nlNode, nlLen, posI); posI+= nlLen;
    ppDataCpyIntBack(hugeInt, nlType, nlLen, posI); posI+= nlLen;
    ppDataCpyIntBack(hugeInt, nlDir, nlLen, posI); posI+= nlLen;

		ppDataCpyDblBack(hugeDbl, nlVal, nlLen, posD); posD+= nlLen;

		if (elLen > 0)
		{
    	ppDataCpyIntBack(hugeInt, elElem, elLen, posI); posI+= elLen;
    	ppDataCpyIntBack(hugeInt, elType, elLen, posI); posI+= elLen;
    	/*ppDataCpyIntBack(hugeInt, elDir, elLen, posI); posI+= elLen;*/
    	ppDataCpyIntBack(hugeInt, elFrom, elLen, posI); posI+= elLen;
			
			ppDataCpyDblBack(hugeDbl, elValL, elLenL, posD); posD+= elLenL;

		}

	  grDir = hugeInt[sizeI-1]  ;
	  grVal = hugeDbl[sizeD-1]  ;

#if 0
	fprintf(msgout,"g = %f [%li]\n",grVal, grDir);
	fprintf(msgout,"? posI = %li (%i), posD= %li (%i)\n", posI,sizeI,posD,sizeD);

	fprintf(msgout,"\n### (%i):\n",ppRank);
for (i=0; i<sizeI; i++) { fprintf(msgout," %li ", hugeInt[i]); } fprintf(msgout,"|\n");
for (i=0; i<sizeD; i++) { fprintf(msgout," %f ", hugeDbl[i]); } fprintf(msgout,"|\n");
#endif

		free(hugeInt); hugeInt = NULL ; sizeI = 0 ;
		free(hugeDbl); hugeDbl = NULL ; sizeD = 0 ;
	}
	else
	{
		free(hugeInt); hugeInt = NULL ; sizeI = 0 ;
		free(hugeDbl); hugeDbl = NULL ; sizeD = 0 ;
	}
	return(rv);
}


/** Sends all input data 
 * @return status
 */
int femSendInitDataPP(void)
{
	int rv = AF_OK ;

	if ((rv = ppBcastSolData()) != AF_OK) {exit(AF_ERR_CMM);} ppSync();

	if ((rv = ppBcastLens()) != AF_OK) {exit(AF_ERR_CMM);} ppSync();

	if ((rv = ppBcastData()) != AF_OK) {exit(AF_ERR_CMM);} /*ppSync();*/

	return(rv);
}

/** Fits K_rows field to needed size (drops unused rows)
 * @param dofFrom starting row
 * @param dofTo ending row
 * @param nNumDOF number of used rows (dofTo - dofNum +1)
 */
int mod_K_rows(long dofFrom, long dofTo, long nNumDOF)
{
	long *tmpRow = NULL;
	long i,ii;
	long sum = 0;

	if ((tmpRow = femIntAlloc(nNumDOF)) == NULL)
	{
		exit(AF_ERR_MEM);
	}
	else
	{
		for (i=dofFrom; i<=dofTo; i++)
		{
			ii = i - 1 ;
			tmpRow[sum] = K_rows[ii];
			sum++;
		}

		femIntFree(K_rows);
		K_rows = tmpRow ;
		tmpRow = NULL ;
	}
#ifdef DEVEL_VERBOSE
#if 0
	for (i=0; i<nNumDOF; i++) { fprintf(msgout,"[?][%li] %li\n",i,K_rows[i]); }
#endif
#endif
	return(AF_OK);
}

/** allocates K,u,F 
 * @return status
 */
int femMPIsol_alloc(long nNumDOF)
{
	int rv = AF_OK;
#ifdef DEVEL_VERBOSE
	int i;

	fprintf(msgout,"KUKU0 (%i < %i) [%li %li]\n", ppRank, ppSize,nNumDOF, nDOFAct);
	for (i=0; i<nNumDOF; i++) {fprintf(msgout," %li ",K_rows[i]);} fprintf(msgout,"\n");
#endif

	if ((rv = femSparMatInitDesc(&K, nNumDOF, nDOFAct, K_rows)) != AF_OK)
	   { goto memFree; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"KUKU1 = %i (%i)\n",rv, ppRank);
#endif

	if ((rv = femVecFullInit(&u, nNumDOF)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&F, nNumDOF)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&Fr, nNumDOF)) != AF_OK) { goto memFree; }

	/* full size displacement vector: */
	if ((rv = femVecFullInit(&u_big, nDOFAct)) != AF_OK) { goto memFree; }


	switch(solNoLinS)
	{
		case 1: /* nothing to do for linear solution */ 
						femVecNull(&u_tot);
						break;
		case 2: /* NRM */ 
						if ((rv = femVecFullInit(&u_tot, nNumDOF)) != AF_OK) 
							 { goto memFree; }
						break;
		default:
						femVecNull(&u_tot);
						break;
	}

	if (rv == AF_OK) {return(rv);}
memFree:
 	femMatFree(&K);
	femVecFree(&u);
	femVecFree(&F);
	femVecFree(&u_big);
	femVecFree(&u_tot);
	return(rv);
}

/* allocates result fields */
int femMPIsol_res_alloc(void)
{
	int rv = AF_OK;
	long sum = 0;
	long val = 0;
	long i;

	femResNull();

	/* results on elements */
	if ((resEFrom=femIntAlloc(eLen)) == NULL)
	{
		rv = AF_ERR_MEM;
		goto memFree;
	}

	for (i=0; i<eLen; i++) { sum += Elem[eType[i]].rvals(i) ; }

	if ((resEVal=femDblAlloc(sum)) == NULL)
	{
		rv = AF_ERR_MEM;
		goto memFree;
	}

	for (i=0; i<eLen; i++) { resEFrom[i] = Elem[eType[i]].rvals(i) ; }

	resELen = sum ;

	sum = 0;
	for (i=0; i<eLen; i++)
	{
		val = resEFrom[i] ;
		resEFrom[i] = sum ;
		sum += val;
	}

	/* reactions */
#ifndef BLOCK_FOR_MPI
	resRLen = 0;

	for (i=0; i<nlLen; i++)
	{
		if ((nlType[i] == 1) || (nlType[i] == 2)||(nlType[i] == 4)||(nlType[i] == 5)||(nlType[i] == 6)||(nlType[i] == 7))
		{ 
			if ((nlType[i] == 1) && (nlVal[i] != 0.0)) {continue;}

			resRLen++; 
		}
	}

	if (resRLen <= 0)
	{
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[W] %s!\n", _("No boundary conditions defined"));
#endif
#if 0 /* if ==0 boundary conditions aren't tested! */
		rv = AF_ERR_SML ;
		goto memFree;
#else
	resRnode = NULL ;
	resRdof  = NULL ;
	resRval  = NULL ;
	resRval0 = NULL ;
#endif
	}
  else
  {
	  if ((resRnode = femIntAlloc(resRLen)) == NULL) { rv = AF_ERR_MEM; goto memFree; }
	  if ((resRdof  = femIntAlloc(resRLen)) == NULL) { rv = AF_ERR_MEM; goto memFree; }
	  if ((resRval  = femDblAlloc(resRLen)) == NULL) { rv = AF_ERR_MEM; goto memFree; }
	  if ((resRval0 = femDblAlloc(resRLen)) == NULL) { rv = AF_ERR_MEM; goto memFree; }

	  sum = 0;
	  for (i=0; i<nlLen; i++)
	  {
		  if ((nlType[i] == 1) || (nlType[i] == 2)||(nlType[i] == 4)||(nlType[i] == 5)||(nlType[i] == 6)||(nlType[i] == 7))
		  {
			  if ((nlType[i] == 1) && (nlVal[i] != 0.0)) {continue;}

			  resRnode[sum] = nlNode[i];
			  resRdof[sum]  = nlDir[i];
			  resRval[sum]  = 0 ;
			  resRval0[sum]  = 0 ;
			  sum++;
		  }
	  }
  }
#endif

	if (rv == AF_OK) {return(rv);}
memFree:
	femResFree();
	return(rv);
}

/** Solution with MPI 
 * @return status
 */
int femSolveMPI(void)
{
	int   rv = AF_OK;
	long *ppElist = NULL;
	long  ppElen = 0;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[I] %s:\n",_("Solution"));
#endif

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

	/* DOF structure */
 	if ((rv = fem_dofs()) != AF_OK) { exit(AF_ERR_CMM); }

	/* finding of rellevant nodes */
	femMyNodes(ppRank, ppSize, &ppNfrom, &ppNnum);

#ifndef _SMALL_FEM_CODE_
	femSaPoSetNode(ppNfrom, ppNnum);
	if ((rv = femSaPoAlloc(solNoLinStepMax, 0)) != AF_OK) {return(rv);}
	femSaPoLen  = solNoLinStepMax ;
#endif

	/* finding of relevant elements */
  if ((ppElist = femMyElems(ppNfrom, ppNnum, &ppElen)) == NULL)
	   { exit(AF_ERR_MEM); }

	/* elimination of unwanted elements */
  if ((femDelOthElem(ppElen, ppElist)) != AF_OK)
		 { exit(AF_ERR_MEM); }

	femIntFree(ppElist);

	/* finding of relevant DOFs */
  femMyDOFs(ppNfrom, ppNnum, &ppDOFfrom, &ppDOFto);

	ppDOFlen = ppDOFto - ppDOFfrom + 1 ;

	/* allocation of solution - related data (K, F, u)*/
	if ((rv = mod_K_rows(ppDOFfrom, ppDOFto, ppDOFlen)) != AF_OK) { goto memFree;}
 	if ((rv = femMPIsol_alloc(ppDOFlen)) != AF_OK) { goto memFree; }
	/* space for results */
 	if ((rv = femMPIsol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */

 	if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 	if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }
 	if ((rv = fem_add_disps(AF_YES,0)) != AF_OK) { goto memFree; }

#if 0
	femMatPrn(&K, "K_div");
	femVecPrn(&F, "F");
#endif

#if 1
	if ((rv = femMPIeqsPCGwJ_New(&K, &F, &u, &u_big, FEM_ZERO/1000.0, nDOFAct)) != AF_OK) { goto memFree; }
#else
	if ((rv = femMPIeqsCGwJ(&K, &F, &u, &u_big, FEM_ZERO/1000.0, nDOFAct)) != AF_OK) { goto memFree; }
#endif
#if 1

	if (femUseSaPo == AF_YES)
	{
		femSaPoInput(1.0, 
			femVecGet(&u,femKpos(femSaPoNode - ppNfrom, U_X)),
			femVecGet(&u,femKpos(femSaPoNode - ppNfrom, U_Y)),
			femVecGet(&u,femKpos(femSaPoNode - ppNfrom, U_Z)),
			AF_NO,
			AF_NO
		) ;
	}


#if 0
	femVecPrn(&u, _("DOF solution"));
#endif

 	if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }

#if 1
	rv = femMPI_SendResults(1.0) ;
#endif

#ifdef DEVEL_VERBOSE
	if (rv == AF_OK) { fprintf(msgout,"[I] %s!\n", _("Solution done")); }
	else { fprintf(msgout,"[E] %s!\n", _("Solution failed")); }
#endif

#endif /* end of GGG */

memFree:
	fem_sol_free();
	femVecFree(&u_big);
	femDataFree();
	femResFree();
	return(rv);
}

/* -------------------------------------------------------- */
/** Non-linear analysis: Newton-Raphson Method
 * @param incr_type method for computation on load increment
 * @return status
 */
int femSolveNRM_MPI(long incr_type)
{
	int rv = AF_OK;
	/*long  ppNfrom;
	long  ppNnum;*/
	long *ppElist = NULL;
	long  ppElen = 0;
	double multF = 1.0 ;   /* load multiplier      */
	double multSum = 0.0 ; /* total load multiplier*/
	double crit  = 1e-6 ;  /* convergence criteria */
	double rsize = 1e10 ;  /* max residuum size    */
	double normF = 0.0 ;
	double normFr= 0.0 ;
	int steps    = 1;      /* number of steps      */
	int substeps = 800;      /* number of iterations */
	int converged = AF_NO ;
	int i,j;
	double my_norms[2];
	double norms[2];
#ifdef NRM_SPDP
	tVector u_spdp; /* speedup vector */
#endif

	/*crit     = solNoLinPre ;*/
	steps    = solNoLinSteps ;
	/*substeps = solNoLinStepMax ;
	rsize    = solNoLinBig ;*/

#ifdef NRM_SPDP
	femVecNull(&u_spdp) ; /* speedup vector */
#endif
	
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[I] NRM %s:\n",_("Solution"));
#endif

	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

	/* DOF structure */
 	if ((rv = fem_dofs()) != AF_OK) { exit(AF_ERR_CMM); }

	/* finding of rellevant nodes */
	femMyNodes(ppRank, ppSize, &ppNfrom, &ppNnum);
		/*printf("INIT [%i] FROM: %li LEN: %li\n",ppRank, ppNfrom, ppNnum);*/

#ifndef _SMALL_FEM_CODE_
	femSaPoSetNode(ppNfrom, ppNnum);
	ppSync();
	if ((rv = femSaPoAlloc(solNoLinStepMax, 0)) != AF_OK) {return(rv);}
	femSaPoLen  = solNoLinStepMax ;
#endif

	/* finding of rellevant elements */
  if ((ppElist = femMyElems(ppNfrom, ppNnum, &ppElen)) == NULL)
	   { exit(AF_ERR_MEM); }

	/* elimination of unwanted elements */
  if ((femDelOthElem(ppElen, ppElist)) != AF_OK)
		 { exit(AF_ERR_MEM); }

	femIntFree(ppElist);

	/* finding of rellevant DOFs */
  femMyDOFs(ppNfrom, ppNnum, &ppDOFfrom, &ppDOFto);

	ppDOFlen = ppDOFto - ppDOFfrom + 1 ;

	/* allocation of solution - related data (K, F, u)*/
	if ((rv = mod_K_rows(ppDOFfrom, ppDOFto, ppDOFlen)) != AF_OK) { goto memFree;}
 	if ((rv = femMPIsol_alloc(ppDOFlen)) != AF_OK) { goto memFree; }
	/* space for results */
 	if ((rv = femMPIsol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */

	if (femTangentMatrix == AF_YES) { femVecSetZero(&u_tot) ; }

#ifdef NRM_SPDP
	femVecFullInit(&u_spdp, ppDOFlen) ; /* speedup vector */
#endif
 	
	/* loop: */
	for (i=1; i<=steps; i++)
	{
#ifdef USE_MPI_LOG
		/*printf("[%i] Step: %i\n",ppRank,i);*/
		if (ppRank == 0) { printf("[ ] Step: %i\n",i); }
#endif
		femVecSetZero(&F);
		if (j == 1) { femVecSetZero(&u); } /* TEST IT !!! */
		femMatSetZero(&K);

#ifdef NRM_SPDP
		if (i > 1) { femVecClone(&u_spdp, &u);} /* speedup */
#endif
		
		multF = femMultNRM(incr_type, i, steps);

		if (femTangentMatrix == AF_YES) { multSum += multF ; }
		else                            { multSum  = multF ; }

 		if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 		if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }

		femValVecMultSelf(multF, &F);
 		if ((rv = fem_add_disps(AF_YES,0)) != AF_OK) { goto memFree; }

		if ((rv = femMPIeqsPCGwJ_New(&K, &F, &u, &u_big, FEM_ZERO/100.0, nDOFAct)) != AF_OK) { goto memFree; }

#ifdef NRM_SPDP
		femVecClone(&u, &u_spdp); /* speedup */
#endif

		if (femTangentMatrix == AF_YES) { femVecAddVec(&u_tot, 1.0, &u) ; }


			my_norms[0] = femVecNormMPI(&F) ;
			my_norms[1] = 0 ;

			MPI_Allreduce(my_norms, norms, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
			normF  = sqrt(norms[0]);


		/* iteration: */
		for (j=1; j<=substeps; j++)
		{
#ifdef USE_MPI_LOG
		/*printf("[%i] Step: %i (mult = %f) Iteration: %i\n",ppRank, i, multSum, j);*/
		if (ppRank == 0) { printf("[ ] Step: %i (mult = %f) Iteration: %i\n", i, multSum, j); }
#endif
			femMatSetZero(&K);
			femVecSetZero(&Fr);
 			if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }

			my_norms[0] = femVecNormMPI(&Fr) ;
			norms[0] = 0 ;

			MPI_Allreduce(my_norms, norms, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
			ppSync();
			normFr = sqrt(norms[0]);

#if 0
			printf("Fr_size = %li (%i) \n",Fr.rows,ppRank);
			printf("%i.%i NORM_SIZE [%i] : %e <> %e*%e = %e\n",i,j,ppRank,
					normFr, normF, rsize, normF*rsize);
#endif

			ppSync() ;

			if (normFr > (normF*rsize))
			{
				/* residuum too big */
				converged = AF_NO ;
#ifdef DEVEL_VERBOSE
				fprintf(msgout,"[E] %s (%e)!\n", _("Unconverged solution - residuum too large"), normFr);
#endif
			}

			/* convergence testing: */
			if ((normFr/normF) <= crit )
			{
				converged = AF_YES ;
#if 0
				if ((rv = femWriteRes( "step.txt" )) != AF_OK) { goto memFree; }
#endif
				break ;
			}
			else
			{
				converged = AF_NO ;
				
				femMatSetZero(&K);
				femVecSetZero(&F);
				femVecSetZero(&u);
					
 				if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }

				if (femTangentMatrix == AF_YES)
				{
					femVecSetZero(&F);
					femVecClone(&Fr, &F);
					femVecSetZero(&Fr);

 					if ((rv = fem_add_disps(AF_YES,0)) != AF_OK) { goto memFree; }
				}
				else
				{
					femVecSetZero(&Fr);
					femVecSetZero(&F);

 					if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }
					femValVecMultSelf(multF, &F);
 					if ((rv = fem_add_disps(AF_YES,0)) != AF_OK) { goto memFree; }
				}

				if ((rv = femMPIeqsPCGwJ_New(&K, &F, &u, &u_big, FEM_ZERO/100.0, nDOFAct)) != AF_OK) { goto memFree; }

/*printf("WWW (%i) step=%i it=%i (%e | %e %e) \n",ppRank,i,j, femVecNorm(&u_big), normFr, femVecNorm(&F));*/

				if (femTangentMatrix == AF_YES) { femVecAddVec(&u_tot, 1.0, &u) ; }
				
			}
		}

		if (converged == AF_NO)
		{
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"[E] %s: %i !\n", _("Unconverged step"), i);
#endif
			goto memFree;
		}
		else
		{
		}
#ifndef _SMALL_FEM_CODE_
		if (femUseSaPo == AF_YES)
		{
			if (femTangentMatrix == AF_YES) 
			{
				femSaPoInput(multSum, 
					femVecGet(&u_tot,femKpos(femSaPoNode - ppNfrom, U_X)),
					femVecGet(&u_tot,femKpos(femSaPoNode - ppNfrom, U_Y)),
					femVecGet(&u_tot,femKpos(femSaPoNode - ppNfrom, U_Z)),
					AF_NO,
					AF_NO
					) ;
			}
			else
			{
				femSaPoInput(multSum, 
					femVecGet(&u,femKpos(femSaPoNode - ppNfrom, U_X)),
					femVecGet(&u,femKpos(femSaPoNode - ppNfrom, U_Y)),
					femVecGet(&u,femKpos(femSaPoNode - ppNfrom, U_Z)),
					AF_NO,
					AF_NO
					) ;
			}
		}
#endif

	}

	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); }
#if 0
	if (femTangentMatrix == AF_YES) { femVecClone(&u_tot, &u); }
#endif
	femMPI_SendResults(multSum) ;
	if (femTangentMatrix == AF_YES) { femVecSwitch(&u_tot, &u); }


#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[I] %s.\n",_("Solution done"));
#endif

memFree:
#ifdef NRM_SPDP
	femVecFree(&u_spdp); /* speedup */
#endif
	fem_sol_free();
	femVecFree(&u_big);
	femDataFree();
	femResFree();
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"Solution return: %i\n",rv);
#endif
	return(rv);
}

/* -------------------------------------------------------- */

int femWriteParRes(
										FILE   *fw,
										int     rank,
										long    dof_desc_len,
										long   *dof_desc,
										long    dof_len,
										double *dofs,
                    long    e_len,
                    long   *e_ids,
										long    res_from_len,
										long   *res_from,
										long    res_len,
										double *res,
										long    react_len,
										double *react,
										long   *react_node,
										long   *react_dof
									)
{
	int rv = AF_OK ;
	int i;

	if (ppRank == 0)
	{
		fprintf(fw,"%i\n", rank);

		fprintf(fw,"%li\n",dof_desc_len);
		for (i=0; i< dof_desc_len; i++) { fprintf(fw,"%li\n", dof_desc[i]); }

		fprintf(fw,"%li\n",dof_len);
		for (i=0; i< dof_len; i++) { fprintf(fw,"%e\n", dofs[i]); }

		fprintf(fw,"%li\n",react_len);
		for (i=0; i< react_len; i++) { fprintf(fw,"%li %li %e\n",react_node[i], react_dof[i], react[i]); }

		fprintf(fw,"%li\n",res_from_len);
		for (i=0; i< res_from_len; i++) { fprintf(fw,"%li %li\n", e_ids[i], res_from[i]); }
	
		fprintf(fw,"%li\n",res_len);
		for (i=0; i< res_len; i++) { fprintf(fw,"%e\n", res[i]); }
	}

	return(rv);
}

int femSendParRes(int rank, FILE *fw)
{
	int rv = AF_OK ;
	long *tmp_dof_desc = NULL ;
	double *tmp_dofs   = NULL ;
	long *tmp_eid      = NULL ;
	long *tmp_resfrom  = NULL ;
	double *tmp_res    = NULL ;
	double *tmp_react  = NULL ;
	long *tmp_react_node  = NULL ;
	long *tmp_react_dof  = NULL ;
	long dof_desc_len, dof_len, resfrom_len, res_len,react_len;
	long data[5];
	MPI_Status status ;

	
	if (ppRank == rank)
	{
		data[0] =  nDOFlen ;
		data[1] =  u.rows  ;
		data[2] =  eLen    ;
		data[3] =  resELen ;
		data[4] =  resRLen ;

		MPI_Send(data, 5, MPI_LONG, 0, 1, MPI_COMM_WORLD) ;
	}


	if (ppRank == 0)
	{
		MPI_Recv(data, 5, MPI_LONG, rank, 1, MPI_COMM_WORLD, &status) ;

		dof_desc_len 	= data[0];
		dof_len 			= data[1];
		resfrom_len 	= data[2];
		res_len 			= data[3];
		react_len 		= data[4];
	}

	ppSync();
	
	if (ppRank == 0)
	{
		if ((tmp_dof_desc = femIntAlloc(dof_desc_len)) == NULL)  {rv = AF_ERR_MEM; goto memFree ;}
		if ((tmp_dofs = femDblAlloc(dof_len)) == NULL)  {rv = AF_ERR_MEM; goto memFree ;}
		if ((tmp_eid = femIntAlloc(resfrom_len)) == NULL)  {rv = AF_ERR_MEM; goto memFree ;}
		if ((tmp_resfrom = femIntAlloc(resfrom_len)) == NULL)  {rv = AF_ERR_MEM; goto memFree ;}
		if ((tmp_res = femDblAlloc(res_len)) == NULL)  {rv = AF_ERR_MEM; goto memFree ;}
		if ((tmp_react = femDblAlloc(react_len)) == NULL)  {rv = AF_ERR_MEM; goto memFree ;}
		if ((tmp_react_node = femIntAlloc(react_len)) == NULL)  {rv = AF_ERR_MEM; goto memFree ;}
		if ((tmp_react_dof = femIntAlloc(react_len)) == NULL)  {rv = AF_ERR_MEM; goto memFree ;}
	}

	ppSync();
	

	if (ppRank == rank)
	{
		MPI_Send(nDOFfld, nDOFlen, MPI_LONG, 0, 1, MPI_COMM_WORLD) ;
	}
	if (ppRank == 0)
	{
		MPI_Recv(tmp_dof_desc, dof_desc_len, MPI_LONG, rank, 1, MPI_COMM_WORLD, &status) ;
	}
	ppSync();

	if (ppRank == rank)
	{
		MPI_Send(u.data, u.rows, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD) ;
	}
	if (ppRank == 0)
	{
		MPI_Recv(tmp_dofs, dof_len, MPI_DOUBLE, rank, 1, MPI_COMM_WORLD, &status) ;
	}
	ppSync();

#if 1 /* eID .. untested code: */
	if (ppRank == rank)
	{
		MPI_Send(eID, eLen, MPI_LONG, 0, 1, MPI_COMM_WORLD) ;
	}
	if (ppRank == 0)
	{
		MPI_Recv(tmp_eid, resfrom_len, MPI_LONG, rank, 1, MPI_COMM_WORLD, &status) ;
	}
#endif
	ppSync();


#if 1
	if (ppRank == rank)
	{
		MPI_Send(resEFrom, eLen, MPI_LONG, 0, 1, MPI_COMM_WORLD) ;
	}
	if (ppRank == 0)
	{
		MPI_Recv(tmp_resfrom, resfrom_len, MPI_LONG, rank, 1, MPI_COMM_WORLD, &status) ;
	}
#endif
#if 1
	ppSync();

	if (ppRank == rank)
	{
		MPI_Send(resEVal, resELen, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD) ;
	}
	if (ppRank == 0)
	{
		MPI_Recv(tmp_res, res_len, MPI_DOUBLE, rank, 1, MPI_COMM_WORLD, &status) ;
	}
	ppSync();
#endif
#if 1
  if (ppRank == rank)
	{
		MPI_Send(resRval, resRLen, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD) ;
	}
	if (ppRank == 0)
	{
		MPI_Recv(tmp_react, react_len, MPI_DOUBLE, rank, 1, MPI_COMM_WORLD, &status) ;
	}
	ppSync();
#endif

#if 1
  if (ppRank == rank)
	{
		MPI_Send(resRnode, resRLen, MPI_LONG, 0, 1, MPI_COMM_WORLD) ;
	}
	if (ppRank == 0)
	{
		MPI_Recv(tmp_react_node, react_len, MPI_LONG, rank, 1, MPI_COMM_WORLD, &status) ;
	}
	ppSync();
#endif

#if 1
  if (ppRank == rank)
	{
		MPI_Send(resRdof, resRLen, MPI_LONG, 0, 1, MPI_COMM_WORLD) ;
	}
	if (ppRank == 0)
	{
		MPI_Recv(tmp_react_dof, react_len, MPI_LONG, rank, 1, MPI_COMM_WORLD, &status) ;
	}
	ppSync();
#endif


#if 1
	if (ppRank == 0)
	{
		rv = femWriteParRes(
										fw,
										rank,
										dof_desc_len,
										tmp_dof_desc,
										dof_len,
										tmp_dofs,
										resfrom_len, /* == e_len*/
                    tmp_eid,
										resfrom_len,
										tmp_resfrom,
										res_len,
										tmp_res,
										react_len,
										tmp_react,
										tmp_react_node,
										tmp_react_dof
									) ;
	}
#endif
memFree:
	if (ppRank == 0)
	{
		free(tmp_react); tmp_react = NULL ;
		free(tmp_react_node); tmp_react_node = NULL ;
		free(tmp_react_dof); tmp_react_dof = NULL ;
		free(tmp_res); tmp_res = NULL ;
		free(tmp_resfrom); tmp_resfrom = NULL ;
		free(tmp_dof_desc); tmp_dof_desc = NULL ;
		free(tmp_dofs); tmp_dofs = NULL ;
	}

	return(rv);
}

/** Sends and saves results */
int femMPI_SendResults(double mult)
{
	int rv = AF_OK ;
#if 1
	int i ;
	FILE *fw = NULL ;

	if (ppRank == 0)
	{
		if ((fw = fopen(fem_output_file(),"w")) == NULL)
		{
#if DEVEL_VERBOSE
			fprintf(msgout, "[E] %s!\n", _("Can NOT open file for results")) ;
#endif
			return(AF_ERR_IO);
		}
	
		fprintf(fw,"M\n %li %li %e\n %i\n", solID, solDepOn, mult, ppSize);

		/* write data from 1st node */
 		if (femWriteParRes(fw,
					          0,
										nDOFlen,
										nDOFfld,
										u.rows,
										u.data,
										eLen,
                    eID,
										eLen, /* == number of results */
										resEFrom,
										resELen,
										resEVal,
										resRLen,
										resRval,
										resRnode,
										resRdof
									) != AF_OK)
 		{
	 		goto memFree;
 		}
	}

#if 0
		femMatFree(&K);
		femMatNull(&K);
#endif
#if 0
		femVecFree(&F);
		femVecNull(&F);
#endif


#if 1
	if (ppSize > 1)
	{
		for (i=1; i<ppSize; i++)
		{
			rv = femSendParRes(i, fw) ;
		}
	}
#endif

memFree:
	if (ppRank == 0)
	{
		fclose(fw);
	}
#endif
	return(rv);
}

#endif /* end of USE_MPI */

/* end of fem_mpi.c */
