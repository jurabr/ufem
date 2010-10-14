/*
   File name: fem_mem.h
   Date:      2003/04/07 10:16
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

  $Id: fem_mem.h,v 1.5 2004/07/06 21:03:44 jirka Exp $
*/


#ifndef __FEM_MEM_H__
#define __FEM_MEM_H__

#include "fem.h"

/* Standard output */
#ifdef RUN_VERBOSE
extern FILE *msgout;
#endif

extern long *femIntAlloc(long length);
extern int femIntFree(long *field);
extern double *femDblAlloc(long length);
extern int femDblFree(double *field);

#endif

/* end of fem_mem.h */
