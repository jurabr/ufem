/*
   File name: fem_gent.c
   Date:      Mon Apr 19 10:46:08 CEST 2004
   Author:    Jiri Brozovsky

   Copyright (C) 2004  Jiri Brozovsky

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
*/

#include "fem_gele.h"
#include "fdb_fem.h"
#ifdef _USE_GFX_


extern long femElemHilightColor ;

extern int gfx_en_001(long pos) ;
extern int gfx_en_002(long pos) ;
extern int gfx_en_003(long pos) ;
extern int gfx_en_004(long pos) ;
extern int gfx_en_005(long pos) ;


int femGfxOneEntity(long i)
{
  int rv = AF_OK ;

  if ( fdbTestSelect(&InputTab[ENTITY], &inputData.intfld, i) != AF_YES) 
	{ 
    return(AF_OK) ; 
  }
  else
  {
    if (plotProp.Select == AF_YES) { glPushName((GLuint)i) ; }

    /* plotting functions are called here: */
    switch(fdbInputGetInt(ENTITY, ENTITY_TYPE, i))
    {
      case  1: gfx_en_001(i) ; break ;
      case  2: gfx_en_002(i) ; break ;
      case  3: gfx_en_003(i) ; break ;
      case  4: gfx_en_004(i) ; break ;
      case  5: gfx_en_005(i) ; break ;
      default: rv = AF_ERR_VAL ; break ;
    }

    if (plotProp.Select == AF_YES) { glPopName() ; }
  }
  return(rv);
}

/** Plots finite elements */
int femGfxEntity(void)
{
  int rv = AF_OK ;
  long i ;
  long to ;
  long j ;
  long ignore = AF_NO ;

	if ((to = fdbInputTabLenAll(ENTITY)) > 0);
  {
    for (i=0; i<to; i++)
    {
      if ((plotProp.SelStuff == GFX_SELE_ENTS)&&(plotProp.SelAct == GFX_SELE_ACT_SELE)&&(plotProp.Select != AF_YES))
      {
        ignore = AF_NO ;

        if (femGfxSelSelectFldCount > 0)
        {
         for (j=0; j<femGfxSelSelectFldCount; j++)
         {
           if (femGfxSelSelectFld[j] == i) {ignore = AF_YES;}
         }
        }
      }

      if (ignore != AF_YES)
      {
        if ((rv = femGfxOneEntity(i)) != AF_OK) {return(rv);}
      }
    }
  }
  return(rv);
}

/** Plots finite elements */
int femGfxHilightedEntities(long *fld, long fldLen)
{
  long i ;

	glLoadIdentity();

  femElemHilightColor = AF_YES ;

  for (i=0; i<fldLen; i++)
  {
    femGfxOneEntity(fld[i]);
  }

  femElemHilightColor = AF_NO ;

  return(AF_OK);
}

#endif
/* end of fem_gent.c */
