/*
   File name: fdb_edef.h
   Date:      2003/08/19 17:07
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

	 FEM Database - element types

	 $Id: fdb_edef.h,v 1.5 2004/04/19 19:00:53 jirka Exp $
*/


#ifndef __FDB_EDEF_H__
#define __FDB_EDEF_H__

#include "fdb_fem.h"
#include "fem_elem.h"


typedef struct       /* element type definition     */
{
	long  id ;         /* element number              */
  long  dim ;        /* dimensionality (1,2,3)*/
	long  nodes ;      /* number of nodes             */
	long  ndofs ;      /* number of DOFs  in one node */
	long *ndof ;       /* list of DOFs in one node    */
	long  reals;       /* number of real constants    */
	long *r ;          /* list of real constants      */
	long  reals_rep;   /* number of real constants - repeating set */
	long *r_rep ;      /* list of real constants - repeating set   */
	long  gtype ;      /* graphics representation iof element      */
	long  getype ;     /* compatible type of geometric entity */
	long  eloads ;     /* number of possible element load types */
	long *eload ;      /* list of possible element load types */
	long  res;         /* number of results */
	long *nres;        /* list of results nres[res]   */
	long  res_rp;      /* number of REPEATING results */
	long *nres_rp;     /* list of REPEATING results nres_rp[res_rp] */
  long  ellist_len ; /* lenght of element load types list */
  long *ellist ;     /* compatible element load types */
  double (* node_res)(long, long, long, long);/* result on node: eresult_pos, etype, enode_pos, res_type */
} tET;

typedef struct       /* element load definition     */
{
	long  type ;       /* element number              */
	long  vals ;       /* number of values            */
} tELo;



extern tET  fdbElementType[] ;
extern long fdbElementTypeLen  ;

extern tELo fdbElementLoadType[] ;
extern long fdbElementLoadTypeLen  ;


extern void fdbDefElemTypes(void);

#endif

/* end of fdb_edef.h */
