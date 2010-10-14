/*
   File name: fem_gclr.c
   Date:      2003/11/21 09:43
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

	 Graphics output for "fem" (pre|post)processor - colormaps
*/

#include "fem_gfx.h"
#ifdef _USE_GFX_

#define GFX_PAL_SIZE 16

tColPal femPalFld[GFX_MAX_PLOT_VIEW] ;
long femElemHilightColor = AF_NO ; /* if == AF_YES then hilightColor is used as elem. color*/

/** Computes R,G,B parts of color for value "val" <0,1> 
 *  @param val size <0,1>
 *  @param r red part of color (result)
 *  @param g green part of color (result)
 *  @param b blue part of color (result)
 *  @result status
 */
int femResColor3f(double val0, float *r, float *g, float *b)
{
	double a ;
	double val ;
	long   i ;

	val = val0 ;

	for (i=0; i< femPal.len; i++)
	{
		if ((val0 >= ((double)i/(double)femPal.len)) && (val0 <= ((double)(i+1)/(double)femPal.len)))
		{
			if (val0 < 0.5)
			{
				val = ((double)i/(double)femPal.len) ;
			}
			else
			{
				val = ((double)(i+1)/(double)femPal.len) ;
			}
			break ;
		}
	}

#if 0
	if ((val < 0.0) || (val > 1.0))
	{
		fprintf(msgout,"[E] %s!\n", _("Size specification out of interval")) ;
		return(AF_ERR_VAL);
	}
#else
	if (val < 0.0) {val = 0.0;}
	if (val > 1.0) {val = 1.0;}
#endif

  if (plotProp.elipticPal == AF_YES)
  {
	  /* eliptic palette */
	  a = 0.5 * sqrt(2.0) * cos (FEM_PI*val) ;

	  *r = 0.5 - (a/sqrt(2.0)) ;
	  *g = sin(FEM_PI*val) ;
	  *b = 0.5 + (a/sqrt(2.0)) ;
    }
  else
  {
	  /* bilinear */
	  if (val <= 0.5)
	  {
		  *r = 0.0 ;
		  *g = 2.0 * val ;
		  *b = 2.0*(0.5 - val) ;
	  }
	  else
	  {
		  *r = 2.0 * (val - 0.5)  ;
		  *g = 2.0 * (1.0 - val) ;
		  *b = 0.0 ;
	  }
  }

	return(AF_OK);
}

void femColPalDefs(tColPal *pal)
{
	pal->len = GFX_PAL_SIZE ;
	pal->max =  1.0 ;
	pal->min = -1.0 ;
}

void femColPalSet(tColPal *pal, long len, double max, double min)
{
	pal->len = len ;
	pal->max = max ;
	pal->min = min ;
}

int femGetResColor3f(double val, float *r, float *g, float *b)
{
	double val_col = 0.0 ;

	if ((val > femPal.max)||(val < femPal.min))
	{
		if (val >femPal.max) { femResColor3f(1, r, g, b); }
		else { femResColor3f(0, r, g, b); }
		
		/*fprintf(msgout,"[E] %s: %e!\n",_("Value out of limits"),val);*/
		return(AF_OK);
	}
	else
	{
		if (val < 0.0)
		{
			if (femPal.min < 0.0)
			{
				val_col = 0.5 - 0.5*fabs(val)/fabs(femPal.min) ;
			}
			else
			{
				fprintf(msgout,"[E] %s: %e!\n", _("Value out of range"),val) ;
				return(AF_ERR_VAL);
			}
		}
		else
		{
			if (val == 0.0)
			{
				val_col = 0.5 ;
			}
			else
			{
				/* val > 0.0 */
				if (femPal.max < 0.0)
				{
					fprintf(msgout,"[E] %s: %e!\n", _("Value out of range"),val) ;
					return(AF_ERR_VAL);
				}
				else
				{
					val_col = 0.5 + 0.5*fabs(val)/fabs(femPal.max) ;
				}
			}
		}

		return(femResColor3f(val_col, r, g, b));
	}
}

/** Color gradient creation */
void femPlotPal(double x, double y, double lenght, double width)
{
	long   i;
	double xi,yi;
	double val ;
	float  r,g,b ;

	/* no palette wanted */
	if (femUI_Mode != FEM_UI_MODE_POST) {return ;}
	
	xi = lenght/8.0 ;
	yi = width/(double)femPal.len;

	for (i=0; i<femPal.len; i++)
	{
		glBegin(GL_POLYGON);

		if (i <= femPal.len/2)
		{
      if ((femPal.min >= 0.0)&&(i != femPal.len/2)) {continue ;}
			val = femPal.min - 2.0*((double)i / ((double)femPal.len))*femPal.min ;
		}
		else
		{
      if (femPal.max <= 0.0) {continue ;}
			val = 2.0*((double)(i - (femPal.len/2)) / ((double)femPal.len))*femPal.max  ;
		}
		femGetResColor3f(val, &r, &g, &b);
		glColor4f( r, g, b, 1.0);

		glVertex2f(x,y+i*yi);
		glVertex2f(x+xi,y+i*yi);
		glVertex2f(x+xi,y+(i+1)*yi);
		glVertex2f(x,y+(i+1)*yi);
		glEnd();
	}

	for (i=0; i<=femPal.len; i++)
	{
	  if (i <= femPal.len/2)
		{
      if ((femPal.min >= 0.0)&&(i != femPal.len/2)) {continue ;}
			val = femPal.min - 2.0*((double)i / ((double)femPal.len))*femPal.min ;
		}
		else
		{
      if (femPal.max <= 0.0) {continue ;}
			val = 2.0*((double)(i - (femPal.len/2)) / ((double)femPal.len))*femPal.max  ;
		}

		if (plotProp.PlotToFile == 0) { glColor4f(0,0,0,1); }
		else                { glColor4f(1,1,1,1); }
		glBegin(GL_LINES);
		  glVertex2f(x, y + i*yi);
		  glVertex2f(x+xi*1.0, y + i*yi);
		glEnd();

		if (plotProp.PlotToFile == 0) { glColor4f(1,1,1,1); }
		else                { glColor4f(0,0,0,1); }

		glBegin(GL_LINES);
		  glVertex2f(x+xi*1.1, y + i*yi);
		  glVertex2f(x+xi*1.3, y + i*yi);
		glEnd();
		
		if (plotProp.PlotToFile == 0)
		{
			glColor4f( plotProp.textColor[0], plotProp.textColor[1],
			           plotProp.textColor[2], plotProp.textColor[3]);
		}

#if 1
	  glRasterPos2f(x+xi*1.8, y + i*yi); /* working but ugly */
#else
	  glRasterPos2f(x+xi*1.8, y + i*yi - (0.4*yi)); /* may make problems on high resolutions */
#endif
		
	  femPlotDoubleHere(val,NULL);
	}
}

void femElemColor(long pos, float *r, float *g, float *b)
{
  ldiv_t result;
  long   num = 0 ;

  if (femElemHilightColor == AF_YES)
  {
    *r = plotProp.hilightColor[0] ;
    *g = plotProp.hilightColor[1] ;
    *b = plotProp.hilightColor[2] ;
    return;
  }

  if (plotProp.elemColET == AF_YES)
  {
    num = fdbInputGetInt(ELEM, ELEM_TYPE, pos) ;
  }

  if (plotProp.elemColRS == AF_YES)
  {
    num = fdbInputGetInt(ELEM, ELEM_RS, pos) ;
  }

  if (plotProp.elemColMat == AF_YES)
  {
    num = fdbInputGetInt(ELEM, ELEM_MAT, pos) ;
  }

  if (plotProp.elemColSet == AF_YES)
  {
    num = fdbInputGetInt(ELEM, ELEM_SET, pos) ;
  }
  
  result = ldiv(num, 6) ;

  switch (result.rem)
  {
    case 1: 
            *r = 0.4;
            *g = 0.5;
            *b = 0.4;
            break;
    case 2: 
            *r = 0.6;
            *g = 0.6;
            *b = 0.7;
            break;
    case 3: 
            *r = 0.5;
            *g = 0.4;
            *b = 0.4;
            break;
    case 4: 
            *r = 0.4;
            *g = 0.4;
            *b = 0.6;
            break;
    case 5: 
            *r = 0.4;
            *g = 0.0;
            *b = 0.4;
            break;
    default:
             *r = 0.4;
             *g = 0.4;
             *b = 0.4;
            break; 
  }
}


void femGeEntColor(long pos, float *r, float *g, float *b)
{
  ldiv_t result;
  long   num = 0 ;

  if (femElemHilightColor == AF_YES)
  {
    *r = plotProp.hilightColor[0] ;
    *g = plotProp.hilightColor[1] ;
    *b = plotProp.hilightColor[2] ;
    return;
  }

  if (plotProp.elemColET == AF_YES)
  {
    num = fdbInputGetInt(ENTITY, ENTITY_TYPE, pos) ;
  }

  if (plotProp.elemColRS == AF_YES)
  {
    num = fdbInputGetInt(ENTITY, ENTITY_RS, pos) ;
  }

  if (plotProp.elemColMat == AF_YES)
  {
    num = fdbInputGetInt(ENTITY, ENTITY_MAT, pos) ;
  }

  if (plotProp.elemColSet == AF_YES)
  {
    num = fdbInputGetInt(ENTITY, ENTITY_SET, pos) ;
  }
  

  
  result = ldiv(num, 6) ;

  switch (result.rem)
  {
    case 1: 
            *r = 0.4;
            *g = 0.5;
            *b = 0.4;
            break;
    case 2: 
            *r = 0.6;
            *g = 0.6;
            *b = 0.7;
            break;
    case 3: 
            *r = 0.5;
            *g = 0.4;
            *b = 0.4;
            break;
    case 4: 
            *r = 0.4;
            *g = 0.4;
            *b = 0.6;
            break;
    case 5: 
            *r = 0.4;
            *g = 0.0;
            *b = 0.4;
            break;
    default:
             *r = 0.4;
             *g = 0.4;
             *b = 0.4;
            break; 
  }
}

#endif
/* end of fem_gclr.c */
