/*
   File name: gfx_e004.c
   Date:      2004/03/20 14:31
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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
	  (brick - 20 nodes)

	 $Id: gfx_e004.c,v 1.2 2005/07/11 17:57:16 jirka Exp $
*/

#include "fem_gele.h"
#ifdef _USE_GFX_

float xyz[12][4] ;

void plot004_side(void)
{
  glBegin(GL_TRIANGLES);
    glVertex3f(xyz[1][1], xyz[1][2], xyz[1][3]);
    glVertex3f(xyz[2][1], xyz[2][2], xyz[2][3]);
    glVertex3f(xyz[4][1], xyz[4][2], xyz[4][3]);

    glVertex3f(xyz[2][1], xyz[2][2], xyz[2][3]);
    glVertex3f(xyz[5][1], xyz[5][2], xyz[5][3]);
    glVertex3f(xyz[4][1], xyz[4][2], xyz[4][3]);

    glVertex3f(xyz[2][1], xyz[2][2], xyz[2][3]);
    glVertex3f(xyz[6][1], xyz[6][2], xyz[6][3]);
    glVertex3f(xyz[5][1], xyz[5][2], xyz[5][3]);

    glVertex3f(xyz[2][1], xyz[2][2], xyz[2][3]);
    glVertex3f(xyz[3][1], xyz[3][2], xyz[3][3]);
    glVertex3f(xyz[6][1], xyz[6][2], xyz[6][3]);

    glVertex3f(xyz[4][1], xyz[4][2], xyz[4][3]);
    glVertex3f(xyz[8][1], xyz[8][2], xyz[8][3]);
    glVertex3f(xyz[7][1], xyz[7][2], xyz[7][3]);

    glVertex3f(xyz[4][1], xyz[4][2], xyz[4][3]);
    glVertex3f(xyz[5][1], xyz[5][2], xyz[5][3]);
    glVertex3f(xyz[8][1], xyz[8][2], xyz[8][3]);

    glVertex3f(xyz[5][1], xyz[5][2], xyz[5][3]);
    glVertex3f(xyz[6][1], xyz[6][2], xyz[6][3]);
    glVertex3f(xyz[8][1], xyz[8][2], xyz[8][3]);

    glVertex3f(xyz[6][1], xyz[6][2], xyz[6][3]);
    glVertex3f(xyz[9][1], xyz[9][2], xyz[9][3]);
    glVertex3f(xyz[8][1], xyz[8][2], xyz[8][3]);
  glEnd();
}

int gfx_e004_geom(long pos)
{
  int rv = AF_OK;
  long nodes = 0 ;
  long node_pos = 0 ;
  float r,g,b ;
  float x,y,z;
  float sx,sy,sz;
  long elem_num ;
  int  i;
	float node[21][4] ;

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

  if (nodes == 20)
  {
	  for (i=0; i<nodes; i++)
    {
      node_pos = fdbEnodePos(pos, i) ;

			x = gfxDefShapeCoord(node_pos, NODE_X) ;
			y = gfxDefShapeCoord(node_pos, NODE_Y) ;
			z = gfxDefShapeCoord(node_pos, NODE_Z) ;

      x = PoSX(x) ; sx += x ;
      y = PoSY(y) ; sy += y ;
      z = PoSZ(z) ; sz += z ;
  
		  node[i+1][1] = x ;
		  node[i+1][2] = y ;
		  node[i+1][3] = z ;
    }
  }
  else  /* 8 nodes */
  {
    if (nodes != 8) {return(AF_ERR_SIZ);}

      node_pos = fdbEnodePos(pos, 0) ;
		  node[1][1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
		  node[1][2] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
		  node[1][3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 1) ;
		  node[3][1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
		  node[3][2] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
		  node[3][3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 2) ;
		  node[5][1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
		  node[5][2] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
		  node[5][3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 3) ;
		  node[7][1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
		  node[7][2] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
		  node[7][3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 4) ;
		  node[13][1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
		  node[13][2] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
		  node[13][3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 5) ;
		  node[15][1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
		  node[15][2] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
		  node[15][3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 6) ;
		  node[17][1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
		  node[17][2] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
		  node[17][3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 7) ;
		  node[19][1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
		  node[19][2] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
		  node[19][3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;


		  node[2][1] = 0.5*(node[1][1] + node[3][1] );
		  node[2][2] = 0.5*(node[1][2] + node[3][2] );
		  node[2][3] = 0.5*(node[1][3] + node[3][3] );

		  node[4][1] = 0.5*(node[3][1] + node[5][1] );
		  node[4][2] = 0.5*(node[3][2] + node[5][2] );
		  node[4][3] = 0.5*(node[3][3] + node[5][3] );

		  node[6][1] = 0.5*(node[5][1] + node[7][1] );
		  node[6][2] = 0.5*(node[5][2] + node[7][2] );
		  node[6][3] = 0.5*(node[5][3] + node[7][3] );

		  node[8][1] = 0.5*(node[1][1] + node[7][1] );
		  node[8][2] = 0.5*(node[1][2] + node[7][2] );
		  node[8][3] = 0.5*(node[1][3] + node[7][3] );

		  node[9][1] = 0.5*(node[1][1] + node[13][1] );
		  node[9][2] = 0.5*(node[1][2] + node[13][2] );
		  node[9][3] = 0.5*(node[1][3] + node[13][3] );

		  node[10][1] = 0.5*(node[3][1] + node[15][1] );
		  node[10][2] = 0.5*(node[3][2] + node[15][2] );
		  node[10][3] = 0.5*(node[3][3] + node[15][3] );

		  node[11][1] = 0.5*(node[5][1] + node[17][1] );
		  node[11][2] = 0.5*(node[5][2] + node[17][2] );
		  node[11][3] = 0.5*(node[5][3] + node[17][3] );

		  node[12][1] = 0.5*(node[7][1] + node[19][1] );
		  node[12][2] = 0.5*(node[7][2] + node[19][2] );
		  node[12][3] = 0.5*(node[7][3] + node[19][3] );

		  node[14][1] = 0.5*(node[13][1] + node[15][1] );
		  node[14][2] = 0.5*(node[13][2] + node[15][2] );
		  node[14][3] = 0.5*(node[13][3] + node[15][3] );

		  node[16][1] = 0.5*(node[15][1] + node[17][1] );
		  node[16][2] = 0.5*(node[15][2] + node[17][2] );
		  node[16][3] = 0.5*(node[15][3] + node[17][3] );

		  node[18][1] = 0.5*(node[17][1] + node[19][1] );
		  node[18][2] = 0.5*(node[17][2] + node[19][2] );
		  node[18][3] = 0.5*(node[17][3] + node[19][3] );

		  node[20][1] = 0.5*(node[13][1] + node[19][1] );
		  node[20][2] = 0.5*(node[13][2] + node[19][2] );
		  node[20][3] = 0.5*(node[13][3] + node[19][3] );

			sx = 0.5*(node[1][1] + node[17][1]) * (double)nodes ;
			sy = 0.5*(node[1][2] + node[17][2]) * (double)nodes ;
			sz = 0.5*(node[1][3] + node[17][3]) * (double)nodes ;
  }

  if ((plotProp.wireOnly != AF_YES)||(plotProp.Select == AF_YES))
  {
    xyz[1][1] = node[1][1] ;
    xyz[1][2] = node[1][2] ;
    xyz[1][3] = node[1][3] ;

    xyz[2][1] = node[2][1] ;
    xyz[2][2] = node[2][2] ;
    xyz[2][3] = node[2][3] ;

    xyz[3][1] = node[3][1] ;
    xyz[3][2] = node[3][2] ;
    xyz[3][3] = node[3][3] ;

    xyz[4][1] = node[8][1] ;
    xyz[4][2] = node[8][2] ;
    xyz[4][3] = node[8][3] ;

    xyz[5][1] = 0.25*( node[2][1]+node[4][1]+node[6][1]+node[8][1] ) ;
    xyz[5][2] = 0.25*( node[2][2]+node[4][2]+node[6][2]+node[8][2] ) ;
    xyz[5][3] = 0.25*( node[2][3]+node[4][3]+node[6][3]+node[8][3] ) ;

    xyz[6][1] = node[4][1] ;
    xyz[6][2] = node[4][2] ;
    xyz[6][3] = node[4][3] ;

    xyz[7][1] = node[7][1] ;
    xyz[7][2] = node[7][2] ;
    xyz[7][3] = node[7][3] ;

    xyz[8][1] = node[6][1] ;
    xyz[8][2] = node[6][2] ;
    xyz[8][3] = node[6][3] ;

    xyz[9][1] = node[5][1] ;
    xyz[9][2] = node[5][2] ;
    xyz[9][3] = node[5][3] ;

    plot004_side();


    xyz[1][1] = node[13][1] ;
    xyz[1][2] = node[13][2] ;
    xyz[1][3] = node[13][3] ;

    xyz[2][1] = node[14][1] ;
    xyz[2][2] = node[14][2] ;
    xyz[2][3] = node[14][3] ;

    xyz[3][1] = node[15][1] ;
    xyz[3][2] = node[15][2] ;
    xyz[3][3] = node[15][3] ;

    xyz[4][1] = node[20][1] ;
    xyz[4][2] = node[20][2] ;
    xyz[4][3] = node[20][3] ;

    xyz[5][1] = 0.25*( node[14][1]+node[16][1]+node[18][1]+node[20][1] ) ;
    xyz[5][2] = 0.25*( node[14][2]+node[16][2]+node[18][2]+node[20][2] ) ;
    xyz[5][3] = 0.25*( node[14][3]+node[16][3]+node[18][3]+node[20][3] ) ;

    xyz[6][1] = node[16][1] ;
    xyz[6][2] = node[16][2] ;
    xyz[6][3] = node[16][3] ;

    xyz[7][1] = node[19][1] ;
    xyz[7][2] = node[19][2] ;
    xyz[7][3] = node[19][3] ;

    xyz[8][1] = node[18][1] ;
    xyz[8][2] = node[18][2] ;
    xyz[8][3] = node[18][3] ;

    xyz[9][1] = node[17][1] ;
    xyz[9][2] = node[17][2] ;
    xyz[9][3] = node[17][3] ;

    plot004_side();


    xyz[1][1] = node[13][1] ;
    xyz[1][2] = node[13][2] ;
    xyz[1][3] = node[13][3] ;

    xyz[2][1] = node[14][1] ;
    xyz[2][2] = node[14][2] ;
    xyz[2][3] = node[14][3] ;

    xyz[3][1] = node[15][1] ;
    xyz[3][2] = node[15][2] ;
    xyz[3][3] = node[15][3] ;

    xyz[4][1] = node[9][1] ;
    xyz[4][2] = node[9][2] ;
    xyz[4][3] = node[9][3] ;

    xyz[5][1] = 0.25*( node[2][1]+node[9][1]+node[10][1]+node[14][1] ) ;
    xyz[5][2] = 0.25*( node[2][2]+node[9][2]+node[10][2]+node[14][2] ) ;
    xyz[5][3] = 0.25*( node[2][3]+node[9][3]+node[10][3]+node[14][3] ) ;

    xyz[6][1] = node[10][1] ;
    xyz[6][2] = node[10][2] ;
    xyz[6][3] = node[10][3] ;

    xyz[7][1] = node[1][1] ;
    xyz[7][2] = node[1][2] ;
    xyz[7][3] = node[1][3] ;

    xyz[8][1] = node[2][1] ;
    xyz[8][2] = node[2][2] ;
    xyz[8][3] = node[2][3] ;

    xyz[9][1] = node[3][1] ;
    xyz[9][2] = node[3][2] ;
    xyz[9][3] = node[3][3] ;

    plot004_side();


    xyz[1][1] = node[19][1] ;
    xyz[1][2] = node[19][2] ;
    xyz[1][3] = node[19][3] ;

    xyz[2][1] = node[18][1] ;
    xyz[2][2] = node[18][2] ;
    xyz[2][3] = node[18][3] ;

    xyz[3][1] = node[17][1] ;
    xyz[3][2] = node[17][2] ;
    xyz[3][3] = node[17][3] ;

    xyz[4][1] = node[12][1] ;
    xyz[4][2] = node[12][2] ;
    xyz[4][3] = node[12][3] ;

    xyz[5][1] = 0.25*( node[6][1]+node[11][1]+node[12][1]+node[18][1] ) ;
    xyz[5][2] = 0.25*( node[6][2]+node[11][2]+node[12][2]+node[18][2] ) ;
    xyz[5][3] = 0.25*( node[6][3]+node[11][3]+node[12][3]+node[18][3] ) ;

    xyz[6][1] = node[11][1] ;
    xyz[6][2] = node[11][2] ;
    xyz[6][3] = node[11][3] ;

    xyz[7][1] = node[7][1] ;
    xyz[7][2] = node[7][2] ;
    xyz[7][3] = node[7][3] ;

    xyz[8][1] = node[6][1] ;
    xyz[8][2] = node[6][2] ;
    xyz[8][3] = node[6][3] ;

    xyz[9][1] = node[5][1] ;
    xyz[9][2] = node[5][2] ;
    xyz[9][3] = node[5][3] ;

    plot004_side();

    xyz[1][1] = node[15][1] ;
    xyz[1][2] = node[15][2] ;
    xyz[1][3] = node[15][3] ;

    xyz[2][1] = node[10][1] ;
    xyz[2][2] = node[10][2] ;
    xyz[2][3] = node[10][3] ;

    xyz[3][1] = node[3][1] ;
    xyz[3][2] = node[3][2] ;
    xyz[3][3] = node[3][3] ;

    xyz[4][1] = node[16][1] ;
    xyz[4][2] = node[16][2] ;
    xyz[4][3] = node[16][3] ;

    xyz[5][1] = 0.25*( node[4][1]+node[10][1]+node[11][1]+node[16][1] ) ;
    xyz[5][2] = 0.25*( node[4][2]+node[10][2]+node[11][2]+node[16][2] ) ;
    xyz[5][3] = 0.25*( node[4][3]+node[10][3]+node[11][3]+node[16][3] ) ;

    xyz[6][1] = node[4][1] ;
    xyz[6][2] = node[4][2] ;
    xyz[6][3] = node[4][3] ;

    xyz[7][1] = node[17][1] ;
    xyz[7][2] = node[17][2] ;
    xyz[7][3] = node[17][3] ;

    xyz[8][1] = node[11][1] ;
    xyz[8][2] = node[11][2] ;
    xyz[8][3] = node[11][3] ;

    xyz[9][1] = node[5][1] ;
    xyz[9][2] = node[5][2] ;
    xyz[9][3] = node[5][3] ;

    plot004_side();

    xyz[1][1] = node[1][1] ;
    xyz[1][2] = node[1][2] ;
    xyz[1][3] = node[1][3] ;

    xyz[2][1] = node[9][1] ;
    xyz[2][2] = node[9][2] ;
    xyz[2][3] = node[9][3] ;

    xyz[3][1] = node[13][1] ;
    xyz[3][2] = node[13][2] ;
    xyz[3][3] = node[13][3] ;

    xyz[4][1] = node[8][1] ;
    xyz[4][2] = node[8][2] ;
    xyz[4][3] = node[8][3] ;

    xyz[5][1] = 0.25*( node[8][1]+node[9][1]+node[12][1]+node[20][1] ) ;
    xyz[5][2] = 0.25*( node[8][2]+node[9][2]+node[12][2]+node[20][2] ) ;
    xyz[5][3] = 0.25*( node[8][3]+node[9][3]+node[12][3]+node[20][3] ) ;

    xyz[6][1] = node[20][1] ;
    xyz[6][2] = node[20][2] ;
    xyz[6][3] = node[20][3] ;

    xyz[7][1] = node[7][1] ;
    xyz[7][2] = node[7][2] ;
    xyz[7][3] = node[7][3] ;

    xyz[8][1] = node[12][1] ;
    xyz[8][2] = node[12][2] ;
    xyz[8][3] = node[12][3] ;

    xyz[9][1] = node[19][1] ;
    xyz[9][2] = node[19][2] ;
    xyz[9][3] = node[19][3] ;

    plot004_side();
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
      for (i=1; i<=8; i++)
      {
	      glVertex3f ( node[i][1], node[i][2], node[i][3] ) ;
      }
	    glVertex3f ( node[1][1], node[1][2], node[1][3] ) ;
	  glEnd();
    
    glBegin(GL_LINE_STRIP);
      for (i=13; i<=20; i++)
      {
	      glVertex3f ( node[i][1], node[i][2], node[i][3] ) ;
      }
	    glVertex3f ( node[13][1], node[13][2], node[13][3] ) ;
	  glEnd();

    glBegin(GL_LINE_STRIP);
	    glVertex3f ( node[1][1], node[1][2], node[1][3] ) ;
	    glVertex3f ( node[9][1], node[9][2], node[9][3] ) ;
	    glVertex3f ( node[13][1], node[13][2], node[13][3] ) ;
	  glEnd();

    glBegin(GL_LINE_STRIP);
	    glVertex3f ( node[3][1], node[3][2], node[3][3] ) ;
	    glVertex3f ( node[10][1], node[10][2], node[10][3] ) ;
	    glVertex3f ( node[15][1], node[15][2], node[15][3] ) ;
	  glEnd();

    glBegin(GL_LINE_STRIP);
	    glVertex3f ( node[7][1], node[7][2], node[7][3] ) ;
	    glVertex3f ( node[12][1], node[12][2], node[12][3] ) ;
	    glVertex3f ( node[19][1], node[19][2], node[19][3] ) ;
	  glEnd();

    glBegin(GL_LINE_STRIP);
	    glVertex3f ( node[5][1], node[5][2], node[5][3] ) ;
	    glVertex3f ( node[11][1], node[11][2], node[11][3] ) ;
	    glVertex3f ( node[17][1], node[17][2], node[17][3] ) ;
	  glEnd();


    sx = (sx/(double)nodes);
	  sy = (sy/(double)nodes);
	  sz = (sz/(double)nodes);

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

void gfx_e004_comp_coord8(long neww, long c1, long c2, float *x0,float *y0,float *z0)
{
  x0[neww] = 0.5 * ( x0[c1] + x0[c2] ) ;
  y0[neww] = 0.5 * ( y0[c1] + y0[c2] ) ;
  z0[neww] = 0.5 * ( z0[c1] + z0[c2] ) ;
}

void gfx_e004_eres_sel_color(long num, float *r, float *g, float *b)
{
  long pos = 1 ;

  switch (num)
  {
    case 1:
    case 2: pos = 1 ; break ;
    case 3:
    case 4:
    case 5: 
    case 6: pos = 2 ; break ;
    case 7:
    case 8: pos = 3 ; break ;
    case 9:
    case 10:
    case 17:
    case 18: pos = 4 ; break ;
    case 11:
    case 12:
    case 13:
    case 14:
    case 19:
    case 20:
    case 21:
    case 22: pos = 5 ; break ;
    case 15:
    case 16:
    case 23:
    case 24: pos = 6 ; break ;
    case 25:
    case 26: pos = 7 ; break ;
    case 27:
    case 28:
    case 29:
    case 30: pos = 8 ; break ;
    case 31:
    case 32: pos = 9 ; break ;
  }
  
  glColor4f(r[pos],g[pos],b[pos],1.0) ;
}

void gfx_e004_eres_one_side(float *x,float *y,float *z, float *r,float *g,float *b)
{
  long num = 0 ;
  long pos1,pos2,pos3;
  long i,j ;

  gfx_e004_comp_coord8(2, 1, 3, x,y,z);
  gfx_e004_comp_coord8(4, 3, 5, x,y,z);
  gfx_e004_comp_coord8(6, 1, 11, x,y,z);
  gfx_e004_comp_coord8(10, 5, 15, x,y,z);
  gfx_e004_comp_coord8(16, 11, 21, x,y,z);
  gfx_e004_comp_coord8(20, 15, 25, x,y,z);
  gfx_e004_comp_coord8(22, 21, 23, x,y,z);
  gfx_e004_comp_coord8(24, 23, 25, x,y,z);
  gfx_e004_comp_coord8(7, 3, 11, x,y,z);
  gfx_e004_comp_coord8(9, 3, 15, x,y,z);
  gfx_e004_comp_coord8(17, 11, 23, x,y,z);
  gfx_e004_comp_coord8(19, 15, 23, x,y,z);
  gfx_e004_comp_coord8(8, 7, 9, x,y,z);
  gfx_e004_comp_coord8(12, 7, 17, x,y,z);
  gfx_e004_comp_coord8(14, 9, 19, x,y,z);
  gfx_e004_comp_coord8(18, 17, 19, x,y,z);

  x[13] = 0.25*(x[7]+x[9]+x[17]+x[19]) ;
  y[13] = 0.25*(y[7]+y[9]+y[17]+y[19]) ;
  z[13] = 0.25*(z[7]+z[9]+z[17]+z[19]) ;

  glBegin(GL_TRIANGLES);
    for (i=0; i<4; i++)
    {
      for (j=0; j<4; j++)
      {
        num++ ;
        gfx_e004_eres_sel_color(num, r, g, b);

        pos1 = 5*i+j+1 ;
        pos2 = 5*(i+1)+j+2 ;
        pos3 = 5*(i+1)+j+1 ;

        glVertex3f(x[pos1], y[pos1], z[pos1]);
        glVertex3f(x[pos2], y[pos2], z[pos2]);
        glVertex3f(x[pos3], y[pos3], z[pos3]);


        num++ ;
        /*gfx_e004_eres_sel_color(num, r, g, b);*/

        pos1 = 5*i+j+1 ;
        pos2 = 5*i+j+2 ;
        pos3 = 5*(i+1)+j+2 ;

        glVertex3f(x[pos1], y[pos1], z[pos1]);
        glVertex3f(x[pos2], y[pos2], z[pos2]);
        glVertex3f(x[pos3], y[pos3], z[pos3]);
      }
    }
  glEnd();
}

void gfx_e004_prep_side_coord(long *b, float *x0,float *y0,float *z0, float *x,float *y,float *z)
{
  long ind[9] = {0,1,3,5,11,15,21,23,25} ;
  long i ;

  for (i=1; i<=8; i++)
  {
    x[ind[i]] = x0[b[i]] ;
    y[ind[i]] = y0[b[i]] ;
    z[ind[i]] = z0[b[i]] ;
  }
}

void gfx_e004_prep_side_rgb(long *c, float *r0,float *g0,float *b0, float *r,float *g,float *b)
{
  long ind[10] = {0,1,2,3,4,5,6,7,8,9} ;
  long i ;

  for (i=1; i<=9; i++)
  {
    r[ind[i]] = r0[c[i]] ;
    g[ind[i]] = g0[c[i]] ;
    b[ind[i]] = b0[c[i]] ;
  }
}

int gfx_e004_eres(long res_pos, long etype, long res)
{
  int rv = AF_OK;
  long  pos ;
  long  elem_num ;
  float val ;
  float r0[28];
  float g0[28];
  float b0[28];
  float r[10];
  float g[10];
  float b[10];
  long  nodes, node_pos;
  float x0[21] ;
  float y0[21] ;
  float z0[21] ;
  float x[26] ;
  float y[26] ;
  float z[26] ;
  long  i;
  long  b1[9] = {0, 1, 2, 3, 8, 4, 7, 6, 5} ;
  long  b2[9] = {0,13,14,15,20,16,19,18,17} ;
  long  b3[9] = {0,13,14,15, 9,10, 1, 2, 3} ;
  long  b4[9] = {0,19,18,17,12,11, 7, 6, 5} ;
  long  b5[9] = {0,15,10, 3,16, 4,17,11, 5} ;
  long  b6[9] = {0,13, 9, 1,20, 8,19,12, 7} ;
  long  c1[10] = {0, 1,10,19, 4,13,22, 7,16,25} ;
  long  c2[10] = {0, 3,12,21, 6,15,24, 9,18,27} ;
  long  c3[10] = {0, 3,12,21, 2,11,20, 1,10,19} ;
  long  c4[10] = {0, 9,18,27, 8,17,26, 7,16,25} ;
  long  c5[10] = {0,21,20,19,24,23,22,27,26,25} ;
  long  c6[10] = {0, 3, 2, 1, 6, 5, 4, 9, 8, 7} ;

	/* result color computation */
  for (i=1; i<=27; i++)
  {
	  val = fdbResElemGetVal(res_pos, etype, res, i) ;
	  femGetResColor3f(val, &r0[i], &g0[i], &b0[i]);
  }

	glLineWidth(2);
#ifdef USE_GL2PS
	gl2psLineWidth(2);
#endif

	/* element location */
  pos = fdbResEPos(res_pos) ;

  elem_num = fdbInputGetInt(ELEM, ELEM_ID, pos) ;
  nodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

  if (nodes == 20)
  {
    for (i=0; i<nodes; i++)
    {
      node_pos = fdbEnodePos(pos, i) ;
  
  		x0[i+1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[i+1] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[i+1] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;
    }
  }
  else /* 8 nodes */
  {
    if (nodes != 8) {return(AF_ERR_SIZ);}
      node_pos = fdbEnodePos(pos, 0) ;
  		x0[1] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[1] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[1] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 1) ;
  		x0[3] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[3] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[3] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 2) ;
  		x0[5] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[5] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[5] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 3) ;
  		x0[7] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[7] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[7] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 4) ;
  		x0[13] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[13] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[13] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 5) ;
  		x0[15] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[15] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[15] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 6) ;
  		x0[17] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[17] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[17] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      node_pos = fdbEnodePos(pos, 7) ;
  		x0[19] = PoSX(gfxDefShapeCoord(node_pos, NODE_X)) ;
  		y0[19] = PoSY(gfxDefShapeCoord(node_pos, NODE_Y)) ;
  		z0[19] = PoSZ(gfxDefShapeCoord(node_pos, NODE_Z)) ;

      gfx_e004_comp_coord8(2, 1, 3, x0,y0,z0);
      gfx_e004_comp_coord8(4, 3, 5, x0,y0,z0);
      gfx_e004_comp_coord8(6, 5, 7, x0,y0,z0);
      gfx_e004_comp_coord8(8, 1, 7, x0,y0,z0);
      gfx_e004_comp_coord8(9, 1, 13, x0,y0,z0);
      gfx_e004_comp_coord8(10, 3, 15, x0,y0,z0);
      gfx_e004_comp_coord8(11, 5, 17, x0,y0,z0);
      gfx_e004_comp_coord8(12, 7, 19, x0,y0,z0);
      gfx_e004_comp_coord8(14, 13, 15, x0,y0,z0);
      gfx_e004_comp_coord8(16, 15, 17, x0,y0,z0);
      gfx_e004_comp_coord8(18, 17, 19, x0,y0,z0);
      gfx_e004_comp_coord8(20, 13, 19, x0,y0,z0);
  }

  gfx_e004_prep_side_coord(b1, x0,y0,z0, x,y,z);
  gfx_e004_prep_side_rgb(c1, r0,g0,b0, r,g,b);
  gfx_e004_eres_one_side(x,y,z, r,g,b);

  gfx_e004_prep_side_coord(b2, x0,y0,z0, x,y,z);
  gfx_e004_prep_side_rgb(c2, r0,g0,b0, r,g,b);
  gfx_e004_eres_one_side(x,y,z, r,g,b);

  gfx_e004_prep_side_coord(b3, x0,y0,z0, x,y,z);
  gfx_e004_prep_side_rgb(c3, r0,g0,b0, r,g,b);
  gfx_e004_eres_one_side(x,y,z, r,g,b);

  gfx_e004_prep_side_coord(b4, x0,y0,z0, x,y,z);
  gfx_e004_prep_side_rgb(c4, r0,g0,b0, r,g,b);
  gfx_e004_eres_one_side(x,y,z, r,g,b);

  gfx_e004_prep_side_coord(b5, x0,y0,z0, x,y,z);
  gfx_e004_prep_side_rgb(c5, r0,g0,b0, r,g,b);
  gfx_e004_eres_one_side(x,y,z, r,g,b);

  gfx_e004_prep_side_coord(b6, x0,y0,z0, x,y,z);
  gfx_e004_prep_side_rgb(c6, r0,g0,b0, r,g,b);
  gfx_e004_eres_one_side(x,y,z, r,g,b);

	glLineWidth(1);
#ifdef USE_GL2PS
	gl2psLineWidth(1);
#endif

  /* ----------------------------------------------- */
  if (plotProp.Select != AF_YES)
  {
    /* wireframe: */

	  glColor4f(0.0, 0.0, 0.0, 1.0);

	  glBegin(GL_LINE_STRIP);
      for (i=1; i<=8; i++)
      {
	      glVertex3f ( x0[i], y0[i], z0[i] ) ;
      }
	    glVertex3f ( x0[1], y0[1], z0[1] ) ;
	  glEnd();
    
    glBegin(GL_LINE_STRIP);
      for (i=13; i<=20; i++)
      {
	      glVertex3f ( x0[i], y0[i], z0[i] ) ;
      }
	    glVertex3f ( x0[13], y0[13], z0[13] ) ;
	  glEnd();

    glBegin(GL_LINE_STRIP);
	    glVertex3f ( x0[1], y0[1], z0[1] ) ;
	    glVertex3f ( x0[9], y0[9], z0[9] ) ;
	    glVertex3f ( x0[13], y0[13], z0[13] ) ;
	  glEnd();

    glBegin(GL_LINE_STRIP);
	    glVertex3f ( x0[3], y0[3], z0[3] ) ;
	    glVertex3f ( x0[10], y0[10], z0[10] ) ;
	    glVertex3f ( x0[15], y0[15], z0[15] ) ;
	  glEnd();

    glBegin(GL_LINE_STRIP);
	    glVertex3f ( x0[7], y0[7], z0[7] ) ;
	    glVertex3f ( x0[12], y0[12], z0[12] ) ;
	    glVertex3f ( x0[19], y0[19], z0[19] ) ;
	  glEnd();

    glBegin(GL_LINE_STRIP);
	    glVertex3f ( x0[5], y0[5], z0[5] ) ;
	    glVertex3f ( x0[11], y0[11], z0[11] ) ;
	    glVertex3f ( x0[17], y0[17], z0[17] ) ;
	  glEnd();
  }

  return(rv);
}

int gfx_e004_nres(long pos, long etype, long res)
{
  int rv = AF_OK;

  return(rv);
}

void femGfxEleInit004(long testtype)
{
  static long type  = 4 ;
  static long nodes = 20 ;

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
  gfxElem[type].nres  = gfx_e004_nres ;
}


#endif
/* end of gfx_e004.c */
