/*
   File name: fdb_geom.h
   Date:      2004/02/11 13:21
   Author:    Jiri Brozovsky

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

	 $Id: fdb_geom.h,v 1.3 2004/11/11 21:41:06 jirka Exp $
*/


#ifndef __FDB_GEOM_H__
#define __FDB_GEOM_H__

#include "fdb_fem.h"

typedef struct
{
	long   type   ; /* type of the entity    */
	long   kps    ; /* number of keypoints   */
	long   divs   ; /* number of different divisions */
	long   lines  ; /* number of "lines"     */
	long  *divtp  ; /* line disision type [>=1] */
  long  *divk1  ; /* 1st keypoint of line  */
  long  *divk2  ; /* 2nd keypoint of line  */
	long   etypes ; /* lenght of "etype"     */
	long  *etype  ; /* allowed element types */
} tGeomEnt;

extern long     fdbGeomEntLen ;
extern tGeomEnt fdbGeomEnt[] ;

extern int fdbGeomEntInit(void);

extern int fdbTestCompEtype(long ent_type, long e_type);

extern int fdbGeomCreateMesh(void);

#endif

/* end of fdb_geom.h */
