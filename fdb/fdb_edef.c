/*
   File name: fdb_edef.c
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

	 $Id: fdb_edef.c,v 1.19 2005/07/11 17:57:02 jirka Exp $
*/

#include "fdb_edef.h"
#include "fdb_res.h"

#define FDB_ET_NUM 21 /* number of defined element types */
#define FDB_EL_NUM  5 /* number of defined load types */

tET  fdbElementType[FDB_ET_NUM] ;
long fdbElementTypeLen = FDB_ET_NUM ;

tELo fdbElementLoadType[FDB_EL_NUM] ;
long fdbElementLoadTypeLen = FDB_EL_NUM ;


/* results on nodes (empty): */
double node_res_000(long res_pos, long etype, long npos, long type)
{
  return(0.0) ;
}

/* results on link: */
double node_res_001(long res_pos, long etype, long npos, long type)
{
	return( fdbResElemGetVal(res_pos, etype, type, 0) ) ;
}


/* results on rectangle: */
double node_res_002_005(long res_pos, long etype, long npos, long type)
{
  long pos = 0 ;

  switch(npos)
  {
    case 0: pos = 1 ; break ;
    case 1: pos = 3 ; break ;
    case 2: pos = 4 ; break ;
    case 3: pos = 3 ; break ;
		default: return(0.0); break;
  }
	return( fdbResElemGetVal(res_pos, etype, type, pos) ) ;
}

/* results on link: */
double node_res_004(long res_pos, long etype, long npos, long type)
{
	return( fdbResElemGetVal(res_pos, etype, type, 0) ) ;
}

/* results on link: */
double node_res_006(long res_pos, long etype, long npos, long type)
{
	return( fdbResElemGetVal(res_pos, etype, type, 0) ) ;
}

/* results on link: */
double node_res_007(long res_pos, long etype, long npos, long type)
{
	return( fdbResElemGetVal(res_pos, etype, type, 0) ) ;
}

/* results on brick with 20 nodes : */
double node_res_008(long res_pos, long etype, long npos, long type)
{
  long pos = 0 ;

  switch(npos)
  {
    case 0: pos =  0 ; break ;
    case 1: pos =  9 ; break ;
    case 2: pos =  18 ; break ;
    case 3: pos =  21 ; break ;
    case 4: pos =  24 ; break ;
    case 5: pos =  15 ; break ;
    case 6: pos =  6 ; break ;
    case 7: pos =  3 ; break ;
    case 8: pos =  1 ; break ;
    case 9: pos =  19 ; break ;
    case 10: pos = 25 ; break ;
    case 11: pos = 7 ; break ;
    case 12: pos = 2 ; break ;
    case 13: pos = 11 ; break ;
    case 14: pos = 20 ; break ;
    case 15: pos = 23 ; break ;
    case 16: pos = 26 ; break ;
    case 17: pos = 17 ; break ;
    case 18: pos = 8 ; break ;
    case 19: pos = 5 ; break ;
		default: return(0.0); break;
  }
	return( fdbResElemGetVal(res_pos, etype, type, pos) ) ;
}

/* results on brick with 8 nodes : */
double node_res_009(long res_pos, long etype, long npos, long type)
{
  long pos = 0 ;

  switch(npos)
  {
    case 0: pos = 0 ; break ;
    case 1: pos = 18 ; break ;
    case 2: pos = 24 ; break ;
    case 3: pos = 6 ; break ;
    case 4: pos = 2 ; break ;
    case 5: pos = 20 ; break ;
    case 6: pos = 26 ; break ;
    case 7: pos = 8 ; break ;
		default: return(0.0); break;
  }
	return( fdbResElemGetVal(res_pos, etype, type, pos) ) ;
}


/* results on beam (with no intermediate points!!!): */
double node_res_003(long res_pos, long etype, long npos, long type)
{
	return( fdbResElemGetVal(res_pos, etype, type, npos) ) ;
}


/* Empty - default element */
void fdbElemTypeInit_000(long type)
{
	fdbElementType[type].id         = 0 ;
	fdbElementType[type].dim        = 0 ;
	fdbElementType[type].nodes      = 0 ;
	fdbElementType[type].ndofs      = 0 ;
	fdbElementType[type].ndof       = NULL ;
	fdbElementType[type].reals      = 0 ;
	fdbElementType[type].r          = NULL ;
	fdbElementType[type].reals_rep  = 0 ;
	fdbElementType[type].r_rep      = NULL ;
	fdbElementType[type].gtype      = 0 ;
	fdbElementType[type].getype     = 0 ;
	fdbElementType[type].eloads     = 0 ;
	fdbElementType[type].eload      = NULL ;

	fdbElementType[type].res        = 0 ;
	fdbElementType[type].nres       = NULL ;
	fdbElementType[type].res_rp     = 0 ;
	fdbElementType[type].nres_rp    = NULL ;

	fdbElementType[type].node_res   = node_res_000 ;

	fdbElementType[type].ellist_len = 0 ;
	fdbElementType[type].ellist     = NULL ;
}

/* 2D structural link */
void fdbElemTypeInit_001(long type)
{
	static long id            = 1 ;
	static long dim           = 1 ;
	static long nodes         = 2 ;
	static long ndofs         = 2 ;
	static long ndof[2]       = {U_X,U_Y} ;
	static long reals         = 4 ;
	static long r[4]          = {RS_AREA, RS_HEIGHT, RS_INERTIA_Y, RS_EXENTR_Y} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 1 ;
	static long getype        = 1 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 4 ;
	static long nres[4]       = {RES_FX,RES_SX,RES_EX,RES_ECR1} ;
	static long res_rp        = 0 ;
	static long *nres_rp      = NULL ;
  static long ellist_len    = 0 ;
  static long *ellist       = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_001 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist     = ellist ;
}

void fdbElemTypeInit_002(long type)
{
	static long id            = 2 ;
	static long dim           = 2 ;
	static long nodes         = 4 ;
	static long ndofs         = 2 ;
	static long ndof[2]       = {U_X,U_Y} ;
	static long reals         = 5 ;
	static long r[5]          = {RS_HEIGHT,RS_ROT1,RS_RFC1,RS_ROT2,RS_RFC2} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 2 ;
	static long getype        = 2 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res      = 0 ;
	static long *nres    = NULL ;
	static long res_rp   = 19 ;
	static long nres_rp[19] = {
		RES_SX, RES_SY, RES_SXY, 
		RES_EX, RES_EY, RES_EXY, 
    RES_USX, RES_USY, RES_USXY,
		RES_STMAX, RES_SCMAX, RES_DIR1, RES_DIR2, RES_DIR3, RES_ECR1, RES_ECR2, RES_STAT1, RES_STAT2, RES_GCR1} ;
  static long ellist_len    = 2 ;
  static long ellist[2]     = {1,2} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_002_005 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

void fdbElemTypeInit_003(long type)  /* beam */
{
	static long id            = 3 ;
	static long dim           = 1 ;
	static long nodes         = 2 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X,U_Y,ROT_Z} ;
	static long reals         = 4 ;
	static long r[4]          = {RS_AREA, RS_INERTIA_Y, RS_STF1,RS_STF2} ;
	static long reals_rep     = 7 ;
	static long r_rep[7]      = {RS_AREA, RS_INERTIA_Y, RS_HEIGHT, RS_STF1, RS_STF2, RS_MTYPE, RS_LNUM} ;
	static long gtype         = 1 ;
	static long getype        = 1 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res      = 3 ;
	static long nres[3]    = {RES_FX,RES_FY,RES_MZ};
	static long res_rp   = 3 ;
	static long nres_rp[3] = {RES_FX,RES_FY,RES_MZ};
  static long ellist_len    = 1 ;
  static long ellist[1]     = {1} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_003 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

void fdbElemTypeInit_004(long type) /* tetrahedron */
{
	static long id            = 4 ;
	static long dim           = 3 ;
	static long nodes         = 4 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X,U_Y,U_Z} ;
	static long reals         = 0 ;
	static long *r            = NULL ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 3 ;
	static long getype        = 3 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res      = 14 ;
	static long nres[14]    = {
		RES_SX,RES_SY,RES_SZ,RES_SYZ,RES_SZX,RES_SXY,
		RES_EX,RES_EY,RES_EZ,RES_EYZ,RES_EZX,RES_EXY,
    RES_CR1,RES_PSI
	};
	static long res_rp   = 0 ;
	static long *nres_rp = NULL;
  static long ellist_len    = 0 ;
  static long *ellist     = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_004 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

void fdbElemTypeInit_005(long type) /* slab */
{
	static long id            = 5 ;
	static long dim           = 2 ;
	static long nodes         = 4 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_Z,ROT_Y,ROT_X} ;
	static long reals         = 2 ;
	static long r[2]          = {RS_HEIGHT,RS_WINKLER} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 2 ;
	static long getype        = 2 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res      = 0 ;
	static long *nres    = NULL ;
	static long res_rp   = 5 ;
	static long nres_rp[5] = { RES_S_MX, RES_S_MY, RES_S_MXY, RES_S_VXZ, RES_S_VYZ} ;
  static long ellist_len    = 1 ;
  static long ellist[1]     = {2} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_002_005 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 2D structural link */
void fdbElemTypeInit_006(long type)
{
	static long id            = 6 ;
	static long dim           = 1 ;
	static long nodes         = 2 ;
	static long ndofs         = 2 ;
	static long ndof[2]       = {U_X,U_Y} ;
	static long reals         = 3 ;
	static long r[3]          = {RS_AREA,RS_INERTIA_Y,RS_INERTIA_Z} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 1 ;
	static long getype        = 1 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 3 ;
	static long nres[3]       = {RES_FX,RES_SX,RES_EX} ;
	static long res_rp        = 0 ;
	static long *nres_rp      = NULL ;
  static long ellist_len    = 0 ;
  static long *ellist     = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_001 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 3D structural link */
void fdbElemTypeInit_007(long type)
{
	static long id            = 7 ;
	static long dim           = 1 ;
	static long nodes         = 2 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X,U_Y,U_Z} ;
	static long reals         = 7 ;
	static long r[7]          = {RS_AREA,RS_INERTIA_Y,RS_INERTIA_Z, RS_HEIGHT, RS_WIDTH, RS_EXENTR_Y, RS_EXENTR_Z};
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 1 ;
	static long getype        = 1 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 3 ;
	static long nres[3]       = {RES_FX,RES_SX,RES_EX} ;
	static long res_rp        = 0 ;
	static long *nres_rp      = NULL ;
  static long ellist_len    = 0 ;
  static long *ellist     = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_001 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 3D brick - 20 nodes */
void fdbElemTypeInit_008(long type)
{
	static long id            = 8 ;
	static long dim           = 3 ;
	static long nodes         = 20 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X,U_Y,U_Z} ;
	static long reals         = 0 ;
	static long *r            = NULL ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 4 ;
	static long getype        = 3 ; /*1D 2D 3D */
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 0 ;
	static long *nres         = NULL ;
	static long res_rp        = 14 ;
	static long nres_rp[14]   = {RES_SX,RES_SY,RES_SZ,RES_SYZ,RES_SZX,RES_SXY, RES_EX,RES_EY,RES_EZ,RES_EYZ,RES_EZX,RES_EXY,RES_CR1,RES_PSI};
  static long ellist_len    = 0 ;
  static long *ellist     = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_008 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 3D brick - 8 nodes */
void fdbElemTypeInit_009(long type)
{
	static long id            = 9 ;
	static long dim           = 3 ;
	static long nodes         = 8 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X,U_Y,U_Z} ;
	static long reals         = 0 ;
	static long *r            = NULL ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 5 ;
	static long getype        = 3 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 0 ;
	static long *nres         = NULL ;
	static long res_rp        = 14 ;
	static long nres_rp[14]   = {RES_SX,RES_SY,RES_SZ,RES_SYZ,RES_SZX,RES_SXY, RES_EX,RES_EY,RES_EZ,RES_EYZ,RES_EZX,RES_EXY,RES_CR1,RES_PSI};
  static long ellist_len    = 2 ;
  static long ellist[2]     = {3,4} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_009 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 3D brick - 8 nodes */
void fdbElemTypeInit_010(long type)
{
	static long id            = 10 ;
	static long dim           = 3 ;
	static long nodes         = 8 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X,U_Y,U_Z} ;
	static long reals         = 1 ;
	static long r[1]          = {RS_WINKLER} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 5 ;
	static long getype        = 3 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 0 ;
	static long *nres         = NULL ;
	static long res_rp        = 26 ;
	static long nres_rp[26]   = {
    RES_SX,RES_SY,RES_SZ,RES_SYZ,RES_SZX,RES_SXY, 
    RES_EX,RES_EY,RES_EZ,RES_EYZ,RES_EZX,RES_EXY,
    RES_CR1,RES_PSI,
    RES_ECR1, RES_ECR2, RES_GCR1,
    RES_P_M11,RES_P_M12,RES_P_M13,
    RES_P_M21,RES_P_M22,RES_P_M23,
    RES_P_M31,RES_P_M32,RES_P_M33};
  static long ellist_len    = 2 ;
  static long ellist[2]     = {3,4} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_009 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 2D triangle - 3 nodes */
void fdbElemTypeInit_011(long type)
{
	static long id            = 11 ;
	static long dim           = 2 ;
	static long nodes         = 3 ;
	static long ndofs         = 2 ;
	static long ndof[2]       = {U_X,U_Y} ;
	static long reals         = 1 ;
	static long r[1]          = {RS_HEIGHT} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 6 ;
	static long getype        = 2 ; /* 2D */
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 7 ;
	static long nres[7]       = { RES_SX, RES_SY, RES_SXY, RES_EX, RES_EY, RES_EXY, RES_STAT1} ;
	static long res_rp        = 0 ;
	static long *nres_rp      = NULL ;
  static long ellist_len    = 0 ;
  static long *ellist       = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_001 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist     = ellist ;
}

/* 3D brick - 20 nodes */
void fdbElemTypeInit_012(long type)
{
	static long id            = 12 ;
	static long dim           = 3 ;
	static long nodes         = 20 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X,U_Y,U_Z} ;
	static long reals         = 0 ;
	static long *r            = NULL ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 5 ;
	static long getype        = 3 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 0 ;
	static long *nres         = NULL ;
	static long res_rp        = 26 ;
	static long nres_rp[26]   = {
    RES_SX,RES_SY,RES_SZ,RES_SYZ,RES_SZX,RES_SXY, 
    RES_EX,RES_EY,RES_EZ,RES_EYZ,RES_EZX,RES_EXY,
    RES_CR1,RES_PSI,
    RES_ECR1, RES_ECR2, RES_GCR1,
    RES_P_M11,RES_P_M12,RES_P_M13,
    RES_P_M21,RES_P_M22,RES_P_M23,
    RES_P_M31,RES_P_M32,RES_P_M33};
  static long ellist_len    = 0 ;
  static long *ellist     = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_008 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 2D beam - special */
void fdbElemTypeInit_013(long type)
{
	static long id            = 13 ;
	static long dim           = 1 ;
	static long nodes         = 2 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X,U_Y,ROT_Z} ;
	static long reals         = 4 ;
	static long r[4]          =  {RS_AREA, RS_INERTIA_Y, RS_STF1,RS_STF2} ;
	static long reals_rep     = 3 ;
	static long r_rep[3]      =  {RS_M_CRV, RS_N_CRV, RS_EI_CRV} ; /* EI curve data */
	static long gtype         = 1 ;
	static long getype        = 1 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 4 ;
	static long nres[4]       = {RES_FX,RES_FY,RES_MZ,RES_EI} ;
	static long res_rp        = 4 ;
	static long nres_rp[4]    = {RES_FX,RES_FY,RES_MZ,RES_EI} ;
  static long ellist_len    = 1 ;
  static long ellist[1]     = {1} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_003 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 3D beam */
void fdbElemTypeInit_014(long type)
{
	static long id            = 14 ;
	static long dim           = 1 ;
	static long nodes         = 2 ;
	static long ndofs         = 6 ;
	static long ndof[6]       = {U_X,U_Y,U_Z,ROT_X,ROT_Y,ROT_Z} ;
	static long reals         = 4 ;
	static long r[4]         =  {
              RS_AREA, RS_INERTIA_X, RS_INERTIA_Y, RS_INERTIA_Z } ;
	static long reals_rep     = 0 ;
	static long *r_rep        =  NULL ;
	static long gtype         = 1 ;
	static long getype        = 1 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 6 ;
	static long nres[6]       = {RES_FX,RES_FY,RES_FZ,RES_MX,RES_MY,RES_MZ};
	static long res_rp        = 6 ;
	static long nres_rp[6]    = {RES_FX,RES_FY,RES_FZ,RES_MX,RES_MY,RES_MZ};
  static long ellist_len    = 1 ;
  static long ellist[1]     = {1} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_003 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 2D beam with layers */
void fdbElemTypeInit_016(long type)
{
	static long id            = 16 ;
	static long dim           = 1 ;
	static long nodes         = 2 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_X, U_Y, ROT_Z} ;
	static long reals         = 2 ;
	static long r[2]          = {RS_STF1, RS_STF2} ;
	static long reals_rep     = 4 ;
	static long r_rep[4]      = { RS_WIDTH, RS_HEIGHT, RS_MTYPE, RS_LNUM };
	static long gtype         = 1 ;
	static long getype        = 1 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 3 ;
	static long nres[3]       = {RES_FX,RES_FY,RES_MZ};
	static long res_rp        = 4 ;
	static long nres_rp[4]    = { RES_EX,RES_SX,RES_ECR1,RES_ECR2 };
  static long ellist_len    = 1 ;
  static long ellist[1]     = {1} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_003 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

/* 2D grille beam */
void fdbElemTypeInit_017(long type)
{
	static long id            = 17 ;
	static long dim           = 1 ;
	static long nodes         = 2 ;
	static long ndofs         = 3 ;
	static long ndof[3]       = {U_Z,ROT_X,ROT_Y} ;
	static long reals         = 5 ;
	static long r[5]          = {RS_AREA, RS_INERTIA_X, RS_INERTIA_Y, RS_WINKLER, RS_WIDTH} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 1 ;
	static long getype        = 1 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 3 ;
	static long nres[3]       =  {RES_FZ,RES_MX,RES_MY};
	static long res_rp        = 3 ;
	static long nres_rp[3]    =  {RES_FZ,RES_MX,RES_MY};
  static long ellist_len    = 1 ;
  static long ellist[1]     = {1} ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_003 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}


/* 2D triangle - 3 nodes, thermal */
void fdbElemTypeInit_018(long type)
{
	static long id            = 18 ;
	static long dim           = 2 ;
	static long nodes         = 3 ;
	static long ndofs         = 1 ;
	static long ndof[1]       = {TEMP} ;
	static long reals         = 1 ;
	static long r[1]          = {RS_HEIGHT} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 6 ;
	static long getype        = 2 ; /* 2D */
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 1 ;
	static long nres[1]       = { RES_TEMP} ;
	static long res_rp        = 0 ;
	static long *nres_rp      = NULL ;
  static long ellist_len    = 0 ;
  static long *ellist       = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_001 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist     = ellist ;
}

void fdbElemTypeInit_019(long type) /* tetrahedron */
{
	static long id            = 19 ;
	static long dim           = 3 ;
	static long nodes         = 4 ;
	static long ndofs         = 1 ;
	static long ndof[1]       = {TEMP} ;
	static long reals         = 0 ;
	static long *r            = NULL ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 3 ;
	static long getype        = 3 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res      = 1 ;
	static long nres[1]  = { RES_TEMP };
	static long res_rp   = 0 ;
	static long *nres_rp = NULL;
  static long ellist_len    = 0 ;
  static long *ellist     = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_004 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}

void fdbElemTypeInit_020(long type) /* 2D isoparametric thermal */
{
	static long id            = 20 ;
	static long dim           = 2 ;
	static long nodes         = 4 ;
	static long ndofs         = 1 ;
	static long ndof[1]       = {TEMP} ;
	static long reals         = 1 ;
  static long r[1]          = {RS_HEIGHT} ;
	static long reals_rep     = 0 ;
	static long *r_rep        = NULL ;
	static long gtype         = 2 ;
	static long getype        = 2 ;
	static long eloads        = 0 ;
	static long *eload        = NULL ;
	static long res           = 0 ;
	static long *nres         = NULL;
	static long res_rp        = 1 ;
	static long nres_rp[1]    = { RES_TEMP };
  static long ellist_len    = 0 ;
  static long *ellist       = NULL ;

	fdbElementType[type].id         = id ;
	fdbElementType[type].dim        = dim ;
	fdbElementType[type].nodes      = nodes ;
	fdbElementType[type].ndofs      = ndofs ;
	fdbElementType[type].ndof       = ndof ;
	fdbElementType[type].reals      = reals ;
	fdbElementType[type].r          = r ;
	fdbElementType[type].reals_rep  = reals_rep ;
	fdbElementType[type].r_rep      = r_rep ;
	fdbElementType[type].gtype      = gtype ;
	fdbElementType[type].getype     = getype ;
	fdbElementType[type].eloads     = eloads ;
	fdbElementType[type].eload      = eload ;

	fdbElementType[type].res        = res ;
	fdbElementType[type].nres       = nres ;
	fdbElementType[type].res_rp     = res_rp ;
	fdbElementType[type].nres_rp    = nres_rp ;

	fdbElementType[type].node_res   = node_res_002_005 ;

	fdbElementType[type].ellist_len = ellist_len ;
	fdbElementType[type].ellist  = ellist ;
}


/** Empty load */
void fdbElemLoadTypeInit_000(long type)
{
	static long vals              = 0 ;

	fdbElementLoadType[type].type = type ;
	fdbElementLoadType[type].vals = vals ;
}

/** Line load (applicable on beams only) */
void fdbElemLoadTypeInit_001(long type)
{
	static long vals              = 3 ; 
  /* vals: direction, size, face */

	fdbElementLoadType[type].type = type ;
	fdbElementLoadType[type].vals = vals ;
}

/** Area load (2D things only) */
void fdbElemLoadTypeInit_002(long type)
{
	static long vals              = 3 ; 
  /* vals: direction, size, face */

	fdbElementLoadType[type].type = type ;
	fdbElementLoadType[type].vals = vals ;
}

/** Area load (2D/3D things) */
void fdbElemLoadTypeInit_003(long type)
{
	static long vals              = 3 ; 
  /* vals: direction, size, face */

	fdbElementLoadType[type].type = type ;
	fdbElementLoadType[type].vals = vals ;
}

/** Volume load (3D things only) */
void fdbElemLoadTypeInit_004(long type)
{
	static long vals              = 2 ; 
  /* vals: direction, size */

	fdbElementLoadType[type].type = type ;
	fdbElementLoadType[type].vals = vals ;
}


/** Definition of element types
 */
void fdbDefElemTypes(void)
{
	/* Element types: */
	fdbElemTypeInit_000(0);
	fdbElemTypeInit_001(1);
	fdbElemTypeInit_002(2);
	fdbElemTypeInit_003(3);
	fdbElemTypeInit_004(4);
	fdbElemTypeInit_005(5);
	fdbElemTypeInit_006(6);
	fdbElemTypeInit_007(7);
	fdbElemTypeInit_008(8);
	fdbElemTypeInit_009(9);
	fdbElemTypeInit_010(10);
	fdbElemTypeInit_011(11);
	fdbElemTypeInit_012(12);
	fdbElemTypeInit_013(13);
	fdbElemTypeInit_014(14);
	fdbElemTypeInit_000(15);
	fdbElemTypeInit_016(16);
	fdbElemTypeInit_017(17);
	fdbElemTypeInit_018(18);
	fdbElemTypeInit_019(19);
	fdbElemTypeInit_020(20);

	/* Element loads: */
	fdbElemLoadTypeInit_000(0);
	fdbElemLoadTypeInit_001(1);
	fdbElemLoadTypeInit_002(2);
	fdbElemLoadTypeInit_003(3);
	fdbElemLoadTypeInit_004(4);
}

/* end of fdb_edef.c */
