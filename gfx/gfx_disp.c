/*
   File name: gfx_load.c
   Date:      2003/11/24 09:01
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

	 Graphics output for "fem" (pre|post)processor - load plotting

	 $Id: gfx_disp.c,v 1.6 2004/01/01 21:35:30 jirka Exp $
*/

#include "fem_gfx.h"
#ifdef _USE_GFX_


void afDispSymbol(
		 double  Size, 
		 long    Dir, 
		 double  x, 
		 double  y, 
		 double  z, 
		 long    Num, 
		 float   r,
		 float   g,
		 float   b,
		 float   tr)
{ 
	double S,F;
	
	F = (plotProp.Max-plotProp.Min)/30.0;
	S = F/4.0;
	
  glLoadIdentity();
  glTranslatef(x,y,z);

 	switch (Dir)
  {
		/* u:  */
    case 1:
    case 13:
    case 25:
			glRotatef(180,0,1,0); break;
    case 2:
    case 14:
    case 26:
			glRotatef(270,0,0,1); break;
    case 3:
    case 15:
    case 27:
			glRotatef(90,0,1,0); break;
		/* rot: */
    case 4:
    case 16:
    case 28:
			glRotatef(180,0,0,1); break;
    case 5:
    case 17:
    case 29:
			glRotatef(270,0,0,1); break;
    case 6:
    case 18:
    case 30:
			glRotatef(90,0,1,0); break;

    default: glLoadIdentity(); return; break;
  }


	glColor4f(r,g,b,tr);

	if ((Dir>=13)&&(Dir<=15)) { glColor4f(1.0, 0.6, 0.3,  1.0) ; }
	if ((Dir>=25)&&(Dir<=27)) { glColor4f(0.6, 0.6, 1.0,  1.0) ; }
	
	if (plotProp.PlotToFile == AF_YES) 
  { 
#ifdef USE_GL2PS
    gl2psPointSize(4); 
#endif
  }
	else 
  { 
    glPointSize(4); 
  }

	glBegin(GL_POINTS);
	  glVertex3f(0,0,0);
	glEnd();

	if (plotProp.PlotToFile == AF_YES) 
  { 
#ifdef USE_GL2PS
    gl2psPointSize(1); 
#endif
  }
	else 
  { 
    glPointSize(1); 
  }

	/* translations */
	if ( (Dir <= 3) || ((Dir>=13)&&(Dir<=15)) || ((Dir>=25)&&(Dir<=27)) ) 
	{

			
	  if (plotProp.PlotToFile == AF_YES) 
    { 
#ifdef USE_GL2PS
      gl2psLineWidth(2); 
#endif
    }
	  else 
    { 
      glLineWidth(2); 
    }

	  glBegin(GL_LINE_STRIP);
	    glVertex3f(0,0,0);
	    glVertex3f(F+0,0,0);
	  glEnd();
    
	  if (plotProp.PlotToFile == AF_YES) 
    { 
#ifdef USE_GL2PS
      gl2psLineWidth(1); 
#endif
    }
	  else 
    { 
      glLineWidth(1); 
    }

	  glBegin(GL_TRIANGLE_FAN);
	    glVertex3f(0.7*F,0,0);
	    glVertex3f(F,S,S);
	    glVertex3f(F,-S,S);
	    glVertex3f(F,-S,-S);
	    glVertex3f(F,S,-S);
	    glVertex3f(F,S,S);
	  glEnd();
	}
	else /* rotations */
	{
	  if (plotProp.PlotToFile == AF_YES) 
    { 
#ifdef USE_GL2PS
      gl2psLineWidth(2); 
#endif
    }
	  else 
    { 
      glLineWidth(2); 
    }

		glBegin(GL_POLYGON);
	    glVertex3f( 0.4*F, 0.3*F,-0.2*F);
	    glVertex3f(-0.4*F, 0.3*F,-0.2*F);
	    glVertex3f(-0.4*F, 0.3*F, 0.2*F);
	    glVertex3f( 0.4*F, 0.3*F, 0.2*F);
		glEnd();

		glBegin(GL_POLYGON);
	    glVertex3f( 0.4*F,-0.3*F,-0.2*F);
	    glVertex3f(-0.4*F,-0.3*F,-0.2*F);
	    glVertex3f(-0.4*F,-0.3*F, 0.2*F);
	    glVertex3f( 0.4*F,-0.3*F, 0.2*F);
		glEnd();

		glBegin(GL_POLYGON);
	    glVertex3f( 0.4*F,-0.3*F,-0.2*F);
	    glVertex3f(-0.4*F,-0.3*F,-0.2*F);
	    glVertex3f(-0.4*F, 0.3*F,-0.2*F);
	    glVertex3f( 0.4*F, 0.3*F,-0.2*F);
		glEnd();

		glBegin(GL_POLYGON);
	    glVertex3f( 0.4*F,-0.2*F, 0.2*F);
	    glVertex3f(-0.4*F,-0.2*F, 0.2*F);
	    glVertex3f(-0.4*F, 0.2*F, 0.2*F);
	    glVertex3f( 0.4*F, 0.2*F, 0.2*F);
		glEnd();

		/*    */
		glBegin(GL_POLYGON);
	    glVertex3f(0,-0.2*F,-0.2*F);
	    glVertex3f(0,-0.2*F, 0.2*F);
	    glVertex3f(0, 0.2*F, 0.2*F);
	    glVertex3f(0, 0.2*F,-0.2*F);
		glEnd();
	
	  if (plotProp.PlotToFile == AF_YES) 
    { 
#ifdef USE_GL2PS
      gl2psLineWidth(2); 
#endif
    }
	  else 
    { 
      glLineWidth(2); 
    }
  }


  if (plotProp.Select != AF_YES)
  {
    /* disp number: */
	  if (plotProp.dispNumber == AF_YES)
	  {
		  femPlotInt(Num,F+0,0,0,NULL);
	  }

	  /* disp size: */
	  if (plotProp.dispSize == AF_YES)
	  {
		  femPlotDouble(fabs(Size),F+0,0,0,NULL);
	  }
  }
}

/* TODO: symbol for stiffness TODO */
void afStiffSymbol(
		 double  Size, 
		 long    Dir, 
		 double  x, 
		 double  y, 
		 double  z, 
		 long    Num, 
		 float   r,
		 float   g,
		 float   b,
		 float   tr)
{ 
	double S,F;

	F = (plotProp.Max-plotProp.Min)/30.0;
	S = F/4.0;

	
  glLoadIdentity();
  glTranslatef(x,y,z);

	switch (Dir)
  {
		/* u:  */
    case 7:
    case 19:
    case 31:
			glRotatef(180,0,1,0); break;
    case 8:
    case 20:
    case 32:
			glRotatef(270,0,0,1); break;
    case 9:
    case 21:
    case 33:
			glRotatef(90,0,1,0); break;
		/* rot: */
    case 10:
    case 22:
    case 34:
			glRotatef(180,0,0,1); break;
    case 11:
    case 23:
    case 35:
			glRotatef(270,0,0,1); break;
    case 12:
    case 24:
    case 36:
			glRotatef(90,0,1,0); break;

    default: glLoadIdentity(); return; break;
  }

	if (fabs(Size) <= FEM_ZERO) {return;}
	
	glColor4f(r,g,b,tr);

	if ((Dir>=19)&&(Dir<=24)) { glColor4f(1.0, 0.6, 0.3,  1.0) ; }
	if ((Dir>=31)&&(Dir<=36)) { glColor4f(0.6, 0.6, 1.0,  1.0) ; }

	
	if (plotProp.PlotToFile == AF_YES) 
  { 
#ifdef USE_GL2PS
    gl2psPointSize(4); 
#endif
  }
	else 
  { 
    glPointSize(4); 
  }

	glBegin(GL_POINTS);
	  glVertex3f(0,0,0);
	glEnd();

	if (plotProp.PlotToFile == AF_YES) 
  { 
#ifdef USE_GL2PS
    gl2psPointSize(1); 
#endif
  }
	else 
  { 
    glPointSize(1); 
  }

	
	/* translation stiffnesses */
	if ( ((Dir >= 7)&&(Dir <=9)) || ((Dir>=19)&&(Dir<=22)) || ((Dir>=31)&&(Dir<=33)) ) 
	{
	  if (plotProp.PlotToFile == AF_YES) 
    { 
#ifdef USE_GL2PS
      gl2psLineWidth(2); 
#endif
    }
	  else 
    { 
      glLineWidth(2); 
    }

	  glBegin(GL_LINE_STRIP);
	    glVertex3f(0,0,0);
	    glVertex3f(F+0,0,0);
	  glEnd();
    
	  if (plotProp.PlotToFile == AF_YES) 
    { 
#ifdef USE_GL2PS
      gl2psLineWidth(1); 
#endif
    }
	  else 
    { 
      glLineWidth(1); 
    }

	  glBegin(GL_TRIANGLE_FAN);
	    glVertex3f(0.7*F,0,0);
	    glVertex3f(F,S,S);
	    glVertex3f(F,-S,S);
	    glVertex3f(F,-S,-S);
	    glVertex3f(F,S,-S);
	    glVertex3f(F,S,S);
	  glEnd();
		
  	glBegin(GL_TRIANGLE_FAN);
	    glVertex3f(0.7*F,0,0);
	    glVertex3f(0.4*F,S,S);
	    glVertex3f(0.4*F,-S,S);
	    glVertex3f(0.4*F,-S,-S);
	    glVertex3f(0.4*F,S,-S);
	    glVertex3f(0.4*F,S,S);
	  glEnd();

		glBegin(GL_TRIANGLE_FAN);
	    glVertex3f(0.1*F,0,0);
	    glVertex3f(0.4*F,S,S);
	    glVertex3f(0.4*F,-S,S);
	    glVertex3f(0.4*F,-S,-S);
	    glVertex3f(0.4*F,S,-S);
	    glVertex3f(0.4*F,S,S);
	  glEnd();

	}
	else /* rotations */
	{
	  if (plotProp.PlotToFile == AF_YES) 
    { 
#ifdef USE_GL2PS
      gl2psLineWidth(2); 
#endif
    }
	  else 
    { 
      glLineWidth(2); 
    }

#if 0
		glBegin(GL_POLYGON);
	    glVertex3f( 0.4*F, 0.3*F,-0.2*F);
	    glVertex3f(-0.4*F, 0.3*F,-0.2*F);
	    glVertex3f(-0.4*F, 0.3*F, 0.2*F);
	    glVertex3f( 0.4*F, 0.3*F, 0.2*F);
		glEnd();

		glBegin(GL_POLYGON);
	    glVertex3f( 0.4*F,-0.3*F,-0.2*F);
	    glVertex3f(-0.4*F,-0.3*F,-0.2*F);
	    glVertex3f(-0.4*F,-0.3*F, 0.2*F);
	    glVertex3f( 0.4*F,-0.3*F, 0.2*F);
		glEnd();

		glBegin(GL_POLYGON);
	    glVertex3f( 0.4*F,-0.3*F,-0.2*F);
	    glVertex3f(-0.4*F,-0.3*F,-0.2*F);
	    glVertex3f(-0.4*F, 0.3*F,-0.2*F);
	    glVertex3f( 0.4*F, 0.3*F,-0.2*F);
		glEnd();

		glBegin(GL_POLYGON);
	    glVertex3f( 0.4*F,-0.2*F, 0.2*F);
	    glVertex3f(-0.4*F,-0.2*F, 0.2*F);
	    glVertex3f(-0.4*F, 0.2*F, 0.2*F);
	    glVertex3f( 0.4*F, 0.2*F, 0.2*F);
		glEnd();

		/*    */
		glBegin(GL_POLYGON);
	    glVertex3f(0,-0.2*F,-0.2*F);
	    glVertex3f(0,-0.2*F, 0.2*F);
	    glVertex3f(0, 0.2*F, 0.2*F);
	    glVertex3f(0, 0.2*F,-0.2*F);
		glEnd();
#else
		glBegin(GL_POLYGON);
	    glVertex3f(0,-0.3*F,-0.3*F);
	    glVertex3f(0, 0.3*F, 0.3*F);
	    glVertex3f(0.5*F, 0.3*F, 0.3*F);
	    glVertex3f(0.5*F,-0.3*F,-0.3*F);
		glEnd();

		glBegin(GL_POLYGON);
	    glVertex3f(0,-0.3*F, 0.3*F);
	    glVertex3f(0, 0.3*F,-0.3*F);
	    glVertex3f(0.5*F,-0.3*F, 0.3*F);
	    glVertex3f(0.5*F, 0.3*F,-0.3*F);
		glEnd();

#endif
	
	  if (plotProp.PlotToFile == AF_YES) 
    { 
#ifdef USE_GL2PS
      gl2psLineWidth(2); 
#endif
    }
	  else 
    { 
      glLineWidth(2); 
    }
  }


  if (plotProp.Select != AF_YES)
  {
    /* disp number: */
	  if (plotProp.dispNumber == AF_YES)
	  {
		  femPlotInt(Num,F+0,0,0,NULL);
	  }

	  /* disp size: */
	  if (plotProp.dispSize == AF_YES)
	  {
		  femPlotDouble(fabs(Size),F+0,0,0,NULL);
	  }
  }
}

int femGfxOneDisp(long i)
{
	double x,y,z ;
	double r,g,b,tr ;
	long node ;
	long nodepos ;

      if ( fdbTestSelect(&InputTab[NDISP], &inputData.intfld, i) != AF_YES) 
		  { 
        return(AF_OK) ; 
      }
      else
      {
				node = fdbInputGetInt(NDISP,NDISP_NODE ,i) ;
        nodepos = fdbNdispNodePos(i) ;

				x = fdbInputGetDbl(NODE, NODE_X, nodepos) ;
				y = fdbInputGetDbl(NODE, NODE_Y, nodepos) ;
				z = fdbInputGetDbl(NODE, NODE_Z, nodepos) ;

				x = PoSX(x) ;
				y = PoSY(y) ;
				z = PoSZ(z) ;

				r = plotProp.dispColor[0] ;
				g = plotProp.dispColor[1] ;
				b = plotProp.dispColor[2] ;
				tr= plotProp.dispColor[3] ;

        if (plotProp.Select == AF_YES) { glPushName((GLuint)i) ; }
				afDispSymbol(
		 				fdbInputGetDbl(NDISP,NDISP_VAL ,i), 
		 				fdbInputGetInt(NDISP,NDISP_TYPE,i), 
		 				x, y, z, fdbInputGetInt(NDISP,NDISP_ID,i), 
		 				r, g, b, tr);

				afStiffSymbol(
		 				fdbInputGetDbl(NDISP,NDISP_VAL ,i), 
		 				fdbInputGetInt(NDISP,NDISP_TYPE,i), 
		 				x, y, z, fdbInputGetInt(NDISP,NDISP_ID,i), 
		 				r, g, b, tr);

        if (plotProp.Select == AF_YES) { glPopName() ; }
      }
  
  return(AF_OK);
}


int femGfxDisps(void)
{
	int rv = AF_OK ;
  long i ;
  long to ;
  long j;
  long ignore = AF_NO ;

	if ((to = fdbInputTabLenAll(NDISP)) > 0);
  {
    for (i=0; i<to; i++)
    {

      if ((plotProp.SelStuff == GFX_SELE_DISPS)&&(plotProp.SelAct == GFX_SELE_ACT_SELE)&&(plotProp.Select != AF_YES))
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
        if ((rv = femGfxOneDisp(i))!= AF_OK)
        {
          return(rv);
        }
      }
    }
    
  }
  return(rv);
}

int femGfxHilightedDisps(long *fld, long fldLen)
{
  float sw_col[4];
  long i ;

	glLoadIdentity();

  for (i=0;i<4; i++)
  { 
    sw_col[i] = plotProp.dispColor[i]; 
    plotProp.dispColor[i]=plotProp.hilightColor[i]; 
  }

  for (i=0; i<fldLen; i++)
  {
    femGfxOneDisp(fld[i]);
  }

  for (i=0;i<4; i++) { plotProp.dispColor[i] = sw_col[i] ; }

  return(AF_OK);
}

#endif
/* end of gfx_load.c */
