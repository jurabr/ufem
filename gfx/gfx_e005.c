/*
   File name: gfx_e005.c
   Date:      2004/03/21 15:59
   Author:    

   Copyright (C) 2004 

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
	  (brick - 8 nodes)
*/

#include "fem_gele.h"
#ifdef _USE_GFX_


extern int gfx_e004_geom(long pos);
extern int gfx_e004_eres(long res_pos, long etype, long res);


/** plots results on node - not very nice, should be improved 
 * (two triangles per side loog ugly) */
int gfx_e005_nres(long pos, long etype, long res)
{
  long nodes = 0 ;
  long node_pos = 0 ;
	long nlist[36] = {0,1,2,0,2,3,4,5,6,4,6,7,0,4,7,0,7,3,1,5,6,1,6,2, 0,5,1,4,5,0,3,6,2,7,6,3};
	long nside[8] = {0,4,7,3,2,6,5,1} ;
  float r,g,b ;
  float x,y,z;
  int  i, j;
  double val ;

#if 0
  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;
#else
  nodes = 12 ; /* not a real number of nodes - it's numbe of triangles */
#endif

  if ((plotProp.wireOnly != AF_YES)||(plotProp.Select == AF_YES) )
  {

    for (i=0; i<nodes; i++)
    {
	  	glBegin(GL_POLYGON);
			for (j=0; j<3; j++)
			{
      	node_pos = fdbEnodePos(pos, nlist[i*3+j]) ;

				x = gfxDefShapeCoord(node_pos, NODE_X) ;
				y = gfxDefShapeCoord(node_pos, NODE_Y) ;
				z = gfxDefShapeCoord(node_pos, NODE_Z) ;

      	x = PoSX(x) ;
      	y = PoSY(y) ;
      	z = PoSZ(z) ;

      	val = fdbAvResGetVal(res, node_pos);
	    	femGetResColor3f(val, &r, &g, &b);
	    	glColor4f(r,g,b,1.0);

		  	glVertex3f ( x, y, z ) ;
			}
	  	glEnd();
    }

  }

  if (plotProp.Select != AF_YES)
  {
    /* wireframe: */

	  glColor4f(0.0, 0.0, 0.0, 1.0);

	  glBegin(GL_LINE_LOOP);
    for (i=0; i<4; i++)
    {
      node_pos = fdbEnodePos(pos, i) ;

			x = gfxDefShapeCoord(node_pos, NODE_X) ;
			y = gfxDefShapeCoord(node_pos, NODE_Y) ;
			z = gfxDefShapeCoord(node_pos, NODE_Z) ;

      x = PoSX(x) ;
      y = PoSY(y) ;
      z = PoSZ(z) ;

		  glVertex3f ( x, y, z ) ;
    }
	  glEnd();

		glBegin(GL_LINE_LOOP);
    for (i=4; i<8; i++)
    {
      node_pos = fdbEnodePos(pos, i) ;

			x = gfxDefShapeCoord(node_pos, NODE_X) ;
			y = gfxDefShapeCoord(node_pos, NODE_Y) ;
			z = gfxDefShapeCoord(node_pos, NODE_Z) ;

      x = PoSX(x) ;
      y = PoSY(y) ;
      z = PoSZ(z) ;

		  glVertex3f ( x, y, z ) ;
    }
	  glEnd();

		glBegin(GL_LINES);
    for (i=0; i<8; i++)
    {
      node_pos = fdbEnodePos(pos, nside[i]) ;

			x = gfxDefShapeCoord(node_pos, NODE_X) ;
			y = gfxDefShapeCoord(node_pos, NODE_Y) ;
			z = gfxDefShapeCoord(node_pos, NODE_Z) ;

      x = PoSX(x) ;
      y = PoSY(y) ;
      z = PoSZ(z) ;

		  glVertex3f ( x, y, z ) ;
    }
	  glEnd();


  }

	return(AF_OK);
}

void femGfxEleInit005(long testtype)
{
  static long type  = 5 ;
  static long nodes = 8 ;

  if (testtype != type)
  {
    fprintf(msgout,"[E] %s: %li <> %li\n[E] %s!\n",
        _("FATAL ERROR - graphics element michmach "),
        type, testtype, _("EXITING"));
    exit (AF_ERR);
  }

  gfxElem[type].type  = type          ;
  gfxElem[type].nodes = nodes         ;

  gfxElem[type].geom  = gfx_e004_geom ;
  gfxElem[type].eres  = gfx_e004_eres ;
  gfxElem[type].nres  = gfx_e005_nres ;
}


#endif
/* end of gfx_e005.c */
