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
*/

#include "fem_gfx.h"
#ifdef _USE_GFX_

void afEloadForceSymbol(
     long    eLPos,
		 double  relSize,
		 float   r,
		 float   g,
		 float   b,
		 float   tr
    ); /* this function is located below */


/* x ---> */
void afForceSymbol(
		 double  relSize,
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
	
	F = (Size/relSize) * ((plotProp.Max-plotProp.Min)/6.0);
	S = F/16.0;
	
  glLoadIdentity();
  glTranslatef(x,y,z);

  switch (Dir)
  {
		/* u:  */
    case 1: glRotatef(180,0,0,1); break;
    case 2: glRotatef(90,0,0,-1); break;
    case 3: glRotatef(90,0,1,0); break;
		/* rot: */
	  case 4: glRotatef(90,1,0,0); break;
    case 5: glRotatef(90,0,0,-1); break;
    case 6: glRotatef(90,0,-1,0); break;
    /* heat: */
    case 7: glRotatef(0,0,0,1); break;
    default: glLoadIdentity(); return; break;
  }

	glColor4f(r,g,b,tr);


	if (Dir <= 3) /* Forces */
  {
	  if (plotProp.PlotToFile == AF_YES) 
		{ 
#ifdef USE_GL2PS
			gl2psLineWidth(1); 
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
    glVertex3f(0,0,0);
    glVertex3f(4*S,S,S);
    glVertex3f(4*S,-S,S);
    glVertex3f(4*S,-S,-S);
    glVertex3f(4*S,S,-S);
    glVertex3f(4*S,S,S);
    glEnd();
  }
  else /* Moments */
  {
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

    if (Dir == 7) /* heat */
    {
	    glBegin(GL_POINTS);
	      glVertex3f(0,0,0);
	    glEnd();

    }
    else
    {
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
      glVertex3f(0,0,-F);
      glVertex3f(0,(F),-F); /*fabs */
      glVertex3f(0,(F),0); /*fabs */
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
    glVertex3f(0,(F)+0,0); /*fabs */
    glVertex3f(S,S+(F),-4*S); /*fabs */
    glVertex3f(-S,S+(F),-4*S); /*fabs */
    glVertex3f(-S,-S+(F),-4*S); /*fabs */
    glVertex3f(S,-S+(F),-4*S); /*fabs */
    glVertex3f(S,S+(F),-4*S); /*fabs */
    glEnd();		
  }
  }
	
  if (plotProp.Select != AF_YES)
  {
	  /* force number: */
	  if (plotProp.loadNumber == AF_YES)
	  {
		  femPlotInt(Num,F+0,0,0,NULL);
	  }

	  /* force size: */
	  if (plotProp.loadSize == AF_YES)
	  {
      if (Dir == 7) /* heat */
      {
		    femPlotDouble((Size),F+0,0,0,NULL);
      }
      else /* moment */
      {
		    femPlotDouble(fabs(Size),F+0,0,0,NULL);
      }
	  }
  }
}

/** Computes multiplier for force/element load size */
void femRelForceSize(double *relSize)
{
	double max = 0 ;
	double min = 0 ;
	double max0 = 0 ;
	double min0 = 0 ;
  long   i ;

  *relSize = 0.0 ;

	if (fdbInputTabLenSel(ELVAL) > 0)
	{
		min0 = fdbInputFindMaxDbl(ELVAL, ELVAL_VAL);
		max0 = fdbInputFindMinDbl(ELVAL, ELVAL_VAL);
	}

	min = fdbInputFindMaxDbl(NLOAD, NLOAD_VAL);
	max = fdbInputFindMinDbl(NLOAD, NLOAD_VAL);

	min0= fabs(min0) ;
	min = fabs(min ) ;

	max0= fabs(max0) ;
	max = fabs(max ) ;

	if (min0 > min) {min=min0;}
	if (max0 > max) {max=max0;}

	if (min > max) {*relSize = min;}
	else           {*relSize = max;}

  /* nothing found? */
	if (*relSize == 0.0)
	{
    /* so how about reactions */
    if (femUI_Mode == FEM_UI_MODE_POST) 
    { 

      if ((ResLen <= 0) || (ResActStep >= ResLen)) 
      { 
		    *relSize = 4000 * plotProp.Max ;
      }
      else
      {
        for (i=0; i<ResReac[ResActStep].i_len; i++)
        {
		 			if (max<ResReac[ResActStep].data[i]) { max = ResReac[ResActStep].data[i] ; }
		 			if (min>ResReac[ResActStep].data[i]) { min = ResReac[ResActStep].data[i] ; }
        }
	      if (min > max) {*relSize = min;}
	      else           {*relSize = max;}
      }
    }
    else
    {
		  *relSize = 4000 * plotProp.Max ;
    }
	}
}


int femGfxOneForce(long i, double relSizeF)
{
	double x,y,z ;
	double r,g,b,tr ;
	long node ;
	long nodepos ;

      if ( fdbTestSelect(&InputTab[NLOAD], &inputData.intfld, i) != AF_YES) 
		  { 
        return(AF_OK);
      }
      else
      {
				node = fdbInputGetInt(NLOAD,NLOAD_NODE ,i) ;
        nodepos = fdbNloadNodePos(i) ;

				x = fdbInputGetDbl(NODE, NODE_X, nodepos) ;
				y = fdbInputGetDbl(NODE, NODE_Y, nodepos) ;
				z = fdbInputGetDbl(NODE, NODE_Z, nodepos) ;

				x = PoSX(x) ;
				y = PoSY(y) ;
				z = PoSZ(z) ;

				r = plotProp.loadColor[0] ;
				g = plotProp.loadColor[1] ;
				b = plotProp.loadColor[2] ;
				tr= plotProp.loadColor[3] ;

        if (plotProp.Select == AF_YES) { glPushName((GLuint)i) ; }
				afForceSymbol(
		 				relSizeF,
		 				fdbInputGetDbl(NLOAD,NLOAD_VAL ,i), 
		 				fdbInputGetInt(NLOAD,NLOAD_TYPE,i), 
		 				x, y, z, fdbInputGetInt(NLOAD,NLOAD_ID,i), 
		 				r, g, b, tr);
        if (plotProp.Select == AF_YES) { glPopName() ; }
      }
      return(AF_OK);
}

int femGfxForces(void)
{
	int rv = AF_OK ;
  long i ;
  long to ;
	double relSizeF= 0.0 ;
  long j;
  long ignore = AF_NO ;

	femRelForceSize(&relSizeF);

	if ((relSizeF ) <= 0.0)
	{
		fprintf(msgout,"[E] %s!\n", 
				_("Cannot plot nodal loads - relative size computation failed"));
		return(AF_ERR_VAL);
	}

	if ((to = fdbInputTabLenAll(NLOAD)) > 0);
  {
    for (i=0; i<to; i++)
    {

      if ((plotProp.SelStuff == GFX_SELE_FORCS)&&(plotProp.SelAct == GFX_SELE_ACT_SELE)&&(plotProp.Select != AF_YES))
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
        if ((rv = femGfxOneForce(i, relSizeF) ) != AF_OK)
        {
          return(rv);
        }
      }
    }
  }

  return(rv);
}


int femGfxEloads(void)
{
	int rv = AF_OK ;
  long i ;
  long to ;
	double relSizeF= 0.0 ;
  long j;
  long ignore = AF_NO ;

	femRelForceSize(&relSizeF);

	if ((relSizeF ) <= 0.0)
	{
		fprintf(msgout,"[E] %s!\n", 
				_("Cannot plot nodal loads - relative size computation failed"));
		return(AF_ERR_VAL);
	}

  if ((to = fdbInputTabLenAll(ELOAD)) > 0);
  {
    for (i=0; i<to; i++)
    {
      if ((plotProp.SelStuff == GFX_SELE_ELDS)&&(plotProp.SelAct == GFX_SELE_ACT_SELE)&&(plotProp.Select != AF_YES))
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
        afEloadForceSymbol(i, relSizeF,
				  plotProp.eloadColor[0],
				  plotProp.eloadColor[1],
				  plotProp.eloadColor[2],
				  plotProp.eloadColor[3]
        );
      }
    }
  }

  return(rv);
}

int femGfxHilightedForces(long *fld, long fldLen)
{
	int rv = AF_OK ;
  long i ;
	double relSizeF= 0.0 ;
  float sw_col[4];

	glLoadIdentity();

	femRelForceSize(&relSizeF);

	if ((relSizeF ) <= 0.0)
	{
		fprintf(msgout,"[E] %s!\n", 
				_("Cannot plot nodal loads - relative size computation failed"));
		return(AF_ERR_VAL);
	}

  for (i=0;i<4; i++)
  { 
    sw_col[i] = plotProp.loadColor[i]; 
    plotProp.loadColor[i]=plotProp.hilightColor[i]; 
  }

  for (i=0; i<fldLen; i++)
  {
    femGfxOneForce(fld[i], relSizeF);
  }

  for (i=0;i<4; i++) { plotProp.loadColor[i] = sw_col[i] ; }

  return(rv);
}

int femGfxHilightedEloads(long *fld, long fldLen)
{
	int rv = AF_OK ;
  long i ;
	double relSizeF= 0.0 ;
  float sw_col[4];

	glLoadIdentity();

	femRelForceSize(&relSizeF);

	if ((relSizeF ) <= 0.0)
	{
		fprintf(msgout,"[E] %s!\n", 
				_("Cannot plot nodal loads - relative size computation failed"));
		return(AF_ERR_VAL);
	}

  for (i=0;i<4; i++)
  { 
    sw_col[i] = plotProp.loadColor[i]; 
    plotProp.loadColor[i]=plotProp.hilightColor[i]; 
  }

  for (i=0; i<fldLen; i++)
  {
    afEloadForceSymbol(i, relSizeF,
				  plotProp.eloadColor[0],
				  plotProp.eloadColor[1],
				  plotProp.eloadColor[2],
				  plotProp.eloadColor[3]
        );
  }

  for (i=0;i<4; i++) { plotProp.loadColor[i] = sw_col[i] ; }

  return(rv);
}

/* TODO: more element load types*/
void afEloadForceSymbol(
     long    eLPos,
		 double  relSize,
		 float   r,
		 float   g,
		 float   b,
		 float   tr
    )
{
  double  size; 
  double  avx,avy,avz ;
  long    id ;
  long    type;
	long    dir ; 
	long    inum ; 
	long    face ; 
  long    epos, from, efrom, ettype, etpos, etype, nval ;
  long    nodes[21] ;
  int     i ;
  int     found = AF_NO ;

  id     = fdbInputGetInt(ELOAD, ELOAD_ID, eLPos) ;
  type   = fdbInputGetInt(ELOAD, ELOAD_TYPE, eLPos) ;
  epos   = fdbInputGetInt(ELOAD, ELOAD_EPOS, eLPos) ;
  efrom  = fdbInputGetInt(ELEM, ELEM_FROM, epos);
  ettype = fdbInputGetInt(ELEM, ELEM_TYPE, epos) ;
  if ( fdbInputCountInt(ETYPE, ETYPE_ID, ettype, &etpos) < 0)
  {
    fprintf(msgout, "[E] %s: %li\n", _("Element type michmach for element"), fdbInputGetInt(ELEM, ELEM_ID, epos));
    return;
  }
  etype = fdbInputGetInt(ETYPE, ETYPE_TYPE, etpos) ;

  nval = fdbInputGetInt(ELOAD, ELOAD_NVAL, eLPos) ;

  if (nval < 2)
  {
    fprintf(msgout,"[E] %s (%li)!\n",_("Invalid number of element data"), nval);
    return;
  }

  from = fdbInputGetInt(ELOAD, ELOAD_FROM, eLPos) ;

  if (from < 0)
  {
    fprintf(msgout,"[E] %s!\n",_("Invalid position of element data"));
    return;
  }

  dir  = (long)fdbInputGetDbl(ELVAL, ELVAL_VAL, from) ;
  size = fdbInputGetDbl(ELVAL, ELVAL_VAL, from+1) ;

  switch (type) /* type of load */
  {
    case 0: /* empty - testing load */
      return; break ;
    case 1:
      switch (etype)
      {
        case 3:
        case 13:
        case 16:
        case 17:
          found = AF_YES ;
          break ;
        case 2:
        case 5:
          found = AF_YES ;
          face = (long) fdbInputGetDbl(ELVAL, ELVAL_VAL, from+2) ;
          switch (face)
          {
            case 1: /* back */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              break ;
            case 2: /* front */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              break ;
            case 3: /* left */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              break ;
            case 4: /* right */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              break ;
            default:
              found = AF_NO ;
              break;
          }
          /**/
          break;
        default: break ;
      }
      if (found == AF_YES)
      {
          avx = 0.0 ;
          avy = 0.0 ;
          avz = 0.0 ;

          for (i=1; i<=2; i++)
          {
            avx += PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i]));
            avy += PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i]));
            avz += PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i]));
            afForceSymbol(
		            relSize, size, dir, 
                PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i])),
                PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i])),
                PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i])),
		            id, 
		            r, g, b, tr) ;
          }
          /* symbol in mid of element: */
          avx *= 0.5 ;
          avy *= 0.5 ;
          avz *= 0.5 ;
          afForceSymbol(relSize,size,dir,avx,avy,avz, id, r,g,b,tr) ;

          glLoadIdentity();
          glBegin(GL_LINE_LOOP);
          for (i=1; i<=2; i++)
          {
            glColor4f(r,g,b,tr);
            glVertex3f(
                PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i])),
                PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i])),
                PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i]))
                );
          }
          glEnd();
      }
      break ;

    case 2: /* Area load on  volume elements */
       switch (etype)
       {
         case 2:
         case 5:
         case 11:

          nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
          nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
          nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
          if (etype != 11)
          {
            inum = 4 ;
            nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
          }
          else
          {
            inum = 3 ; /* it's because e011 is triangle! */
          }

          avx = 0.0 ;
          avy = 0.0 ;
          avz = 0.0 ;

          for (i=1; i<=inum; i++)
          {
            avx += PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i]));
            avy += PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i]));
            avz += PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i]));
            afForceSymbol(
		            relSize, size, dir, 
                PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i])),
                PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i])),
                PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i])),
		            id, 
		            r, g, b, tr) ;
          }

          /* symbol in mid of element: */
          avx = avx / ((double)inum) ;
          avy = avy / ((double)inum) ;
          avz = avz / ((double)inum) ;
          afForceSymbol(relSize,size,dir,avx,avy,avz, id, r,g,b,tr) ;

          glLoadIdentity();
          glBegin(GL_LINE_LOOP);
          for (i=1; i<=inum; i++)
          {
            glColor4f(r,g,b,tr);
            glVertex3f(
                PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i])),
                PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i])),
                PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i]))
                );
          }
          glEnd();
        break;
         default: break ;
       }
      break;

    case 3: /* Area load on  volume elements */
       switch (etype)
       {
         case 9:
         case 10:
          face = (long) fdbInputGetDbl(ELVAL, ELVAL_VAL, from+2) ;

          switch (face)
          {
            case 1: /* back */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              break ;
            case 2: /* front */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+4) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+5) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+6) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+7) ;
              break ;
            case 3: /* left */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+4) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+7) ;
              break ;
            case 4: /* right */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+5) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+6) ;
              break ;
            case 5: /* top */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+5) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+4) ;
              break ;
            case 6: /* bottom */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+6) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+7) ;
              break ;
            default: 
                    fprintf(msgout,"[E] %s: %li!\n",_("Invalid element face"), face);
										return;
                    break;
          }

          avx = 0.0 ;
          avy = 0.0 ;
          avz = 0.0 ;

          for (i=1; i<=4; i++)
          {
            avx += PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i]));
            avy += PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i]));
            avz += PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i]));
            afForceSymbol(
		            relSize, size, dir, 
                PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i])),
                PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i])),
                PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i])),
		            id, 
		            r, g, b, tr) ;
          }

          /* symbol in mid of element: */
          avx *= 0.25 ;
          avy *= 0.25 ;
          avz *= 0.25 ;
          afForceSymbol(relSize,size,dir,avx,avy,avz, id, r,g,b,tr) ;

          glLoadIdentity();
          glBegin(GL_LINE_LOOP);
          for (i=1; i<=4; i++)
          {
            glColor4f(r,g,b,tr);
            glVertex3f(
                PoSX(fdbInputGetDbl(NODE,NODE_X, nodes[i])),
                PoSY(fdbInputGetDbl(NODE,NODE_Y, nodes[i])),
                PoSZ(fdbInputGetDbl(NODE,NODE_Z, nodes[i]))
                );
          }
          glEnd();
        break;
         default: break ;
       }
      break;
    default: /* nothing to do */ break;
  }
}

#endif
/* end of gfx_load.c */
