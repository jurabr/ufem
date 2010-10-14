/*
   File name: fem_gele.c
   Date:      2003/11/23 11:10
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

	 $Id: fem_gele.c,v 1.10 2004/11/11 21:41:20 jirka Exp $
*/

#include "fem_gele.h"
#include "fdb_fem.h"
#ifdef _USE_GFX_

#define NUMBER_OF_GFX_ELEMENTS 7

extern void femGfxEleInit000(long testtype) ;
extern void femGfxEleInit001(long testtype) ;
extern void femGfxEleInit002(long testtype) ;
extern void femGfxEleInit003(long testtype) ;
extern void femGfxEleInit004(long testtype) ;
extern void femGfxEleInit005(long testtype) ;
extern void femGfxEleInit006(long testtype) ;

extern long femElemHilightColor ;


tGfEle gfxElem[NUMBER_OF_GFX_ELEMENTS];
long   gfxEleLen = NUMBER_OF_GFX_ELEMENTS ;


/** Initialization of elements */
void femInitGfxElements(void)
{
  femGfxEleInit000(0) ;
  femGfxEleInit001(1) ;
  femGfxEleInit002(2) ;
  femGfxEleInit003(3) ;
  femGfxEleInit004(4) ;
  femGfxEleInit005(5) ;
  femGfxEleInit006(6) ;
}

int femGfxOneEleGeom(long i)
{
  long epos ;
  long etype ;
  long etype_prog ;
  long getype ;

  if ( fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
	{ 
    return(AF_OK) ; 
  }
  else
  {
    /* finding type of stuff to be plotted*/
    etype = fdbInputGetInt(ELEM, ELEM_TYPE, i);

	  if (fdbInputCountInt(ETYPE, ETYPE_ID, etype, &epos) < 1)
       { return (AF_ERR_VAL); }

    etype_prog = fdbInputGetInt(ETYPE, ETYPE_TYPE, epos);

    getype = fdbElementType[etype_prog].gtype ;

    if ((getype <= 0) || (getype >= gfxEleLen))
    { 
      return(AF_ERR_VAL); 
    }

    /* plotting */
    if (plotProp.Select == AF_YES) { glPushName((GLuint)i) ; }
    gfxElem[getype].geom(i) ;
    if (plotProp.Select == AF_YES) { glPopName() ; }
  }
  return(AF_OK);
}

/** Plots finite elements */
int femGfxEleGeom(void)
{
  int rv = AF_OK ;
  long i ;
  long to ;
  long j ;
  long ignore = AF_NO ;

	if ((to = fdbInputTabLenAll(ELEM)) > 0);
  {
    for (i=0; i<to; i++)
    {

      if ((plotProp.SelStuff == GFX_SELE_ELEMS)&&(plotProp.SelAct == GFX_SELE_ACT_SELE)&&(plotProp.Select != AF_YES))
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
        if ((rv = femGfxOneEleGeom(i)) != AF_OK) {return(rv);}
      }
    }
  }
  return(AF_OK);
}

/** Plots finite elements */
int femGfxHilightedElements(long *fld, long fldLen)
{
  long i ;

	glLoadIdentity();

  femElemHilightColor = AF_YES ;

  for (i=0; i<fldLen; i++)
  {
    femGfxOneEleGeom(fld[i]);
  }

  femElemHilightColor = AF_NO ;

  return(AF_OK);
}

#endif
/* end of fem_gele.c */
