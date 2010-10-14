/*
   File name: fem_gele.h
   Date:      2003/11/23 11:08
   Author:    Jiri Brozovsky

   Copyright (C) 2003  Jiri Brozovsky

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

	 Graphics output for "fem" (pre|post)processor - element plotting

	 $Id: fem_gele.h,v 1.5 2004/01/04 20:11:56 jirka Exp $
*/

#include "fem_gfx.h"
#include "fdb_fem.h"
#include "fdb_edef.h"
#include "fdb_res.h"
#ifdef _USE_GFX_


typedef struct
{
  long type ; /* id number of type */
  long nodes; /* number of nodes   */
  int  (* geom)(long);/* position of element */
  int  (* eres)(long, long, long);/* pos. of result., elem. type, result */
  int  (* nres)(long, long, long);/* pos. of element!, elem. type, result */
}tGfEle;


extern tGfEle gfxElem[];
extern long   gfxEleLen;


extern void femInitGfxElements(void);
extern int femGfxEleGeom(void);

#endif
/* end of fem_gele.h */
