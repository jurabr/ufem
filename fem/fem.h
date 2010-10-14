/*
   File name: fem.h
   Date:      2003/04/07 09:02
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

	 FEM Solver - header file

  Id: fem.h,v 1.5 2003/04/24 17:19:16 jirka Exp $
*/


#ifndef __FEM_H__
#define __FEM_H__

#define _FEM_VERSION_ "0.1.11"

/* Included headers: */
#include <stdio.h>
#include <stdlib.h>

#ifdef _USE_THREADS_
#include <pthread.h>
#endif

/* Definition of return values */
#define AF_OK       0    /* OK                        */
#define AF_ERR     -1    /* error                     */
#define AF_ERR_IO  -2    /* error during IO operation */
#define AF_ERR_VAL -3    /* illegal value detected    */
#define AF_ERR_MEM -4    /* out of memory             */
#define AF_ERR_TYP -5    /* illegal type of something */
#define AF_ERR_SIZ -9    /* illegal size of something */
#define AF_ERR_SML -10   /* too small (short)         */
#define AF_ERR_BIG -11   /* too big (long)            */
#define AF_ERR_EMP -12   /* empty                     */
#define AF_ERR_ZER -13   /* zero value                */
#define AF_ERR_CMM -14   /* error in communication    */

#define AF_YES 1         /* yes                       */
#define AF_NO  0         /* no                        */

/* Mathematics: */
#define FEM_PI    3.141592653589793238462643383279502884
#define FEM_ZERO  1e-7

/* Thread variables: */

#ifdef _USE_THREADS_
#define AF_MAX_THREADS 4    /* max. number of threads! */

extern long femUseThreads ; /* if threads are used  */
extern long femThreadNum  ; /* number of threads    */
extern long femThreadMin  ; /* number of threads    */
#endif


/* Way to L10N in future: */
#ifndef _USE_L10N_
#define _(str) str
#endif

/* To avoid problems (RUN_VERBOSE must be defined for any output): */
#ifdef DEVEL_VERBOSE 
#ifndef RUN_VERBOSE
#define RUN_VERBOSE
#endif
#endif

#endif

/* end of fem.h */
