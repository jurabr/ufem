/*
   File name: gfx_e002.c
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

	 $Id: gfx_e002.c,v 1.9 2004/01/05 22:31:28 jirka Exp $
*/

#include "fem_gele.h"
#ifdef _USE_GFX_

float node[9][3] ; /* coordinates of all nodes */

int gfx_e002_geom(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  float sx,sy,sz;
  long elem_num ;
  int  i;

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


  if ((plotProp.wireOnly != AF_YES)||(plotProp.Select == AF_YES) )
  {
	  glBegin(GL_POLYGON);

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

		  glVertex3f ( x, y, z ) ;
    }

	  glEnd();
  }

	glLineWidth(1);
#ifdef USE_GL2PS
	gl2psLineWidth(1);
#endif

  if (plotProp.Select != AF_YES)
  {
    /* wireframe: */

    sx = 0 ;
    sy = 0 ;
    sz = 0 ;

    if (plotProp.wireOnly != AF_YES)
    {
	    glColor4f(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
      glColor4f(r,g,b,1.0);
    }

	  glBegin(GL_LINE_LOOP);

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

		  glVertex3f ( x, y, z ) ;
    }

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

void gfx_e002_rect(long n1i, long n2i, long n3i, long n4i)
{
  long n1,n2,n3,n4;

  n1 = n1i - 1 ;
  n2 = n2i - 1 ;
  n3 = n3i - 1 ;
  n4 = n4i - 1 ;
  
	glBegin(GL_POLYGON);

    glVertex3f( node[n1][0], node[n1][1], node[n1][2]);
    glVertex3f( node[n2][0], node[n2][1], node[n2][2]);
    glVertex3f( node[n3][0], node[n3][1], node[n3][2]);
    glVertex3f( node[n4][0], node[n4][1], node[n4][2]);
  glEnd();
}

int gfx_e002_eres(long res_pos, long etype, long res)
{
  int rv = AF_OK;
  long pos ;
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  long elem_num ;
  int  i;
  double val ;
  float node0[4][4] ; /* coordinates of element nodes */

	/* element location */
  pos = fdbResEPos(res_pos) ;


  elem_num = fdbInputGetInt(ELEM, ELEM_ID, pos) ;
  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

  /* element nodes: */
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

    node0[i][0]  = PoSX(x) ;
    node0[i][1]  = PoSY(y) ;
    node0[i][2]  = PoSZ(z) ;
  }

  /* all "nodes": */

  /* 1 = A */
  node[0][0]  = node0[0][0];
  node[0][1]  = node0[0][1];
  node[0][2]  = node0[0][2];

  /* 2 = A+B */
  node[1][0]  = (node0[0][0]+node0[1][0])/2.0;
  node[1][1]  = (node0[0][1]+node0[1][1])/2.0;
  node[1][2]  = (node0[0][2]+node0[1][2])/2.0;

  /* 3 = B */
  node[2][0]  = node0[1][0];
  node[2][1]  = node0[1][1];
  node[2][2]  = node0[1][2];

  /* 4 = A+D */
  node[3][0]  = (node0[0][0]+node0[3][0])/2.0;
  node[3][1]  = (node0[0][1]+node0[3][1])/2.0;
  node[3][2]  = (node0[0][2]+node0[3][2])/2.0;

  /* 5 = A+B+C+D */
  node[4][0]  = (node0[0][0]+node0[1][0]+node0[2][0]+node0[3][0])/4.0;
  node[4][1]  = (node0[0][1]+node0[1][1]+node0[2][1]+node0[3][1])/4.0;
  node[4][2]  = (node0[0][2]+node0[1][2]+node0[2][2]+node0[3][2])/4.0;

  /* 6 = B+C */
  node[5][0]  = (node0[1][0]+node0[2][0])/2.0;
  node[5][1]  = (node0[1][1]+node0[2][1])/2.0;
  node[5][2]  = (node0[1][2]+node0[2][2])/2.0;

  /* 7 = D */
  node[6][0]  = node0[3][0];
  node[6][1]  = node0[3][1];
  node[6][2]  = node0[3][2];

  /* 8 = D+C */
  node[7][0]  = (node0[3][0]+node0[2][0])/2.0;
  node[7][1]  = (node0[3][1]+node0[2][1])/2.0;
  node[7][2]  = (node0[3][2]+node0[2][2])/2.0;

  /* 9 = C */
  node[8][0]  = node0[2][0];
  node[8][1]  = node0[2][1];
  node[8][2]  = node0[2][2];

	glLineWidth(2);
#ifdef USE_GL2PS
	gl2psLineWidth(2);
#endif

	val = fdbResElemGetVal(res_pos, etype, res, 1) ;
	femGetResColor3f(val, &r, &g, &b);
	glColor4f(r,g,b,1.0);
	gfx_e002_rect(1, 2, 5, 4);

	val = fdbResElemGetVal(res_pos, etype, res, 3) ;
	femGetResColor3f(val, &r, &g, &b);
	glColor4f(r,g,b,1.0);
	gfx_e002_rect(2, 3, 6, 5);

	val = fdbResElemGetVal(res_pos, etype, res, 2) ;
	femGetResColor3f(val, &r, &g, &b);
	glColor4f(r,g,b,1.0);
	gfx_e002_rect(4, 5, 8, 7);

	val = fdbResElemGetVal(res_pos, etype, res, 4) ;
	femGetResColor3f(val, &r, &g, &b);
	glColor4f(r,g,b,1.0);
	gfx_e002_rect(5, 6, 9, 8);

	glLineWidth(1);
#ifdef USE_GL2PS
	gl2psLineWidth(1);
#endif


  /* wireframe: */
  if (plotProp.wireRes == AF_YES)
  {
    glColor4f(0.0, 0.0, 0.0, 1.0);

    glBegin(GL_LINE_LOOP);

    for (i=0; i<nodes; i++)
    {
	    glVertex3f ( node0[i][0], node0[i][1], node0[i][2] ) ;
    }

    glEnd();
  }
  return(rv);
}

int gfx_e002_nres(long pos, long etype, long res)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  int  i;
  double val ;

  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

  if ((plotProp.wireOnly != AF_YES)||(plotProp.Select == AF_YES) )
  {
	  glBegin(GL_POLYGON);

    for (i=0; i<nodes; i++)
    {
      node_pos = fdbEnodePos(pos, i) ;

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


  if (plotProp.Select != AF_YES)
  {
    /* wireframe: */

	  glColor4f(0.0, 0.0, 0.0, 1.0);

	  glBegin(GL_LINE_LOOP);

    for (i=0; i<nodes; i++)
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

  }

  return(rv);
}

void femGfxEleInit002(long testtype)
{
  static long type  = 2 ;
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

  gfxElem[type].geom  = gfx_e002_geom ;
  gfxElem[type].eres  = gfx_e002_eres ;
  gfxElem[type].nres  = gfx_e002_nres ;
}

#endif
/* end of gfx_e002.c */
