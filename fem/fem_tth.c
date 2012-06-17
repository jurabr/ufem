/*
   File name: fem_tth.c
   Date:      2012/06/17 21:49
   Author:    Jiri Brozovsky

   Copyright (C) 2012 Jiri Brozovsky

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

   Transient thermal analysis
*/

#include "fem_mat.h"
#include "fem_elem.h"

/* from fem_sol.c: */
extern void fem_sol_null(void);
extern void fem_sol_free(void);
extern int fem_sol_alloc(void);
extern int fem_sol_res_alloc(void);
extern int fem_dofs(void);
extern int fem_add_loads(void);
extern int fem_add_disps(long disp_mode);
extern int fem_fill_K(long mode);
extern int fem_fill_M(void);

extern long  nDOFAct  ; /* total number of ACTIVE DOFs in structure (== size of "K" matrix) */
extern long  nDOFlen  ; /* lenght of nDOFfld                        */
extern long *nDOFfld  ; /* description of DOFs in nodes             */
extern long *K_rows   ; /* numbers of items in "K" rows K_rows[nDOFAct] */

extern tMatrix K; /* structure stiffness matrix    */
extern tVector F; /* structure load vector         */
extern tVector Fr;/* unballanced forces vector     */
extern tVector u; /* structure displacement vector */

/* dynamics: */
extern tMatrix C;         /* thermal capacity matrix */

/** Wrapper for linear system solvers
 * @param Ks stiffness matrix
 * @param Fs load vector
 * @param us displacement vector (result)
 * @return status
 */ 
extern int femLinEqSystemSolve(tMatrix *Ks, tVector *Fs, tVector *us);


/** Simple implicit dynamics solver: see Bitnar, Rericha: "Metoda
 * konecnych prvku v dynamice konstrukci", SNTL, Prague, 1981, p. 122
 * Newmark time integration procedure is used
 *
 * @param ofld random output data filed (only != NULL for Monte Carlo)
 * @return statis
 */
int femSolveThermTrans(void)
{
	int    rv = AF_OK ;
  long   i ;

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
	fprintf(msgout,"[i]   %s:\n",_("assembling of mass matrix"));
#endif
 	if ((rv = femMatAllocCloneStruct(&K, &M)) != AF_OK) { goto memFree; }
 	if ((rv = fem_fill_M()) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("assembling of math matrix done"));
#endif

#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s:\n",_("loads and supports"));
#endif
 	if ((rv = fem_add_disps(AF_YES)) != AF_OK) { goto memFree; }
#ifdef RUN_VERBOSE
	fprintf(msgout,"[i]   %s.\n",_("loads and supports done"));
#endif

  /* TODO solution here */

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




/* end of fem_tth.c */
