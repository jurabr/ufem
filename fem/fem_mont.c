/*
   File name: fem_mont.c
   Date:      2006/08/15 12:05
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

   FEM Solver - tools for building the code in the form of dynamic
   library loadable by "Monte" reliability tool.
*/

#include "fem.h"
#include "fem_mem.h"
#include "fem_sol.h"
#include <string.h>

#ifdef USE_MONTE
#include "fem_mont.h"


extern void fem_sol_null(void);
extern int fem_dofs(void);
extern int fem_sol_alloc(void);
extern int fem_sol_res_alloc(void);
extern void fem_sol_free(void);
extern int fem_fill_K(long mode);
extern int fem_add_loads(void);
extern int fem_add_disps(long disp_mode);
extern int    femSolveDynNewmark(double *ofld);

extern long  nDOFlen  ; /* lenght of nDOFfld                        */
extern long *nDOFfld  ; /* description of DOFs in nodes             */

extern char *fem_ssfile ; /* substep result file */

#ifdef FEM_MONT_FSAVE
static long fem_monte_file_num = 0 ; /* file number for results */
#endif

/* DATA STRUCTURES */

long monte_i_len = 0 ; /* input variables */
long monte_o_len = 0 ; /* output variables */

long *monte_io_var_pos     = NULL ; /* random variable index*/
long *monte_io_var_type    = NULL ; /* node, elem */
long *monte_io_item        = NULL ; /* position of node, elem.. */
long *monte_io_subitem     = NULL ; /* x,y, property number,...*/
long *monte_io_subitemiter = NULL ; /* repeating value (if needed) */

long monte_fail_funct_id   = -1   ; /* still ignored */

/* FUNCTIONS */

void monte_io_null(void)
{
  monte_i_len = 0 ; /* input variables */
  monte_o_len = 0 ; /* output variables */
  monte_fail_funct_id = -1 ;

  monte_io_var_pos     = NULL ; /* random variable index*/
  monte_io_var_type    = NULL ; /* node, elem */
  monte_io_item        = NULL ; /* position of node, elem.. */
  monte_io_subitem     = NULL ; /* x,y, property number,...*/
  monte_io_subitemiter = NULL ; /* repeating valee (if needed) */
}

void monte_io_free(void)
{
  free ( monte_io_var_pos     ) ;
  free ( monte_io_var_type    ) ;
  free ( monte_io_item        ) ;
  free ( monte_io_subitem     ) ;
  free ( monte_io_subitemiter ) ;

  monte_io_null() ;
}

int monte_io_alloc(long ilen, long olen)
{
  double n ;

  monte_io_null();

  n = ilen + olen ;

  if (n <= 0) {return(AF_OK);}
  
  if ((monte_io_var_pos=femIntAlloc(n)    )== NULL) {goto memFree;}
  if ((monte_io_var_type=femIntAlloc(n)   )== NULL) {goto memFree;}
  if ((monte_io_item    =femIntAlloc(n)   )== NULL) {goto memFree;}
  if ((monte_io_subitem =femIntAlloc(n)   )== NULL) {goto memFree;}
  if ((monte_io_subitemiter=femIntAlloc(n))== NULL) {goto memFree;}

  monte_i_len = ilen ;
  monte_o_len = olen ;

  return(AF_OK);
memFree:
  monte_io_free();
  return(AF_ERR_MEM);
}

/* MONTE FUNCTIONS */
long monte_dlib_interface_type(void) 
{
  return(2) ; /* 2 is for the advanced type */
}

void monte_nums_of_vars(char *param, long *ilen, long *olen, long *ffunc)
{
  *ilen = monte_i_len ; /* required number of input variables */
  *olen = monte_o_len ; /* returned number of output variables */
  *ffunc = -1 ;         /* TODO ... currently not available */
  return ;
}

char *monte_ivar_name(char *param, long pos)
{
  static char name[257];
  char dir[] = "_";
  int i ;

  if (pos >= monte_i_len) { return("empty"); }
  for (i=0; i<257; i++) { name[i] = '\0'; }
 
  switch (monte_io_var_type[pos])
  {
    case MONTE_VTYPE_RS:
      sprintf(name,"RS_%li_P%li_I%li",
          rsID[monte_io_item[pos]],
          monte_io_subitem[pos],
          monte_io_subitemiter[pos]
          );
			return(name);
        break;
    case MONTE_VTYPE_MAT:
      sprintf(name,"MAT_%li_P%li_I%li",
          mpID[monte_io_item[pos]],
          monte_io_subitem[pos],
          monte_io_subitemiter[pos]
          );
			return(name);
        break;
    case MONTE_VTYPE_N:
      switch(monte_io_subitem[pos])
      {
        case 1: dir[0]='X' ; break;
        case 2: dir[0]='Y' ; break;
        case 3: dir[0]='Z' ; break;
      }
      sprintf(name,"N_%li_%s",
          nID[monte_io_item[pos]],
          dir
          );
			return(name);
        break;
  case MONTE_VTYPE_NLD:
      sprintf(name,"NLD_N%li_T%li_D%li",
          nID[nlNode[monte_io_item[pos]]],
          monte_io_subitem[pos],
          monte_io_subitemiter[pos]
          );
			return(name);
        break;
  case MONTE_VTYPE_NLPOS: 
      sprintf(name,"NLP_%li_POS%li_T%li_D%li",
          /*nID[nlNode[monte_io_item[pos]]],*/
          pos+1,
          monte_io_item[pos]+1,
          monte_io_subitem[pos],
          monte_io_subitemiter[pos]
          );
			return(name);
        break;

  case MONTE_VTYPE_EL:
      sprintf(name,"EL_E%li_T%li_I%li",
          eID[elElem[monte_io_item[pos]]],
          monte_io_subitem[pos],
          monte_io_subitemiter[pos]
          );
			return(name);
        break;

    case MONTE_VTYPE_DAMP:
				if (monte_io_item[pos] == 1)
				     { sprintf(name,"DAMP_ALPHA"); }
				else { sprintf(name,"DAMP_BETA"); }
			return(name);
        break;

    default: return("empty"); break;
  }



  return("empty");
}


/** Returnes name of output variable
 * @parame param parameter passed from Monte
 * @param pos number of variable in field (0..n-1)
 * @return string with variable name
 */
char *monte_ovar_name(char *param, long pos)
{
  static char name[257];
  char dir[]  = "__";
  int i ;

  if (pos >= monte_o_len) { return("empty"); }
  for (i=0; i<257; i++) { name[i] = '\0'; }

  pos += monte_i_len ;
  
  switch (monte_io_var_type[pos])
  {
    case MONTE_VTYPE_RES_D:
      switch(monte_io_subitem[pos])
      {
        case 1: dir[0] = 'U'; dir[1]='X' ; break;
        case 2: dir[0] = 'U'; dir[1]='Y' ; break;
        case 3: dir[0] = 'U'; dir[1]='Z' ; break;
        case 4: dir[0] = 'R'; dir[1]='X' ; break;
        case 5: dir[0] = 'R'; dir[1]='Y' ; break;
        case 6: dir[0] = 'R'; dir[1]='Z' ; break;
      }
      sprintf(name,"DISP_%s_%li",dir,nID[monte_io_item[pos]]);
			return(name);
      break;

    case MONTE_VTYPE_RES_D_MAX:
      switch(monte_io_subitem[pos])
      {
        case 1: dir[0] = 'A'; dir[1]='X' ; break;
        case 2: dir[0] = 'A'; dir[1]='Y' ; break;
        case 3: dir[0] = 'A'; dir[1]='Z' ; break;
        case 4: dir[0] = 'B'; dir[1]='X' ; break;
        case 5: dir[0] = 'B'; dir[1]='Y' ; break;
        case 6: dir[0] = 'B'; dir[1]='Z' ; break;
      }
      sprintf(name,"DISP_%s_%li",dir,nID[monte_io_item[pos]]);
			return(name);
      break;

    case MONTE_VTYPE_RES_D_MIN:
      switch(monte_io_subitem[pos])
      {
        case 1: dir[0] = 'P'; dir[1]='X' ; break;
        case 2: dir[0] = 'P'; dir[1]='Y' ; break;
        case 3: dir[0] = 'P'; dir[1]='Z' ; break;
        case 4: dir[0] = 'Q'; dir[1]='X' ; break;
        case 5: dir[0] = 'Q'; dir[1]='Y' ; break;
        case 6: dir[0] = 'Q'; dir[1]='Z' ; break;
      }
      sprintf(name,"DISP_%s_%li",dir,nID[monte_io_item[pos]]);
			return(name);
      break;

    case MONTE_VTYPE_RES_R: 
      switch(monte_io_subitem[pos])
      {
        case 1: dir[0] = 'F'; dir[1]='X' ; break;
        case 2: dir[0] = 'F'; dir[1]='Y' ; break;
        case 3: dir[0] = 'F'; dir[1]='Z' ; break;
        case 4: dir[0] = 'M'; dir[1]='X' ; break;
        case 5: dir[0] = 'M'; dir[1]='Y' ; break;
        case 6: dir[0] = 'M'; dir[1]='Z' ; break;
      }
      sprintf(name,"REAC_%s_%li",dir,nID[monte_io_item[pos]]);

			return(name);
      break;
    case MONTE_VTYPE_RES_E: 
      sprintf(name,"ERES_E%li_T%li_I%li",
          eID[monte_io_item[pos]],
          monte_io_subitem[pos],
          monte_io_subitemiter[pos]
          );
			return(name);
        break;

    case MONTE_VTYPE_RES_SUM_E: 
      sprintf(name,"ERES_SUM_T%li",
          monte_io_subitem[pos]
          );
			return(name);
        break;

    case MONTE_VTYPE_RES_MAX_E: 
      sprintf(name,"ERES_MAX_T%li",
          monte_io_subitem[pos]
          );
			return(name);
        break;

    case MONTE_VTYPE_RES_MIN_E: 
      sprintf(name,"ERES_MIN_T%li",
          monte_io_subitem[pos]
          );
			return(name);
        break;

    case MONTE_VTYPE_RES_FAIL_E: 
      sprintf(name,"ERES_FAIL_T%li",
          monte_io_subitem[pos]
          );
			return(name);
        break;

    default: return("empty"); break;
  }

  return("empty");
}

/* prepares base name of output files (if any) */
#ifdef FEM_MONT_FSAVE
int monte_prep_outfile(char *param)
{
  long len = 0;
  long i;
  
  if (param == NULL) { fem_monte_file_num = -1 ; return(AF_ERR);}
  if (len=strlen(param) < 1) { fem_monte_file_num = -1 ; return(AF_ERR);}
  if ((fem_ssfile=(char *)malloc((len+1)*sizeof(char))) == NULL) { fem_monte_file_num = -1 ; return(AF_ERR);}

  for (i=0; i<=len; i++){fem_ssfile[i] = '\0';}
  strcpy(fem_ssfile, param);

  return(AF_OK);
}
#endif

int monte_init_lib_stuff(char *param)
{
#ifdef RUN_VERBOSE
	msgout = stderr ; /* for output from "fprintf(msgout,...)" */
#endif

  /* TODO: workaround to enable dynamics - find a better solution */
  solUseCGSSOR = AF_YES ;
  femDynamics  = AF_YES ; 
  femNewmarkEL = AF_YES ;

  if (param == NULL) { return(-1); }
  if (strlen(param) < 1) { return(-1); }

#ifdef FEM_MONT_FSAVE
  monte_prep_outfile(param);
#endif
	
	/* Setting of data to NULL */
  if ((femDataNull()) != AF_OK) {return(-1);}
  if ((femResNull()) != AF_OK) {return(-1);}

	femPreparsedData = AF_YES ;

	if ((femReadInput(param)) != AF_OK) {return(-1);}

 	if ((femElemTypeInit()) != AF_OK) { return(-1); }
 	if ((femMatTypeInit()) != AF_OK) { return(-1); }

  fem_sol_null();
  femResNull();

 	if ((fem_dofs()) != AF_OK) { return(-1); }
 	if ((fem_sol_alloc()) != AF_OK) { return(-1); }
 	if ((fem_sol_res_alloc()) != AF_OK) { return(-1); }

  return(0);
}

int monte_clean_lib_stuff(char *param)
{
	fem_sol_free();
	femDataFree();
	femResFree();
  return(0);
}

/* linear static solver wrapper: */
int monte_linear_solver(void)
{
  int rv = AF_OK ;

	femMatSetZeroBig(&K);
	femVecSetZeroBig(&F);
	femVecSetZeroBig(&u);

 	if ((rv = fem_fill_K(AF_NO)) != AF_OK) { goto memFree; }

 	if ((rv = fem_add_loads()) != AF_OK) { goto memFree; }
 	if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }

	if ((rv = femEqsCGwJ(&K, &F, &u, FEM_ZERO/10000.0, K.rows)) != AF_OK) { goto memFree; }
 	if ((rv = fem_fill_K(AF_YES)) != AF_OK) { goto memFree; }

memFree:
  return(rv);
}

/** Filling of results data filed (ofld)
 * @param ofld field to put data in
 * @return status
 */
int monte_fill_ofld_data(double *ofld)
{
	long i, j, k, pos ;
	double val ;

	for (i=monte_i_len; i< (monte_i_len+monte_o_len); i++)
  {
    ofld[monte_io_var_pos[i-monte_i_len]] = 0 ;

    switch (monte_io_var_type[i])
    {
      case MONTE_VTYPE_RES_D:  /* displacements */
        if ((pos = monte_io_item[i]*KNOWN_DOFS + monte_io_subitem[i]-1) > nDOFlen) {break;}
        pos = nDOFfld[pos] ;
        ofld[monte_io_var_pos[i-monte_i_len]] = femVecGet(&u, pos) ;
        break ;

        /* ************************************** */

      case MONTE_VTYPE_RES_D_MAX:  /* MAX displacements */
        if ((pos = monte_io_item[i]*KNOWN_DOFS + monte_io_subitem[i]-1) > nDOFlen) {break;}
        pos = nDOFfld[pos] ;
        val = femVecGet(&u, pos) ;
        if (ofld[monte_io_var_pos[i-monte_i_len]]  < val)
           { ofld[monte_io_var_pos[i-monte_i_len]] = val ; }
        break ;

      case MONTE_VTYPE_RES_D_MIN:  /* MIN displacements */
        if ((pos = monte_io_item[i]*KNOWN_DOFS + monte_io_subitem[i]-1) > nDOFlen) {break;}
        pos = nDOFfld[pos] ;
        val = femVecGet(&u, pos) ;
        if (ofld[monte_io_var_pos[i-monte_i_len]] > val)
           { ofld[monte_io_var_pos[i-monte_i_len]] = val ; }
        break ;

        /* ************************************** */

      case MONTE_VTYPE_RES_R: /* reactions */
        for (j=0; j<resRLen; j++)
        {
          if ((resRnode[j] == monte_io_item[i])&&(resRdof[j]==monte_io_subitem[i]))
          {
            ofld[monte_io_var_pos[i-monte_i_len]] = resRval[j] ;
            break ;
          }
        }
        break ;

      case MONTE_VTYPE_RES_E: /* element results */
        ofld[monte_io_var_pos[i-monte_i_len]] = femGetEResVal(
          monte_io_item[i], 
          monte_io_subitem[i], 
          monte_io_subitemiter[i]
        ) ;
      break ;

      case MONTE_VTYPE_RES_SUM_E: /* sum of element results */
        ofld[monte_io_var_pos[i-monte_i_len]] = 0.0 ;
        for(j=0; j<eLen; j++)
        {
          for (k=0; k<=Elem[femGetETypePos(j)].res_rp;k++)
          {
            ofld[monte_io_var_pos[i-monte_i_len]] += femGetEResVal(
            j, 
            monte_io_subitem[i], 
            k
          ) ;
          }
        }
      break ;

      case MONTE_VTYPE_RES_MAX_E: /* max of element results */
        if (femNewmarkEL != AF_YES)
           { ofld[monte_io_var_pos[i-monte_i_len]] = 0.0 ; }
        for(j=0; j<eLen; j++)
        {
          for (k=0; k<=Elem[femGetETypePos(j)].res_rp;k++)
          {
            if (
                (val = femGetEResVal( j, monte_io_subitem[i], k)) 
                > 
                ofld[monte_io_var_pos[i-monte_i_len]]
                ) 
            {
              ofld[monte_io_var_pos[i-monte_i_len]] = val ;
            }
          }
        }
      break ;

      case MONTE_VTYPE_RES_MIN_E: /* min of element results */
        if (femNewmarkEL != AF_YES)
           { ofld[monte_io_var_pos[i-monte_i_len]] = 0.0 ; }
        for(j=0; j<eLen; j++)
        {
          for (k=0; k<=Elem[femGetETypePos(j)].res_rp;k++)
          {
            if (
                (val = femGetEResVal( j, monte_io_subitem[i], k)) 
                < 
                ofld[monte_io_var_pos[i-monte_i_len]]
                ) 
            {
              ofld[monte_io_var_pos[i-monte_i_len]] = val ;
            }
          }
        }
      break ;

      case MONTE_VTYPE_RES_FAIL_E: /* failure state */
        if (femNewmarkEL != AF_YES)
           { ofld[monte_io_var_pos[i-monte_i_len]] = 0.0 ; }
        val = -1 ;

        for(j=0; j<eLen; j++)
        {
          for (k=0; k<=Elem[femGetETypePos(j)].res_rp;k++)
          {
            if (
                (val = femGetEResVal( j, monte_io_subitem[i], k)) 
                >  
                0.0
                ) 
            {
              ofld[monte_io_var_pos[i-monte_i_len]] = 1.0 ;
              break ;
            }
            if (val  > 0.0) { break ; } /* speedup */
          }
          if (val  > 0.0) { break ; } /* even speder speedup */
        }
      break ;

    }
  }
	return(AF_OK);
}

/* Provides linear static/dynamic solution for Monte
 * @parame param parameter passed from Monte
 * @param ifld random input data field
 * @param ofld output data field
 */
int monte_solution(char *param, double *ifld, double *ofld)
{
	int rv = 0;
  long i, j, pos ;

  /* TODO use ifld */
  for (i=0; i< monte_i_len; i++)
  {
    switch(monte_io_var_type[i])
    {
      case MONTE_VTYPE_RS:
          femSetRSValPos(
              rsType[monte_io_item[i]], 
              monte_io_item[i], 
              monte_io_subitem[i], 
              monte_io_subitemiter[i],
              ifld[monte_io_var_pos[i]]
              ) ;
        break ;
      case MONTE_VTYPE_MAT: 
          pos =  mpType[monte_io_item[i]] ;
          pos = femGetRepValIndex(
                    monte_io_item[i], 
                    monte_io_subitem[i], 
										monte_io_subitemiter[i],
										mpValL, mpLenL,
										mpFrom, mpLen,
                    Mat[pos].val, Mat[pos].num, 
                    Mat[pos].val_rp, Mat[pos].num_rp
										);
          if (pos  > -1)
          {
            mpValL[pos] = ifld[monte_io_var_pos[i]] ;
          }

        break ;

      case MONTE_VTYPE_N:
        switch (monte_io_subitem[i])
        {
          case 1: n_x[monte_io_item[i]] = ifld[monte_io_var_pos[i]] ;break ;
          case 2: n_y[monte_io_item[i]] = ifld[monte_io_var_pos[i]] ; break ;
          case 3: n_z[monte_io_item[i]] = ifld[monte_io_var_pos[i]] ;break ;
        }
        break ;
        
      case MONTE_VTYPE_NLD:
        for (j=0; j<nlLen; j++)
        {
          if ((nlNode[j] == monte_io_item[i])
              && (nlType[j] == monte_io_subitem[i])
              && (nlDir[j] == monte_io_subitemiter[i])
              )
          {
            nlVal[j] = ifld[monte_io_var_pos[i]] ;
            break ;
          }
        }
      break ;
        
      case MONTE_VTYPE_NLPOS: /* TODO fix this: */
        for (j=0; j<nlLen; j++)
        {
          if ((j == monte_io_item[i])
              && (nlType[j] == monte_io_subitem[i])
              && (nlDir[j] == monte_io_subitemiter[i])
              )
          {
            nlVal[j] = ifld[monte_io_var_pos[i]] ;
            break ;
          }
        }
      break ;

      case MONTE_VTYPE_EL:
        for (j=0; j<nlLen; j++)
        {
          if ((elElem[j] == monte_io_item[i])
              && (elType[j] == monte_io_subitem[i])
              )
          {
            elValL[elFrom[j+monte_io_subitemiter[i]]] = ifld[monte_io_var_pos[i]] ;
            break ;
          }
        }
      break ;

			case MONTE_VTYPE_DAMP:
				if (monte_io_item[i] == 1)
				   { dynAlpha = ifld[monte_io_var_pos[i]] ; }
				else
				   { dynBeta = ifld[monte_io_var_pos[i]] ;  }
      break ;

    }
  }

  femTangentMatrix = AF_NO ; /* workaround - we need empty result fields! */

  /* call solver here */
  if (femNewmarkEL == AF_YES) /* transient dynamics: */
  {
    if ((rv =  femSolveDynNewmark(ofld)) != AF_OK) { goto memFree ; }
  }
  else /* falling back to linear solution */
  {
    if ((rv =  monte_linear_solver()) != AF_OK) { goto memFree ; }
  }
 
  /* RESULTS: fill ofld */
	monte_fill_ofld_data(ofld) ;
  

#ifdef FEM_MONT_FSAVE
  if (fem_monte_file_num > -1)
  {
    /* might be usefull in some cases... */
    fem_monte_file_num++ ;
	  if ((rv = femWriteRes( femSubStepFname(fem_monte_file_num))) != AF_OK) { goto memFree; }
  }
#endif

memFree:
	return(rv);
}

#endif

/** reads reliability data from input file - it is used (empty) even
 * if library in not build
 * @param fr file stream
 * @return status
 */ 
int fem_monte_read_data(FILE *fr)
{
#ifdef USE_MONTE
  long ilen, olen;
  long i ;

  if (fscanf(fr,"%li %li", &ilen, &olen) < 2) {return(AF_OK); /* nothing to do */}

  if (ilen <= 0) { return(AF_OK); /* nothing to do */ }

  if (monte_io_alloc(ilen, olen) != AF_OK) {return(AF_ERR_MEM);}
  
  for (i=0; i<(ilen+olen); i++)
  {
		if (
    fscanf(fr, "%li %li %li %li %li",
        &monte_io_var_pos[i],
        &monte_io_var_type[i],
        &monte_io_item[i],
        &monte_io_subitem[i],
        &monte_io_subitemiter[i]
        ) < 5) 
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n", ("Incomplete or unavailable random data"));
#endif
			return(AF_ERR_IO);
		}
#ifdef DEVEL_VERBOSE
  fprintf(msgout, "rand: %li %li %li %li %li\n",
        monte_io_var_pos[i],
        monte_io_var_type[i],
        monte_io_item[i],
        monte_io_subitem[i],
        monte_io_subitemiter[i]
        );
#endif /* DEVEL_VERBOSE*/
  }

#endif /* USE_MONTE */
  return(AF_OK);
}

/* end of fem_mont.c */
