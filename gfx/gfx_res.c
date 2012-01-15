/*
   File name: gfx_res.c
   Date:      2003/12/23 17:42
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

	 FEM postprocessor: computation of deformad shape coordinates

	 $Id: gfx_res.c,v 1.8 2005/02/12 18:34:58 jirka Exp $
*/

#include "fem_gfx.h"
#include "fdb_res.h"
#include "fdb_edef.h"
#include "fem_gele.h"

#ifdef _USE_GFX_


/** Returns coordinate of deformed structure for plotting (if available)
 * @param node_pos index of node
 * @param node_dir  direction (NODE_X, NODE_Y or NODE_Z)
 * @return modified coordinate
 */
double gfxDefShapeCoord(long node_pos, long node_dir)
{
	double coord = 0;
	long   pos;

	switch(node_dir)
	{
		case NODE_X: pos = 0 ; break ;
		case NODE_Y: pos = 1 ; break ;
		case NODE_Z: pos = 2 ; break ;
		default: 		 fprintf(msgout,"[E] %s!\n", _("Invalid coordinate direction"));
								 return(0.0);
	}

	coord = fdbInputGetDbl(NODE, node_dir, node_pos) ;

	if ( (femUI_Mode == FEM_UI_MODE_POST)&&(ResLen > 0)&&(plotStuff.def_shape == AF_YES) )
	{
		if (ResNode[ResActStep].empty != AF_YES)
		{
			if  (ResNode[ResActStep].data_len > ((node_pos*KNOWN_DOFS) + pos) )
			{
				coord += (plotStuff.dof_mult*plotStuff.dof_mult_usr
						*
						ResNode[ResActStep].data[ResNode[ResActStep].from[node_pos] + pos]
						) ;
			}
		}
	}

	return(coord);
}

/** Computes deformed shape multiplier
 * @return status
 */
void gfxSetDefShapeMult(void)
{
	double mult = 0;
  double max  = 0 ;
  double min  = 0 ;

	if ( (femUI_Mode == FEM_UI_MODE_POST)&&(ResLen > 0)&&(plotStuff.def_shape == AF_YES) )
	{
		if (ResNode[ResActStep].empty != AF_YES)
		{
      if (fdbResMaxMinAny(&ResNode[ResActStep], &max, &min) == AF_OK)
      {
        mult = fabs(max) ;
        if (mult < fabs(min)) {mult = fabs(min);}

        if ( (mult > 0) && (fabs(plotProp.Max) > 0) ) 
        { 
          plotStuff.dof_mult = plotProp.Max / (10*mult) ;
        }
        else          
        { 
          plotStuff.dof_mult = 1.0 ; 
        }
      }
      else
      {
        plotStuff.def_shape = AF_NO ;
        plotStuff.dof_mult = 1.0 ;
      }
		}
	}
}

/** Plots results on elements
 *
 * @return status
 */
int femGfxElemResults(long res_type)
{
  int   rv = AF_OK ;
  long i ;
	double max = 0.0 ;
	double min = 0.0 ;

  long epos ;
  long etype ;
  long etype_prog ;
  long getype ;


  if (femUI_Mode != FEM_UI_MODE_POST) { return(AF_ERR); }
  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_ERR); }

	if ((rv=fdbResMaxMinElem(res_type, &max, &min)) != AF_OK)
	{
		fprintf(msgout, "[E] %s!\n", _("Invalid results - cannot be plotted"));
		return(rv);
	}

	glLoadIdentity();

	femColPalSet(&femPal, plotProp.palLen, max, min); /* setting of colors */

  for (i=0; i< ResElem[ResActStep].i_len; i++)
  {
    if ( fdbInputTestSelect(ELEM, ResElem[ResActStep].i_pos[i]) != AF_YES) 
	  { 
      continue ; 
    }
    else
    {
      /* finding type of stuff to be plotted */
      etype = fdbInputGetInt(ELEM, ELEM_TYPE, ResElem[ResActStep].i_pos[i]);

	    if (fdbInputCountInt(ETYPE, ETYPE_ID, etype, &epos) < 1)
         { return (AF_ERR_VAL); }

      etype_prog = fdbInputGetInt(ETYPE, ETYPE_TYPE, epos);

      getype = fdbElementType[etype_prog].gtype ;

      if ((getype <= 0) || (getype >= gfxEleLen))
      { 
        return(AF_ERR_VAL); 
      }

      /* plotting */
      if (femCheckHaveResult(etype_prog, res_type) == AF_YES)
      {
        /* results */
        gfxElem[getype].eres(i, etype_prog, res_type) ;
      }
      else
      {
        /* geometry - no results for this element */
        gfxElem[getype].geom(i) ;
      }
    }
  }

  return(rv);
}

/** Plots results on nodes
 *
 * @return status
 */
int femGfxNodeResults(long res_type)
{
  int   rv = AF_OK ;
  long i ;
	double max = 0.0 ;
	double min = 0.0 ;

  long epos ;
  long etype ;
  long etype_prog ;
  long getype ;
  long set_id ;

  if (femUI_Mode != FEM_UI_MODE_POST) { return(AF_OK); }
  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_ERR); }

	glLoadIdentity();

  set_id = ResElem[ResActStep].set_id ;

  if ((rv=fdbAvResPrepareData(1, &set_id, &res_type)) != AF_OK)
  { 
    return(rv) ; 
  }

  if (res_type < 0) /* displacements */
  {
    if (labs(res_type) > KNOWN_DOFS ) {return(AF_ERR_VAL);}
    fdbResMaxMinNode(labs(res_type)-1, &max, &min);
  }
  else /* Really averaged results */
  {
    fdbAvResMaxMin(0, &max, &min) ;
  }

	femColPalSet(&femPal, plotProp.palLen, max, min); /* setting of colors */

  for (i=0; i<fdbInputTabLenAll(ELEM); i++)
  {
    if ( fdbInputTestSelect(ELEM, i) != AF_YES) 
	  { 
      continue ; 
    }
    else
    {
      /* finding type of stuff to be plotted*/
      etype = fdbInputGetInt(ELEM, ELEM_TYPE, i);

	    if (fdbInputCountInt(ETYPE, ETYPE_ID, etype, &epos) < 1)
      { 
       continue ;
      }

      etype_prog = fdbInputGetInt(ETYPE, ETYPE_TYPE, epos);

      getype = fdbElementType[etype_prog].gtype ;

      if ((getype <= 0) || (getype >= gfxEleLen))
      { 
        continue ;
      }

      gfxElem[getype].nres(i, etype_prog, res_type) ;
    }
  }

  return(rv);
}


#endif
/* end of gfx_res.c */
