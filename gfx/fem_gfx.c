/*
   File name: fem_gfx.c
   Date:      2003/10/25 13:11
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

	 Graphics output for "fem" (pre|post)processor - main file

*/

#include "fem_gfx.h"
#ifdef _USE_GFX_
#include "fem_gele.h"
#endif

/* lights and material stuff: */
#ifdef _USE_LIGHTS_
  GLfloat light0_pos[4]   = { -50.0, 50.0, 50.0, 0.0 };
  GLfloat light0_color[4] = { .9, .9, .9, 1.0 }; /* white light */
  GLfloat light1_pos[4]   = {  50.0, 50.0, 0.0, 0.0 };
  GLfloat light1_color[4] = { .9, .9, 1, 1.0 };  /* cold blue light */
#endif

long  activePlotView = 0 ; /* what view is used */
long  maxPlotView    = 1 ; /* number of actually available views */
long  PlotViewType   = 0 ; /* type of view */

tPlotProp  plotPropFld[GFX_MAX_PLOT_VIEW] ;
tPlotStuff plotStuffFld[GFX_MAX_PLOT_VIEW];
tGuiProp   guiProp  ;
#ifdef _USE_GFX_
tPlotTran  plotTranFld[GFX_MAX_PLOT_VIEW] ;

char *femPlotFile = NULL ;

void femCoordCross(int full); /* prototype of coordinate cross drawing */

/** Computes viewport parameters
 * @param ix actual window width
 * @param iy actual window height
 * @param x0 computed x0
 * @param y0 computed y0
 * @param x computed width
 * @param y computed height
 * @return status
 */
int femComputeViewPortProps(
    long ix, 
    long iy, 
    long *x0, 
    long *y0, 
    long *x,
    long *y)
{
  int rv = AF_OK ;
	long valx1 ;

  switch (PlotViewType)
  {
    case 1: /* 1 row 2 collumns */
      switch (activePlotView)
      {
        case 1:
          *x0 = (long) (ix/2) + 1 ;
          *y0 = 0 ;
          *x  = ix - *x0 ;
          *y  = iy ;
          break;
        case 0: 
        default:
          *x0 = 0 ;
          *y0 = 0 ;
          *x  = (long) (ix/2) - 1 ;
          *y  = iy ;
          break;
      }
      break;

    case 2: /* 2 rows 1 collumn */
      switch (activePlotView)
      {
        case 1:
          *x0 = 0;
          *y0 = (long) (iy/2) + 1 ;
          *x  = ix - *x0 ;
          *y  = iy - *y0 ;
          break;
        case 0: 
        default:
          *x0 = 0 ;
          *y0 = 0 ;
          *x  = ix - *x0 ;
          *y  = (long) (iy/2) - 1 - *y0 ;
          break;
      }
      break;

    case 3: /* 2 rows 2 collumn */
      switch (activePlotView)
      {
        case 1:
          *x0  = (long) (ix/2) + 1 ;
          *y0 = 0 ;
          *x  = ix - *x0 ;
          *y  = (long) (iy/2) + 1 - *y0 ;
          break;

        case 2:
          *x0  = 0 ;
          *y0 = (long) (iy/2) - 1 ;
          *x  = (long) (ix/2) - 1 - *x0 ;
          *y  = iy - *y0 ;
          break;

        case 3:
          *x0  = (long) (ix/2) + 1 ;
          *y0 = (long) (iy/2) + 1 ;
          *x  = ix - *x0 ;
          *y  = iy - *y0 ;
          break;

        case 0: 
        default:
          *x0 = 0 ;
          *y0 = 0 ;
          *x  = (long) (ix/2) - 1 - *x0 ;
          *y  = (long) (iy/2) - 1 - *y0 ;
          break;
      }
      break;


    case 4: /* 1x1 + 2x1 */
			/* compute 2nd collumn size: */
			valx1 = (long)((double)(ix-2*GFX_DESC_LEN-50)*(0.66)) + GFX_DESC_LEN;

      switch (activePlotView)
      {
        case 1:
          *x0  = (long) valx1 + 1 ;
          *y0 = 0 ;
          *x  = ix - *x0 ;
          *y  = (long) (iy/2) + 1 - *y0 ;
          break;

        case 2:
          *x0 =  (long) valx1 + 1 ;
          *y0 = (long) (iy/2) - 1 ;
          *x  = ix - *x0 ;
          *y  = iy - *y0 ;
          break;

        case 0: 
        default:
          *x0 = 0 ;
          *y0 = 0 ;
          *x  = valx1 ;
          *y  = iy - *y0 ;
          break;
      }
      break;


    case 5: /* 1x1 + 1x2 */
      switch (activePlotView)
      {
        case 1:
          *x0  = 0 ;
          *y0 = 0 ;
          *x  = (long) (ix/2) - 1 - *x0 ;
          *y  = (long) (iy/2) - 1 - *y0 ;
          break;

        case 2:
          *x0 = (long) (ix/2) - 1 ;
          *y0 = 0 ;
          *x  = ix - *x0 ;
          *y  = (long) (iy/2) - 1 - *y0 ;
          break;

        case 0: 
        default:
          *x0 = 0 ;
          *y0 = (long) (iy/2) + 1 ;
          *x  = ix - *x0 ;
          *y  = iy - *y0 ;
          break;
      }
      break;


    case 6: /* 1x1 + 3x1 */
			/* compute 2nd collumn size: */
			valx1 = (long)((double)(ix-2*GFX_DESC_LEN-50)*(0.66)) + GFX_DESC_LEN;

      switch (activePlotView)
      {
        case 1:
          *x0  = (long) valx1 + 1 ;
          *y0 = 0 ;
          *x  = ix - *x0 ;
          *y  = (long) (iy/3) - 1 - *y0 ;
          break;

        case 2:
          *x0 =  (long) valx1 + 1 ;
          *y0 = (long) (iy/3) + 1 ;
          *x  = ix - *x0 ;
          *y  = (long) (iy/3) - 1 ;
          break;

        case 3:
          *x0 =  (long) valx1 + 1 ;
          *y0 = (long) (2*iy/3) + 1 ;
          *x  = ix - *x0 ;
          *y  = iy - *y0 ;
          break;


        case 0: 
        default:
          *x0 = 0 ;
          *y0 = 0 ;
          *x  = valx1 ;
          *y  = iy - *y0 ;
          break;
      }
      break;


    case 7: /* 1x1 + 1x3 */
      switch (activePlotView)
      {
        case 1:
          *x0  = 0 ;
          *y0 = 0 ;
          *x  = (long) (ix/3) - 1 ;
          *y  = (long) (iy/3) - 1 - *y0 ;
          break;

        case 2:
          *x0 = (long) (ix/3) + 1 ;
          *y0 = 0 ;
          *x  =  (long) (ix/3) - 1 ;
          *y  = (long) (iy/3) - 1 - *y0 ;
          break;

        case 3:
          *x0 = (long) (2*ix/3) - 1 ;
          *y0 = 0 ;
          *x  = ix - *x0 ;
          *y  = (long) (iy/3) - 1 - *y0 ;
          break;

        case 0: 
        default:
          *x0 = 0 ;
          *y0 = (long) (iy/3) + 1 ;
          *x  = ix - *x0 ;
          *y  = iy - *y0 ;
          break;
      }
      break;



    case 0 : /* single window, full screen */
    default:
      *x0 = 0 ;
      *y0 = 0 ;
      *x =  ix - *x0 ;
      *y = iy - *y0 ;
      break;
  }

  return(rv);
}

/** Sets viewport parameters for a given viewport type
 * @param set type ov viewport to be set
 * @return status
 */
int femViewPortSet(long set)
{
  int rv = AF_OK ;

  switch (set)
  {
    case 1:
      PlotViewType   = 1 ;
      maxPlotView    = 2 ;
      activePlotView = 0 ;
      break;

    case 2:
      PlotViewType   = 2 ;
      maxPlotView    = 2 ;
      activePlotView = 0 ;
      break;

    case 3:
      PlotViewType   = 3 ;
      maxPlotView    = 4 ;
      activePlotView = 0 ;
      break;

    case 4:
      PlotViewType   = 4 ;
      maxPlotView    = 3 ;
      activePlotView = 0 ;
      break;

    case 5:
      PlotViewType   = 5 ;
      maxPlotView    = 3 ;
      activePlotView = 0 ;
      break;

    case 6:
      PlotViewType   = 6 ;
      maxPlotView    = 4 ;
      activePlotView = 0 ;
      break;

    case 7:
      PlotViewType   = 7 ;
      maxPlotView    = 4 ;
      activePlotView = 0 ;
      break;

    case 0:
    default:
      PlotViewType   = 0 ;
      maxPlotView    = 1 ;
      activePlotView = 0 ;
      break;
  }

  return(rv);
}

/** Computes additional parameters (zoom and translation) for boxed view 
 * @param xb box left x
 * @param yb box bottom y
 * @param wb box width
 * @param hb box height
 * @param x0 window left x
 * @param y0 window bottom y
 * @param w0 window width
 * @param h0 window height
 * @return status
 */
int femComputeZoomBox(int xb, int yb, int wb, int hb, int x0, int y0, int w0, int h0)
{
  double xc, yc ;
  double xc0, yc0 ;
  double xmove, ymove, sizemult ;
  double zoomx, zoomy ;

  /* some aspect ratio code needed!!!!! */

#ifdef DEVEL_VERBOSE
fprintf(msgout,"Box:%i %i %i %i|%i %i %i %i\n",
    xb, yb, wb, hb, x0, y0, w0, h0);
#endif


#if 1
  sizemult = plotProp.Max - plotProp.Min + 0.4*(plotProp.Max-plotProp.Min) ;
#else
  sizemult = plotProp.Max - plotProp.Min ;
#endif

#if 1
  sizemult /= plotTran.zoom ;
#endif

  /* center of box: */
  xc = (double)xb+0.5*((double)wb) ;
  yc = (double)yb+0.5*((double)hb) ;

  xc0 = (double)x0+0.5*((double)w0) ;
  yc0 = (double)y0+0.5*((double)h0) ;

  /* moving parameters (to fill move_x and move_y): */
#if 1
  xmove = sizemult*(xc0 - 1.2*xc)/fabs((double)w0);
  ymove = sizemult*(yc0 - yc)/fabs((double)h0);
#else
  xmove = sizemult*(x0 - xb)/fabs((double)w0);
  ymove = sizemult*(y0 - yb)/fabs((double)h0);
#endif

  /* zoom */
  zoomx = fabs ( (double)w0 / (double)wb ) ;
  zoomy = fabs ( (double)h0 / (double)hb ) ;

#if 1
  if (zoomx < zoomy) { zoomx = zoomy ; }
  if (zoomx < 1.0)  { return(AF_ERR_VAL);} /* for _stupid_ users */
#endif

#if 1
  zoomx *= 0.8 ;
#endif

  /* filling of translation data (modifies original) */
  plotTran.move_x += (xmove/1) ;
  plotTran.move_y -= (ymove/1) ;
#if 1
  plotTran.zoom   *= zoomx ; /* untested - is it OK? */
#endif

  return(AF_OK);
}

int femTransView(void)
{
  glScalef(plotTran.zoom,
           plotTran.zoom,
           plotTran.zoom);

  glTranslatef(plotTran.move_x,
               plotTran.move_y,
               plotTran.move_z);

  glRotatef(plotTran.rot_x, 1.0, 0.0, 0.0) ; /* x  */
  glRotatef(plotTran.rot_y, 0.0, 1.0, 0.0) ; /* y  */
  glRotatef(plotTran.rot_z, 0.0, 0.0, 1.0) ; /* z  */
  return(AF_OK);
}

/** Sets default values for tranformations */
int femResetTran(void)
{
  plotTran.move_x = 0.0 ;
	plotTran.move_y = 0.0 ;
	plotTran.move_z = 0.0 ;

	plotTran.rot_x = 0.0 ; /* in DEG !! */
	plotTran.rot_y = 0.0 ;
	plotTran.rot_z = 0.0 ;

	plotTran.zoom  = 1.0 ;

  plotTran.move_step = 1.0 ;
  plotTran.rot_step  = 1.0 ;
  plotTran.zoom_step = 0.1 ;

  plotTran.max_view = 0.0 ;
  plotTran.max_fix_pal = 0.0 ;
  plotTran.min_fix_pal = 0.0 ;
  return(AF_OK);
}

void femSetTramMaxView(double max)
{
  if (fabs(max) < FEM_ZERO)
  {
    plotTran.max_view = 0.0 ;
  }
  else
  {
    plotTran.max_view = fabs(max) ;
  }
}

void femSetTramColPalMaxMin(double min, double max)
{
  if (fabs(max) < FEM_ZERO)
  {
    plotTran.max_fix_pal = 0.0 ;
    plotTran.min_fix_pal = 0.0 ;
  }
  else
  {
    plotTran.max_fix_pal = fabs(max) ;
    plotTran.min_fix_pal = min ;
  }
}

#endif

/** Sets title of plot */
int femSetPlotTitle(char *str)
{
  int len = 0 ;
  if (plotProp.PlotTitle != NULL)
  {
    free(plotProp.PlotTitle) ;
    plotProp.PlotTitle = NULL ;
  }

  len = strlen(str) ;
  if (len <1) {return(AF_ERR_EMP);}

  if ((plotProp.PlotTitle = (char *)malloc((len+1)*sizeof(char))) == NULL)
  {
    return(AF_ERR_MEM) ;
  }
  else
  {
    strcpy(plotProp.PlotTitle, str);
  }

  return(AF_OK);
}

void femDefaultPlotProp(void)
{
  long i ;
#ifdef _USE_GFX_
  femInitGfxElements();
#endif

  for (i=0; i<GFX_MAX_PLOT_VIEW; i++)
  {
    activePlotView = i ;
  
	plotProp.bgColor[0] = 0.0 ;
	plotProp.bgColor[1] = 0.0 ;
	plotProp.bgColor[2] = 0.0 ;
	plotProp.bgColor[3] = 1.0 ;

	plotProp.fgColor[0] = 0.6 ;
	plotProp.fgColor[1] = 1.0 ;
	plotProp.fgColor[2] = 1.0 ;
	plotProp.fgColor[3] = 1.0 ;

	plotProp.textColor[0] = 1.0 ;
	plotProp.textColor[1] = 1.0 ;
	plotProp.textColor[2] = 1.0 ;
	plotProp.textColor[3] = 1.0 ;

	plotProp.nodeColor[0] = 0.8 ;
	plotProp.nodeColor[1] = 0.8 ;
	plotProp.nodeColor[2] = 0.3 ;
	plotProp.nodeColor[3] = 1.0 ;

	plotProp.loadColor[0] = 1.0 ;
	plotProp.loadColor[1] = 0.0 ;
	plotProp.loadColor[2] = 0.0 ;
	plotProp.loadColor[3] = 1.0 ;

	plotProp.eloadColor[0] = 1.0 ;
	plotProp.eloadColor[1] = 0.3 ;
	plotProp.eloadColor[2] = 0.05 ;
	plotProp.eloadColor[3] = 1.0 ;

	plotProp.dispColor[0] = 0.0 ;
	plotProp.dispColor[1] = 1.0 ;
	plotProp.dispColor[2] = 1.0 ;
	plotProp.dispColor[3] = 1.0 ;

	plotProp.hilightColor[0] = 0.0 ;
	plotProp.hilightColor[1] = 1.0 ;
	plotProp.hilightColor[2] = 0.0 ;
	plotProp.hilightColor[3] = 1.0 ;

	plotProp.gridColor[0] = 1.0 ;
	plotProp.gridColor[1] = 0.0 ;
	plotProp.gridColor[2] = 0.0 ;
	plotProp.gridColor[3] = 1.0 ;

	plotProp.reactColor[0] = 0.0 ;
	plotProp.reactColor[1] = 0.0 ;
	plotProp.reactColor[2] = 1.0 ;
	plotProp.reactColor[3] = 1.0 ;

	plotProp.nodeNumber = AF_NO ;
	plotProp.elemNumber = AF_NO ;
	plotProp.dispNumber = AF_NO ;
	plotProp.loadNumber = AF_NO ;

	plotProp.smallNode  = AF_NO ;

	plotProp.dispSize    = AF_NO ;
	plotProp.loadSize    = AF_NO ;

	plotProp.kpNumber     = AF_NO ;
	plotProp.geNumber     = AF_NO ;

	plotProp.divNumber    = AF_YES ;

	plotProp.elipticPal   = AF_YES ;
	plotProp.palLen       = 16 ;

  plotProp.elemColET    = AF_NO ;
  plotProp.elemColRS    = AF_NO ;
  plotProp.elemColMat   = AF_YES ;
  plotProp.elemColSet   = AF_NO ;

  plotProp.wireOnly     = AF_NO ;
  plotProp.wireLine     = AF_NO ;
  plotProp.wireRes      = AF_YES ;

  plotProp.antiAlias    = AF_NO ;

	plotProp.Max   =  1.0 ;
	plotProp.Min   = -1.0 ;

	plotProp.DiffX = 0.0 ;
	plotProp.DiffX = 0.0 ;
	plotProp.DiffX = 0.0 ;

	plotProp.Select   = AF_NO ;
	plotProp.SelectHl = AF_NO ;
	plotProp.SelMulti = AF_NO ;
	plotProp.SelStuff = GFX_SELE_NONE ;
	plotProp.SelAct = GFX_SELE_ACT_NONE ;

	plotProp.ActMot = AF_YES ;

	plotProp.SelRectPrev = AF_NO ;
	plotProp.SelRectMode = AF_YES ;

	plotProp.PlotToFile = AF_NO ;
#ifdef USE_GL2PS
	plotProp.plotFormat = GL2PS_EPS ;
#else
	plotProp.plotFormat = 0 ;
#endif
	plotProp.saveBitmap = AF_NO ;
	plotProp.bitmapType = GFX_PPM ;

  plotProp.showJobname = AF_NO ;

  plotProp.PlotTitle = NULL ;
  femSetPlotTitle(fdbCSysTypeStr()) ;

#ifdef _USE_GFX_
	femColPalDefs(&femPal);

  femResetTran() ;
#endif


	plotStuff.autoreplot = AF_YES ;

	plotStuff.frame = AF_YES ;

	plotStuff.node = AF_YES ;
	plotStuff.elem = AF_YES ;
	plotStuff.disp = AF_YES ;
	plotStuff.load = AF_YES ;

	plotStuff.kp     = AF_NO ;
	plotStuff.line   = AF_NO ;
	plotStuff.area   = AF_NO ;
	plotStuff.volume = AF_NO ;

	plotStuff.grid   = AF_NO ;

	plotStuff.def_shape = AF_NO ;
	plotStuff.dof_mult  =   1.0 ;
	plotStuff.dof_mult_usr= 1.0 ;
	plotStuff.react     = AF_NO ;
	plotStuff.eres      = AF_NO ;
	plotStuff.nres      = AF_NO ;
	plotStuff.path      = AF_NO ;
	plotStuff.pzoom     = 1.0   ;
	plotStuff.eresType  = 0     ;
	plotStuff.lrMult    = 20    ;

	femUI_Mode = FEM_UI_MODE_PREP ;

  guiProp.none        = AF_YES;
  guiProp.width       =   0   ;
  guiProp.height      =   0   ;
  guiProp.x0          =  -1   ;
  guiProp.y0          =  -1   ;
  }
  activePlotView = 0 ;
}

#ifdef _USE_GFX_
/* Sets minimum and maximum positions */
int plotSetMinMax(long tab_id, long col_x, long col_y, long col_z)
{
	double Min,Max;
	double MMin[3],MMax[3];
	double Aver,Diff,Sum;
	double DiffX,DiffY,DiffZ;
	long   i;
	
	MMax[0] = fdbInputFindMaxDbl(tab_id, col_x) ;
	MMin[0] = fdbInputFindMinDbl(tab_id, col_x) ;

	MMax[1] = fdbInputFindMaxDbl(tab_id, col_y) ;
	MMin[1] = fdbInputFindMinDbl(tab_id, col_y) ;

	MMax[2] = fdbInputFindMaxDbl(tab_id, col_z) ;
	MMin[2] = fdbInputFindMinDbl(tab_id, col_z) ;

	Max = MMax[0];
	Min = MMin[0];

	for (i=1; i<3; i++)
	{
		if (Max < MMax[i]) {Max = MMax[i]; }
		if (Min < MMin[i]) {Min = MMin[i]; }
	}


	Aver=(Max+Min)/2; /* "center" of the structure  */
	Sum = (Max-Min)/2;

	Diff = Aver;

	DiffX = (MMax[0] + MMin[0]) / 2.0 ;
	DiffY = (MMax[1] + MMin[1]) / 2.0 ;
	DiffZ = (MMax[2] + MMin[2]) / 2.0 ;

	plotProp.Min = -Sum;
	plotProp.Max =  Sum;

	plotProp.DiffX = DiffX;
	plotProp.DiffY = DiffY;
	plotProp.DiffZ = DiffZ;

	if (fabs(Sum) <= 0.0)
	{
		plotProp.Min = -1.0;
		plotProp.Max =  1.0;
	}

	return(AF_OK);
}


/** Prepares plotting */
int femPrePlot(long x0, long y0, long x1, long y1, int NoClean)
{
   double Ratio=1;
   double Min,Max;
   double zPom=1;
#ifdef USE_FOG 
  /* FOG - useless but cool: */
   GLfloat fogColor[4] = {0.5, 0.5, 0.9, 1.0};
   GLuint  fogMode = GL_EXP ;
#endif

   if (activePlotView == 0)
   {
#ifdef USE_FOG 
  if (plotProp.Select != AF_YES)
  {
    glEnable(GL_FOG) ;
    glFogi (GL_FOG_MODE, fogMode) ;
    glFogfv(GL_FOG_COLOR, fogColor) ;
    glFogf(GL_FOG_DENSITY, 0.75) ;
    glHint(GL_FOG_HINT, GL_DONT_CARE) ;
    glFogf(GL_FOG_START, 1.0) ;
    glFogf(GL_FOG_END, 5.0) ;

    glClearColor(0.5, 0.5, 0.5, 1.0);
  }
#endif

#ifdef _USE_LIGHTS_
  /* light */
  glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_color);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
  glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_color);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);

  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10000.0);
  glEnable(GL_COLOR_MATERIAL);
#endif

   }

  if (plotProp.Select != AF_YES)
  {
    if (activePlotView == 0)
    {
      
 	  /* remove back faces */
    glDisable(GL_CULL_FACE);
	
#if 1
 	  glEnable(GL_DEPTH_TEST); /* Makes blank screen on slack :-(( */
    glClearDepth(1.0) ;
    glDepthFunc(GL_LEQUAL); /* GL_LESS was removed .. bad results */
#endif

    /* comment this for realistic fog (e.g. no black background) */
    if (plotProp.PlotToFile == AF_YES)
    {
		    glClearColor(1,1,1,1);
    }
    else
    {
	    if (NoClean != AF_YES) 
	    { 
		    glClearColor( plotProp.bgColor[0], plotProp.bgColor[1],
		    plotProp.bgColor[2], plotProp.bgColor[3]) ;
	    }
    }

    /* clearing of the drawing color buffer and depth buffers */
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 	  /* speedups */
#if 1
    glEnable(GL_DITHER);
 	  glShadeModel(GL_SMOOTH);
	  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	  glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
#endif

    /* antialiasing */
    if (plotProp.antiAlias == AF_YES)
    {
#if 0
      glEnable(GL_POINT_SMOOTH) ; 
      /*glEnable(GL_LINE_SMOOTH) ;*/ /* it makes forces to be black !? */
#endif
    }
    else
    {
      glDisable(GL_POINT_SMOOTH) ;
      glDisable(GL_LINE_SMOOTH) ;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); /* for text - alignment of bytes */

	  if ((x1 < (GFX_DESC_LEN + 50))||(y1 < GFX_MIN_HEIGHT))
	  {
		  /* canvas is too small */
		  fprintf(msgout,"[E] %s!\n", _("Canvas too small - please make graphics window larger"));
		  return(AF_ERR_SIZ);
	  }
    }

	  glLoadIdentity();

	  glViewport( x0+5, y0+5, (x1 - (GFX_DESC_LEN + 10)), (y1 - 10) ); 

    glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();

	  glOrtho(0,1, 0,1, -1,1);
	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity(); 

	  /* frame color: */
	  if (plotProp.PlotToFile == AF_YES) { glColor4f(0,0,0,1); }
	  else                
	  { 
		  glColor4f( plotProp.fgColor[0], plotProp.fgColor[1], 
		  plotProp.fgColor[2], plotProp.fgColor[3]); 
	  }

		/* frame around drawing */
    if (plotStuff.frame == AF_YES)
    {
		  glBegin(GL_LINE_LOOP);
		  glVertex2f(0.0,0.0);
		  glVertex2f(1.0,0.0);
		  glVertex2f(1.0,1.0);
		  glVertex2f(0.0,1.0);
		  glEnd();
    }

	  /* viewport for structure */
	  glViewport( x0+10, y0+10, 
	  (x1 - (GFX_DESC_LEN + 20)), 
	  (y1 - 20) ); 

    /* Jobname print: */
    if (femJobname != NULL)
    {
	    if (plotProp.PlotToFile == 1) { glColor4f(0,0,0,1); }
	    else 
	    { 
		    glColor4f( plotProp.textColor[0], plotProp.textColor[1],
				          plotProp.textColor[2], plotProp.textColor[3]); 
	    }
	    glRasterPos2f(0.0,0.0);
	    femPlotStringHere(femJobname, NULL);
    }
	}
	
	Ratio = ((double)x1 - (20.0 + (double)GFX_DESC_LEN)) / ((double)y1 - 20.0 );

	/* should by changed to something complicated after keypoints
	   will be coded: */
  if (plotStuff.grid == AF_YES)
  {
	  plotProp.Min =  0;
    if ((double)pick_grid_siz_x*pick_grid_dx > (double)pick_grid_siz_y*pick_grid_dy)
         { plotProp.Max =  pick_grid_siz_x*(double)pick_grid_dx; }
    else { plotProp.Max =  pick_grid_siz_y*(double)pick_grid_dy; }

	  plotProp.DiffX = pick_grid_x0;
	  plotProp.DiffY = pick_grid_y0;
	  plotProp.DiffZ = pick_grid_z0;
  }
  else
  {
    if (fdbInputTabLenAll(KPOINT) > 0)
    {
	    plotSetMinMax(KPOINT, KPOINT_X, KPOINT_Y, KPOINT_Z) ;
    }
    else
    {
	    plotSetMinMax(NODE, NODE_X, NODE_Y, NODE_Z) ;
    }
  }

  if (plotProp.Select != AF_YES)
  {
	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
  }

	Min = plotProp.Min - (0.20*(plotProp.Max-plotProp.Min));
	Max = plotProp.Max + (0.20*(plotProp.Max-plotProp.Min));
#if 1
	zPom = plotTran.zoom*10; /* to avoid invisibility of objects */
#endif
	
	if (Ratio > 1) /* x1 > y1 */
	{
		glOrtho(Min*Ratio,Max*Ratio,Min,Max, -(Max-Min)*zPom,Max*zPom);
	}
	else /* x1 < y1 */
	{
		glOrtho(Min,Max,Min/Ratio,Max/Ratio, -(Max-Min)*zPom,Max*zPom);
	}

  if (plotProp.Select != AF_YES)
  {
	  /* geometry transformations */
    femTransView();
	
	  /* structure: */
	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
  }

	return(AF_OK);
}

void femCoordCross(int full)
{
	double R=0.3;
	double S;
	double X=0;
	double Y=0;
	double Z=0;
	
	S = R/8;

  if (full != AF_YES) 
  { 
    X = PoSX(0) ;
    Y = PoSY(0) ;
    Z = PoSZ(0) ;
    /* glTranslatef(X,Y,Z);  */
  }
  else
  {

	  glLoadIdentity();
	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();

    glTranslatef(0.5,0.5,0);

    glRotatef(plotTran.rot_x, 1.0, 0.0, 0.0) ; /* x  */
    glRotatef(plotTran.rot_y, 0.0, 1.0, 0.0) ; /* y  */
    glRotatef(plotTran.rot_z, 0.0, 0.0, 1.0) ; /* z  */
  }

	/* X axis */
	glColor4f(1,0,0,1);
  if (full == AF_YES) { glBegin(GL_POLYGON); }
  else {glBegin(GL_LINE_STRIP); }
	glVertex3f(X,Y,Z);
	glVertex3f(X+R,Y,Z);
	glVertex3f(X,Y,Z+S);
	glEnd();
  if (full == AF_YES) { glBegin(GL_POLYGON); }
  else {glBegin(GL_LINE_STRIP); }
	glVertex3f(X,Y,Z);
	glVertex3f(X+R,Y,Z);
	glVertex3f(X,Y+S,Z);
	glEnd();

  if (full == AF_YES) {	femPlotString("x",X+R,Y,Z,NULL); }

  /* Y axis */
	glColor4f(0,0,1,1);
  if (full == AF_YES) { glBegin(GL_POLYGON); }
  else {glBegin(GL_LINE_STRIP); }
	glVertex3f(X,Y,Z);
	glVertex3f(X,Y+R,Z);
	glVertex3f(X+S,Y,Z);
	glEnd();
  if (full == AF_YES) { glBegin(GL_POLYGON); }
  else {glBegin(GL_LINE_STRIP); }
	glVertex3f(X,Y,Z);
	glVertex3f(X,Y+R,Z);
	glVertex3f(X,Y,Z+S);
	glEnd();
	
	if (full == AF_YES) {	femPlotString("y",X,Y+R,Z,NULL); }
	
  /* Z axis */
	glColor4f(0,1,0,1);
  if (full == AF_YES) { glBegin(GL_POLYGON); }
  else {glBegin(GL_LINE_STRIP); }
	glVertex3f(X,Y,Z);
	glVertex3f(X,Y,Z+R);
	glVertex3f(X+S,Y,Z);
	glEnd();
  if (full == AF_YES) { glBegin(GL_POLYGON); }
  else {glBegin(GL_LINE_STRIP); }
	glVertex3f(X,Y,Z);
	glVertex3f(X,Y,Z+R);
	glVertex3f(X,Y+S,Z);
	glEnd();
	
  if (full == AF_YES) {	femPlotString("z",X,Y,Z+R,NULL); }
}

/* Returnes actual date */
char *femActDate(void)
{	
  time_t t;
	struct tm *tt;
	char  str[40];
	static char str1[80]="";
	
	strcpy(str1,"");
	time(&t);
	tt = localtime(&t);
	strftime(str,20," %d. %m. %Y",tt);
	/*strcat(str1,_("Date"));*/
	strcat(str1,str);
  return(str1);
}

void femPostPlot(long x0, long y0,int x1, int y1, int Mode)
{
	int   fontWidth = 16 ;
  long  width, length ;
  float item = 1;
	char  str[FEM_STR_LEN+1];
	int   i;

  femCoordCross(AF_NO);

	for (i=0; i<FEM_STR_LEN; i++) { str[i] = '\0'; }

  width  = y1  ;
  length = x1  ;

	item = (float) width / (float) fontWidth ;
	item = 1/item ;
	
  glLoadIdentity();
		
	glViewport(
			x0+(length - (GFX_DESC_LEN - 5)), 
			y0+5,  
			length - (length - (GFX_DESC_LEN - 5)), 
			(width - 5));
			
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
	glOrtho(0,1, 0,1, -1,1);
	
	
	if (plotProp.PlotToFile == 1) { glColor4f(0,0,0,1); }
	else 
	{ 
		glColor4f( plotProp.textColor[0], plotProp.textColor[1],
				       plotProp.textColor[2], plotProp.textColor[3]); 
	}
	
	glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();


  glRasterPos2f(0.0 ,1.0 - 1.2*item);
  if (activePlotView == 0)
  {
	  femPlotStringHere(FEM_TUI_RELEASE,NULL);
  }
  else
  {
    sprintf(str,"%s [%i]", FEM_TUI_RELEASE, (int)activePlotView);
	  femPlotStringHere(str,NULL);
    for(i=0; i<FEM_STR_LEN+1; i++){str[i]='\0';}
  }
	
	/* time/step number: */
	if (femUI_Mode == FEM_UI_MODE_POST)
	{
		if ( (ResLen > 0) && (ResActStep >= 0) )
		   { 
         sprintf(str, "%s: %4li: %2.3f", _("Set"), 
             ResNode[ResActStep].set_id,
             ResNode[ResActStep].set_time
             ); 
       }
		else { strncat(str, _("Set unknown"), FEM_STR_LEN); }
	}
	else { sprintf(str, "%s: %li",_("Time"),fdbSetInputLoadSet(0)); }

	glRasterPos2f(0,1-(3.5*item));
	femPlotStringHere(str ,NULL);
	
	glRasterPos2f(0,1-(2.5*item));
	femPlotStringHere(plotProp.PlotTitle,NULL);

  /* print model data statistics:  */
	if (femUI_Mode == FEM_UI_MODE_PREP)
  {

    sprintf(str, "%s:%10li", _("ETyps"), fdbInputTabLenAll(ETYPE));
	  glRasterPos2f(0,1-(5.5*item)); femPlotStringHere(str,NULL);

    sprintf(str, "%s:%10li", _("RSets"), fdbInputTabLenAll(RSET));
	  glRasterPos2f(0,1-(6.5*item)); femPlotStringHere(str,NULL);

    sprintf(str, "%s:%10li", _("Mats "), fdbInputTabLenAll(MAT));
	  glRasterPos2f(0,1-(7.5*item)); femPlotStringHere(str,NULL);


    sprintf(str, "%s:%10li", _("KPs  "), fdbInputTabLenAll(KPOINT));
	  glRasterPos2f(0,1-(9.5*item)); femPlotStringHere(str,NULL);

    sprintf(str, "%s:%10li", _("GEnts"), fdbInputTabLenAll(ENTITY));
	  glRasterPos2f(0,1-(10.5*item)); femPlotStringHere(str,NULL);


    sprintf(str, "%s:%10li", _("Nodes"), fdbInputTabLenAll(NODE));
	  glRasterPos2f(0,1-(12.5*item)); femPlotStringHere(str,NULL);

    sprintf(str, "%s:%10li", _("Elems"), fdbInputTabLenAll(ELEM));
	  glRasterPos2f(0,1-(13.5*item)); femPlotStringHere(str,NULL);

    sprintf(str, "%s:%10li", _("Disps"), fdbInputTabLenAll(NDISP));
	  glRasterPos2f(0,1-(14.5*item)); femPlotStringHere(str,NULL);

    sprintf(str, "%s:%10li", _("Loads"), fdbInputTabLenAll(NLOAD));
	  glRasterPos2f(0,1-(15.5*item)); femPlotStringHere(str,NULL);

  }
	
	glRasterPos2f(0,0.01);
	femPlotStringHere(femActDate(),NULL);
	
  femPlotPal(
          0.05, (5 + 1.5 * fontWidth +  GFX_DESC_LEN/2)/ (float)width + item, 
							0.9,
				1 - (item*6.5 + ((5 + 1.5 * fontWidth +  GFX_DESC_LEN/2)/ (float)width))  
					);


	/* Plot coordinate system cross */
	glViewport(
			x0+(length - (GFX_DESC_LEN - 5)) + 5 , 
			 y0+5 + 1.5 * fontWidth,  
			GFX_DESC_LEN/2, 
			GFX_DESC_LEN/2);

  femCoordCross(AF_YES);

  if (activePlotView >= (maxPlotView-1))
  {
	/* viewport to whole canvas - gl2ps hotfix */
  	glLoadIdentity();
		glViewport(x0,y0, (int)(length)-x0,(int)(width)-y0);
    glGetIntegerv(GL_VIEWPORT, plotProp.viewport) ;
  }
}

int femPlotOneNode(long i)
{
  int rv = AF_OK ;
	double x,y,z ;

  if ( fdbTestSelect(&InputTab[NODE], &inputData.intfld, i) != AF_YES) 
	{
	  return(AF_OK); /* not selected */
	}

	 x = PoSX(fdbInputGetDbl(NODE, NODE_X, i));
	 y = PoSY(fdbInputGetDbl(NODE, NODE_Y, i));
	 z = PoSZ(fdbInputGetDbl(NODE, NODE_Z, i));
     
   if (plotProp.Select == AF_YES) { glPushName((GLuint)i) ; }
       
	 glBegin(GL_POINTS);
     glColor4f(
     plotProp.nodeColor[0],
     plotProp.nodeColor[1],
     plotProp.nodeColor[2],
     plotProp.nodeColor[3]); 
	  glVertex3f(x, y, z);
	 glEnd();

   if (plotProp.Select == AF_YES) { glPopName() ; }

	 if ((plotProp.nodeNumber == AF_YES) && (plotProp.Select != AF_YES))
	 {
		 femPlotInt( fdbInputGetInt(NODE, NODE_ID, i) , x, y, z, NULL);
	 }

  return(rv);
}

/** Plots nodes */
int femPlotNodes(void)
{
  int    rv = AF_OK ;
	long   i ;
  long   j ;
  long ignore = AF_NO ;

  if (plotProp.smallNode == AF_YES)
  {
	  glPointSize(GFX_SMALLPOINT);
  }
  else
  {
	  glPointSize(GFX_POINT);
  }
#ifdef USE_GL2PS
	if (plotProp.PlotToFile == AF_YES){gl2psPointSize(GFX_POINT);}
#endif
	
	for (i=0; i<fdbInputTabLenAll(NODE); i++) 
  {
    if ((plotProp.SelStuff == GFX_SELE_NODES)&&(plotProp.SelAct == GFX_SELE_ACT_SELE)&&(plotProp.Select != AF_YES))
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

    if ((plotProp.SelStuff == GFX_SELE_NODES)&&(plotProp.SelAct == GFX_SELE_CRT_ELEM)&&(plotProp.Select != AF_YES))
    {
      ignore = AF_NO ;

      if (femGfxCrtCount > 0)
      {
        for (j=0; j<femGfxCrtCount; j++)
        {
          if (femGfxCrtFld[j] == i) {ignore = AF_YES;}
        }
      }
    }

    if (ignore != AF_YES)
    {
      femPlotOneNode(i) ; 
    }
  }

	glPointSize(1);
#ifdef USE_GL2PS
	if (plotProp.PlotToFile == AF_YES){gl2psPointSize(1);}
#endif
  return(rv);
}

/** Plots nodes */
int femPlotHilightedNodes(long *fld, long fldLen)
{
  int    rv = AF_OK ;
#ifdef _USE_GUI_
	int    i ;
  float  sw_col[4];

  glLoadIdentity();

  /* plot hilighted nodes */
  if ( (fldLen > 0) && (fld != NULL) )
  {
	  glPointSize(2+GFX_POINT);
    
    for (i=0;i<4; i++)
    { 
      sw_col[i] = plotProp.nodeColor[i]; 
      plotProp.nodeColor[i]=plotProp.hilightColor[i]; 
    }

    for (i=0; i<fldLen; i++) { femPlotOneNode(fld[i]) ; }

    for (i=0;i<4; i++) { plotProp.nodeColor[i] = sw_col[i] ; }

	  glPointSize(1);
  }
#endif
  return(rv);
}

/** Plots various stuff */
int femPlotStuff(int length0, int width0, int Mode)
{
  int rv = AF_OK ;

  if (plotStuff.node == AF_YES)
  {
    if (femPlotNodes() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Plotting of nodes failed"));
    }
  }

  if (plotStuff.elem == AF_YES)
  {
    if (femGfxEleGeom() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Plotting of elements failed"));
    }
  }

  if (plotStuff.disp == AF_YES)
  {
    if (femGfxDisps() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Plotting of nodal boundary conditions failed"));
    }
  }

  if (plotStuff.load == AF_YES)
  {
    if (femGfxForces() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Plotting of nodal loads failed"));
		}
    if (femGfxEloads() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Plotting of element loads failed"));
		}
  }

	glLoadIdentity();

  if (plotStuff.kp == AF_YES)
  {
    if (femPlotKpoints() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Plotting of keypoints failed"));
		}
  }

  if ((plotStuff.line == AF_YES)||(plotStuff.area == AF_YES)||(plotStuff.volume == AF_YES))
  {
    if (femGfxEntity() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Plotting of geometry entities failed"));
		}
  }

  if (plotStuff.grid == AF_YES)
  {
    femGfxDrawGrid();
  }

  /** Hilighted stuff (for selections etc.) */
#ifdef _USE_GUI_
  /* plot hilighted nodes */
  if ( (femGfxCrtFldLen > 0) && (femGfxCrtFld != NULL) )
  { 
    switch (plotProp.SelAct)
    {
      case GFX_SELE_CRT_NDISP : 
      case GFX_SELE_CRT_NLOAD : 
      case GFX_SELE_CRT_ELEM : 
        femPlotHilightedNodes(femGfxCrtFld, femGfxCrtCount); break ;
      case GFX_SELE_CRT_ENTS : 
        femPlotHilightedKpoints(femGfxCrtFld, femGfxCrtCount);break ;
    /*femPlotHilightedNodes(femGfxCrtFld, femGfxCrtCount) ; */
    }
  }

  if (plotProp.SelectHl == AF_YES)
  {
    if ((femGfxSelSelectFldCount > 0) && (femGfxSelSelectFld != NULL))
    {
      switch (plotProp.SelStuff)
      {
        case GFX_SELE_NODES : femPlotHilightedNodes(femGfxSelSelectFld, femGfxSelSelectFldCount); break ;
        case GFX_SELE_ELEMS : femGfxHilightedElements(femGfxSelSelectFld, femGfxSelSelectFldCount);break ;
        case GFX_SELE_DISPS : femGfxHilightedDisps(femGfxSelSelectFld, femGfxSelSelectFldCount);break ;
        case GFX_SELE_FORCS : femGfxHilightedForces(femGfxSelSelectFld, femGfxSelSelectFldCount);break ;
        case GFX_SELE_ELDS : femGfxHilightedEloads(femGfxSelSelectFld, femGfxSelSelectFldCount);break ;
        case GFX_SELE_KPOINTS : femPlotHilightedKpoints(femGfxSelSelectFld, femGfxSelSelectFldCount);break ;
        case GFX_SELE_ENTS : femGfxHilightedEntities(femGfxSelSelectFld, femGfxSelSelectFldCount);break ;
      }
    }
  }
#endif


  /* Postprocessor ============================  */
  if (femUI_Mode == FEM_UI_MODE_POST)
  {
    gfxSetDefShapeMult(plotTran.max_view);
    
    if (plotStuff.react == AF_YES)
    {
      if ((rv=femGfxResReacts() != AF_OK))
      {
        fprintf(msgout, "[E] %s!\n", _("Plotting of reactions failed"));
		  }
    }
		
		if ((rv=plotStuff.eres == AF_YES))
    {
      if (femGfxElemResults(plotStuff.eresType) != AF_OK)
      {
        fprintf(msgout, "[E] %s!\n", _("Plotting of results on elements failed"));
		  }
    }
    
    if ((rv=plotStuff.nres == AF_YES))
    {
      if ( femGfxNodeResults (plotStuff.eresType) != AF_OK)
      {
        fprintf(msgout, "[E] %s!\n", _("Plotting of results on elements failed"));
		  }
    }

		if ((rv=plotStuff.path == AF_YES))
    {
      if (femPlotPathItem(plotStuff.eresType) != AF_OK)
      {
        fprintf(msgout, "[E] %s!\n", _("Plotting of results on path failed"));
		  }
    }
 

  } /* end of post processor */

  return(rv);
}

#endif /* _USE_GFX_*/

/** Writes GFX-related part of data to a file
 * @part fw wile stream for writing
 * @return status
 */
int femGfxWriteConfig_data(char *fname, char *atype)
{
	int rv = AF_OK ;
#ifdef _USE_GFX_
  FILE   *fw   = NULL ;

  errno = 0 ;

  if ((fw = fopen(fname,atype)) == NULL) { return(AF_ERR_IO); }


	fprintf(fw,"\n!* Graphics style properties:\n");

	/* General features: */
	fprintf(fw, "plotprop,autoReplot,%i\n", plotStuff.autoreplot);
	fprintf(fw, "plotprop,frame,%i\n", plotStuff.frame);

	fprintf(fw, "plotprop,smallNode,%i\n", plotProp.smallNode);
	fprintf(fw, "plotprop,wireOnly,%i\n", plotProp.wireOnly);
	fprintf(fw, "plotprop,wireRes,%i\n", plotProp.wireRes);
	fprintf(fw, "plotprop,wireLine,%i\n", plotProp.wireLine);

	fprintf(fw,"\n!* Numbering of items:\n");

	/* Stuff numbering: */
	fprintf(fw, "plotprop,nodeNumber,%i\n", plotProp.nodeNumber);
	fprintf(fw, "plotprop,elemNumber,%i\n", plotProp.elemNumber);
	fprintf(fw, "plotprop,dispNumber,%i\n", plotProp.dispNumber);
	fprintf(fw, "plotprop,loadNumber,%i\n", plotProp.loadNumber);
	fprintf(fw, "plotprop,kpNumber,%i\n", plotProp.kpNumber);
	fprintf(fw, "plotprop,geNumber,%i\n", plotProp.geNumber);

	/* Line division numbers (for meshing): */
	fprintf(fw, "plotprop,divNumber,%i\n", plotProp.divNumber);


	/* Data (load) sizes: */
	fprintf(fw, "plotprop,dispSize,%i\n", plotProp.dispSize);
	fprintf(fw, "plotprop,loadSize,%i\n", plotProp.loadSize);


	fprintf(fw,"\n!* Colors:\n");

	/* Colors pallet properties: */
	fprintf(fw, "plotprop,elipticPal,%i\n", plotProp.elipticPal);
	fprintf(fw, "plotprop,palLen,%li\n", plotProp.palLen);

	/* Coloring style */
	fprintf(fw, "plotprop,elemColET,%i\n", plotProp.elemColET);
	fprintf(fw, "plotprop,elemColRS,%i\n", plotProp.elemColRS);
	fprintf(fw, "plotprop,elemColMat,%i\n", plotProp.elemColMat);

	/* Stuff colors in RGB style: */
	fprintf(fw, "plotcolor,foreground,%f,%f,%f,%f\n", 
			plotProp.fgColor[0],
			plotProp.fgColor[1],
			plotProp.fgColor[2],
			plotProp.fgColor[3]);
	fprintf(fw, "plotcolor,background,%f,%f,%f,%f\n", 
			plotProp.bgColor[0],
			plotProp.bgColor[1],
			plotProp.bgColor[2],
			plotProp.bgColor[3]);
	fprintf(fw, "plotcolor,text,%f,%f,%f,%f\n", 
			plotProp.textColor[0],
			plotProp.textColor[1],
			plotProp.textColor[2],
			plotProp.textColor[3]);
	fprintf(fw, "plotcolor,node,%f,%f,%f,%f\n", 
			plotProp.nodeColor[0],
			plotProp.nodeColor[1],
			plotProp.nodeColor[2],
			plotProp.nodeColor[3]);
	fprintf(fw, "plotcolor,load,%f,%f,%f,%f\n", 
			plotProp.loadColor[0],
			plotProp.loadColor[1],
			plotProp.loadColor[2],
			plotProp.loadColor[3]);
	fprintf(fw, "plotcolor,eload,%f,%f,%f,%f\n", 
			plotProp.eloadColor[0],
			plotProp.eloadColor[1],
			plotProp.eloadColor[2],
			plotProp.eloadColor[3]);

	fprintf(fw, "plotcolor,disp,%f,%f,%f,%f\n", 
			plotProp.dispColor[0],
			plotProp.dispColor[1],
			plotProp.dispColor[2],
			plotProp.dispColor[3]);
	fprintf(fw, "plotcolor,hilight,%f,%f,%f,%f\n", 
			plotProp.hilightColor[0],
			plotProp.hilightColor[1],
			plotProp.hilightColor[2],
			plotProp.hilightColor[3]);

  if (fclose(fw) != 0) {return(AF_ERR_IO);}
#endif
	return(rv);
}


/* end of fem_gfx.c */
