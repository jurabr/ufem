/*
   File name: fem_mrun.c
   Date:      2008/08/15 19:50
   Author:    Jiri Brozovsky

   Copyright (C) 2008 Jiri Brozovsky

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

   FEM solver: testing of 2D (and later 3D) constitutive routines
*/

#include "fem_elem.h"
#include "fem_mat.h"
#include "fem_math.h"

long   femTestConstElem = -1  ;
long   femTestConstIpn  = -1  ;
double femTestConstA    = 0.0 ;
double femTestConstEX   = 0.0 ;
double femTestConstEY   = 0.0 ;
double femTestConstEZ   = 0.0 ;
double femTestConstEXY  = 0.0 ;
double femTestConstEYZ  = 0.0 ;
double femTestConstEZX  = 0.0 ;

/* Runs constitutive data testing routine with proper parameters
 * @param ePos position of selected element
 * @param ipoint integration point
 * @param A element area 
 * @param e_x total strain in X direction
 * @param e_y total strain in Y direction
 * @param e_xy total strain in XY direction
 * @return status
 */
int femDryRun2D(long ePos, long ipoint, double A, double e_x, double e_y, double e_xy)
{
  int     rv = AF_OK  ;
  FILE   *fw = NULL   ;
  long    i           ;
  long    dim     = 3 ;
  long    Problem = 0 ; /* sorry, no plane strain here */
  long    mtype   = 0 ;
  double  mult    = 0 ;
  tVector sigma       ;
  tVector sigma_r     ;
  tVector sigma_sum   ;
  tVector epsilon     ;
  tMatrix D           ;

  if (fem_ofile == NULL)
  { 
    fw = stdout ;
  }
  else
  {
    if (strlen(fem_ofile) >= 1)
    {
      if ( (fw=fopen(fem_ofile, "w")) == NULL )
      {
        fw = stdout ;
      }
    }
    else
    {
      fw = stdout ;
    }
  }

  femMatNull(&D) ;
  femVecNull(&sigma) ;
  femVecNull(&sigma_r) ;
  femVecNull(&sigma_sum) ;
  femVecNull(&epsilon) ;

  if ((rv=femFullMatInit(&D,  dim, dim)) != AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&sigma  , dim)) != AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&sigma_r, dim)) != AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&sigma_sum, dim)) != AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&epsilon, dim)) != AF_OK) { goto memFree; }

 	if ((rv = femElemTypeInit()) != AF_OK) { goto memFree; }
 	if ((rv = femMatTypeInit()) != AF_OK) { goto memFree; }

  fem_sol_null();
  femResNull();

 	if ((rv = fem_dofs()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_alloc()) != AF_OK) { goto memFree; }
 	if ((rv = fem_sol_res_alloc()) != AF_OK) { goto memFree; } /* __must__ be done _before_ adding of loads! */

  mtype = Mat[femGetMPTypePos(femGetEMPPos(ePos))].type ;

  mult = 1.0 / (double)solNoLinSteps ;

  femVecPut(&epsilon, 1, mult * e_x) ; 
  femVecPut(&epsilon, 2, mult * e_y) ; 
  femVecPut(&epsilon, 3, mult * e_y) ; 

  for (i=1; i<=solNoLinSteps; i++)
  {
    femVecSetZero(&sigma_r) ;
    femVecSetZero(&sigma) ;

    fem_D_2D( 
      ePos, ipoint, A, 
      &epsilon, &sigma, &sigma_r, 
      AF_YES, Problem, &D
      );

#if 0
    if ( (femVecNorm(&sigma) <= FEM_ZERO) && (femVecNorm(&sigma_r) <= FEM_ZERO) )
    {
      femMatVecMult(&D, &epsilon, &sigma) ;
    }
#else
    femVecSetZero(&sigma);
    femMatVecMult(&D, &epsilon, &sigma) ;
#endif

    femVecAddVec(&sigma_sum, 1.0, &sigma) ;

    
    femAddEResVal(ePos, RES_EX, ipoint+1,  e_x) ;
    femAddEResVal(ePos, RES_EY, ipoint+1,  e_y) ;
    femAddEResVal(ePos, RES_EXY, ipoint+1,  e_xy) ;

#if 0
    femAddEResVal(ePos, RES_SX, ipoint+1,  femVecGet(&sigma,1)) ;
    femAddEResVal(ePos, RES_SY, ipoint+1,  femVecGet(&sigma,2)) ;
    femAddEResVal(ePos, RES_SXY, ipoint+1, femVecGet(&sigma,3)) ;
#else
    femPutEResVal(ePos, RES_SX, ipoint+1,  femVecGet(&sigma_sum,1)) ;
    femPutEResVal(ePos, RES_SY, ipoint+1,  femVecGet(&sigma_sum,2)) ;
    femPutEResVal(ePos, RES_SXY, ipoint+1, femVecGet(&sigma_sum,3)) ;
#endif

      fprintf(fw, "%li %e %e %e %e %e %e\n",
        i,
        femVecGet(&sigma_sum, 1),
        femVecGet(&sigma_sum, 2),
        femVecGet(&sigma_sum, 3),
        (double)i * mult * e_x  ,
        (double)i * mult * e_y  ,
        (double)i * mult * e_xy 
        );
  }
  

memFree:
  femMatFree(&D) ;
  femVecFree(&sigma) ;
  femVecFree(&sigma) ;
  femVecFree(&sigma_sum) ;
  femVecFree(&epsilon) ;

	fem_sol_free();
	femDataFree();
	femResFree();

  fclose(fw) ;

  return(rv);
}


/* Does constitutive equation testing for 2D plane stress
 * (in 3D maybe in future)
 * @return status
 */
int femDryRun(void)
{
  long i ;
  long ePos = -1 ;

  for (i=0; i<eLen; i++)
  {
    if (eID[i] == femTestConstElem)
    { 
      ePos = eID[i] ;
      break ;
    }
  }

  if (eLen < 0) 
  {
    fprintf(msgout, "[E} %s!\n", _("Requested element not found") );
    return(AF_ERR_VAL);
  }

  return ( femDryRun2D(
        ePos, 
        femTestConstIpn, 
        femTestConstA, 
        femTestConstEX, 
        femTestConstEY, 
        femTestConstEXY
        ) 
      ) ;

}

/* end of fem_mrun.c */
