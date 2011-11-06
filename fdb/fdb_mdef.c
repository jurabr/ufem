/*
   File name: fdb_mdef.c
   Date:      2003/08/20 09:41
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

	 $Id: fdb_mdef.c,v 1.7 2004/11/11 21:41:06 jirka Exp $
*/

#include "fdb_mdef.h"


tMT  fdbMatType[14] ;
long fdbMatTypeLen = 14 ;


/* empty - default material */
void fdbMatTypeInit_000(long type)
{
	fdbMatType[type].id         = 0 ;
	fdbMatType[type].vals       = 0 ;
	fdbMatType[type].val        = NULL ;
	fdbMatType[type].vals_rp    = 0 ;
	fdbMatType[type].val_rp     = NULL ;
}

/* linear material (Hooke) */
void fdbMatTypeInit_001(long type)
{
	static long id            = 1 ;
	static long vals          = 6 ;
	static long val[6]        = {MAT_DENS, MAT_EX, MAT_NU, MAT_ALPHA, MAT_KXX, MAT_PRICE};
	static long vals_rp       = 5 ;
	static long val_rp[5]     = {MAT_DENS, MAT_EX, MAT_NU, MAT_ALPHA, MAT_KXX};

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

/* quassibrittle concrete */
void fdbMatTypeInit_002(long type)
{
	static long id            = 2 ;
	static long vals          = 9 ;
	static long val[9]        = {MAT_DENS, MAT_EX, MAT_NU, MAT_G, MAT_STEN, MAT_SCMPR, MAT_GF, MAT_E1, MAT_STEN1};
	static long vals_rp       = 9 ;
	static long val_rp[9]     = {MAT_DENS, MAT_EX, MAT_NU, MAT_G, MAT_STEN, MAT_SCMPR, MAT_GF, MAT_E1, MAT_STEN1};

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

/* plastic concrete */
void fdbMatTypeInit_003(long type)
{
	static long id            = 3 ;
	static long vals          = 11 ;
	static long val[11]        = {MAT_DENS, MAT_EX, MAT_NU,
  MAT_F_YC,MAT_F_YBC,MAT_F_YT,MAT_F_UC,MAT_F_UBC,MAT_F_UT,MAT_E1, MAT_PRICE} ;
	static long vals_rp       = 2 ;
	static long val_rp[2]     = {MAT_RAMB_K, MAT_RAMB_N} ;

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

/* drucker-prager */
void fdbMatTypeInit_004(long type)
{
	static long  id        = 4 ;
	static long  vals      = 11 ;
	static long  val[11]    = {MAT_DENS, MAT_EX, MAT_NU, MAT_G, MAT_COHES, MAT_FRICT, MAT_F_YC, MAT_HARD, MAT_RAMB_K, MAT_RAMB_N, MAT_PRICE};
	static long  vals_rp   = 6 ;
	static long  val_rp[6] = {MAT_DENS, MAT_EX, MAT_NU, MAT_G, MAT_F_YC, MAT_HARD};

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

/* linear for with stiffnesses */
void fdbMatTypeInit_005(long type)
{
	static long  id        = 5 ;
	static long  vals      = 8 ;
	static long  val[8]    = {MAT_DENS, MAT_EX, MAT_NU, MAT_G,MAT_F_YC,MAT_F_YT,MAT_F_UC,MAT_F_UT};
	static long  vals_rp   = 8 ;
	static long  val_rp[8] = {MAT_DENS, MAT_EX, MAT_NU, MAT_G,MAT_F_YC,MAT_F_YT,MAT_F_UC,MAT_F_UT};

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}


void fdbMatTypeInit_006(long type)
{
	static long  id        = 6 ;
	static long  vals      = 8 ;
	static long  val[8]    = {MAT_DENS, MAT_EX, MAT_NU, MAT_G, MAT_E1, 
  MAT_F_UC,MAT_F_UBC,MAT_F_UT};
	static long  vals_rp   = 0 ;
	static long  *val_rp   = NULL ;

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

void fdbMatTypeInit_007(long type)
{
	static long  id        = 7 ;
	static long  vals      = 10 ;
	static long  val[10]   = {MAT_DENS, MAT_EX,MAT_EY,MAT_EZ, MAT_NUXY,MAT_NUYZ,MAT_NUZX, MAT_GXY,MAT_GYZ,MAT_GZX};
	static long  vals_rp   = 0 ;
	static long  *val_rp   = NULL ;

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

void fdbMatTypeInit_008(long type)
{
	static long  id        = 8 ;
	static long  vals      = 12 ;
	static long  val[12]    = {MAT_DENS, MAT_EX, MAT_NU,
    MAT_STEN, MAT_SCMPR, MAT_GF, MAT_E1, MAT_STEN1, MAT_SELE, MAT_SHTYPE, MAT_FCTYPE, MAT_CRTYPE};
	static long  vals_rp   = 1 ;
	static long  val_rp[1] =  {MAT_WX};

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

void fdbMatTypeInit_009(long type)
{
	static long  id        = 9 ;
	static long  vals      = 8 ;
	static long  val[8]    = {MAT_DENS, MAT_EX, MAT_NU,
    MAT_STEN, MAT_SCMPR, MAT_GF, MAT_E1, MAT_RAD};
	static long  vals_rp   = 0 ;
	static long  *val_rp   = NULL ;

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

void fdbMatTypeInit_010(long type)
{
	static long  id        = 10 ;
	static long  vals      = 6 ;
	static long  val[8]    = {MAT_DENS, MAT_EX, MAT_NU,
    MAT_STEN, MAT_SCMPR, MAT_E1};
	static long  vals_rp   = 0 ;
	static long  *val_rp   = NULL ;

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

/* plastic concrete + quassibrittle */
void fdbMatTypeInit_011(long type)
{
	static long id            = 11 ;
	static long vals          = 11 ;
	static long val[11]        = {MAT_DENS, MAT_EX, MAT_NU,
  MAT_F_YC,MAT_F_YBC,MAT_F_YT,MAT_F_UC,MAT_F_UBC,MAT_F_UT,MAT_E1, MAT_GF} ;
	static long vals_rp       = 2 ;
	static long val_rp[2]     = {MAT_RAMB_K, MAT_RAMB_N} ;

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

/* Lourenco's masonry */
void fdbMatTypeInit_012(long type)
{
	static long id            = 12 ;
	static long vals          = 1 ;
	static long val[1]        = {MAT_DENS};
	static long vals_rp       = 6 ;
	static long val_rp[6]     = {MAT_EX,MAT_NU, MAT_F_UC,MAT_F_UT,MAT_F_UBC, MAT_GF};

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}

/* Simplified Lourenco's masonry (equivalen for 2D) */
void fdbMatTypeInit_013(long type)
{
	static long id            = 13 ;
	static long vals          = 1 ;
	static long val[1]        = {MAT_DENS};
	static long vals_rp       = 7 ;
	static long val_rp[7]     = {MAT_EX,MAT_NU, MAT_F_UC,MAT_F_UT,MAT_F_UBC, MAT_GF, MAT_GFC};

	fdbMatType[type].id         = id ;
	fdbMatType[type].vals       = vals ;
	fdbMatType[type].val        = val ;
	fdbMatType[type].vals_rp    = vals_rp ;
	fdbMatType[type].val_rp     = val_rp ;
}




/** Definition of materials
 */
void fdbDefMatTypes(void)
{
	fdbMatTypeInit_000(0);
	fdbMatTypeInit_001(1);
	fdbMatTypeInit_002(2);
	fdbMatTypeInit_003(3);
	fdbMatTypeInit_004(4);
	fdbMatTypeInit_005(5);
	fdbMatTypeInit_006(6);
	fdbMatTypeInit_007(7);
	fdbMatTypeInit_008(8);
	fdbMatTypeInit_009(9);
	fdbMatTypeInit_010(10);
	fdbMatTypeInit_011(11);
	fdbMatTypeInit_012(12);
	fdbMatTypeInit_013(13);
}


/* end of fdb_mdef.c */
