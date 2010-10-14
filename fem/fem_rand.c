/*
   File name: fem_rand.c
   Date:      2004/01/31 19:45
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

   FEM Solver - Monte Carlo Simulation

   $Id: fem_rand.c,v 1.6 2004/02/26 17:43:24 jirka Exp $
*/

#include "fem_rand.h"

extern long  nDOFAct; /* from fem_sol.c */
extern tVector u;     /* from fem_sol.c */
extern long resRLen;

#define RAND_BIG  1000000000
#define RAND_SEED  161803398
#define RAND_ZERO  0
#define RAND_FAC   (1.0/RAND_BIG)


tRanDF  *RandDF    = NULL ;
long     RandDFlen = 0 ;

long   RandValLen = 0 ;
long  *randVal    = NULL ; /* type of random value (tRanDF) */
long  *randData   = NULL ; /* type of data (0..real, 1..mat, 2..node, 3..nload, 4..eload) */
long  *randSet    = NULL ; /* position (or id) of data set (node id..) */
long  *randPos    = NULL ; /* data type (x,y,..) */
long  *randRpos   = NULL ; /* repeating set number (if neeeded) */

tVector  u_rand1 ;              /* displacements   */
double  *resRval_rand1 = NULL ; /* reactions       */
double  *resEVal_rand1 = NULL ; /* element results */
tVector  u_rand2 ;              /* displacements   */
double  *resRval_rand2 = NULL ; /* reactions       */
double  *resEVal_rand2 = NULL ; /* element results */

tVector  u_randmax ;              /* displacements   */
double  *resRval_randmax = NULL ; /* reactions       */
double  *resEVal_randmax = NULL ; /* element results */

tVector  u_randmin ;              /* displacements   */
double  *resRval_randmin = NULL ; /* reactions       */
double  *resEVal_randmin = NULL ; /* element results */

double  *randMult      = NULL ; /* random data multipliers field */

long fem_rand_count    = 0 ;
long fem_rand_fail_num = 0 ;

long fem_rand_init     = 152 ;


/* backup of original data */
double   *n_x_rand    = NULL ; /* node x coordinate */
double   *n_y_rand    = NULL ; /* node y coordinate */
double   *n_z_rand    = NULL ; /* node z coordinate */
double   *rsValL_rand = NULL ; /* real set values   */
double   *mpValL_rand = NULL ; /* real set values   */
double   *nlVal_rand  = NULL ; /* value of load     */
double   *elValL_rand = NULL ; /* values of load    */


/** Random number generator (see Knuth: Art of Computer Programming 3) 
 */
double fem_rand(long *inpl)
{
  static int ind   ;
  static int inda  ;
  static long fld[56] ;
  static int stf = 0 ;
  long   valj, valk ;
  long   i, j, k ;

  /* initialization */
  if ( (*inpl < 0) || (stf == 0) )
  {
    stf = 1 ;
    valj  = labs(RAND_SEED - labs(*inpl)) ;
    valj %= RAND_BIG ;
    fld[55] = valj ;
    valk = 1 ;

    for (i=1; i<=54; i++)
    {
      j = (21*i) % 55 ;
      fld[j] = valk ;
      valk = valj - valk ;

      if (valk < RAND_ZERO) { valk += RAND_BIG ; }

      valj = fld[j] ;
    } /* for i */

    for (k=1; k<=4; k++)
    {
      for (i=1; i<=55; i++)
      {
        fld[i] -= fld[1+(i+30) % 55] ;
        if (fld[i] < RAND_ZERO) { fld[i] += RAND_BIG ; }
      }
    }
    *inpl  = 1 ;
    ind  = 0 ;
    inda = 31 ;
  }

  /* normal run */
  if (++ind >= 56)
  {
    ind = 1 ;
  }

  if (++inda >= 56)
  {
    inda = 1 ;
  }

  valj = fld[ind] - fld[inda] ;

  if (valj < RAND_ZERO)
  {
    valj += RAND_BIG ;
  }

  fld[ind] = valj ;

  return(valj*RAND_FAC);
}

/** Free RandDF data (if any) */
void femFreeRandDF(void)
{
  long i;

  if (RandDF != NULL)
  {
    if (RandDFlen > 0)
    {
      for (i=0; i<RandDFlen; i++)
      {
        RandDF[i].len = 0 ;
        free(RandDF[i].prob); RandDF[i].prob = NULL ;
        free(RandDF[i].size); RandDF[i].size = NULL ;
      }
      free(RandDF);
    }

    RandDF = NULL ;
  }

  RandDFlen = 0 ;
}


/** Reads random data from a file */
int fem_read_rand_data(char *fname)
{
  FILE *fr = NULL ;
  long len = 0 ;
  long i, j;

  if ((fr = fopen(fname,"r")) == NULL) {return(AF_ERR_IO);}

  /* random data tables ---------------- */
  fscanf(fr, "%li", &len) ;

  if (len <= 0) 
  {
    fclose(fr);
    return(AF_ERR_VAL);
  }

  /* data allocations */
  if ((RandDF=(tRanDF *)malloc(len*sizeof(tRanDF))) == NULL)
  {
    fclose(fr);
    return(AF_ERR_MEM);
  }

  RandDFlen = len ;

  for (i=0; i<len; i++)
  {
    RandDF[i].len  = 0 ;
    RandDF[i].prob = NULL ;
    RandDF[i].size = NULL ;
  }

  for (i=0; i<RandDFlen; i++)
  {
    fscanf(fr,"%li", &RandDF[i].len);

    if (RandDF[i].len <= 0)
    {
      femFreeRandDF();
      fclose(fr);
      return(AF_ERR_VAL) ;
    }

    if ((RandDF[i].prob = femDblAlloc(RandDF[i].len)) == NULL)
    {
      femFreeRandDF();
      fclose(fr);
      return(AF_ERR_MEM) ;
    }
    
    if ((RandDF[i].size = femDblAlloc(RandDF[i].len)) == NULL)
    {
      femFreeRandDF();
      fclose(fr);
      return(AF_ERR_MEM) ;
    }

    for (j=0; j<RandDF[i].len; j++)
    {
      fscanf(fr,"%lf %lf", &RandDF[i].prob[j], &RandDF[i].size[j]);
    }
  }


  /* random data positions ------------- */

  fscanf(fr,"%li", &len);

  if (len <= 0) 
  {
    femFreeRandDF();
    fclose(fr);
    return(AF_ERR_VAL);
  }

  RandValLen = len ;

  if ((randVal = femIntAlloc(len)) == NULL)  {goto memFree;}
  if ((randData = femIntAlloc(len)) == NULL) {goto memFree;}
  if ((randSet = femIntAlloc(len)) == NULL)  {goto memFree;}
  if ((randPos = femIntAlloc(len)) == NULL)  {goto memFree;}
  if ((randRpos = femIntAlloc(len)) == NULL) {goto memFree;}

  for (i=0; i<len; i++)
  {
    fscanf(fr, "%li %li %li %li %li",
        &randVal[i],
        &randData[i],
        &randSet[i],
        &randPos[i],
        &randRpos[i]
        );
  }
  
  return(fclose(fr));

memFree:
  femFreeRandDF();

  RandValLen = 0 ;

  femIntFree(randVal);
  femIntFree(randData);
  femIntFree(randSet);
  femIntFree(randPos);
  femIntFree(randRpos);

  fclose(fr);
  return(AF_ERR_MEM);
}

void fem_rand_backup_null(void)
{
  n_x_rand    = NULL ;
  n_y_rand    = NULL ;
  n_z_rand    = NULL ;
  rsValL_rand = NULL ;
  mpValL_rand = NULL ;
  nlVal_rand  = NULL ;
  elValL_rand = NULL ;

  randMult    = NULL ;
}

void fem_rand_backup_free(void)
{
  femDblFree(n_x_rand);
  femDblFree(n_y_rand);
  femDblFree(n_z_rand);
  femDblFree(rsValL_rand);
  femDblFree(mpValL_rand);
  femDblFree(nlVal_rand);
  femDblFree(elValL_rand);

  femDblFree(randMult);
}

int fem_rand_backup_alloc(void)
{
  int  rv = AF_OK ;

  fem_rand_backup_null();

  if ((n_x_rand=femDblAlloc(nLen)) == NULL) {rv = AF_ERR_MEM; goto memFree;}
  if ((n_y_rand=femDblAlloc(nLen)) == NULL) {rv = AF_ERR_MEM; goto memFree;}
  if ((n_z_rand=femDblAlloc(nLen)) == NULL) {rv = AF_ERR_MEM; goto memFree;}
  if ((rsValL_rand=femDblAlloc(rsLenL)) == NULL) 
	{
		if (rsLenL > 0)
		{
			rv = AF_ERR_MEM; 
			goto memFree;
		}
	}
  if ((mpValL_rand=femDblAlloc(mpLenL)) == NULL) {rv = AF_ERR_MEM; goto memFree;}
  if ((nlVal_rand=femDblAlloc(nlLen)) == NULL) {rv = AF_ERR_MEM; goto memFree;}
  if ((elValL_rand=femDblAlloc(elLenL)) == NULL) 
	{
		if (elLenL > 0)
		{
			rv = AF_ERR_MEM; 
			goto memFree;
		}
	}

  if ((randMult=femDblAlloc(RandDFlen)) == NULL) {rv = AF_ERR_MEM; goto memFree;}

  return(rv);
memFree:
  fem_rand_backup_free();
  return(rv);
}

/** copies original input data to the  */
int fem_rand_backup_orig_data(void)
{
  int  rv = AF_OK ;
  long i ;

  if ((rv=fem_rand_backup_alloc()) != AF_OK) {return(rv);}

  for (i=0; i<nLen; i++)
  {
    n_x_rand[i] = n_x[i] ;
    n_y_rand[i] = n_y[i] ;
    n_z_rand[i] = n_z[i] ;
  }

  for (i=0; i<rsLenL; i++) { rsValL_rand[i] = rsValL[i] ; }
  for (i=0; i<mpLenL; i++) { mpValL_rand[i] = mpValL[i] ; }
  for (i=0; i<nlLen; i++)  { nlVal_rand[i]  = nlVal[i] ; }
  for (i=0; i<elLenL; i++) { elValL_rand[i] = elValL[i] ; }

  return(rv);
}


void fem_rand_free_res(void)
{
  femVecFree(&u_rand1);
  free(resEVal_rand1); resEVal_rand1 = NULL ;
  free(resRval_rand1); resRval_rand1 = NULL ;
  femVecFree(&u_rand2);
  free(resEVal_rand2); resEVal_rand2 = NULL ;
  free(resRval_rand2); resRval_rand2 = NULL ;

  femVecFree(&u_randmax);
  free(resEVal_randmax); resEVal_randmax = NULL ;
  free(resRval_randmax); resRval_randmax = NULL ;

  femVecFree(&u_randmin);
  free(resEVal_randmin); resEVal_randmin = NULL ;
  free(resRval_randmin); resRval_randmin = NULL ;
}

void fem_rand_null_res(void)
{
  femVecNull(&u_rand1) ; 
  resEVal_rand1 = NULL ;
  resRval_rand1 = NULL ;
  femVecNull(&u_rand2) ; 
  resEVal_rand2 = NULL ;
  resRval_rand2 = NULL ;

  femVecNull(&u_randmax) ; 
  resEVal_randmax = NULL ;
  resRval_randmax = NULL ;

  femVecNull(&u_randmin) ; 
  resEVal_randmin = NULL ;
  resRval_randmin = NULL ;
}

/** allocates space for random results  */
int fem_rand_alloc_res(void)
{
  int rv = AF_OK ;

  fem_rand_count    = 0 ;
  fem_rand_fail_num = 0 ;

  fem_rand_null_res();

  if ((rv = femVecFullInit(&u_rand1, nDOFAct)) != AF_OK) 
  {
    goto memFree;
  }

  if ((resRval_rand1 = femDblAlloc(nlLen)) == NULL)
  {
    rv = AF_ERR_MEM;
    goto memFree;
  }

  if ((resEVal_rand1 = femDblAlloc(resELen)) == NULL)
  {
    rv = AF_ERR_MEM;
    goto memFree;
  }


  if ((rv = femVecFullInit(&u_rand2, nDOFAct)) != AF_OK) 
  {
    goto memFree;
  }

  if ((resRval_rand2 = femDblAlloc(nlLen)) == NULL)
  {
    rv = AF_ERR_MEM;
    goto memFree;
  }

  if ((resEVal_rand2 = femDblAlloc(resELen)) == NULL)
  {
    rv = AF_ERR_MEM;
    goto memFree;
  }

	/* - */

  if ((rv = femVecFullInit(&u_randmax, nDOFAct)) != AF_OK) 
  {
    goto memFree;
  }

  if ((resRval_randmax = femDblAlloc(nlLen)) == NULL)
  {
    rv = AF_ERR_MEM;
    goto memFree;
  }

  if ((resEVal_randmax = femDblAlloc(resELen)) == NULL)
  {
    rv = AF_ERR_MEM;
    goto memFree;
  }


  if ((rv = femVecFullInit(&u_randmin, nDOFAct)) != AF_OK) 
  {
    goto memFree;
  }

  if ((resRval_randmin = femDblAlloc(nlLen)) == NULL)
  {
    rv = AF_ERR_MEM;
    goto memFree;
  }

  if ((resEVal_randmin = femDblAlloc(resELen)) == NULL)
  {
    rv = AF_ERR_MEM;
    goto memFree;
  }

  return(rv);
memFree:
  fem_rand_free_res();
  return(rv);
}

/** saves results for random solution */
int fem_rand_add_res(void)
{
  long i;
  double val1, val2;

  fem_rand_count++ ; /* new simulation */

  /* all vectors must be VEC_FULL! */
  if ( (u.type != VEC_FULL) || (u_rand1.type != VEC_FULL) || (u_rand2.type != VEC_FULL) )
  {
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("These vectors must be VEC_FULL: u, u_rand1, u_rand2"));
#endif
    return(AF_ERR_VAL);
  }
  
  for (i=0; i<nDOFAct; i++)
  {
    val1 = u.data[i] ;
    val2 = val1*val1 ;

    u_rand1.data[i] += val1 ;
    u_rand2.data[i] += val2 ;

		if (val1 > u_randmax.data[i]) {u_randmax.data[i] = val1;}
		if (val1 < u_randmin.data[i]) {u_randmin.data[i] = val1;}
  }

  for (i=0; i<resRLen; i++)
  {
    val1 = resRval[i] ;
    val2 = val1*val1 ;

    resRval_rand1[i] += val1 ;
    resRval_rand2[i] += val2 ;

		if (val1 > resRval_randmax[i]) {resRval_randmax[i] = val1;}
		if (val1 < resRval_randmin[i]) {resRval_randmin[i] = val1;}
  }

  for (i=0; i<resELen; i++)
  {
    val1 = resEVal[i] ;
    val2 = val1*val1 ;

    resEVal_rand1[i] += val1 ;
    resEVal_rand2[i] += val2 ;

		if (val1 > resEVal_randmax[i]) {resEVal_randmax[i] = val1;}
		if (val1 < resEVal_randmin[i]) {resEVal_randmin[i] = val1;}
  }

  return(AF_OK);
}

/** replaces result data with "rand1" */
int fem_rand_swap_res1(void)
{
  long i;

  /* all vectors must be VEC_FULL! */
  if ( (u.type != VEC_FULL) || (u_rand1.type != VEC_FULL) )
  {
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("These vectors must be VEC_FULL: u, u_rand1, u_rand2"));
#endif
    return(AF_ERR_VAL);
  }
  
  for (i=0; i<nDOFAct; i++) { u.data[i] = u_rand1.data[i] ; }
  for (i=0; i<resRLen; i++) { resRval[i] = resRval_rand1[i] ; }
  for (i=0; i<resELen; i++) { resEVal[i] = resEVal_rand1[i] ; }

  return(AF_OK);
}

/** replaces result data with "rand2" */
int fem_rand_swap_res2(void)
{
  long i;

  /* all vectors must be VEC_FULL! */
  if ( (u.type != VEC_FULL) || (u_rand2.type != VEC_FULL) )
  {
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("These vectors must be VEC_FULL: u, u_rand1, u_rand2"));
#endif
    return(AF_ERR_VAL);
  }
  
  for (i=0; i<nDOFAct; i++) { u.data[i] = u_rand2.data[i] ; }
  for (i=0; i<resRLen; i++) { resRval[i] = resRval_rand2[i] ; }
  for (i=0; i<resELen; i++) { resEVal[i] = resEVal_rand2[i] ; }

  return(AF_OK);
}

/** replaces result data with "max" */
int fem_rand_swap_resmax(void)
{
  long i;

  /* all vectors must be VEC_FULL! */
  if ( (u.type != VEC_FULL) || (u_randmax.type != VEC_FULL) )
  {
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("These vectors must be VEC_FULL: u, u_rand1, u_randmax"));
#endif
    return(AF_ERR_VAL);
  }
  
  for (i=0; i<nDOFAct; i++) { u.data[i] = u_randmax.data[i] ; }
  for (i=0; i<resRLen; i++) { resRval[i] = resRval_randmax[i] ; }
  for (i=0; i<resELen; i++) { resEVal[i] = resEVal_randmax[i] ; }

  return(AF_OK);
}

/** replaces result data with "min" */
int fem_rand_swap_resmin(void)
{
  long i;

  /* all vectors must be VEC_FULL! */
  if ( (u.type != VEC_FULL) || (u_randmin.type != VEC_FULL) )
  {
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("These vectors must be VEC_FULL: u, u_rand1, u_randmin"));
#endif
    return(AF_ERR_VAL);
  }
  
  for (i=0; i<nDOFAct; i++) { u.data[i] = u_randmin.data[i] ; }
  for (i=0; i<resRLen; i++) { resRval[i] = resRval_randmin[i] ; }
  for (i=0; i<resELen; i++) { resEVal[i] = resEVal_randmin[i] ; }

  return(AF_OK);
}


/** computes random number from histogram */
double fem_rand_simval(long df_id)
{
  double val = 1.0 ;
  double rval;
  int    i ;

  if (df_id >= RandDFlen)
  {
#ifdef DEVEL_VERBOSE
    fprintf(msgout,"[E] %s: %li!\n", _("Invalid histogram"), df_id);
#endif
    return(1.0);
  }

  rval = fem_rand(&fem_rand_init);
  /*rval = rand();*/

  for (i=0; i<(RandDF[df_id].len-1); i++)
  {
    if ( (rval > RandDF[df_id].prob[i]) && (rval <= RandDF[df_id].prob[i+1]) )
    {
      val = RandDF[df_id].size[i] ;
      break;
    }
  }

/*printf("%f  %f\n",val, rval);*/
  
  return(val);
}

/** computes random data for Monte Carlo solution */
int fem_rand_create_random_data(void)
{
  int  rv = AF_OK ;
  double val ;
  long   pos, type;
  long i ;

  /* set default input data: */

  for (i=0; i<nLen; i++)
  {
    n_x[i] = n_x_rand[i] ;
    n_y[i] = n_y_rand[i] ;
    n_z[i] = n_z_rand[i] ;
  }

  for (i=0; i<rsLenL; i++) { rsValL[i] = rsValL_rand[i] ; }
  for (i=0; i<mpLenL; i++) { mpValL[i] = mpValL_rand[i] ; }
  for (i=0; i<nlLen; i++)  { nlVal[i]  = nlVal_rand[i]  ; }
  for (i=0; i<elLenL; i++) { elValL[i] = elValL_rand[i] ; }


  /* create random data (== make it crazy): --------- */

  /* field of random value multipliers  */
  for (i=0; i<RandDFlen; i++)
  {
    randMult[i] = fem_rand_simval(i) ;
  }

  for (i=0; i<RandValLen; i++)
  {
    /* random data multiplier */
    val = randMult[randVal[i]] ;

    /* type of the modified data */
    switch (randData[i])
    {
      case 0: /* real set values */
              type = rsType[randSet[i]];

              pos = femGetRepValIndex(
					      randSet[i],  /* real set position */
					      randPos[i],  /* value type */
					      randRpos[i], /* repeat set number */
					      rsValL, 
					      rsLenL, 
					      rsFrom, 
					      rsLen, 
					      Elem[type].real, 
					      Elem[type].rs, 
					      Elem[type].real_rp, 
					      Elem[type].rs_rp 
					      );

              rsValL[pos] *= val ;
              break;

      case 1: /* material values */
              type = mpType[randSet[i]];

              pos = femGetRepValIndex (	
	 							randSet[i],  /* material set position */
								randPos[i],  /* material data type */
								randRpos[i], /* repeat set number */
								mpValL, 
								mpLenL, 
								mpFrom, 
								mpLen, 
								Mat[type].val, 
								Mat[type].num, 
								Mat[type].val_rp, 
								Mat[type].num_rp
								);

              mpValL[pos] *= val ;
              break;

      case 2: /* nodes */
              if (randSet[i] >= nLen) { return(AF_ERR_VAL); }
              switch(randPos[i])
              {
                case 0: n_x[randSet[i]] *= val;  break;
                case 1: n_y[randSet[i]] *= val;  break;
                case 2: n_z[randSet[i]] *= val;  break;
                default: return(AF_ERR); break;
              }
              break;

      case 3: /* nodal loads */
              if (randSet[i] >= nlLen) { return(AF_ERR_VAL); }
              nlVal[randSet[i]] *= val ;
              break;

      case 4: /* element loads */
              pos = elFrom[randSet[i]+randPos[i]] ;
              if (pos >= elLenL) { return(AF_ERR_SIZ); }
              elValL[pos] *= val ;
              break;

      default: 
#ifdef DEVEL_VERBOSE
              fprintf(msgout,"[E] %s: %li!\n",
                  _("Invalid data type to be randomized found"),
                  randData[i]);
#endif
              return(AF_ERR_VAL);
              break;
    }
  }

  return(rv);
}

/* end of fem_rand.c */
