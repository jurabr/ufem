/*
   File name: fem_sol.c
   Date:      2003/04/09 14:00
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more etails.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.

	 FEM Solver - solver
*/

#include "fem_sol.h"
#include "fem_rand.h"

#ifdef USE_MPI
extern long ppNfrom;
extern long ppNnum;
#endif

int  femTangentMatrix = AF_YES;
int  fem2ndOrder      = AF_NO ;
int  fem2ndOrderIter  = AF_NO ;
int  femRunSolIterBC  = AF_NO ;
int  femTestIterBC    = AF_NO ;
long femIterBCchange  = 0 ;
long femIterEchange   = 0 ;

long femHaveThermDOFs = AF_NO ;
long femHaveThermLoad = AF_NO ;

long  nDOFAct  = 0 ;    /* total number of ACTIVE DOFs in structure (== size of "K" matrix) */
long  nDOFlen  = 0 ;    /* lenght of nDOFfld                        */
long *nDOFfld  = NULL ; /* description of DOFs in nodes             */
long *K_rows   = NULL ; /* numbers of items in "K" rows K_rows[nDOFAct] */
long *K_rowsAL = NULL ; /* K_rows for full ALM */

tMatrix K; /* structure stiffness matrix    */
tVector F; /* structure load vector         */
tVector Fr;/* unballanced forces vector     */
tVector u; /* structure displacement vector */
tVector u_tot; /* structure displacement vector - total displacements (NRM) */

tVector u_i ;  /* status field for BC iterations */

tVector du0 ; /* substep displacement vector for ALM */
tVector dut ;  /* substep unballanced displacement vector for ALM */
tVector dur ;  /* substep unballanced displacement vector for ALM */
tVector du  ;  /* substep unballanced displacement vector for ALM */

tVector F_0 ;  /* backup load vector for ALM and full ALM; also for NRM */

tVector dFr  ;   /* step sum of unballanced forces for full ALM  */
tVector FF   ; /* whole load vector full ALM  */

tMatrix KF  ;  /* structure and step matrix for full ALM */
tVector uLa ;  /* displacement and lambda vectro for full ALM */
tVector Fra ;  /* load and arc lenght vector for full ALM */

tVector uTemp; /* thermal loads field */


/* dynamics - general : */
tMatrix M;         /* structure mass matrix    */

/* dynamics - eigenproblems : */
tVector eig_omega; /* vector of eigenvalues                */
tVector eig_x;     /* i-th iteration vector of eigenvalues */
tVector eig_xM;    /* i-th iteration vector of eigenvalues */
tMatrix eig_shap;  /* matrix of eigenvectors               */
tMatrix eig_oMK;   /* (-omega*M+ K matrix)                 */
tVector *eig_y ;   /* FIELD of Gram-Schmidt data           */

/* dynamics - newmark: */
tMatrix C;          /* damping matrix */
tMatrix KK;         /* combined stiffness matrix combined stiffness matrix */
tVector pp;         /* combined load vector               */
tVector dr;         /* displacement change                */
tVector ra;         /* temporary vector                   */
tVector rb;         /* temporary vector                   */
tVector r0;         /* previous displacement              */
tVector rr0;        /* previous velocity                  */
tVector rrr0;       /* previous acceleration              */
tVector rr1;        /* current velocity                   */
tVector rrr1;       /* current acceleration               */
tVector F_0 ;       /* initial load vector                */

#ifdef _USE_THREADS_
pthread_mutex_t *mutexK   = NULL ; /* mutexes for K  */
pthread_mutex_t *mutexF   = NULL ; /* mutexes for F  */
pthread_mutex_t *mutexFr  = NULL ; /* mutexes for Fr */

typedef struct /* for threads if fem_fill_K */
{
  long mode ;
  long from ;
  long to ;
}tThKe ;

extern int femAddThermLoads(void);

/* creates mutexes for "K", "F", "Fr" */
int thrInit_K_mutex(void)
{
  long i ;

  if ((mutexK=(pthread_mutex_t *)malloc(K.rows*sizeof(pthread_mutex_t)))==NULL) 
     { return(AF_ERR_MEM) ; }

  if ((mutexF=(pthread_mutex_t *)malloc(K.rows*sizeof(pthread_mutex_t)))==NULL) 
  {
    free (mutexK) ; mutexK = NULL ;
    return(AF_ERR_MEM) ; 
  }

  if ((mutexFr=(pthread_mutex_t *)malloc(K.rows*sizeof(pthread_mutex_t)))==NULL) 
  {
    free (mutexK) ; mutexK = NULL ;
    free (mutexF) ; mutexF = NULL ;
    return(AF_ERR_MEM) ; 
  }

  for (i=0; i<K.rows; i++) 
  { 
    pthread_mutex_init (&mutexK[i], NULL);
    pthread_mutex_init (&mutexF[i], NULL);
    pthread_mutex_init (&mutexFr[i], NULL);
  }
  return (AF_OK);
}

/* frees mutexes for "K" */
void thrFree_K_mutex(void)
{
  free(mutexK) ; mutexK = NULL ;
  free(mutexF) ; mutexF = NULL ;
  free(mutexFr) ; mutexFr = NULL ;
}
#endif

/* Computes total potential energy of the structure */
double femCompPE(tMatrix *K, tVector *u, int Print)
{
  double Pi = 0 ;
	long   size ;
	tVector Ku;

	size = K->rows ;
	femVecNull(&Ku);
	if ((femVecFullInit(&Ku, size))!=AF_OK){goto memFree;}

	femMatVecMultBig(K,u, &Ku);
	Pi = femVecVecMultBig(&Ku, u) ;

	if (Print == AF_YES) { fprintf(msgout, "[ ] %s: %e\n", _("Potential energy"), Pi); }

memFree:
	femVecFree(&Ku);
  return(Pi);
}



/* sets some of matrices and vectors to NULL */
void fem_sol_null(void)
{
	femMatNull(&K);
	femVecNull(&F);
	femVecNull(&Fr);
	femVecNull(&u);

	femVecNull(&u_tot);

	femVecNull(&u_i);

  femVecNull(&uTemp);


	nDOFfld = NULL ;
	K_rows = NULL ;
	K_rowsAL = NULL ;

  if (femDynamics == AF_YES)
  {
	  femMatNull(&M);
    if (femEigenModal == AF_YES)
    {
      solNoLinS = 1 ;

	    femVecNull(&eig_omega);
	    femMatNull(&eig_shap);
	    femMatNull(&eig_oMK);

	    femVecNull(&eig_x);
	    femVecNull(&eig_xM);

	    eig_y = NULL ;
    }
    else
    {
      if (femNewmarkEL == AF_YES)
      {
#if 0
	      femMatNull(&KK);
	      femMatNull(&C);
#endif

	      femVecNull(&pp);
	      femVecNull(&dr);
	      femVecNull(&ra);
	      femVecNull(&rb);
	      femVecNull(&r0);
	      femVecNull(&rr0);
	      femVecNull(&rrr0);
	      femVecNull(&rr1);
	      femVecNull(&rrr1);
	      femVecNull(&F_0);
      }
    }
  }

  if (femThermTrans == AF_YES)
  {
	  femMatNull(&M);
	  femMatNull(&KK);
	  femMatNull(&C);

	  femVecNull(&F_0);
	  femVecNull(&r0);
	  femVecNull(&rr0);
	  femVecNull(&pp);
  }

	switch(solNoLinS)
	{
		case 1: /* nothing to do for linear solution */ 
						break;
		case 2: /* NRM */ 
		case 6: /* DDS */ 
						femVecNull(&u_tot);
						break;
	  case 3: /* ALM */ 
						femVecNull(&u_tot);
						femVecNull(&du0);
						femVecNull(&dut);
						femVecNull(&dur);
						femVecNull(&du );
						femVecNull(&F_0);
						break;
		case 4: /* full ALM */
						femVecNull(&u_tot);
						femVecNull(&dFr);
						femVecNull(&FF);
						femVecNull(&du0);
						
						femVecNull(&uLa);
						femVecNull(&Fra);

						femMatNull(&KF);
						break;
		case 5: /* iterative 2nd order solver */
						femVecNull(&u_tot);
						femVecNull(&du );
						femVecNull(&F_0);
						break;
	}
}

/* sets some of matrices and vectors to NULL */
void fem_sol_free(void)
{
  long i ;
	femMatFree(&K);
	femVecFree(&F);
	femVecFree(&Fr);
	femVecFree(&u);

	femIntFree(nDOFfld);
	femIntFree(K_rows);
	nDOFAct = 0 ;
	nDOFlen = 0 ;

	if (femRunSolIterBC == AF_YES)
	{
	  femVecFree(&u_i);
	}

  if ( (femHaveThermLoad == AF_YES)||(femReadPrevThr == AF_YES) )
  {
    femVecFree(&uTemp);
  }

  if (femDynamics == AF_YES)
  {
	  femMatFree(&M);
    if (femEigenModal == AF_YES)
    {
	    femVecFree(&eig_omega);
	    femMatFree(&eig_shap);
	    femMatFree(&eig_oMK);

	    femVecFree(&eig_x);
	    femVecFree(&eig_xM);

      if ((femEigenNum > 0)&&(eig_y != NULL))
      {
        for (i=0; i<femEigenNum; i++)
        {
	        femVecFree(&eig_y[i]);
        }
        free(eig_y);
      }
    }
    else
    {
      if (femNewmarkEL == AF_YES)
      {
#if 0
	      femMatFree(&KK);
	      femMatFree(&C);
#endif

	      femVecFree(&pp);
	      femVecFree(&dr);
	      femVecFree(&ra);
	      femVecFree(&rb);
	      femVecFree(&r0);
	      femVecFree(&rr0);
	      femVecFree(&rrr0);
	      femVecFree(&rr1);
	      femVecFree(&rrr1);
	      femVecFree(&F_0);
      }
    }
  }
  
  if (femThermTrans == AF_YES)
  {
	  femMatFree(&M);
	  femMatFree(&KK);
	  femMatFree(&C);

	  femVecFree(&F_0);
	  femVecFree(&r0);
	  femVecFree(&rr0);
	  femVecFree(&pp);
  }

	switch(solNoLinS)
	{
		case 1: /* nothing to do for linear solution */ 
            if (femReadPrevStep==AF_YES)
            {
						  femVecFree(&u_tot);
            }
						break;
		case 2: /* NRM */ 
		case 6: /* DDS */ 
						femVecFree(&u_tot);
						break;
	  case 3: /* ALM */ 
#if 1
						femVecFree(&u_tot);
						femVecNull(&du0);
						femVecNull(&dut);
						femVecNull(&dur);
						femVecNull(&du );
						femVecFree(&F_0);
						break;
#endif
		case 4: /* full ALM */
						femVecFree(&u_tot);
						femVecFree(&dFr);
						femVecFree(&FF);
						femVecFree(&du0);
						
						femVecFree(&uLa);
						femVecFree(&Fra);

						femMatFree(&KF);
		
						femIntFree(K_rowsAL);
						break;
    case 5: /* iterative 2nd*/
						femVecFree(&u_tot);
						femVecFree(&du );
						femVecFree(&F_0);
						break;
	}

#ifdef _USE_THREADS_
  thrFree_K_mutex(); 
#endif
}

/** allocates K,u,F 
 * @return state value
 */
int fem_sol_alloc(void)
{
	int rv = AF_OK;
  long i ;

#if 1
	if ((rv = femSparMatInitDesc(&K, nDOFAct, nDOFAct, K_rows)) != AF_OK)
#else
	if ((rv = femFullMatInit(&K, nDOFAct, nDOFAct)) != AF_OK)
#endif
	   { goto memFree; }
	if ((rv = femVecFullInit(&u, nDOFAct)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&F, nDOFAct)) != AF_OK) { goto memFree; }
	if ((rv = femVecFullInit(&Fr, nDOFAct)) != AF_OK) { goto memFree; }

	if (femRunSolIterBC == AF_YES)
	{
		if ((rv = femVecFullInit(&u_i, nDOFAct)) != AF_OK) { goto memFree; }
	}

  /* test for thermal loads */
  if (((femHaveThermLoad=femTestThermStructElems()) == AF_YES)||(femReadPrevThr == AF_YES))
  {
	  if ((rv = femVecFullInit(&uTemp, nLen)) != AF_OK) { goto memFree; }

    for (i=0; i<nlLen; i++) /* filling of temp field: */
    {
      if (nlDir[i] == TEMP)
      {
        femVecAdd( &uTemp, nlNode[i]+1, nlVal[i] );
      }
    }
  }

  if (femDynamics == AF_YES)
  {
    if (femEigenModal == AF_YES)
    {
	    if ((rv = femSparMatInitDesc(&M, nDOFAct, nDOFAct, K_rows)) != AF_OK)
	       { goto memFree; }

	    if ((rv=femSparMatInitDesc(&eig_oMK,nDOFAct,nDOFAct, K_rows)) != AF_OK)
         { goto memFree; }

	    if ((rv=femFullMatInit(&eig_shap,nDOFAct,femEigenNum)) != AF_OK) { goto memFree; }

	    if ((rv = femVecFullInit(&eig_omega, nDOFAct)) != AF_OK) { goto memFree; }
	    if ((rv = femVecFullInit(&eig_x, nDOFAct)) != AF_OK) { goto memFree; }
	    if ((rv = femVecFullInit(&eig_xM, nDOFAct)) != AF_OK) { goto memFree; }

      if (femEigenNum > 0)
      {
        if ((eig_y=(tVector *)malloc(femEigenNum*sizeof(tVector))) == NULL)
        {
          rv = AF_ERR_MEM ;
          goto memFree ;
        }
        for (i=0; i<femEigenNum; i++)
        {
	        if ((rv = femVecFullInit(&eig_y[i], nDOFAct)) != AF_OK) { goto memFree; }
        }
      }
    }
    else
    {
      if (femNewmarkEL == AF_YES)
      {
#if 0
	      if ((rv = femSparMatInitDesc(&KK, nDOFAct, nDOFAct, K_rows)) != AF_OK)
	         { goto memFree; }
	      if ((rv = femSparMatInitDesc(&C, nDOFAct, nDOFAct, K_rows)) != AF_OK)
	         { goto memFree; }
#endif

	      if ((rv = femVecFullInit(&pp, nDOFAct)) != AF_OK) { goto memFree; }
	      if ((rv = femVecFullInit(&dr, nDOFAct)) != AF_OK) { goto memFree; }
	      if ((rv = femVecFullInit(&ra, nDOFAct)) != AF_OK) { goto memFree; }
	      if ((rv = femVecFullInit(&rb, nDOFAct)) != AF_OK) { goto memFree; }
	      if ((rv = femVecFullInit(&r0, nDOFAct)) != AF_OK) { goto memFree; }
	      if ((rv = femVecFullInit(&rr0, nDOFAct)) != AF_OK) { goto memFree; }
	      if ((rv = femVecFullInit(&rrr0, nDOFAct)) != AF_OK) { goto memFree; }

	      if ((rv = femVecFullInit(&rr1, nDOFAct)) != AF_OK) { goto memFree; }
	      if ((rv = femVecFullInit(&rrr1, nDOFAct)) != AF_OK) { goto memFree; }
	      if ((rv = femVecFullInit(&F_0, nDOFAct)) != AF_OK) { goto memFree; }
      }
    }
  }

  if (femThermTrans == AF_YES)
  {
	  if ((rv = femSparMatInitDesc(&M, nDOFAct, nDOFAct, K_rows)) != AF_OK)
	     { goto memFree; }
	  if ((rv = femSparMatInitDesc(&KK, nDOFAct, nDOFAct, K_rows)) != AF_OK)
	     { goto memFree; }
	  if ((rv = femSparMatInitDesc(&C, nDOFAct, nDOFAct, K_rows)) != AF_OK)
	     { goto memFree; }

	  if ((rv = femVecFullInit(&F_0, nDOFAct)) != AF_OK) { goto memFree; }
	  if ((rv = femVecFullInit(&r0, nDOFAct)) != AF_OK) { goto memFree; }
	  if ((rv = femVecFullInit(&rr0, nDOFAct)) != AF_OK) { goto memFree; }
	  if ((rv = femVecFullInit(&pp, nDOFAct)) != AF_OK) { goto memFree; }
  }

  
	switch(solNoLinS)
	{
		case 2: /* NRM */ 
		case 6: /* DDS */ 
						if ((rv = femVecFullInit(&u_tot, nDOFAct)) != AF_OK) {goto memFree;}
						break;
		case 3: /* ALM */ 
#if 1
						if ((rv = femVecFullInit(&u_tot, nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&du0, nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&dut, nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&dur, nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&du , nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&F_0, nDOFAct)) != AF_OK) {goto memFree;}
						break;
#endif
		case 4: /* full ALM */
						if ((rv = femSparMatInitDesc(&KF, nDOFAct+1, nDOFAct+1, K_rowsAL)) != AF_OK)
	   					 { goto memFree; }
						if ((rv = femVecFullInit(&uLa, nDOFAct+1)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&Fra, nDOFAct+1)) != AF_OK) {goto memFree;}

						if ((rv = femVecFullInit(&u_tot, nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&dFr, nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&FF, nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&du0, nDOFAct)) != AF_OK) {goto memFree;}

						break;
		case 5: /* iterative 2nd */
						if ((rv = femVecFullInit(&u_tot, nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&du , nDOFAct)) != AF_OK) {goto memFree;}
						if ((rv = femVecFullInit(&F_0, nDOFAct)) != AF_OK) {goto memFree;}
						break;

		case 1: 
    default: /* nothing to do for linear solution */ 
            if (femReadPrevStep == AF_YES)
            {
						  if ((rv = femVecFullInit(&u_tot, nDOFAct)) != AF_OK) {goto memFree;}
            }
						break;
	}

#ifdef _USE_THREADS_
  /* K mutex: */
  if ((thrInit_K_mutex()) != AF_OK) {goto memFree;}
#endif

	if (rv == AF_OK) {return(rv);}
memFree:
	fem_sol_free();
#ifdef _USE_THREADS_
  thrFree_K_mutex(); 
#endif
	return(rv);
}

/* allocates result fields */
int fem_sol_res_alloc(void)
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
	resRLen = 0;
	
	for (i=0; i<nlLen; i++)
	{
		if ((nlType[i] == 1) || (nlType[i] == 2)|| (nlType[i] == 4)|| (nlType[i] == 5)|| (nlType[i] == 6)|| (nlType[i] == 7)) 
		{ 
			if ((nlType[i] == 1) && (nlVal[i] != 0.0)) {continue;}

			resRLen++; 
		}
	}

	if (resRLen <= 0)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[W] %s!\n", _("No boundary conditions defined"));
#endif
#if 0 /* if ==0 boundary conditions aren't tested! (usefull for winkler etc.) */
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

  /* there will be reading of data from previous step (if any) */
  if (femReadPrevStep == AF_YES)
  {
    rv = femReadRes(fem_rfile) ;
  }

  /* there will be reading of data from previous step (if any) */
  if (femReadPrevThr == AF_YES)
  {
    rv = femReadThermRes(fem_thrfile, &uTemp) ;
  }

	if (rv == AF_OK) {return(rv);}
memFree:
	femResFree();
	return(rv);
}

/** creates K_e matrices and fills K
 * @param mode mode  
 * @return state value
 */
int fem_fill_part_of_K(long mode, long e_from, long e_to)
{
	int rv = AF_OK;
	long ii;
	long eT = 0;
	long size = 0;
	tMatrix K_e;
	tVector F_e;
	tVector Fr_e;

	femMatNull(&K_e);
	femVecNull(&F_e);
	femVecNull(&Fr_e);

	for (ii=e_from; ii<e_to; ii++)
	{
		femMatNull(&K_e);
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"K_e[%li] start:\n",eID[ii]);
#endif

		eT = femGetETypePos(ii); /* element type */

		if ((eT < 1)||(eT >= elemLen)) { femMatNull(&K_e); return(AF_ERR_TYP); }
		size = Elem[eT].nodes * Elem[eT].dofs ; /* K_e size */

		/* K_e allocation */
    if ((rv = femFullMatInit(&K_e, size, size)) != AF_OK) 
       { femMatNull(&K_e); return(rv); }

		if (mode == AF_YES)
		{
			/* Allocation of vectors */
      if ((rv = femVecFullInit(&F_e, size)) != AF_OK) 
      { 
				femMatFree(&K_e); 
				femVecNull(&F_e); 
				return(rv); 
			}
      if ((rv = femVecFullInit(&Fr_e, size)) != AF_OK) 
      { 
				femMatFree(&K_e); 
				femVecFree(&F_e); 
				femVecNull(&Fr_e); 
				return(rv); 
			}
			femVecSetZeroBig(&Fr_e);
		}

		/* filling of element stiffness matrix: */
		if ((rv = Elem[eT].stiff(ii, mode, &K_e, &F_e, &Fr_e)) != AF_OK) 
    { 
      femMatFree(&K_e); return(rv); 
    }


		/* localization K-e -> K */
		if ((rv = femLocK_e(&K_e, ii)) != AF_OK) 
      	{ femMatNull(&K_e); return(rv); }

		
		if (mode == AF_YES) /* results are computed */
		{
#ifndef BLOCK_FOR_MPI
			/* localization F-e -> F */
			if ((rv = femLocF_e(&F_e, ii, &F, AF_YES)) != AF_OK) 
      { 
				femMatFree(&K_e); 
				femVecFree(&F_e); 
				femVecFree(&Fr_e); 
				return(rv); 
			}
#endif
			if ((rv = femLocF_e(&Fr_e, ii, &Fr,AF_NO)) != AF_OK) 
      { 
				femMatFree(&K_e); 
				femVecFree(&F_e); 
				femVecFree(&Fr_e); 
				return(rv); 
			}
		}

		if (mode == AF_YES) /* results are computed */
		{
			femVecFree(&F_e);
			femVecFree(&Fr_e);
		}
		
		femMatFree(&K_e);
	}

	return(rv);
}


/* computes reactions */
int fem_fill_K_React(long mode, long r_from, long r_to)
{
#ifndef BLOCK_FOR_MPI
  int i ;
	long pos;

	if (mode == AF_YES)
	{
		for (i=r_from; i<r_to; i++)
		{
#ifdef USE_MPI
			if ((resRnode[i]<ppNfrom)||(resRnode[i]>=(ppNfrom+ppNnum))) { resRval[i]=0.0; continue; }
			if ((pos = femKpos(resRnode[i]-ppNfrom, resRdof[i])) <= 0) { return(AF_ERR_VAL); }
#else
			if ((pos = femKpos(resRnode[i], resRdof[i])) <= 0) { return(AF_ERR_VAL); }
#endif
			resRval[i] += femVecGet(&F, pos);
		}
	}
#endif
  return(AF_OK);
}

#ifdef _USE_THREADS_
void *thFemFillPartOfK(void *param)
{
  tThKe *p = (tThKe*) param ;
  if (p->to <= 0) {return(NULL);}

  fem_fill_part_of_K(p->mode, p->from, p->to);
  return(NULL);
}

void *thFemFillKReact(void *param)
{
  tThKe *p = (tThKe*) param ;
  if (p->to <= 0) {return(NULL);}

  fem_fill_K_React(p->mode, p->from, p->to);
  return(NULL);
}
#endif

/** Creates K matrix from Ke matrices
 * @param mode mode: AF_YES for making of new matrix, AF_NO to use old
 * @return status
 */
int fem_fill_K(long mode)
{
#ifdef _USE_THREADS_
  /* threaded version */
	long i;
  tThKe data[AF_MAX_THREADS];
  pthread_t Thread_ID[AF_MAX_THREADS];
  ldiv_t  dnum ;
#endif

	if (mode == AF_YES) { femVecSetZeroBig(&Fr); }

#ifdef _USE_THREADS_
  if (femUseThreads != AF_YES)
  {
    /* use serial version instead */
    if ((fem_fill_part_of_K(mode, 0, eLen)) != AF_OK) {return(AF_ERR);}
    if ((fem_fill_K_React(mode, 0, resRLen)) != AF_OK) {return(AF_ERR);}
  }
  else
  {
    /* "K" matrix */
    if (eLen < femThreadMin)
    {
      if ((fem_fill_part_of_K(mode, 0, eLen)) != AF_OK) {return(AF_ERR);}
    }
    else
    { 
      dnum = ldiv(eLen, femThreadNum) ;
      if (dnum.quot < 1)
      {
        dnum.quot = 0 ;
        dnum.rem  = eLen ;
      }

      for (i=0; i<femThreadNum; i++)
      {
        data[i].mode = mode ;
        data[i].from = i*dnum.quot ;

        data[i].to = (i+1)*dnum.quot ;
        if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

        pthread_create(&Thread_ID[i],NULL,thFemFillPartOfK,&data[i]);
      }

      for (i=0; i<femThreadNum; i++)
      {
        pthread_join(Thread_ID[i], NULL) ;
      }
    }

    /* Reactions  */
    if (mode == AF_YES)
    {
      if (resRLen < femThreadMin)
      {
        if ((fem_fill_K_React(mode, 0, resRLen)) != AF_OK) {return(AF_ERR);}
      }
      else
      {
        dnum = ldiv(resRLen, femThreadNum) ;
        if (dnum.quot < 1)
        {
          dnum.quot = 0 ;
          dnum.rem  = resRLen ;
        }

        for (i=0; i<femThreadNum; i++)
        {
          data[i].mode = mode ;
          data[i].from = i*dnum.quot ;

          data[i].to = (i+1)*dnum.quot ;
          if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

          pthread_create(&Thread_ID[i],NULL,thFemFillKReact,&data[i]);
        }

        for (i=0; i<femThreadNum; i++)
        {
          pthread_join(Thread_ID[i], NULL) ;
        }
      }
    }
  }
#else
  /* serial code */
  if ((fem_fill_part_of_K(mode, 0, eLen)) != AF_OK) {return(AF_ERR);}
  if ((fem_fill_K_React(mode, 0, resRLen)) != AF_OK) {return(AF_ERR);}
#endif
  return(AF_OK);
}

/* FILL M START ************************************************ */

/** creates M_e matrices and fills M
 * @return status value
 */
int fem_fill_part_of_M(long e_from, long e_to)
{
	int rv = AF_OK;
	long ii;
	long eT = 0;
	long size = 0;
	tMatrix M_e;
	tVector F_e;
	tVector Fr_e;

	femMatNull(&M_e);
	femVecNull(&F_e);
	femVecNull(&Fr_e);

	for (ii=e_from; ii<e_to; ii++)
	{
		femMatNull(&M_e);
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"M_e[%li] start:\n",eID[ii]);
#endif

		eT = femGetETypePos(ii); /* element type */

		if ((eT < 1)||(eT >= elemLen)) { femMatNull(&M_e); return(AF_ERR_TYP); }
		size = Elem[eT].nodes * Elem[eT].dofs ; /* M_e size */

		/* M_e allocation */
    if ((rv = femFullMatInit(&M_e, size, size)) != AF_OK) 
       { femMatNull(&M_e); return(rv); }
		
		/* filling of element mass matrix: */
		if ((rv = Elem[eT].mass(ii, &M_e)) != AF_OK) 
    { 
      femMatFree(&M_e); return(rv); 
    }


		/* localization M-e -> M */
		if ((rv = femLocM_e(&M_e, ii)) != AF_OK) 
      	{ femMatNull(&M_e); return(rv); }
		
		femMatFree(&M_e);
	}

	return(rv);
}


#ifdef _USE_THREADS_
void *thFemFillPartOfM(void *param)
{
  tThKe *p = (tThKe*) param ;
  if (p->to <= 0) {return(NULL);}

  fem_fill_part_of_M(p->from, p->to);
  return(NULL);
}
#endif

/** Creates M matrix from Me matrices
 * @param mode mode: AF_YES for making of new matrix, AF_NO to use old
 * @return status
 */
int fem_fill_M(void)
{
#ifdef _USE_THREADS_
  /* threaded version */
	long i;
  tThKe data[AF_MAX_THREADS];
  pthread_t Thread_ID[AF_MAX_THREADS];
  ldiv_t  dnum ;
#endif

#ifdef _USE_THREADS_
  if (femUseThreads != AF_YES)
  {
    /* use serial version instead */
    if ((fem_fill_part_of_M(0, eLen)) != AF_OK) {return(AF_ERR);}
  }
  else
  {
    /* "K" matrix */
    if (eLen < femThreadMin)
    {
      if ((fem_fill_part_of_M(0, eLen)) != AF_OK) {return(AF_ERR);}
    }
    else
    { 
      dnum = ldiv(eLen, femThreadNum) ;
      if (dnum.quot < 1)
      {
        dnum.quot = 0 ;
        dnum.rem  = eLen ;
      }

      for (i=0; i<femThreadNum; i++)
      {
        /*data[i].mode = mode ;*/
        data[i].from = i*dnum.quot ;

        data[i].to = (i+1)*dnum.quot ;
        if (i == (femThreadNum-1)) { data[i].to += dnum.rem ; }

        pthread_create(&Thread_ID[i],NULL,thFemFillPartOfK,&data[i]);
      }

      for (i=0; i<femThreadNum; i++)
      {
        pthread_join(Thread_ID[i], NULL) ;
      }
    }
  }
#else
  /* serial code */
  if ((fem_fill_part_of_M(0, eLen)) != AF_OK) {return(AF_ERR);}
#endif
  return(AF_OK);
}
/* FILL M END ************************************************** */

/** Adds loads and boundary conditions 
 * @param transient analysis step (0 for all other!); starts from 1!
 */
int fem_add_loads(long step)
{
	int rv = AF_OK; 
	long i;
  double mult ;

	/* gravitation  */
	if (fabs(grVal) > 0)
	{
   	if ((rv = femAddGravLoad(grDir, grVal)) != AF_OK)
		{
			return(rv);
		}
	}

	/* loads on elements */
	for (i=0; i< elLen; i++)
	{
   	if ((rv = femAddElemLoad()) != AF_OK)
		{
			return(rv);
		}
	}

  /* Thermal loads from structural analysis */
  if (femHaveThermLoad == AF_YES)
  {
    if ((rv = femAddThermLoads()) != AF_OK)
		{
			return(rv);
		}
  }

	/* loads on nodes: */
	for (i=0; i< nlLen; i++)
	{
    if (step > 0)
    {
      if (nlTrPos[i] > -1) 
      {
        mult = transMult[nlTrPos[i]][step-1]*nlVal[i]; 
		    if ((rv = femApplyNLoad( nlNode[i], nlType[i], nlDir[i], mult)) != AF_OK)
		    {
#ifdef RUN_VERBOSE
			    fprintf(msgout,"[W] %s: %s=%li %s=%li\n",_("Transent load not applied"),_("node"),nID[nlNode[i]],_("direction"),nlDir[i]);
#endif
		    }
      }
    }
    else 
    {
		  if ((rv = femApplyNLoad( nlNode[i], nlType[i], nlDir[i], nlVal[i])) != AF_OK)
		  {
#ifdef RUN_VERBOSE
			  fprintf(msgout,"[W] %s: %s=%li %s=%li\n",_("Load not applied"),_("node"),nID[nlNode[i]],_("direction"),nlDir[i]);
#endif
		  }
    }
	}

	return(rv);
}


int fem_add_disps(long disp_mode)
{
	int rv = AF_OK; 
	long i;

  /* boundary conditions on nodes - __must__ be added __last__: */
	for (i=0; i< nlLen; i++)
	{
#ifdef USE_MPI
		/*printf("[%i] FROM: %li LEN: %li\n",ppRank, ppNfrom, ppNnum);*/
		if ( (nlNode[i]  < ppNfrom) && (nlNode[i] >= (ppNfrom+ppNnum)) ) 
		   { continue ; }
#endif
    if (disp_mode == AF_YES) /* default behaviour */
    {
		  if ((rv = femApplyNBC( nlNode[i], nlType[i], nlDir[i], nlVal[i])) != AF_OK)
		  {
			  return(rv);
		  }
    }
    else /* replaces disp value with 0.0 */
    {
		  if ((rv = femApplyNBC( nlNode[i], nlType[i], nlDir[i], 0.0)) != AF_OK)
		  {
			  return(rv);
		  }
    }
	}
	
	return(rv);
}

/** computes number of DOFs, creates some field for DOF describing */
int fem_dofs(void) /* original code */
{
	int rv = AF_OK;
	long i,j,k;
	long sum     = 0 ;
	long act_sum = 0 ;
	long npos ;
	long dpos ;
	long etype = 0 ;

	sum = nLen*KNOWN_DOFS ;

	if ((nDOFfld = femIntAlloc(sum)) == NULL)
	   { rv = AF_ERR_MEM; goto memFree; }
	nDOFlen = sum ;

	/* computes number of active DOFs in nodes (nDOFfld) */
	act_sum = 0;
	for (i=0; i<eLen; i++)
	{
		etype = femGetETypePos(i) ;
		for (j=0; j<Elem[etype].nodes; j++)	
		{
      npos = femGetENodePos(i,j) ; 
			if (npos < 0) 
			{ 
#ifdef RUN_VERBOSE
				fprintf(msgout,"[E] %s (n=%li pos=%li): %li!\n", _("Invalid node position in element"),nID[i],j+1,npos);
#endif
				rv = AF_ERR_VAL; 
				goto memFree; 
			}

			for (k=0; k<Elem[etype].dofs; k++)
			{
				dpos = nDOFstart(npos) + Elem[etype].ndof[k] - 1 ; /* "-1" because DOF starts from "1" not "0" */
#ifdef DEVEL_VERBOSE
				fprintf(msgout,"line %li (n=%li, dof=%li)\n", dpos, npos, Elem[etype].ndof[k]);
#endif
				if (nDOFfld[dpos] == 0)
				{
					nDOFfld[dpos] = 1;
					act_sum++;
				}
			}
		}
	}

	if (act_sum < 1)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("No degrees of freedom found"));
#endif
		return(AF_ERR_SIZ);
	}

	/* size of "K" matrix: */
	nDOFAct = act_sum;

	/* creates K matrix description */
	act_sum = 1;
	for (i=0; i<nDOFlen; i++)
	{
		if (nDOFfld[i] != 0)
		{
			nDOFfld[i] = act_sum;
			act_sum++;
		}
	}
	/* as result of previous loop nDOFfld includes unused DOFS (0)
	 * and used ones (1...nDOFAct)  
	 * so K indexes are [1..nDOFAct][1..nDOFAct]
	 */
#if DEVEL_VERBOSE
	fprintf(msgout,"\nK matrix structure: \n");
	for (i=0; i<nLen; i++)
	{
		fprintf(msgout,"%li: ", nID[i]);
		for (j=0; j<KNOWN_DOFS; j++)
		{
			 fprintf(msgout,"%li ", nDOFfld[i*KNOWN_DOFS + j]);
		}
		fprintf(msgout,"\n");
	}
#endif

	/* field for row lenghts ("K" matrix)  */
	if ((K_rows = femIntAlloc(nDOFAct)) == NULL)
	   { rv = AF_ERR_MEM; goto memFree; }

	if ((rv = femKhit()) != AF_OK) {goto memFree;}
#if DEVEL_VERBOSE
	for (i=0; i<nDOFAct; i++)
	{ fprintf(msgout,"K[%li] = %li\n",i+1, K_rows[i]); }
#endif


	if (solNoLinS == 4) /* full ALM only */
	{
		if ((K_rowsAL = femIntAlloc(nDOFAct+1)) == NULL)
	   	 { rv = AF_ERR_MEM; goto memFree; }

		for (i=0; i<nDOFAct; i++) { K_rowsAL[i] = K_rows[i] + 1 ; }
		K_rowsAL[nDOFAct] = nDOFAct + 1 ;
	} /* end of full ALM code */


	if (rv == AF_OK) {return(rv);}
memFree:
	return(rv);
}

/** finite element analysis - special boundary conditions can be used (iteration!)
 * TODO: special filed (u_tot) for support status needed
 *  @return state value
 */
int femSolveBC(void)
{
	int rv = AF_OK;
	int substeps = 60; /* number of iterations */
	long i ;
#ifndef BLOCK_FOR_MPI
	long j ;
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] %s:\n",_("Solution"));
	fprintf(msgout,"[i]   %s.\n",_("one-directional supports are used (if any)"));
#endif

  femTangentMatrix = AF_NO ; /* !!! */

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */

	  for (i=0; i<substeps; i++)
	  {
#ifdef RUN_VERBOSE
	  fprintf(msgout,"[i]   %s: %4li\n",_("iteration"),i+1);
#endif
			if (i > 0)
			{
				femVecSetZeroBig(&F);
				femVecSetZeroBig(&u);
				femMatSetZeroBig(&K);

		  	femIterBCchange = 0 ;
#ifndef BLOCK_FOR_MPI
		  	for (j=0;j<resRLen; j++) { resRval[j] = 0.0; }
#endif
			}

 			if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 			if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }
 			if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }

#ifdef DEVEL_VERBOSE
		  if (i>0) {fprintf(msgout,"[ ]   %s: %li\n",_("iteration"),i);}
#endif

      if (solUseBiCGs != AF_YES)
      {
				if (solUseCGSSOR != AF_YES)
				{
		    	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
				}
				else
				{
		    	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
				}
      }
      else
      {
		    if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
      }

		  femIterEchange = 0 ;
 		  if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }
#ifndef BLOCK_FOR_MPI
		  for (j=0;j<resRLen; j++) { resRval0[j] = resRval[j]; }
#endif

#ifdef RUN_VERBOSE
			if ( i > 1)
			{
		  	fprintf(msgout,"[ ]    %s: %4li, %s : %4li\n", _("BC change"), femIterBCchange, _("ES change"), femIterEchange);
			}
#endif

			if (i > 0)
			{
		  	if ( ((femIterBCchange <= ((long)(nlLen/100)))||(femIterBCchange <= 1))
						&&
						(femIterEchange < 1))  
				{ break ; }
			}
	  }

  if (femReadPrevStep == AF_YES) { femVecAddVec(&u, 1.0, &u_tot); }
	if ((rv = femWriteRes(fem_output_file())) != AF_OK) { goto memFree; }

	if (femComputePE == AF_YES)
	{
    femCompPE(&K, &u, AF_YES ) ;
	}

memFree:
	fem_sol_free();
	femDataFree();
	femResFree();

#ifdef RUN_VERBOSE
	if (rv == AF_OK) { fprintf(msgout,"[I] %s.\n",_("Solution done")); }
	else { fprintf(msgout,"[E] %s!\n",_("Solution failed")); }
#endif
	return(rv);
}

/** finite element analysis - attempt to use 2nd order theory with
 *  an iterative solver
 *  @return state value
 */
int femSolve2ndIter(void)
{
	int rv = AF_ERR;
	int substeps = 4;    /* number of iterations */
	long i ;
#ifndef BLOCK_FOR_MPI
	long j, jj, pos, count ;
  double normF  = 0.0 ;
  double normKu = 0.0 ;
#endif

  femTangentMatrix = AF_YES ;

#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] %s:\n",_("Solution"));
#endif

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */

	for (i=0; i<substeps; i++)
	{
 		if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
 		if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }
 		if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }

#ifdef DEVEL_VERBOSE
		if (i>0) {fprintf(msgout,"[ ]   %s: %li\n",_("iteration"),i+1);}
#endif

    if (solUseBiCGs != AF_YES)
    {
			if (solUseCGSSOR != AF_YES)
			{
		  	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
			}
			else
			{
		  	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
			}
    }
    else
    {
		  if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
    }

#if 1
    femVecAddVec(&Fr,1.0,&u);
#endif
    femVecPrn(&F,"FF");
    femVecPrn(&Fr,"Fr1");
    femVecPrn(&u,"uu");

 		if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }

    femVecPrn(&Fr,"Fr2");

    if (i == 0)
    {
      normF = femVecNorm(&Fr);
    }
    else
    {
      normKu = femVecNorm(&u);
#ifdef DEVEL_VERBOSE
        fprintf(msgout,"[ ] Iter %li %e/%e=%e.\n",i,normF,normKu,normF/normKu);
#endif
      if ((normKu/normF) < 0.001)
      {
        rv = AF_OK ;
#ifdef RUN_VERBOSE
        fprintf(msgout,"[i] %s.\n",_("Solution converged")); 
#endif

        break;
      }
    }

    /** adding of disps to x,y,z: */
#ifndef BLOCK_FOR_MPI
    count = 0 ;
    for (j=0; j<nLen; j++)
    {
      pos = (j*KNOWN_DOFS) + 0 ;
      if (nDOFfld[pos] == 1) 
      {
        n_x[j] += femVecGet(&u, count+0);
      }

      pos = (j*KNOWN_DOFS) + 1 ;
      if (nDOFfld[pos] == 1) 
      {
        n_y[j] += femVecGet(&u, count+1);
      }

      pos = (j*KNOWN_DOFS) + 2 ;
      if (nDOFfld[pos] == 1) 
      {
        n_z[j] += femVecGet(&u, count+2);
      }

      for (jj=0; jj<KNOWN_DOFS; jj++)
      {
        if (nDOFfld[j*KNOWN_DOFS +jj] == 1) {count++;}
      }
    }
#endif

#if 0
      femVecClone(&F, &Fr) ;
#endif
	}

#ifndef BLOCK_FOR_MPI
#if 0
	for (j=0;j<resRLen; j++){resRval[j] = resRval0[j];}
#endif
#endif
  if (femReadPrevStep == AF_YES) { femVecAddVec(&u, 1.0, &u_tot); }
	if ((rv = femWriteRes(fem_output_file())) != AF_OK) { goto memFree; }

memFree:
	fem_sol_free();
	femDataFree();
	femResFree();

#ifdef RUN_VERBOSE
	if (rv == AF_OK) { fprintf(msgout,"[I] %s.\n",_("Solution done")); }
	else { fprintf(msgout,"[E] %s!\n",_("Solution failed")); }
#endif
	return(rv);
}

/** finite element analysis ;-) ------------------------------------
 *  @return state value
 */
int femSolve(void)
{
	int rv = AF_OK;

	/* special b.c.: */
	if (femRunSolIterBC == AF_YES) { return(femSolveBC()); }
  /* iterative 2nd order: */
	if (fem2ndOrderIter == AF_YES) { return(femSolve2ndIter()); }

	/* "normal" solution: */
#ifdef RUN_VERBOSE
	fprintf(msgout,"[I] %s:\n",_("Solution"));
#endif

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("data checking and allocations"));
#endif
 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done before adding of loads! */
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("data checking and allocations done"));
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("assembling of stiffness matrix"));
#endif
 	if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("assembling of stiffness matrix done"));
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("loads and supports"));
#endif
 	if ((rv = fem_add_loads(0)) != AF_OK) { goto memFree; }
 	if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("loads and supports done"));
#endif

  
#ifndef _SMALL_FEM_CODE_
#ifdef _MATRIX_SAVING_
  if (femSaveMatFStat == AF_YES) { femMatPrnF(femSaveMatFName, &K); }
  if (femSaveSparMatFStat == AF_YES) { femSparseMatPrnF(femSaveSparMatFName, &K); }
  if (femSaveSparMarketMatFStat == AF_YES) { femSparseMarketMatPrnF(femSaveSparMarketMatFName, &K); }
  if (femSaveVecFStat == AF_YES) { femVecPrnF(femSaveVecFName, &F); }
#endif
#endif

  if (femBreakSolu == AF_YES) {goto memFree;}

#if 1 /* 0 == kitnarf comparison testing */
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("solution of linear equations"));
#endif
  if (solUseBiCGs != AF_YES)
  {
		if (solUseCGSSOR != AF_YES)
		{
	  	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
		}
		else
		{
	  	if ((rv = femEqsCGwSSOR(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
		}
  }
  else
  {
	  if ((rv = femEqsBiCCSwJ(&K, &F, &u, FEM_ZERO/10000.0, nDOFAct)) != AF_OK) { goto memFree; }
  }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("solution of linear equations done"));
#endif

#else
  femVecSetZero(&u);
  femVecPut(&u,1,0.1);
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("element results"));
#endif
 	if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("element results done"));
#endif

  if (femReadPrevStep == AF_YES) { femVecAddVec(&u, 1.0, &u_tot); }
	if ((rv = femWriteRes(fem_output_file())) != AF_OK) { goto memFree; }

  if (femHaveThermDOFs == AF_YES) /* thermal DOFS (if any) */
  {
    if (fem_throfile != NULL)
    {
      rv = femWriteThermDOFS(fem_throfile, &u); 
    }
  }
    
#ifndef _SMALL_FEM_CODE_
	if (fem_spec_out_file != NULL)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[i]  %s:\n", _("Saving results in custom format"));
#endif
		switch (fem_spec_out_type)
		{
  		case 1: 
				if ( femWriteNodeResVTK(fem_spec_out_file)  == AF_OK)
					{ fprintf(msgout,"[i]    %s.\n", _("succesfully saved as VTK (legacy)")); }
				else
					{ fprintf(msgout,"[i]    %s!\n", _("writing of data failed")); }
				break ;
			case 0:
			default:
				if ( femWriteNodeResTxt(fem_spec_out_file) == AF_OK)
					{ fprintf(msgout,"[i]    %s.\n", _("succesfully saved as ANSI text")); }
				else
					{ fprintf(msgout,"[i]    %s!\n", _("writing of data failed")); }
				break ;

		}
	}
#endif

	if (femComputePE == AF_YES)
	{
    femCompPE(&K, &u, AF_YES ) ;
	}

memFree:
	fem_sol_free();
	femDataFree();
	femResFree();

#ifdef RUN_VERBOSE
	if (rv == AF_OK) { fprintf(msgout,"[I] %s.\n",_("Solution done")); }
	else { fprintf(msgout,"[E] %s!\n",_("Solution failed")); }
#endif
	return(rv);
}


/** computes total price of sturcture based on element volumes
 *  @return state value
 */
int femComputePrice(void)
{
	int rv = AF_OK;

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }

  fprintf( stdout, "%f\n", fem_get_struct_price(AF_NO) ) ;

memFree:
	fem_sol_free();
	femDataFree();

	return(rv);
}

/* end of fem_sol.c */
