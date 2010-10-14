/*
   File name: fdb_mdef.h
   Date:      2003/08/20 09:40
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.

	 FEM Database - materials

	 $Id: fdb_mdef.h,v 1.1 2003/09/14 20:43:15 jirka Exp $
*/


#ifndef __FDB_MDEF_H__
#define __FDB_MDEF_H__

#include "fdb_fem.h"
#include "fem_mat.h"


typedef struct
{
	long  id;     /* material type number                    */
	long  vals;   /* number of properties                    */
	long *val;    /* list of properties (val[num])           */
	long  vals_rp;/* number of REPEATING properties          */
	long *val_rp; /* list of REPEATING properties (val[num]) */
} tMT;



extern tMT  fdbMatType[] ;
extern long fdbMatTypeLen  ;


extern void fdbDefMatTypes(void);



#endif

/* end of fdb_mdef.h */
