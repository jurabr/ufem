/*
   File name: gfx_sele.c
   Date:      2003/12/10 19:07
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

   FEM - gfx. selections
   
   $Id: gfx_sele.c,v 1.21 2005/07/11 17:57:16 jirka Exp $
*/

#include "cint.h"
#ifdef _USE_GFX_
#include "fem_gfx.h"

extern int femTransView(void);
extern int femComputeZoomBox(int xb,int yb,int wb,int hb,int x0,int y0,int w0,int h0);

extern int femGfxEntity(void) ;

#ifdef _USE_GUI_
extern int femSetCmdLine(char *str);
extern int femPlot(void);
extern void guiShowPickFrame(void);
extern void guiHidePickFrame(void);
extern long pick_grid_siz_x ;
extern long pick_grid_siz_y ;
#endif

int use_box = AF_NO ;

char *femSelectActiveCommand = NULL ; /* currently active command */

char *femGfxCrtParam  = NULL ; /* command parameters  */
long *femGfxCrtFld    = NULL ; /* picked item field   */
long  femGfxCrtFldLen = 0 ;    /* lenght of field (number of nodes we want) */
long  femGfxCrtCount  = 0 ;    /* number of currently picked items */
long  femGfxCrtEnodes = 0 ;    /* number of nodes of created element */

char *femGfxSelSelectCmd      = NULL ;
char *femGfxSelSelectParams   = NULL ;
long *femGfxSelSelectFld      = NULL ;
long  femGfxSelSelectFldLen   = 0 ;
long  femGfxSelSelectFldCount = 0 ;

void femGfxSetActiveSelectComman(char *cmd)
{
	femSelectActiveCommand = ciAllocStr(cmd) ;
}

void femGfxUnSetActiveSelectComman(void)
{
	if (femSelectActiveCommand != NULL)
	{
		free (femSelectActiveCommand);
		femSelectActiveCommand = NULL ;
	}
	else
	{
		femSelectActiveCommand = NULL ;
	}
}



/** Cancels all selecting actions */
void femGfxCancelAllSelects(void)
{
#ifdef _USE_GUI_

	guiHidePickFrame();
  if (femGfxCrtParam != NULL)
  {
    free(femGfxCrtParam);
    femGfxCrtParam = NULL ;
  }

  if (femGfxCrtFld != NULL)
  {
    free(femGfxCrtFld);
    femGfxCrtFld = NULL ;
  }

  femGfxCrtFldLen = 0 ;
  femGfxCrtCount  = 0 ;
  femGfxCrtEnodes = 0 ;

  if (femGfxSelSelectCmd != NULL)
  {
    free(femGfxSelSelectCmd);
    femGfxSelSelectCmd = NULL ;
  }

  if (femGfxSelSelectParams != NULL)
  {
    free(femGfxSelSelectParams);
    femGfxSelSelectParams = NULL ;
  }
  
  if (femGfxSelSelectFld != NULL)
  {
    free(femGfxSelSelectFld);
    femGfxSelSelectFld = NULL ;
  }

  femGfxSelSelectFldLen   = 0 ;
  femGfxSelSelectFldCount = 0 ;

  plotProp.Select   = AF_NO ;
  plotProp.SelAct   = GFX_SELE_ACT_NONE ;
  plotProp.SelStuff = GFX_SELE_NONE ;

#endif
}

void femGfxFinishAllSelects(void)
{
#ifdef _USE_GUI_
	guiHidePickFrame();
	if (femSelectActiveCommand != NULL)
	{
		ciRunCmd(femSelectActiveCommand) ;
		free (femSelectActiveCommand);
		femSelectActiveCommand = NULL ;
	}
	else
	{
		femGfxCancelAllSelects();
	}
#endif
}

/** Selects items to field 
 * @param pos position of picked item in NODE,ELEM,...
 * @param mode AF_YES==add, AF_NO==remove
 * @return status
 * */
int femGfxSeleItemFunc(long pos, long mode)
{
#ifdef _USE_GUI_
  long len = 0;
  long i, j ;
  long found = 0 ;


  if ((femGfxSelSelectFldLen == 0) && (femGfxSelSelectFld == NULL) )
  {
    switch(plotProp.SelStuff)
    {
      case  GFX_SELE_NODES : len = fdbInputTabLenAll(NODE); break;
      case  GFX_SELE_ELEMS : len = fdbInputTabLenAll(ELEM); break;
      case  GFX_SELE_DISPS : len = fdbInputTabLenAll(NDISP); break;
      case  GFX_SELE_FORCS : len = fdbInputTabLenAll(NLOAD); break;
      case  GFX_SELE_ELDS : len = fdbInputTabLenAll(ELOAD); break;
      case  GFX_SELE_KPOINTS:len = fdbInputTabLenAll(KPOINT); break;
      case  GFX_SELE_ENTS  : len = fdbInputTabLenAll(ENTITY); break;
      case  GFX_SELE_ENTDIV: len = fdbInputTabLenAll(ENTITY); break;
      case  GFX_SELE_GRID  : len = pick_grid_siz_x*pick_grid_siz_y; break;
      default: return(AF_ERR_VAL); break ;
    }

    /* first item - must allocate field */
    if ((femGfxSelSelectFld = (long *)malloc(len*sizeof(long))) == NULL)
    {
      return(AF_ERR_MEM);
    }   

    femGfxSelSelectFldLen   = len ;
    femGfxSelSelectFldCount = 0 ;

    for (i=0; i<len; i++) { femGfxSelSelectFld[i] = -1 ; }
  }

  if (mode == AF_YES)
  {
    found = 0;

    for (i=0; i<femGfxSelSelectFldCount; i++)
    {
      if (pos == femGfxSelSelectFld[i]) 
      {
        found = 1 ;
        break;
      }
    }

    if (femGfxSelSelectFldCount >= femGfxSelSelectFldLen)
    {
      if (found != 1)
      {
        return(AF_ERR_VAL);
      }
      else
      {
        return(AF_OK);
      }
    }

    if (found != 1)
    {
      femGfxSelSelectFld[femGfxSelSelectFldCount] = pos ;
      femGfxSelSelectFldCount++ ;

      plotProp.SelectHl = AF_YES ;
    }

  }
  else
  {
    for (i=0; i<femGfxSelSelectFldCount; i++)
    {
      if (pos == femGfxSelSelectFld[i]) 
      {
        len = femGfxSelSelectFldCount ;
        for (j=i; j<(len-1); j++)
        {
          femGfxSelSelectFld[j] = femGfxSelSelectFld[j+1] ;
        }
        femGfxSelSelectFldCount--;
        return(AF_OK);
      }
    }
  }
#endif
  return(AF_OK);
}


/** Finishes selecting of items to field and provides custom action */
int femGfxSeleItemFuncFinish(void)
{
  int rv = AF_OK ;
#ifdef _USE_GUI_
  char tmp[FEM_STR_LEN+1];
  char unselstr[FEM_STR_LEN+1];
  long i;
  long tab; 
  long tab_col;
  long pos ;

  if (plotProp.SelAct == GFX_SELE_ACT_NONE) {return(AF_OK);}
  if (plotProp.SelStuff == GFX_SELE_NONE) {return(AF_OK);}
  if (femGfxSelSelectFld == NULL){return(AF_OK);}
  if (femGfxSelSelectFldCount <= 0){return(AF_OK);}
  if (femGfxSelSelectFldLen <= 0){return(AF_OK);}

  for (i=0; i<=FEM_STR_LEN; i++) { tmp[i] = '\0' ; }
  for (i=0; i<=FEM_STR_LEN; i++) { unselstr[i] = '\0' ; }

  
  switch(plotProp.SelStuff)
  {
    case  GFX_SELE_NODES : tab = NODE ; tab_col = NODE_ID ; break;
    case  GFX_SELE_ELEMS : tab = ELEM ; tab_col = ELEM_ID ; break;
    case  GFX_SELE_DISPS : tab = NDISP ; tab_col = NDISP_ID ; break;
    case  GFX_SELE_FORCS : tab = NLOAD ; tab_col = NLOAD_ID ; break;
    case  GFX_SELE_ELDS : tab = ELOAD ; tab_col = ELOAD_ID ; break;
    case  GFX_SELE_KPOINTS:tab = KPOINT; tab_col = KPOINT_ID ; break;
    case  GFX_SELE_ENTS  : tab = ENTITY; tab_col = ENTITY_ID ; break;
    case  GFX_SELE_ENTDIV: tab = ENTITY; tab_col = ENTITY_ID ; break;
    default: rv = AF_ERR_VAL ; goto memFree; break;
  }

  for (i=0; i<femGfxSelSelectFldCount; i++)
  {
    pos = femGfxSelSelectFld[i] ;
    femGfxSelSelectFld[i] = fdbInputGetInt(tab,tab_col, pos) ;
  }

  for (i=0; i<femGfxSelSelectFldCount; i++)
  {
    sprintf(tmp,"%li", femGfxSelSelectFld[i]);

    if (i == 0)
    {
      if (ciParNum(femGfxSelSelectCmd) > 1)
      {
        if (strcmp(ciGetParStr(femGfxSelSelectCmd,1),"ad") == 0)
        {
          strcpy(unselstr, ciGetParStr(femGfxSelSelectCmd,0));
          strcat(unselstr,",none");
          ciRunCmd(unselstr);
        }
      }
    }

    if (femGfxSelSelectParams != NULL)
    {

      rv = (ciRunCmd( ciStrCat3CmdPers(
            femGfxSelSelectCmd,
            tmp, 
            femGfxSelSelectParams))) ;
    }
    else
    {
      rv = (ciRunCmd( ciStrCat2CmdPers(
            femGfxSelSelectCmd,
            tmp ))) ;
    }

    if (rv != AF_OK) {goto memFree;}
  }
  
memFree:
  free (femGfxSelSelectFld) ;
  femGfxSelSelectFld = NULL;
  femGfxSelSelectFldLen = 0 ;
  femGfxSelSelectFldCount = 0 ;

  free(femGfxSelSelectCmd)  ;
  femGfxSelSelectCmd = NULL ;
  free(femGfxSelSelectParams)  ;
  femGfxSelSelectParams = NULL ;

  plotProp.SelStuff = GFX_SELE_NONE ;
  plotProp.SelAct   = GFX_SELE_ACT_NONE ;
  plotProp.SelectHl = AF_NO ;
#endif
  return(rv);
}


/** Allows modification of picked item (pastes its data to command 
 * line in GUI) 
 * @param pos position of picked item in NODE,ELEM,...
 * @return status
 */
int femGfxEditItemFunc(long pos)
{
#ifdef _USE_GUI_
  char str[FEM_STR_LEN+1] ;
  char str2[FEM_STR_LEN+1] ;
  double x, y, z ;
  long i, vpos, vlen, id, v2,v3, ipos ;

  for (i=0; i<FEM_STR_LEN+1; i++) {str[i] = '\0';}
  for (i=0; i<FEM_STR_LEN+1; i++) {str2[i] = '\0';}

  switch(plotProp.SelStuff)
  {
    case (GFX_SELE_NONE) : return(AF_ERR_EMP); break ;
    case (GFX_SELE_NODES) : 
        fdbCSysGetCylCoordDEG(
             fdbInputGetDbl(NODE, NODE_X, pos),
             fdbInputGetDbl(NODE, NODE_Y, pos),
             fdbInputGetDbl(NODE, NODE_Z, pos),
		         &x, &y, &z);
         sprintf(str, "n, %li, %e, %e, %e",
             fdbInputGetInt(NODE, NODE_ID, pos),
             x, y, z);
         break ;
    case (GFX_SELE_DISPS) : 
         sprintf(str, "d, %li, %li, %e, %li, %li",
             fdbInputGetInt(NDISP, NDISP_NODE, pos),
             fdbInputGetInt(NDISP, NDISP_TYPE, pos),
             fdbInputGetDbl(NDISP, NDISP_VAL, pos),
             fdbInputGetInt(NDISP, NDISP_SET, pos),
             fdbInputGetInt(NDISP, NDISP_ID, pos)
             );
         break ;
    case (GFX_SELE_FORCS) : 
         sprintf(str, "f, %li, %li, %e, %li, %li",
             fdbInputGetInt(NLOAD, NLOAD_NODE, pos),
             fdbInputGetInt(NLOAD, NLOAD_TYPE, pos),
             fdbInputGetDbl(NLOAD, NLOAD_VAL, pos),
             fdbInputGetInt(NLOAD, NLOAD_SET, pos),
             fdbInputGetInt(NLOAD, NLOAD_ID, pos)
             );
         break ;
    case (GFX_SELE_ELDS) : 
         /* TODO loads type>4 not defined! */
         id = fdbInputGetInt(ELOAD, ELOAD_ID, pos);
         if (fdbInputCountInt(ELVAL_VAL,ELVAL_ELID, id, &ipos) < 0)
         {
           break ;
         }
         if (
           (fdbInputGetInt(ELOAD, ELOAD_TYPE, pos) > 0)&&
           (fdbInputGetInt(ELOAD, ELOAD_TYPE, pos) < 4)
           )
         {
           sprintf(str, "f, %li, %li, %li, %e, %li, %li, %li",
             fdbInputGetInt(ELOAD, ELOAD_ELEM, pos),
             fdbInputGetInt(ELOAD, ELOAD_TYPE, pos),
             (long)fdbInputGetDbl(ELVAL, ELVAL_VAL, ipos+0),
             fdbInputGetDbl(ELVAL, ELVAL_VAL, ipos+1),
             (long)fdbInputGetDbl(ELVAL, ELVAL_VAL, ipos+2),
             fdbInputGetInt(ELOAD, ELOAD_SET, pos),
             id
             );
         }
         if (fdbInputGetInt(ELOAD, ELOAD_TYPE, pos) == 4)
         {
           sprintf(str, "f, %li, %li, %li, %e, %li, %li",
             fdbInputGetInt(ELOAD, ELOAD_ELEM, pos),
             fdbInputGetInt(ELOAD, ELOAD_TYPE, pos),
             (long)fdbInputGetDbl(ELVAL, ELVAL_VAL, ipos+0),
             fdbInputGetDbl(ELVAL, ELVAL_VAL, ipos+1),
             fdbInputGetInt(ELOAD, ELOAD_SET, pos),
             id
             );
         }
         break ;


    case (GFX_SELE_ELEMS) : 
             sprintf(str, "ep, %li, %li, %li, %li, %li",
             fdbInputGetInt(ELEM, ELEM_ID, pos),
             fdbInputGetInt(ELEM, ELEM_TYPE, pos),
             fdbInputGetInt(ELEM, ELEM_RS, pos),
             fdbInputGetInt(ELEM, ELEM_MAT, pos),
             fdbInputGetInt(ELEM, ELEM_SET, pos)
             );
         break ;

    case (GFX_SELE_KPOINTS) : 
        fdbCSysGetCylCoordDEG(
             fdbInputGetDbl(KPOINT, KPOINT_X, pos),
             fdbInputGetDbl(KPOINT, KPOINT_Y, pos),
             fdbInputGetDbl(KPOINT, KPOINT_Z, pos),
		         &x, &y, &z);
         sprintf(str, "k, %li, %e, %e, %e",
             fdbInputGetInt(KPOINT, KPOINT_ID, pos),
             x, y, z);
         break ;
    case (GFX_SELE_ENTS) : 
             sprintf(str, "gep, %li, %li, %li, %li, %li, %li",
             fdbInputGetInt(ENTITY, ENTITY_TYPE, pos),
             fdbInputGetInt(ENTITY, ENTITY_ID, pos),
             fdbInputGetInt(ENTITY, ENTITY_ETYPE, pos),
             fdbInputGetInt(ENTITY, ENTITY_RS, pos),
             fdbInputGetInt(ENTITY, ENTITY_MAT, pos),
             fdbInputGetInt(ENTITY, ENTITY_SET, pos)
             );
         break ;
    case (GFX_SELE_ENTDIV) : 
             id = fdbInputGetInt(ENTITY, ENTITY_ID, pos) ;
             if ((vlen=fdbInputCountInt(ENTDIV, ENTDIV_ENT,id,&vpos)) <= 0)
             {
               return(AF_ERR_VAL);
             }
             if (vlen > 1) { v2 = fdbInputGetInt(ENTDIV, ENTDIV_DIV, vpos+1); }
             else { v2 = 0 ; }
             if (vlen > 2) { v3 = fdbInputGetInt(ENTDIV, ENTDIV_DIV, vpos+2); }
             else { v3 = 0 ; }
             sprintf(str, "gediv, %li, %li, %li, %li",
             id,
             fdbInputGetInt(ENTDIV, ENTDIV_DIV, vpos),
             v2,
             v3
             );
         break ;


    default : return(AF_ERR_EMP); break ;
  }

  femSetCmdLine(str);
#endif
  return(AF_OK) ;
}

void femGfxCleanSelCrtStuff(void)
{
  if (femGfxCrtParam  != NULL)
  {
    free(femGfxCrtParam);
    femGfxCrtParam = NULL ;
  }

  if (femGfxCrtFld  != NULL)
  {
    free(femGfxCrtFld);
    femGfxCrtFld = NULL ;
  }

  femGfxCrtFldLen = 0 ;
  femGfxCrtCount  = 0 ;
  femGfxCrtEnodes = 0 ;

  plotProp.SelAct   = GFX_SELE_ACT_NONE ;
  plotProp.SelStuff = GFX_SELE_NONE ;
}

/** Tests if item is not picked second time */
int femGfxCreateTestSingle(long pos)
{
  long i;

  for (i=0; i<femGfxCrtCount; i++)
  {
    if (femGfxCrtFld[i] == pos)
    {
      return(AF_ERR);
    }
  }
  return(AF_OK) ;
}


/** Creates loads and displacements by picking individual nodes */
int femGfxCreateItemFunc(long pos)
{
  int rv = AF_OK ;
#ifdef _USE_GUI_
  char tmp[FEM_STR_LEN+1];
  char tmp2[FEM_STR_LEN+1];
  long i, j;

  for (i=0; i<=FEM_STR_LEN; i++) 
  { 
    tmp[i] = '\0'; 
    tmp2[i] = '\0'; 
  }

  fprintf(msgout,"[ ] %s: %li\n", _("Node picked"),fdbInputGetInt(NODE, NODE_ID, pos) );

  switch (plotProp.SelAct)
  {
    case GFX_SELE_CRT_NDISP: 
      sprintf(tmp,"%li", fdbInputGetInt(NODE, NODE_ID, pos));
      rv = (ciRunCmd( ciStrCat3CmdPers("d", tmp, femGfxCrtParam))) ;
      return(rv);
      break ;

    case GFX_SELE_CRT_NLOAD:  
      sprintf(tmp,"%li", fdbInputGetInt(NODE, NODE_ID, pos));
      rv=(ciRunCmd( ciStrCat3CmdPers("f", tmp, femGfxCrtParam))) ;
      return(rv);
      break ;

    case GFX_SELE_CRT_ELEM: /* creating the finite element */
      if (femGfxCrtEnodes <= 0) {return(AF_ERR);}

      if (femGfxCrtFldLen <= 0)
      {
        if ((femGfxCrtFld = (long *)malloc(femGfxCrtEnodes*sizeof(long))) == NULL)
        {
          return(AF_ERR_MEM);
        }
        else
        {
          femGfxCrtCount = 0;
          femGfxCrtFldLen = femGfxCrtEnodes ;
        }
      }

      if (femGfxCrtCount >= (femGfxCrtFldLen - 1))
      {

        /* last node */
        if ((rv=femGfxCreateTestSingle(pos)) != AF_OK){return(rv);}
        femGfxCrtFld[femGfxCrtCount] = pos ;
        femGfxCrtCount++;

        /* finish the work */
        strcpy(tmp,"e,");
        for (i=0; i<femGfxCrtFldLen; i++)
        {
          strncat(tmp,",",FEM_STR_LEN);
          for (j=0; j<=FEM_STR_LEN; j++) { tmp2[j] = '\0'; }
          sprintf(tmp2,"%li", fdbInputGetInt(NODE, NODE_ID, femGfxCrtFld[i]));
          strncat(tmp,tmp2,FEM_STR_LEN);
        }

        rv = ciRunCmd(tmp) ;

        /* cleanup: */
        femGfxCrtCount = 0;
        femGfxCrtFldLen = 0 ;
        free (femGfxCrtFld);
        femGfxCrtFld = NULL ;
      }
      else
      {
        if ((rv=femGfxCreateTestSingle(pos)) != AF_OK){return(rv);}
        femGfxCrtFld[femGfxCrtCount] = pos ;
        femGfxCrtCount++;
      }
      
      break;

    case GFX_SELE_CRT_ENTS: /* creating the finite element */
      if (femGfxCrtEnodes <= 0) {return(AF_ERR);}

      if (femGfxCrtFldLen <= 0)
      {
        if ((femGfxCrtFld = (long *)malloc(femGfxCrtEnodes*sizeof(long))) == NULL)
        {
          return(AF_ERR_MEM);
        }
        else
        {
          femGfxCrtCount = 0;
          femGfxCrtFldLen = femGfxCrtEnodes ;
        }
      }

      if (femGfxCrtCount >= (femGfxCrtFldLen - 1))
      {

        /* last node */
        if ((rv=femGfxCreateTestSingle(pos)) != AF_OK){return(rv);}
        femGfxCrtFld[femGfxCrtCount] = pos ;
        femGfxCrtCount++;

        /* finish the work */
#if 0
        strcpy(tmp,"ge,");
        strcpy(tmp,"1,"); /*MUST BE CHANGED => ONLY LINE FR NOW!!! */
#else
        sprintf(tmp,"ge,%li,",fdbSetInputDefGEnt(0));
#endif
        for (i=0; i<femGfxCrtFldLen; i++)
        {
          strncat(tmp,",",FEM_STR_LEN);
          for (j=0; j<=FEM_STR_LEN; j++) { tmp2[j] = '\0'; }
          sprintf(tmp2,"%li", 
              fdbInputGetInt(KPOINT, KPOINT_ID, femGfxCrtFld[i]));
          strncat(tmp,tmp2,FEM_STR_LEN);
        }

        rv = ciRunCmd(tmp) ;

        /* cleanup: */
        femGfxCrtCount = 0;
        femGfxCrtFldLen = 0 ;
        free (femGfxCrtFld);
        femGfxCrtFld = NULL ;
      }
      else
      {
        if ((rv=femGfxCreateTestSingle(pos)) != AF_OK){return(rv);}
        femGfxCrtFld[femGfxCrtCount] = pos ;
        femGfxCrtCount++;
      }
      
      break;

    case GFX_SELE_CRT_PATH: 
      sprintf(tmp,"%li", fdbInputGetInt(NODE, NODE_ID, pos));
      rv = (ciRunCmd( ciStrCat2CmdPers("pn", tmp))) ;
      return(rv);
      break ;
      
    default: return(AF_ERR_VAL); break;
  }

#endif
  return(rv);
}

int femGfxGridCreateFunc(long pos)
{
  int rv = AF_OK ;

  return(rv);
}


int femGfxCancelCreateItemFunc(long pos)
{
  int rv = AF_OK ;
#ifdef _USE_GUI_
  long i,j;

  if ((femGfxCrtFld != NULL) && (femGfxCrtCount > 0))
  {
    for (i=0; i<femGfxCrtCount; i++)
    {
      if (femGfxCrtFld[i] == pos)
      {
        for (j=i; j<(femGfxCrtCount-1); j++)
        {
          femGfxCrtFld[j] = femGfxCrtFld[j+1] ;
        }
        femGfxCrtCount--;
        return(rv);
      }
    }
  }
#endif
  return(rv);
}

/** Creates node or keypoint from grid selection (single item only!)
 * @param values position in the selection buffer
 * @param type of cleated entity: GFX_SELE_CRT_GR_N or
 * GFX_SELE_CRT_GR_K
 * @return status
 */
int femGfxCreateFromGrid(long pos, long type)
{
  double x, y, z ;

  pick_grid_compute_xyz(pos, &x, &y, &z) ;
    
  switch(type)
  {
    case GFX_SELE_CRT_GR_N:
      return (f_n_new_change(0, x, y, z)) ;
      break ;
    case GFX_SELE_CRT_GR_K:
      return (f_k_new_change(0, x, y, z, NULL)) ;
      break ;
  }

  return(AF_ERR_VAL);
}

/** Allocates selection buffer
 * @param size size of buffer
 * @return allocated aray (NULL if failed)
 */
GLuint *gfxInitSelBuff(long size)
{
  GLuint *buff = NULL ;
  long i ;

  if ((buff = (GLuint *)malloc(size*sizeof(GLuint))) == NULL)
  {
    return(NULL) ;
  }
  else
  {
    for (i=0; i<size; i++) { buff[i] = 0 ; }
    return(buff) ;
  }
  
}

/** Picks individual items
 */
int femGfxPickStuff(double x, double y, double pick_x, double pick_y,  long mode, long x0, long y0, long x1, long y1, long button)
{
  int rv = AF_OK ;
#ifdef DEVEL_VERBOSE
  long  i;
#endif
  long  j;
  long len = 0 ;
  long sel_num = 0 ;
  GLuint *selectBuf= NULL ;
  GLint   viewport[4] ;

  switch (mode)
  {
    case (GFX_SELE_NONE): return(AF_OK); break;
    case (GFX_SELE_NODES): len = fdbInputTabLenSel(NODE); break;
    case (GFX_SELE_ELEMS): len = fdbInputTabLenSel(ELEM); break;
    case (GFX_SELE_DISPS): len = fdbInputTabLenSel(NDISP); break;
    case (GFX_SELE_FORCS): len = fdbInputTabLenSel(NLOAD); break;
    case (GFX_SELE_ELDS): len = fdbInputTabLenSel(ELOAD); break;

    case (GFX_SELE_KPOINTS):len = fdbInputTabLenSel(KPOINT); break;
    case (GFX_SELE_ENTS):  len = fdbInputTabLenSel(ENTITY); break;
    case (GFX_SELE_ENTDIV):  len = fdbInputTabLenSel(ENTITY); break;

    case (GFX_SELE_GRID):  len = pick_grid_siz_x*pick_grid_siz_y; break;

    case (GFX_SELE_SPACE):  len = 1; break; /* for compatibility */
    default: return(AF_ERR_VAL) ; break;
  }
  
  if (len < 1) {return(AF_ERR_EMP);}

  if ((selectBuf=gfxInitSelBuff(len*4)) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("No memory for picking"));
    return(AF_ERR_MEM);
  }

  plotProp.Select = AF_YES;

  glSelectBuffer(len*4, selectBuf) ;
  glRenderMode(GL_SELECT); 

  glMatrixMode(GL_PROJECTION) ;
  glPushMatrix() ;
  glLoadIdentity() ;

  /* viewport for structure - should be identical 
   * to one in femPrePlot() !!!!  */
	  glViewport( x0+10, y0+10, 
	  (x1 - (GFX_DESC_LEN + 20)), 
	  (y1 - 20) ); 
	
  glGetIntegerv(GL_VIEWPORT, viewport) ;

  /* "+20" is because of "y1 - 20" in glViewport */
  gluPickMatrix(x, viewport[3] - y + 20, pick_x, pick_y, viewport) ;

  femPrePlot(x0, y0, x1, y1, AF_NO);

  femTransView();
  glMatrixMode(GL_MODELVIEW) ;
  glLoadIdentity();

  glInitNames();

  switch (mode)
  {
    case (GFX_SELE_NODES): rv = femPlotNodes(); break;
    case (GFX_SELE_ELEMS): rv = femGfxEleGeom(); break;
    case (GFX_SELE_DISPS): rv = femGfxDisps(); break;
    case (GFX_SELE_FORCS): rv = femGfxForces(); break;
    case (GFX_SELE_ELDS): rv = femGfxEloads(); break;

    case (GFX_SELE_KPOINTS): rv = femPlotKpoints(); break;
    case (GFX_SELE_ENTS) : rv = femGfxEntity(); break;
    case (GFX_SELE_ENTDIV) : rv = femGfxEntity(); break;

    case (GFX_SELE_GRID) : rv = femGfxDrawGrid(); break;

    case (GFX_SELE_SPACE) : rv = AF_OK; break;
  }

  glFlush();

  sel_num = glRenderMode(GL_RENDER); 

#ifdef DEVEL_VERBOSE
  fprintf(msgout, "[ ] %s: %li (rv=%i)\n", _("Number of hits"), sel_num, rv);

  if (sel_num > 0)
  {
    for (i=0; i<sel_num*4; i++)
    {
      fprintf(msgout," %i ", selectBuf[i]) ;
    }
    fprintf(msgout," \n ");
  }
#endif

  if (sel_num > 0)
  {
    switch(button)
    {
      case 1: if (plotProp.SelAct == GFX_SELE_ACT_EDIT)
              {
                if (selectBuf[0] > 0)
#if 1
                  { rv = femGfxEditItemFunc(selectBuf[3+((sel_num-1)*4)]) ; }
#else
                  { for (j=0; j<sel_num; j++) { rv = femGfxEditItemFunc(selectBuf[4*j+3]) ; } }
#endif
                else { rv = AF_ERR_EMP ; }
              }

              if (
                  (plotProp.SelAct == GFX_SELE_CRT_NDISP)
                ||
                  (plotProp.SelAct == GFX_SELE_CRT_NLOAD)
                ||
                  (plotProp.SelAct == GFX_SELE_CRT_ELEM)
                ||
                  (plotProp.SelAct == GFX_SELE_CRT_ENTS)
                ||
                  (plotProp.SelAct == GFX_SELE_CRT_PATH)
                 )
              {
                if (selectBuf[0] > 0)
#if 1
                { 
									if ( 
											((plotProp.SelAct == GFX_SELE_CRT_NDISP) || (plotProp.SelAct == GFX_SELE_CRT_NLOAD))
											&& sel_num > 1)
									{
										for (j=0;j<sel_num;j++) rv = femGfxCreateItemFunc(selectBuf[4*j+3]) ; 
									}
									else
									{
										rv = femGfxCreateItemFunc(selectBuf[3+((sel_num-1)*4)]) ; 
									}
								}
#else
                  { for (j=0;j<sel_num;j++) rv = femGfxCreateItemFunc(selectBuf[4*j+3]) ; }
#endif
                else { rv = AF_ERR_EMP ; }
              }
              if (plotProp.SelAct == GFX_SELE_ACT_SELE)
              {
                if (selectBuf[0] > 0)
#if 0
                  { rv = femGfxSeleItemFunc(selectBuf[3], AF_YES) ; }
#else
                  { for (j=0;j<sel_num;j++) rv = femGfxSeleItemFunc(selectBuf[4*j+3], AF_YES) ; }
#endif
                else { rv = AF_ERR_EMP ; }

              }
              if (
                   (plotProp.SelAct == GFX_SELE_CRT_GR_K) ||
                   (plotProp.SelAct == GFX_SELE_CRT_GR_N)
                 )
              {
                rv = femGfxCreateFromGrid(selectBuf[3+((sel_num-1)*4)], plotProp.SelAct) ;
              }
              break ;
      case 2: if (plotProp.SelAct == GFX_SELE_ACT_SELE)
              {
                rv = femGfxSeleItemFuncFinish() ; 
              }
#ifdef _USE_GUI_
              guiHidePickFrame();
              femGfxFinishAllSelects();
#endif
              break ;
      case 3: if (plotProp.SelAct == GFX_SELE_CRT_ELEM)
              {
                if (selectBuf[0] > 0)
#if 0
                  {femGfxCancelCreateItemFunc(selectBuf[3+((sel_num-1)*4)]) ;}
#else
                  { for (j=0; j<sel_num;j++) femGfxCancelCreateItemFunc(selectBuf[4*j+3]) ;}
#endif
              }
              if (plotProp.SelAct == GFX_SELE_ACT_SELE)
              {
                if (selectBuf[0] > 0)
#if 0
                  { rv = femGfxSeleItemFunc(selectBuf[3], AF_NO) ; }
#else
                  { for (j=0; j<sel_num;j++) rv = femGfxSeleItemFunc(selectBuf[4*j+3], AF_NO) ; }
#endif
                else { rv = AF_ERR_EMP ; }

              }
              break ;
    }
  }
  else
  {
    if (button == 1)
    {
      if (plotProp.SelAct == GFX_SELE_PICK_BOX) /* zoom box */
      {
        rv = femComputeZoomBox( (int)(x-pick_x/2), (int)(y-pick_y/2), pick_x, pick_y, x0, y0, x1-x0, y1-y0) ;
        rv = femGfxSeleItemFuncFinish() ; 
#ifdef _USE_GUI_
        guiHidePickFrame();
        femGfxFinishAllSelects();
#endif
      }
    }
    if (button == 2)
    {
      if (plotProp.SelAct == GFX_SELE_ACT_SELE)
      {
        rv = femGfxSeleItemFuncFinish() ; 
      }
#ifdef _USE_GUI_
      guiHidePickFrame();
      femGfxFinishAllSelects();
#endif
    }
  }

  plotProp.Select = AF_NO;
  free(selectBuf) ; 
  selectBuf = NULL ;

  if (plotStuff.autoreplot == AF_YES) {femPlot(); }

  return(rv);
}

int femGfxPickFunc (double x, double y, long x0, long y0, long x1, long y1, long button)
{
  int rv = AF_OK ;

  rv = femGfxPickStuff(x, y, 6, 6, plotProp.SelStuff, x0, y0, x1, y1, button) ;

  return(rv);
}

int femGfxPickBox (double x, double y, double w, double h, long x0, long y0, long x1, long y1, long button)
{
  int rv = AF_OK ;

	if ((w == 0.0) || (h == 0.0)) { return(rv);} /* to avoid random boxes  */

  rv = femGfxPickStuff(x + (w/2), y + (h/2), w, h, plotProp.SelStuff, x0, y0, x1, y1, button) ;

  return(rv);
}

/** Tells if GUI selection box can be used or no
 * @return AF_YES (can be used) or AF_NO (cannot)
 * */
int femGfxCanBox(void)
{
	switch (plotProp.SelAct)
	{
		case GFX_SELE_ACT_EDIT:
		case GFX_SELE_CRT_ELEM:
		case GFX_SELE_CRT_ENTS:
		case GFX_SELE_CRT_GR_N:
		case GFX_SELE_CRT_GR_K:
		case GFX_SELE_CRT_PATH:
		/*case GFX_SELE_CRT_NLOAD:*/
		/*case GFX_SELE_CRT_NDISP:*/
			return(AF_NO);
			break ;
	}
	return(AF_YES);
}
#endif /* _USE_GFX_*/

/* end of gfx_sele.c */
