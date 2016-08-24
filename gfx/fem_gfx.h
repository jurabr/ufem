/*
   File name: fem_gfx.h
   Date:      2003/10/25 13:13
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

	 Graphics output for "fem" (pre|post)processor

	 $Id: fem_gfx.h,v 1.30 2005/02/21 22:05:53 jirka Exp $
*/


#ifndef __FEM_GFX_H__
#define __FEM_GFX_H__

#ifdef _USE_GFX_
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <time.h>
#endif

#ifdef USE_GL2PS
#include "gl2ps.h"
#endif

#include "cint.h"
#include "fem_tui.h"
#include "fdb_fem.h"
#include "fdb_res.h"

#define GFX_DESC_LEN   160
#define GFX_MIN_HEIGHT 60
#define GFX_POINT      4
#define GFX_SMALLPOINT 1
#define GFX_MIN_ZOOM   0.00001

#define GFX_SELE_NONE     0
#define GFX_SELE_NODES    1
#define GFX_SELE_ELEMS    2
#define GFX_SELE_DISPS    3
#define GFX_SELE_FORCS    4
#define GFX_SELE_ELDS     5
#define GFX_SELE_KPOINTS  6
#define GFX_SELE_ENTS     7
#define GFX_SELE_ENTDIV   8
#define GFX_SELE_SPACE    9 /* to select nothing - zoom etc. */
#define GFX_SELE_GRID    10 /* to use selection grid */

#define GFX_SELE_ACT_NONE   0
#define GFX_SELE_ACT_EDIT   1
#define GFX_SELE_CRT_NDISP  2
#define GFX_SELE_CRT_NLOAD  3
#define GFX_SELE_CRT_ELEM   4
#define GFX_SELE_CRT_ENTS   5
#define GFX_SELE_CRT_GR_N   6
#define GFX_SELE_CRT_GR_K   7
#define GFX_SELE_ACT_SELE   8
#define GFX_SELE_PICK_BOX   9
#define GFX_SELE_CRT_PATH  10
#define GFX_SELE_CRT_P_BE  11

#define GFX_PPM  0
#define GFX_TIFF 1

#define GFX_MAX_PLOT_VIEW 8

#ifdef _USE_GFX_
#define PoSX(Val) (Val-plotProp.DiffX)
#define PoSY(Val) (Val-plotProp.DiffY)
#define PoSZ(Val) (Val-plotProp.DiffZ)

#define gfxLineResSize(val) (val/fabs(femPal.max))*(plotProp.Max/plotStuff.lrMult)
#endif

typedef struct
{
	float bgColor[4];    /* background color                         */
	float fgColor[4];    /* foreground color - text, etc.            */
	float textColor[4];  /* text color          */
	float nodeColor[4];  /* node color          */
	float loadColor[4];  /* load color          */
	float eloadColor[4];  /* load color          */
	float dispColor[4];  /* displacement color  */
	float gridColor[4];  /* grid color  */

	float hilightColor[4];  /* hilighted item's color  */

	float reactColor[4];  /* load color          */

	int   nodeNumber;    /* AF_YES/AF_NO .. plot numbers of nodes    */
	int   elemNumber;    /* AF_YES/AF_NO .. plot numbers of elements */
	int   dispNumber;    /* AF_YES/AF_NO .. plot numbers of boundary conditions */
	int   loadNumber;    /* AF_YES/AF_NO .. plot numbers of loads (forces,...) */

	int   smallNode;     /* AF_YES/AF_NO .. if nodes will be small */

	int   dispSize;      /* AF_YES/AF_NO .. plot sizes of boundary conditions */
	int   loadSize;      /* AF_YES/AF_NO .. plot sizes of loads (forces,...) */

	int   kpNumber;      /* AF_YES/AF_NO .. plot numbers of keypoints */
	int   geNumber;    /* AF_YES/AF_NO .. plot numbers of lines    */
	int   divNumber;     /* AF_YES/AF_NO .. plot numbers of divisions */
	int   elipticPal;    /* AF_YES/AF_NO .. plot elliptic  palette (if any) */
	long    palLen;        /* lenght of color palette                  */

  int   elemColET;     /* elements are colored by element type */
  int   elemColRS;     /* elements are colored by real set */
  int   elemColMat;    /* elements are colored by material */
  int   elemColSet;    /* elements are colored by material */

  int   wireOnly;      /* only wireframe will be plotted */
  int   wireLine;      /* wireframe results on beams will be plotted */
  int   wireRes;       /* include wireframe when plotting results    */
  int   antiAlias;     /* if antialiasing is used */

	double  Max ;          
	double  Min ;

	double  DiffX ;
	double  DiffY ;
	double  DiffZ ;

  long    Select ;       /* selection mode (AF_YES|AF_NO) */
  long    SelectHl ;     /* if selection is hilighted (AF_YES|AF_NO) */
  long    SelMulti;      /* select multi or single items (AF_YES|AF_NO) */
  long    SelStuff;      /* what can be selected          */
  long    SelAct;        /* what to do with selections    */

  long    ActMot ;       /* motion mode is active (AF_YES | AF_NO) */

  long    SelRectPrev;   /* if selection box is visible */
  long    SelRectMode;   /* if selection box is actually used */

  long    PlotToFile;    /* (AF_YES|AF_NO) */
  int   viewport[4] ;  /* viewport (for gl2ps) */
  int   plotFormat;    /* format of file produced by gl2ps */
  int   saveBitmap;    /* bitmap is saved instead of (E)PS file */
  int   bitmapType;    /* bitmap type (if any) */

  int   showJobname ; /* if jobname is shown in plot window */

  char   *PlotTitle;     /* Title of plot                 */
}tPlotProp;

typedef struct
{
	double move_x ;
	double move_y ;
	double move_z ;
	double rot_x ;
	double rot_y ;
	double rot_z ;
	double zoom ;

	double move_step ; /* step for moving - in lenght units */
	double rot_step ;  /* angle step  */
	double zoom_step ; /* zoom step   */

  double max_view ; /* user-changeable fixed deformation limit */
  double max_fix_pal ; /* user-changeable fixed deformation limit */
  double min_fix_pal ; /* user-changeable fixed deformation limit */
}tPlotTran ;

typedef struct
{
  int autoreplot ; /* automatic replot    */
	int frame ; /* plot max-min frame only  */

	int node ; /* plot nodes                */
	int elem ; /* plot elements             */
	int disp ; /* plot displacements        */
	int load ; /* plot loads                */

	int kp ;   /* plot keypoints            */
	int line ; /* plot lines                */
	int area ; /* plot areas                */
	int volume;/* plot volumes              */

	int grid;  /* plot grid                 */

	int def_shape;/* plot deformed shape    */
	double dof_mult;/* multiplier for deformed shape */
	double dof_mult_usr;/* user-defined multiplier for deformed shape */

	int react;    /* plot reactions         */

	int eres;     /* plot element results   */
	int nres;     /* plot nodal results     */
	int path;     /* plot path     */
	double pzoom; /* plot path zoom    */
	long  eresType; /* type of element result */
  double lrMult;  /* line result multiplier */
}tPlotStuff ;

typedef struct
{
	long   len ;
	double max ;
	double min ;
}tColPal;

typedef struct
{
  long none ;    /* AF_YES means ignore it   */
  long width ;   /* width of GUI window      */
  long height ;  /* height of GUI window     */
  long x0 ;      /* "x" of upper left corner */
  long y0 ;      /* "y" of upper left corner */
}tGuiProp;

extern long  activePlotView ;
extern long  maxPlotView    ;
extern long  PlotViewType   ;
extern tPlotProp  plotPropFld[] ;
extern tPlotStuff plotStuffFld[];
extern tGuiProp   guiProp;

#define plotProp  plotPropFld[activePlotView]
#define plotStuff plotStuffFld[activePlotView]

#ifdef _USE_GFX_
extern tPlotTran  plotTranFld[] ;
extern tColPal    femPalFld[] ;

#define plotTran  plotTranFld[activePlotView]
#define femPal    femPalFld[activePlotView]

extern char *femPlotFile ;


extern char *femGfxCrtParam  ;
extern long *femGfxCrtFld    ;
extern long  femGfxCrtFldLen ;
extern long  femGfxCrtCount  ;
extern long  femGfxCrtEnodes ;

extern char *femGfxSelSelectCmd      ;
extern char *femGfxSelSelectParams   ;
extern long *femGfxSelSelectFld      ;
extern long  femGfxSelSelectFldLen   ;
extern long  femGfxSelSelectFldCount ;

/* lights and material stuff: */
extern float materialAmbient[]  ;
extern float materialDiffuse[]  ;
extern float materialSpecular[] ;
extern float lightAmbient[]     ;
extern float lightDiffuse[]     ;
extern float lightSpecular[]    ;
extern float lightPosition[]    ;

/* paths: */
extern long gfxActFromPath ; 
extern long gfxActToPath   ;

extern int femResetTran(void);
extern void femSetTramMaxView(double max);
extern void femSetTramColPalMaxMin(double min, double max);
extern void femDefaultPlotProp(void) ;
extern int femPrePlot(long x0, long y0, long x1, long y1, int NoClean);
extern void femPostPlot(long x0, long y0, int length, int width, int Mode);
extern int femPlotStuff(int length, int width, int Mode);


extern void femPlotString(char *Str, double X, double Y, double Z, double *RGB);
extern void femPlotStringHere(char *Str, double *RGB);
extern void femPlotDoubleHere(double Val, double *RGB);
extern void femPlotInt(int Val, double X, double Y, double Z, double *RGB);
extern void femPlotDouble(double Val, double X, double Y, double Z, double *RGB);

extern void femPlotPal(double x, double y, double lenght, double width);

extern int femSetPlotTitle(char *str);

extern void femColPalDefs(tColPal *pal);
extern void femColPalSet(tColPal *pal, long len, double max, double min);
extern int femGetResColor3f(double val, float *r, float *g, float *b);

extern void femElemColor(long pos, float *r, float *g, float *b);
extern void femGeEntColor(long pos, float *r, float *g, float *b);

extern int femPlotNodes(void);
extern int femGfxEleGeom(void);
extern int femGfxForces(void);
extern int femGfxEloads(void);
extern int femGfxDisps(void);

extern int femGfxResReacts(void);

extern double gfxDefShapeCoord(long node_pos, long node_dir);
extern void gfxSetDefShapeMult(double max0);
extern int femGfxElemResults(long res_type);
extern int femGfxNodeResults(long res_type);


extern int femPlotHilightedNodes(long *fld, long fldLen);
extern int femGfxHilightedElements(long *fld, long fldLen);
extern int femGfxHilightedForces(long *fld, long fldLen);
extern int femGfxHilightedEloads(long *fld, long fldLen);
extern int femGfxHilightedDisps(long *fld, long fldLen);

extern void femGfxCleanSelCrtStuff(void);
extern int femGfxSeleItemFuncFinish(void);
extern void femGfxCancelAllSelects(void);


extern int femPlotKpoints(void);
extern int femPlotHilightedKpoints(long *fld, long fldLen);

extern int femGfxEntity(void);
extern int femGfxHilightedEntities(long *fld, long fldLen);

extern int femGfxDrawGrid(void);

extern int femComputeViewPortProps( long ix, long iy, long *x0, long *y0, long *x, long *y) ;
extern int femViewPortSet(long set);


extern int femPlotPathItem(long res_type);

#endif
#endif

/* end of fem_gfx.h */
