/*
   File name: fem_elem.h
   Date:      2003/04/11 21:27
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

	 FEM Solver - solver - header file
*/


#ifndef __FEM_SOL_H__
#define __FEM_SOL_H__

/* Included headers: */
#include <stdlib.h>
#include "fem.h"
#include "fem_dama.h"
#include "fem_data.h"
#include "fem_elem.h"
#include "fem_mat.h"
#include "fem_mem.h"
#include "fem_math.h"
#include "fem_para.h"

#ifdef _USE_SIGNALS_
#include <signal.h>
#endif

extern int femTangentMatrix ;
extern int  fem2ndOrder     ;
extern int  fem2ndOrderIter ;
extern int  femRunSolIterBC  ;
extern long femIterBCchange  ;
extern int  femTestIterBC   ;

extern tMatrix K;
extern tMatrix M;
extern tVector u;
extern tVector F;
extern tVector u_tot;


extern long femHaveThermDOFs ;
extern long femHaveThermLoad ;

/** computes starting position in nDOFfld for node on position node_pos */
#define nDOFstart(node_pos) (node_pos*KNOWN_DOFS)

extern int femSolve(void);
extern int femSolveNRM(long incr_type);
extern int femSolveALM(long incr_type);
extern int femSolveFullALM(void);
extern int femSolveDirDisps(long incr_type);

extern int femSolveMC(void);

extern long femKpos(long node_pos, long dof);
extern int femKhit(void);

extern int femLocKM_e(tMatrix *K_e, tMatrix *K, long ePos);
#define femLocK_e(K_e_vec, ePos_val) femLocKM_e(K_e_vec, &K, ePos_val)
#define femLocM_e(K_e_vec, ePos_val) femLocKM_e(K_e_vec, &M, ePos_val)

extern int femLocF_e(tVector *F_e, long ePos, tVector *F, long isF);
extern int femLocUtoU_e(tVector *U, long ePos, tVector *U_e);

extern int femApplyNLoad(long nPos, long Type, long Dir, double Val);
extern int femApplyNBC(long nPos, long Type, long Dir, double Val);
extern int femAddGravLoad(long Dir, double Val);
extern int femAddElemLoad(void);

extern int femSolveEigenInvIter(long max_iter, double eps);
extern int femSolveEigenLanczos(long max_iter, double eps);

extern double fem_get_struct_price(long volume_only);
extern int femComputePrice(void);
extern double femCompPE(tMatrix *K, tVector *u, int Print);

extern int femTestThermStructElems(void);

#ifdef FEM_USE_SILO
extern int femWriteNodeResSILO(char *fname) ;
#endif

#endif

/* end of fem_elem.h */
