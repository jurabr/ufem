/*
   File name: fem_nlmc.c
   Date:      2006/11/06 19:02
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

   Simple nonlocal-like model for masonry
*/

#include "fem_elem.h"
#include "fem_mat.h"

#ifndef _SMALL_FEM_CODE_
#ifdef USE_NLMC

#if 0
#ifdef _USE_THREADS_
#define NLMC_THREADS 1
#endif
#endif

#ifdef NLMC_THREADS
#include <pthread.h>
#ifndef _USE_THREADS_
#define AF_MAX_THREADS 4
long femThreadNum = AF_MAX_THREADS ;
#endif
#endif

extern int femGetPrincStress2D(tVector *sigma_x, tVector *sigma_1, double *phi);

long    nlmc_ip_len = 0 ;
double *nlmc_ip_x   = NULL ;
double *nlmc_ip_y   = NULL ;
double *nlmc_ip_z   = NULL ;
double *nlmc_ip_ang = NULL ; /* result - s1 angle */
double *nlmc_ip_sum = NULL ; /* result - sum of s1 stress in point */
long   *nlmc_ip_e   = NULL ; /* */
long   *nlmc_ip_i   = NULL ;
long   *nlmc_ip_m   = NULL ; /* material number */
long   *nlmc_ip_ch  = NULL ; /* checked data sign */
long   *nlmc_e_from = NULL ; /* fast nlmc_ip search [eLen] */

long    nlmc_elist_len = 0 ;     /* number of items in nlmc_elist_list */
long   *nlmc_elist_from = NULL ; /* "from" positions in nlmc_elist_list [nlmc_ip_len] */
long   *nlmc_elist_num  = NULL ; /* "from" positions in nlmc_elist_list [nlmc_ip_len] */
long   *nlmc_elist_tnum = NULL ; /* "from" positions in nlmc_elist_list [nlmc_ip_len] */
long   *nlmc_elist_list = NULL ; /* list of elements athat are aroound element "i" [nlmc_elist_len] */
double *nlmc_elist_dist = NULL ; /* list of elements athat are aroound element "i" [nlmc_elist_len] */

long   *nlmc_egrp     = NULL ; /* element group (cluster)     */
long    nlmc_egrp_max = 0 ;    /* max. found egrp number      */
long    nlmc_neib_len = 0 ;    /* number of friendly elements */


void nlmc_ip_null(void)
{
  nlmc_ip_len = 0 ;
  nlmc_ip_x   = NULL ;
  nlmc_ip_y   = NULL ;
  nlmc_ip_z   = NULL ;
  nlmc_ip_ang = NULL ;
  nlmc_ip_sum = NULL ;
  nlmc_ip_e   = NULL ;
  nlmc_ip_i   = NULL ;
  nlmc_ip_m   = NULL ;
  nlmc_ip_ch  = NULL ;

  nlmc_e_from = NULL ;

  nlmc_egrp = NULL ;

  nlmc_elist_len  = 0 ;
  nlmc_elist_from = NULL ;
  nlmc_elist_num = NULL ;
  nlmc_elist_tnum = NULL ;

  nlmc_elist_list = NULL ;
  nlmc_elist_dist = NULL ;
}

void nlmc_ip_free(void)
{
  free(nlmc_ip_x );
  free(nlmc_ip_y );
  free(nlmc_ip_z );
  free(nlmc_ip_ang );
  free(nlmc_ip_sum );
  free(nlmc_ip_e );
  free(nlmc_ip_i );
  free(nlmc_ip_m );
  free(nlmc_ip_ch );

  free(nlmc_e_from);

  free(nlmc_egrp);

  free(nlmc_elist_from);
  free(nlmc_elist_num);
  free(nlmc_elist_tnum);

  free(nlmc_elist_list);
  free(nlmc_elist_dist);

  nlmc_ip_null();
}

int nlmc_ip_alloc(long len)
{
  int rv = AF_OK ;

	if (len < 1) {return(AF_ERR_VAL);}

	nlmc_ip_null();

  if ((nlmc_ip_x = femDblAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_ip_y = femDblAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_ip_z = femDblAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_ip_ang = femDblAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_ip_sum = femDblAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_ip_e = femIntAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_ip_i = femIntAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_ip_m = femIntAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_ip_ch = femIntAlloc(len)) == NULL) {goto memFree;}

  if ((nlmc_elist_from = femIntAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_elist_num = femIntAlloc(len)) == NULL) {goto memFree;}
  if ((nlmc_elist_tnum = femIntAlloc(len)) == NULL) {goto memFree;}
	/* _elist_list have to be allocated later ! */

  if ((nlmc_e_from = femIntAlloc(eLen)) == NULL) {goto memFree;}

  if ((nlmc_egrp = femIntAlloc(eLen)) == NULL) {goto memFree;}


	nlmc_ip_len = len ;
  
  return(rv);
memFree:
  nlmc_ip_free();
  return(AF_ERR_MEM);
}

void nlmc_ip_print(void)
{
#ifdef DEVEL_VERBOSE
  long i ;

  for (i=0; i<nlmc_ip_len; i++)
  {
    fprintf(msgout,
        "%li [%e %e %e] e=%li i=%li m=%li ch=%li\n",
  i,
  nlmc_ip_x   [i],
  nlmc_ip_y   [i],
  nlmc_ip_z   [i],
  nlmc_ip_e   [i],
  nlmc_ip_i   [i],
  nlmc_ip_m   [i],
  nlmc_ip_ch  [i]
        
        );
  }
 
#endif
}

/** Sets all items of nlmc_ip_ch[] to zero */
void nlmc_ip_ch_zero(void)
{
	long i ;

	for (i=0; i<nlmc_ip_len; i++) { nlmc_ip_ch[i] = 0 ; }
}

/** Fills _ip_ fields fith x,y,z data */
int nlmc_ip_fill(void)
{
	int rv = AF_OK ;
	long len, id, eT, mT ;
	long pos ;
  long i_len ;
	long i, j;

	len = 0;

	for (i=0; i<eLen; i++)
	{
		len += femElemIPoints(i) ;
	}

	if (len <= 0) { return(AF_ERR) ; }

	if ((rv = nlmc_ip_alloc(len)) != AF_OK) { return(rv); }

	pos = 0 ;

	for (i=0; i<eLen; i++)
	{
		eT = femGetETypePos(i); 
		mT = femGetSystemMatType(i);
		id = i ; /* eID[i] ; */

    nlmc_e_from[i] = pos ;

    i_len = femElemIPoints(i);

    for (j=1; j<=i_len; j++)
    {
			Elem[eT].res_p_loc(
					i, j,
					&nlmc_ip_x[pos],
					&nlmc_ip_y[pos],
					&nlmc_ip_z[pos]
					) ;

			nlmc_ip_i[pos] = j-1  ;
			nlmc_ip_e[pos] = id ;
			nlmc_ip_m[pos] = mT ;
			pos++ ;
		}
	}

nlmc_ip_print();

	return(rv);
}

/** Checks if elements with indexes "ePos" and "i" have common side
 * @param ePos original element
 * @param i second element
 * @param min number of nodes on the common side
 * @return 0 if no, 1 if yes
 */
long nlmcCheckSide(long ePos, long i, long min)
{
  long found = 0 ;
  long j, k ;
  long n, num ;

  num = 0 ;

  for (j=0; j<Elem[eType[ePos]].nodes; j++)
  {
    n = eNodesL[eFrom[ePos]+j] ;
    for (k=0; k<Elem[eType[i]].nodes; k++)
    {
      if (n == eNodesL[eFrom[i]+k] ) {num ++;}
    }
  }

  if (num >= min) {found = 1;}
  
  return(found) ;
}

double nlmc_get_distance_1_2(long ip1, long ip2)
{
  double dx, dy ;

  dx =  nlmc_ip_x[ip2]-nlmc_ip_x[ip1] ;
  dy =  nlmc_ip_y[ip2]-nlmc_ip_y[ip1] ;

	return ( sqrt((dx*dx) + (dy*dy)) ) ;
}

/* alloc (!) and fill the nlmc_elist_list[] field...
 * @param maxlen radius of ipoint area
 * @return status
 */
int nlmc_elist_fill(void)
{
	long   i, j ;
	long   sum    = 0 ;
	long   subsum = 0 ;
	double len    = 0.0 ;
  double maxlen ;

	/* compute number of items in each line */
	sum = 0 ;
	for (i=0; i<nlmc_ip_len; i++)
  {

		if (i > 0)
		{
			nlmc_elist_from[i] = sum ;
		}
    if (nlmc_ip_m[i] != 9) {continue;}

    maxlen = femGetMPValPos(nlmc_ip_e[i], MAT_RAD, 0);

		for (j=0; j<nlmc_ip_len; j++)
		{
			if ((len=nlmc_get_distance_1_2(i,j)) <= maxlen)
			{
				nlmc_elist_tnum[i]++;
				if (nlmc_ip_m[i] == nlmc_ip_m[j])
				{
          if (nlmc_egrp[nlmc_ip_e[i]] == nlmc_egrp[nlmc_ip_e[j]])
          {
						nlmc_elist_num[i]++;
						sum++;
          }
				}
			}
		}
	}

	if (sum == 0)
	{
		nlmc_elist_len = 0 ;
		return(AF_OK);
	}

	/* allocate nlmc_elist_list */
  if ((nlmc_elist_list = femIntAlloc(sum)) == NULL) 
	{
		fprintf(msgout,"[E] %s!\n", _("Can NOT allocate NLMC field (material type 9 data)"));
		return(AF_ERR_MEM);
	}

  if ((nlmc_elist_dist = femDblAlloc(sum)) == NULL) 
	{
		free (nlmc_elist_list); nlmc_elist_list = NULL ;
		fprintf(msgout,"[E] %s!\n", _("Can NOT allocate NLMC field (material type 9 data)"));
		return(AF_ERR_MEM);
	}


  nlmc_elist_len = sum ;
  sum = 0 ;

  /* fill nlmc_elist_list */
	for (i=0; i<nlmc_ip_len; i++)
	{
    if (nlmc_ip_m[i] != 9) {continue;}

    maxlen = femGetMPValPos(nlmc_ip_e[i], MAT_RAD, 0);

		for (j=0; j<nlmc_ip_len; j++)
		{
      subsum = 0 ;
			if ((len=nlmc_get_distance_1_2(i,j)) <= maxlen)
			{
				if (nlmc_ip_m[i] == nlmc_ip_m[j])
				{
          if (nlmc_egrp[nlmc_ip_e[i]] == nlmc_egrp[nlmc_ip_e[j]])
          {
            nlmc_elist_list[sum] = j ;
            nlmc_elist_dist[sum] = len ;
            subsum++ ;
						sum++;
            if (subsum == nlmc_elist_num[i]) {continue;}
          }
				}
			}
		}
/*printf("I=%4li NUM=%4li\n",i,nlmc_elist_num[i]);*/
	}

	return(AF_OK);
}

/* Creates group of elements */
int nlmc_egrp_fill(long mtype)
{
  long i, j, k ;
  long ivar ;

  nlmc_egrp_max = 0 ;

  for (i=0; i<eLen; i++)
  {
    if (nlmc_egrp[i] == 0)
    {
      if ( femGetSystemMatType(i) !=  mtype)
      {
        /* unrelated material - nothing to do */
        nlmc_egrp[i] = -1 ;
        continue ;
      }
      else
      {
        nlmc_egrp_max++ ;
        nlmc_egrp[i] = nlmc_egrp_max ;

        nlmc_neib_len = 0 ;

        for (j=0; j<eLen; j++)
        {
          if (i == j) {continue;}

          if ( nlmcCheckSide(i, j, 2) == 1)
          {
            if (femGetSystemMatType(j) != mtype) {continue;}

            nlmc_neib_len++ ;
            if (nlmc_egrp[j] == 0)
            {
              /* found unused friendly element */
              nlmc_egrp[j] = nlmc_egrp[i] ;
            }
            else
            {
              /* friendly but used element: renumber its group */
              ivar = nlmc_egrp[j] ;
              
              for (k=0; k<eLen; k++)
              {
                if (nlmc_egrp[k] == ivar)
                {
                  nlmc_egrp[k] = nlmc_egrp[i] ;
                }
              }
            }
          }
          if (nlmc_neib_len == 4) { break ; } /* all 4 found */
        }
      }
    }
  }

  return(AF_OK);
}

/** Computes total s1 and s2 stress on a given area
 * @param nlmc_ip_pos position of integration point in a center
 * @param radius redius of operation
 * @param s_x vector of total xy stresses (to be computed)
 * @param s_1 vector of total principal stresses (to be computed)
 * @param number number of affected elements
 * @param total number of tested elements
 * @return status
 */
int nlmc_elistTotalStress2D(
    long    nlmc_ip_pos,
		double  radius, 
		tVector *s_x, 
		tVector *s_1, 
		long   *number,
		long   *total,
    double *angle
    )
{
	int  rv = AF_OK ;
	long i ;

	*number = nlmc_elist_num[nlmc_ip_pos] ;
	*total  = nlmc_elist_tnum[nlmc_ip_pos] ;

	femVecPut(s_x, 1, 0.0);
	femVecPut(s_x, 2, 0.0);
	femVecPut(s_x,3, 0.0);

  for (i=nlmc_elist_from[nlmc_ip_pos]; i<(nlmc_elist_from[nlmc_ip_pos]+nlmc_elist_num[nlmc_ip_pos]); i++)
	{
		femVecAdd(s_x, 1, femGetEResVal(nlmc_ip_e[nlmc_elist_list[i]], RES_SX, nlmc_ip_i[nlmc_elist_list[i]]+1)) ;
		femVecAdd(s_x, 2, femGetEResVal(nlmc_ip_e[nlmc_elist_list[i]], RES_SY, nlmc_ip_i[nlmc_elist_list[i]]+1)) ;
		femVecAdd(s_x, 3, femGetEResVal(nlmc_ip_e[nlmc_elist_list[i]], RES_SXY,nlmc_ip_i[nlmc_elist_list[i]]+1)) ;
	}

  rv = femGetPrincStress2D(s_x, s_1, angle);

  return(rv);
}

/** Searches all material points to find points and finds elements to destroy
 * @param et element type (not an id number!) to test 
 * @param mat material type (not an id number!) to test 
 * @param tot total stress level on area
 * @param radius radius to check
 * @param rsnum result type to be changed
 * @param rsval new value of result
 * @return status
 */ 
int nlmcComputeStatus2DPart(
		long et, 
		long mat, 
		long i_from, 
		long i_to,
		double tot_plus, 
		double tot_minus, 
		double radius, 
		long rsnum, 
		double rsval_plus,
		double rsval_minus)
{
	int  rv = AF_OK ;
	long i, number, total ;
	tVector s_x, s_1 ;
  double  angle ;
	double val = 0.0 ;

	femVecNull(&s_x);
	femVecNull(&s_1);

	femVecFullInit(&s_x, 3);
	femVecFullInit(&s_1, 3);

	for (i=0; i<nlmc_ip_len; i++)
	{
		if (nlmc_ip_m[i] == mat)
		{
			nlmc_elistTotalStress2D(i, radius, &s_x, &s_1, &number, &total, &angle);

			if ((number > 0) && (total > 0))
			{
				val = femVecGet(&s_1,1)/((FEM_PI*radius*radius)*((double)number/(double)total)) ;

        nlmc_ip_ang[i] = angle ;
				if (val >= tot_plus)
				{
					nlmc_ip_ch[i] = 1 ;
				}

				val = femVecGet(&s_1,2)/((FEM_PI*radius*radius)*((double)number/(double)total)) ;
				
				if (val <= tot_minus)
				{
					nlmc_ip_ch[i] = -1 ;
				}

        nlmc_ip_sum[i] = val ;
			}
		}
	}

	femVecFree(&s_x);
	femVecFree(&s_1);

	return(rv);
}

#ifndef NLMC_THREADS
/** Searches all material points to find points and finds elements to destroy
 * @param et element type (not an id number!) to test 
 * @param mat material type (not an id number!) to test 
 * @param tot total stress level on area
 * @param radius radius to check
 * @param rsnum result type to be changed
 * @param rsval new value of result
 * @return status
 */ 
int nlmcComputeStatus2D(
		long et, 
		long mat, 
		double tot_plus, 
		double tot_minus, 
		double radius, 
		long rsnum, 
		double rsval_plus,
		double rsval_minus)
{
	return( nlmcComputeStatus2DPart(
		et, mat, 0, nlmc_ip_len,
		tot_plus, tot_minus, radius, 
		rsnum, rsval_plus, rsval_minus));
}
#else
/* there are some threads: */
void *thNlmcComputeStatus2D(void *params)
{
	long i_from ;
	long i_to;
	long et;
	long mat;
	double tot_plus;
	double tot_minus;
	double radius;
	long rsnum;
	double rsval_plus;
	double rsval_minus;
	long double *param;

	param = (long double *)params ;

	et           = (long) param[0] ;
	mat          = (long) param[1] ;
	i_from       = (long) param[2] ;
	i_to         = (long) param[3] ;
	tot_plus     = (double)param[4] ;
	tot_minus    = (double)param[5] ;
	radius       = (double)param[6] ;
	rsnum        = (long)	 param[7] ;
	rsval_plus   = (double)param[8] ;
	rsval_minus  = (double)param[9] ;

#if 0
	printf("PARAMS 1: %li %li %li %li %e %e %e %li %e %e\n",
			et, mat, i_from, i_to, tot_plus, tot_minus, radius, rsnum, rsval_plus, rsval_minus);
#endif

	nlmcComputeStatus2DPart(
		et, mat, i_from, i_to,
		tot_plus, tot_minus, radius, 
		rsnum, rsval_plus, rsval_minus);

	return(NULL);
}

int nlmcComputeStatus2D(
		long et, 
		long mat, 
		double tot_plus, 
		double tot_minus, 
		double radius, 
		long rsnum, 
		double rsval_plus,
		double rsval_minus)
{
	long double param[10];
	long i_from, i_to ;
  pthread_t Thread_ID[AF_MAX_THREADS];
  ldiv_t  dnum ;
	long i ;

	param[0] = (long double) et           ;
	param[1] = (long double) mat          ;
	param[2] = (long double) i_from       ;
	param[3] = (long double) i_to         ;
	param[4] = (long double) tot_plus     ;
	param[5] = (long double) tot_minus    ;
	param[6] = (long double) radius       ;
	param[7] = (long double) rsnum        ;
	param[8] = (long double) rsval_plus   ;
	param[8] = (long double) rsval_minus  ;

#if 0
	printf("PARAMS 0: %li %li %li %li %e %e %e %li %e %e\n",
			et, mat, i_from, i_to, tot_plus, tot_minus, radius, rsnum, rsval_plus, rsval_minus);
#endif
	
	/* THREADS HERE: */
  dnum = ldiv(nlmc_ip_len, femThreadNum) ;
  if (dnum.quot < 1)
  {
    dnum.quot = 0 ;
    dnum.rem  = nlmc_ip_len ;
  }

  for (i=0; i<femThreadNum; i++)
  {
		param[2] = (long double) (i*dnum.quot) ;
		if (i < (femThreadNum-1))
		{
			param[3] = (long double) ((i+1)*dnum.quot) ;
		}
		else
		{
			param[3] = (long double) ((i+1)*dnum.quot + dnum.rem) ;
		}

    pthread_create(&Thread_ID[i],NULL,thNlmcComputeStatus2D,&param);
  }

  for (i=0; i<femThreadNum; i++)
  {
    pthread_join(Thread_ID[i], NULL) ;
  }

	return(AF_OK);
}
#endif

int nlmcUpdateStatus_e2m9(void)
{
  long mat = 9 ; 
  long rv  = RES_ECR1 ;  /* reduced stiffness  */
  long rv2 = RES_STAT1 ; /* material status    */
  long rva = RES_DIR1 ;  /* total stress angle */
  long rvs = RES_ECR2 ;  /* total stress size  */
  long rvnum = RES_STAT2 ; /* number of items */
  long rvtnum = RES_GCR1 ; /* number of items */
  long modvalp= MAT_E1 ;
  long modvalm= MAT_E1 ;
  double val ;
  long i ;

  for (i=0; i<nlmc_ip_len; i++)
  {
    if (nlmc_ip_m[i] == mat)
    {
			femPutEResVal(nlmc_ip_e[i], rv2, nlmc_ip_i[i]+1, nlmc_ip_ch[i]);
			femPutEResVal(nlmc_ip_e[i], rvs, nlmc_ip_i[i]+1, nlmc_ip_sum[i]);

#if 1
			femPutEResVal(nlmc_ip_e[i], rvnum, nlmc_ip_i[i]+1, nlmc_elist_num[i]);
#else
			femPutEResVal(nlmc_ip_e[i], rvnum, 1, nlmc_egrp[nlmc_ip_e[i]]);
#endif
			femPutEResVal(nlmc_ip_e[i], rvtnum, nlmc_ip_i[i]+1, nlmc_elist_tnum[i]);
#if 0
printf("SUM: %6.2f\n", nlmc_ip_sum[i]);
#endif
        if (nlmc_ip_ch[i] == 1)
        {
/* printf("(+)");*/
          val = fabs ( femGetMPValPos(nlmc_ip_e[i], modvalp, 0) );
					femPutEResVal(nlmc_ip_e[i], rv, nlmc_ip_i[i]+1, val);
					femPutEResVal(nlmc_ip_e[i], rva, nlmc_ip_i[i]+1, nlmc_ip_ang[i]);
        }
        if (nlmc_ip_ch[i] == -1)
        {
/*printf("(-)");*/
          val = fabs ( femGetMPValPos(nlmc_ip_e[i], modvalm, 0) );
					femPutEResVal(nlmc_ip_e[i], rv, nlmc_ip_i[i]+1, val);
					femPutEResVal(nlmc_ip_e[i], rva, nlmc_ip_i[i]+1, nlmc_ip_ang[i]);
        }
    }
  }

  return(AF_OK);
}


int nlmInit2D_m902(void)
{
  int rv = AF_OK ;
  
	nlmc_ip_null();
	if ((rv=nlmc_ip_fill()) != AF_OK) {goto memFree;}
	if ((rv=nlmc_egrp_fill(9)) != AF_OK) {goto memFree;}
  if ((rv=nlmc_elist_fill()) != AF_OK) {goto memFree;}
  return(rv);

memFree:
	nlmc_ip_free();
  return(rv);
}

int nlmExit2D_m902(void)
{
	nlmc_ip_free();
  return(AF_OK);
}

/** Runs nlmcComputeStatus2D() on the data
 * Limitations: 
 * a) only one brick material can be used
 * b) it will be very slow
 * b) works only for material 009 and element 002
 *
 */
int nlmSetStatus2D_m902(long Mode)
{
	int    rv = AF_OK;
	double rsval_minus, rsval_plus, tot_minus, tot_plus, radius;
	long   et, mat ;
	long   rsnum ;
	long   i ;
	long   sum = 0 ;
  long   pos = 0 ;
	
  if (nlmc_ip_len <= 0) { return(AF_OK); } /* nothing to do */

	if (Mode == AF_YES)
	{
		et     = 2 ;
		mat    = 9 ;

  	rsnum  = RES_ECR1 ; 

		/* check if there is a brittle (brick) material: 009 */
		sum = 0 ;
		for (i=0; i< mpLen; i++)
		{
			if (mat == femGetMPTypePos(i)) { pos = i ; sum++; break ; }
		}
		if (sum <= 0)
		{
			/* nothing to do */
			return(AF_OK);
		}

		/* get material properties */
		tot_minus = femGetRepVal (
			pos, MAT_SCMPR, 0, 
			mpValL, mpLenL, mpFrom, mpLen, 
			Mat[mat].val, Mat[mat].num, Mat[mat].val_rp, Mat[mat].num_rp);

		tot_plus  =  femGetRepVal (
			pos, MAT_STEN, 0, 
			mpValL, mpLenL, mpFrom, mpLen, 
			Mat[mat].val, Mat[mat].num, Mat[mat].val_rp, Mat[mat].num_rp);
			
		if (tot_minus > 0) {tot_minus *= (-1.0);}

		rsval_minus = femGetRepVal (
			pos, MAT_E1, 0, 
			mpValL, mpLenL, mpFrom, mpLen, 
			Mat[mat].val, Mat[mat].num, Mat[mat].val_rp, Mat[mat].num_rp);

		rsval_plus  =  femGetRepVal (
			pos, MAT_E1, 0, 
			mpValL, mpLenL, mpFrom, mpLen, 
			Mat[mat].val, Mat[mat].num, Mat[mat].val_rp, Mat[mat].num_rp);

    radius = femGetRepVal (
      i, MAT_RAD, 0,
      mpValL, mpLenL, 
      mpFrom, mpLen, 
      Mat[mat].val, 
      Mat[mat].num, 
      Mat[mat].val_rp, 
      Mat[mat].num_rp); 
 
 
      if (radius <= 0.0)
      {
		    radius = 0.06 ; /* was: 0.2 [m] !!! */
#ifdef RUN_VERBOSE
        fprintf(msgout,"[w] %s!\n", _("Non-local radius not defined"));
#endif
      }

#ifdef DEVEL_VERBOSE
		fprintf(msgout,"rvals: %e < %e (%e %e)\n", tot_minus, tot_plus,rsval_plus, rsval_minus);
#endif

		/* run nlmcComputeStatus2D() */
		rv =  nlmcComputeStatus2D( et, mat, 
		tot_plus, tot_minus, radius, 
		rsnum, rsval_plus, rsval_minus);
	}

	return(rv);
}

#endif
#endif /* SMALL_FEM_CODE */
/* end of fem_nlmc.c */
