/*
   File name: gfx_e000.c
   Date:      2003/11/23 11:20
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

	 $Id: gfx_e000.c,v 1.3 2003/12/29 17:39:01 jirka Exp $
*/

#include "fem_gele.h"
#ifdef _USE_GFX_


int gfx_e000_geom(long pos)
{
  int rv = AF_OK;

  return(rv);
}

int gfx_e000_eres(long pos, long etype, long res)
{
  int rv = AF_OK;

  return(rv);
}

int gfx_e000_nres(long pos, long etype, long res)
{
  int rv = AF_OK;

  return(rv);
}

void femGfxEleInit000(long testtype)
{
  static long type  = 0 ;
  static long nodes = 0 ;

  if (testtype != type)
  {
    fprintf(msgout,"[E] %s: %li <> %li\n[E] %s!\n",
        _("FATAL ERROR - graphics element michmach "),
        type, testtype, _("EXITING"));
    exit (AF_ERR);
  }

  gfxElem[type].type  = type          ;
  gfxElem[type].nodes = nodes         ;

  gfxElem[type].geom  = gfx_e000_geom ;
  gfxElem[type].eres  = gfx_e000_eres ;
  gfxElem[type].nres  = gfx_e000_nres ;
}

#endif
/* end of gfx_e000.c */
