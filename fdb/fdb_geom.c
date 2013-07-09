/*
   File name: fdb_geom.c
   Date:      2004/02/11 13:48
   Author:   Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

	 FEM Database - definitions of geometric entities
*/

#include "fdb_geom.h"

long     fdbGeomEntLen = 6 ;
tGeomEnt fdbGeomEnt[6] ;

/* empty element */
int fdbGeomEntInit000(long num)
{
	int rv = AF_OK ;
	static long  type   = 0 ;
	static long  kps    = 0 ;
	static long  divs   = 0 ;
	static long  lines  = 0 ;
  static long  *divtp = NULL ;
  static long  *divk1 = NULL ;
  static long  *divk2 = NULL ;
	static long  etypes = 0 ;
	static long *etype  = NULL ;

	if (num != type) {fprintf(msgout,"[E] %s %li!\n",
			_("Fatal error at the geometry type"),num);
			exit(AF_ERR_VAL);}

	fdbGeomEnt[num].type   = type ;
	fdbGeomEnt[num].kps    = kps ;
	fdbGeomEnt[num].divs   = divs ;
	fdbGeomEnt[num].lines  = lines ;
	fdbGeomEnt[num].divtp  = divtp ;
	fdbGeomEnt[num].divk1  = divk1 ;
	fdbGeomEnt[num].divk2  = divk2 ;
	fdbGeomEnt[num].etypes = etypes ;
	fdbGeomEnt[num].etype  = etype ;
	return(rv);
}

/* straight line */
int fdbGeomEntInit001(long num)
{
	int rv = AF_OK ;
	static long  type     = 1 ;
	static long  kps      = 2 ;
	static long  divs     = 1 ;
	static long  lines    = 1 ;
  static long  divtp[1] = {1} ;
  static long  divk1[1] = {0} ;
  static long  divk2[1] = {1} ;
	static long  etypes   = 6 ;
	static long  etype[6] = {1,3,6,7,14,17} ;

	if (num != type) {fprintf(msgout,"[E] %s %li!\n",
			_("Fatal error at the geometry type"),num);
			exit(AF_ERR_VAL);}

	fdbGeomEnt[num].type   = type ;
	fdbGeomEnt[num].kps    = kps ;
	fdbGeomEnt[num].divs   = divs ;
	fdbGeomEnt[num].lines  = lines ;
	fdbGeomEnt[num].divtp  = divtp ;
	fdbGeomEnt[num].divk1  = divk1 ;
	fdbGeomEnt[num].divk2  = divk2 ;
	fdbGeomEnt[num].etypes = etypes ;
	fdbGeomEnt[num].etype  = etype ;
	return(rv);
}

/* rectangle */
int fdbGeomEntInit002(long num)
{
	int rv = AF_OK ;
	static long  type     = 2 ;
	static long  kps      = 4 ;
	static long  divs     = 2 ;
	static long  lines    = 4 ;
  static long  divtp[4] = {1,2,1,2} ;
  static long  divk1[4] = {0,1,2,3} ;
  static long  divk2[4] = {1,2,3,0} ;
	static long  etypes   = 5 ;
	static long  etype[5] = {2,5,11,18,20} ;

	if (num != type) {fprintf(msgout,"[E] %s %li!\n",
			_("Fatal error at the geometry type"),num);
			exit(AF_ERR_VAL);}

	fdbGeomEnt[num].type   = type ;
	fdbGeomEnt[num].kps    = kps ;
	fdbGeomEnt[num].divs   = divs ;
	fdbGeomEnt[num].lines  = lines ;
	fdbGeomEnt[num].divtp  = divtp ;
	fdbGeomEnt[num].divk1  = divk1 ;
	fdbGeomEnt[num].divk2  = divk2 ;
	fdbGeomEnt[num].etypes = etypes ;
	fdbGeomEnt[num].etype  = etype ;
	return(rv);
}

/* brick */
int fdbGeomEntInit003(long num)
{
	int rv = AF_OK ;
	static long  type     = 3 ;
	static long  kps      = 8 ;
	static long  divs     = 3 ;
	static long  lines    = 12 ;
  static long  divtp[12] = {1,2,1,2,1,2,1,2,3,3,3,3} ;
  static long  divk1[12] = {0,1,2,3,4,5,6,7,0,1,2,3} ;
  static long  divk2[12] = {1,2,3,0,5,6,7,4,4,5,6,7} ;
	static long  etypes   = 2 ;
	static long  etype[2] = {9,10} ;

	if (num != type) {fprintf(msgout,"[E] %s %li!\n",
			_("Fatal error at the geometry type"),num);
			exit(AF_ERR_VAL);}

	fdbGeomEnt[num].type   = type ;
	fdbGeomEnt[num].kps    = kps ;
	fdbGeomEnt[num].divs   = divs ;
	fdbGeomEnt[num].lines  = lines ;
	fdbGeomEnt[num].divtp  = divtp ;
	fdbGeomEnt[num].divk1  = divk1 ;
	fdbGeomEnt[num].divk2  = divk2 ;
	fdbGeomEnt[num].etypes = etypes ;
	fdbGeomEnt[num].etype  = etype ;
	return(rv);
}

/* curvilinear brick */
int fdbGeomEntInit004(long num)
{
	int rv = AF_OK ;
	static long  type     = 4 ;
	static long  kps      = 20 ;
	static long  divs     = 3 ;
	static long  lines    = 12 ;
  static long  divtp[12] = {1,2,1,2, 1,2,1,2, 3, 3, 3, 3} ;
  static long  divk1[12] = {0,2,4,6, 8,10,12,14,0, 2, 4, 6} ;
  static long  divk2[12] = {2,4,6,0,10,12,14, 8,8,10,12,14} ;
	static long  etypes   = 2 ;
	static long  etype[2] = {9,10} ;

	if (num != type) {fprintf(msgout,"[E] %s %li!\n",
			_("Fatal error at the geometry type"),num);
			exit(AF_ERR_VAL);}

	fdbGeomEnt[num].type   = type ;
	fdbGeomEnt[num].kps    = kps ;
	fdbGeomEnt[num].divs   = divs ;
	fdbGeomEnt[num].lines  = lines ;
	fdbGeomEnt[num].divtp  = divtp ;
	fdbGeomEnt[num].divk1  = divk1 ;
	fdbGeomEnt[num].divk2  = divk2 ;
	fdbGeomEnt[num].etypes = etypes ;
	fdbGeomEnt[num].etype  = etype ;
	return(rv);
}

/* curvilinear rectangle */
int fdbGeomEntInit005(long num)
{
	int rv = AF_OK ;
	static long  type     = 5 ;
	static long  kps      = 8 ;
	static long  divs     = 2 ;
	static long  lines    = 4 ;
  static long  divtp[4] = {1,2,1,2} ;
  static long  divk1[4] = {0,2,4,6} ;
  static long  divk2[4] = {2,4,6,0} ;
	static long  etypes   = 3 ;
	static long  etype[3] = {2,5,20} ;

	if (num != type) {fprintf(msgout,"[E] %s %li!\n",
			_("Fatal error at the geometry type"),num);
			exit(AF_ERR_VAL);}

	fdbGeomEnt[num].type   = type ;
	fdbGeomEnt[num].kps    = kps ;
	fdbGeomEnt[num].divs   = divs ;
	fdbGeomEnt[num].lines  = lines ;
	fdbGeomEnt[num].divtp  = divtp ;
	fdbGeomEnt[num].divk1  = divk1 ;
	fdbGeomEnt[num].divk2  = divk2 ;
	fdbGeomEnt[num].etypes = etypes ;
	fdbGeomEnt[num].etype  = etype ;
	return(rv);
}


/* Initialization of the geometry */
int fdbGeomEntInit(void)
{
	fdbGeomEntInit000(0);
	fdbGeomEntInit001(1);
	fdbGeomEntInit002(2);
	fdbGeomEntInit003(3);
	fdbGeomEntInit004(4);
	fdbGeomEntInit005(5);
	return(AF_OK);
}

/** Tests if the given element type can be used on entity 
 * @param ent_type type of the entity
 * @param type of the element (NOT the user id!)
 * @return AF_YES (can be used)/ AF_NO (sorry..)
 */
int fdbTestCompEtype(long ent_type, long e_type)
{
	long i;

	if (ent_type >= fdbGeomEntLen)
	{
		fprintf(msgout,"[E] %s: %li!\n", _("Invalid type of entity requested"), ent_type);
		return(AF_NO) ;
	}

	for (i=0; i<fdbGeomEnt[ent_type].etypes; i++)
	{
		if (fdbGeomEnt[ent_type].etype[i] == e_type)
		{
			return(AF_YES) ;
		}
	}

	return(AF_NO);
}

/* end of fdb_geom.c */
