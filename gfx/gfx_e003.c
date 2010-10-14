/*
   File name: gfx_e003.c
   Date:      2003/11/29 16:26
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
	  (tetrahedron)

	 $Id: gfx_e003.c,v 1.8 2004/01/05 22:31:28 jirka Exp $
*/

#include "fem_gele.h"
#ifdef _USE_GFX_


int gfx_e003_geom(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  float sx,sy,sz;
  long elem_num ;
  int  i;
	float node[4][3] ;

  sx = 0.0 ;
  sy = 0.0 ;
  sz = 0.0 ;

	glLineWidth(2);
#ifdef USE_GL2PS
	gl2psLineWidth(2);
#endif

  femElemColor(pos, &r, &g, &b);
	glColor4f(r,g,b,1.0);

  elem_num = fdbInputGetInt(ELEM, ELEM_ID, pos) ;
  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

	for (i=0; i<nodes; i++)
  {
    node_pos = fdbEnodePos(pos, i) ;

#if 0
     x = (float) fdbInputGetDbl(NODE,  NODE_X, node_pos) ;
     y = (float) fdbInputGetDbl(NODE,  NODE_Y, node_pos) ;
     z = (float) fdbInputGetDbl(NODE,  NODE_Z, node_pos) ;
#else
			x = gfxDefShapeCoord(node_pos, NODE_X) ;
			y = gfxDefShapeCoord(node_pos, NODE_Y) ;
			z = gfxDefShapeCoord(node_pos, NODE_Z) ;
#endif

	/* some manipulation with coordinates due to results will be required later*/

     x = PoSX(x) ; sx += x ;
     y = PoSY(y) ; sy += y ;
     z = PoSZ(z) ; sz += z ;

		 node[i][0] = x ;
		 node[i][1] = y ;
		 node[i][2] = z ;
  }

  if ((plotProp.wireOnly != AF_YES)||(plotProp.Select == AF_YES))
  {
	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();
  }

	glLineWidth(1);
#ifdef USE_GL2PS
	gl2psLineWidth(1);
#endif

  if (plotProp.Select != AF_YES)
  {
    /* wireframe: */

    if (plotProp.wireOnly != AF_YES)
    {
	    glColor4f(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
      glColor4f(r,g,b,1.0);
    }

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();


    sx = (sx/nodes);
	  sy = (sy/nodes);
	  sz = (sz/nodes);

	  if (plotProp.elemNumber == AF_YES) 
    { 
	    glColor4f(plotProp.fgColor[0],
                plotProp.fgColor[1],
                plotProp.fgColor[2],
                plotProp.fgColor[3]);

      femPlotInt(elem_num, sx, sy, sz, NULL); 
    }
  }

  return(rv);
}

int gfx_e003_eres(long res_pos, long etype, long res)
{
  int rv = AF_OK;
	long pos ;
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  long elem_num ;
  int  i;
	float node[4][3] ;
	double val = 0.0 ;

	glLineWidth(2);
#ifdef USE_GL2PS
	gl2psLineWidth(2);
#endif


	/* result color computation */
	val = fdbResElemGetVal(res_pos, etype, res, 0) ;
	femGetResColor3f(val, &r, &g, &b);

	/* element location */
  pos = fdbResEPos(res_pos) ;
	
	glColor4f(r,g,b,1.0);

  elem_num = fdbInputGetInt(ELEM, ELEM_ID, pos) ;
  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

	for (i=0; i<nodes; i++)
  {
     node_pos = fdbEnodePos(pos, i) ;

#if 0
     x = (float) fdbInputGetDbl(NODE,  NODE_X, node_pos) ;
     y = (float) fdbInputGetDbl(NODE,  NODE_Y, node_pos) ;
     z = (float) fdbInputGetDbl(NODE,  NODE_Z, node_pos) ;
#else
			x = gfxDefShapeCoord(node_pos, NODE_X) ;
			y = gfxDefShapeCoord(node_pos, NODE_Y) ;
			z = gfxDefShapeCoord(node_pos, NODE_Z) ;
#endif

     x = PoSX(x) ; 
     y = PoSY(y) ;
     z = PoSZ(z) ;

		 node[i][0] = x ;
		 node[i][1] = y ;
		 node[i][2] = z ;
  }

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();

	glLineWidth(1);
#ifdef USE_GL2PS
	gl2psLineWidth(1);
#endif

  /* wireframe: */
  if (plotProp.wireRes == AF_YES)
  {

	  glColor4f(0.0, 0.0, 0.0, 1.0);

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();
  }

  return(rv);
}

int gfx_e003_nres(long pos, long etype, long res)
{
  int rv = AF_OK;
  /* ---------------------------------- */
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  int  i;
	float node[4][3] ;
  double val[4] ;

  femElemColor(pos, &r, &g, &b);
	glColor4f(r,g,b,1.0);

  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

	for (i=0; i<nodes; i++)
  {
    node_pos = fdbEnodePos(pos, i) ;

			x = gfxDefShapeCoord(node_pos, NODE_X) ;
			y = gfxDefShapeCoord(node_pos, NODE_Y) ;
			z = gfxDefShapeCoord(node_pos, NODE_Z) ;

     x = PoSX(x) ;
     y = PoSY(y) ;
     z = PoSZ(z) ;

		 node[i][0] = x ;
		 node[i][1] = y ;
		 node[i][2] = z ;

     val[i] = fdbAvResGetVal(res, node_pos);
  }

  if ((plotProp.wireOnly != AF_YES)||(plotProp.Select == AF_YES))
  {
	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	      femGetResColor3f(val[0], &r, &g, &b); glColor4f(r,g,b,1.0);
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	      femGetResColor3f(val[1], &r, &g, &b); glColor4f(r,g,b,1.0);
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	      femGetResColor3f(val[2], &r, &g, &b); glColor4f(r,g,b,1.0);
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	      femGetResColor3f(val[0], &r, &g, &b); glColor4f(r,g,b,1.0);
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	      femGetResColor3f(val[2], &r, &g, &b); glColor4f(r,g,b,1.0);
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	      femGetResColor3f(val[3], &r, &g, &b); glColor4f(r,g,b,1.0);
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	      femGetResColor3f(val[0], &r, &g, &b); glColor4f(r,g,b,1.0);
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	      femGetResColor3f(val[3], &r, &g, &b); glColor4f(r,g,b,1.0);
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	      femGetResColor3f(val[1], &r, &g, &b); glColor4f(r,g,b,1.0);
	  glEnd();

	  glBegin(GL_POLYGON);
	  	glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	      femGetResColor3f(val[1], &r, &g, &b); glColor4f(r,g,b,1.0);
	  	glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	      femGetResColor3f(val[2], &r, &g, &b); glColor4f(r,g,b,1.0);
	  	glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	      femGetResColor3f(val[3], &r, &g, &b); glColor4f(r,g,b,1.0);
	  glEnd();
  }

  if (plotProp.Select != AF_YES)
  {
    /* wireframe: */
	  glColor4f(0.0, 0.0, 0.0, 1.0);

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[0][0], node[0][1], node[0][2] ) ;
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glVertex3f ( node[1][0], node[1][1], node[1][2] ) ;
	  glEnd();

	  glBegin(GL_LINE_STRIP);
	  glVertex3f ( node[2][0], node[2][1], node[2][2] ) ;
	  glVertex3f ( node[3][0], node[3][1], node[3][2] ) ;
	  glEnd();
  }
  /* ---------------------------------- */
  return(rv);
}

void femGfxEleInit003(long testtype)
{
  static long type  = 3 ;
  static long nodes = 4 ;

  if (testtype != type)
  {
    fprintf(msgout,"[E] %s: %li <> %li\n[E] %s!\n",
        _("FATAL ERROR - graphics element michmach "),
        type, testtype, _("EXITING"));
    exit (AF_ERR);
  }

  gfxElem[type].type  = type          ;
  gfxElem[type].nodes = nodes         ;

  gfxElem[type].geom  = gfx_e003_geom ;
  gfxElem[type].eres  = gfx_e003_eres ;
  gfxElem[type].nres  = gfx_e003_nres ;
}

#endif
/* end of gfx_e003.c */
