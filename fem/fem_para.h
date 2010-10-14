/*
   File name: fem_para.h
   Date:      2003/05/26 23:41
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

	 FEM Solver - command line parameters handling

	 $Id: fem_para.h,v 1.7 2005/01/01 21:01:26 jirka Exp $
*/


#ifndef __FEM_PARA_H__
#define __FEM_PARA_H__

#include "fem.h"
#include "fem_mem.h"
#include <string.h>

#ifndef _SMALL_FEM_CODE_
#include "fem_spnt.h"
#endif


extern long femPreparsedData  ; 
extern long femReadStdInput   ; 
extern long femWriteStdOutput ; 
extern long femUseSaPo        ;
extern long femReadPrevStep   ;
extern long femPrevStdInput   ; 
extern char *fem_ifile ;
extern char *fem_ofile ;
extern char *fem_rfile ;

extern long  femBreakSolu ;

extern long  femDynamics   ;
extern long  femEigenModal ;
extern long  femEigenNum   ;

extern int fem_parse_params(int argc, char *argv[]);
extern char *fem_output_file(void);

extern FILE *fem_sol_norm_file  ;

#ifndef _SMALL_FEM_CODE_
extern char *fem_spec_out_file   ;
extern long  fem_spec_out_type   ;
#if _MATRIX_SAVING_
extern long  femSaveMatFStat ;
extern long  femSaveSparMatFStat ;
extern long  femSaveSparMarketMatFStat ;
extern long  femSaveVecFStat ;

extern char *femSaveMatFName ;
extern char *femSaveSparMatFName ;
extern char *femSaveSparMarketMatFName ;
extern char *femSaveVecFName ;
#endif
#endif

extern long  femUseMC      ;
extern long  femMCSims     ;
extern long  femMCSaveMode ;
extern char *femMCfile     ;


extern char *femSubStepFname(long snumber);
extern long femSubStepCheckNumber(long number);

extern char *femMCGenFname(char *name, long sim, long mode);
#endif

/* end of fem_para.h */
