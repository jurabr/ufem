/*
   File name: cmd_plot.c
   Date:      2003/11/17 18:22
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

	 FEM User Interface - commands related to f.e.m model

	 $Id: cmd_plot.c,v 1.47 2005/02/21 22:06:01 jirka Exp $
*/

#include "fem_tui.h"
#include "cint.h"
#include "fdb_edef.h"
#include "fdb_mdef.h"
#include "fdb_geom.h"

#ifdef _USE_GUI_
extern int femPlot(void);
extern void guiResizeMainWin(void);
extern void gui_file_select_command(char *command);
extern void femDataDialogSmall(char *title, char *cmd, 
									 long norep_len, 
									 char **norep_title,
									 char **norep_value,
									 long Apply);
extern int femPrepDlg_R_MP(char *cmd, long id);
extern void femGfxFinishAllSelects(void);
extern void femGfxSetActiveSelectComman(char *cmd);
extern void femGfxUnSetActiveSelectComman(void);
extern void guiHidePickFrame(void);
extern void	guiShowPickFrame(void);
extern int femGUIWriteConfig_data(char *fname, char *atype) ;
extern int femCmdNumProcSolver ;
#endif

#ifdef _USE_GFX_
#include "fem_gfx.h"
extern int femGfxWriteConfig_data(char *fname, char *atype);
#endif
#ifdef _USE_G2D_
#include "fem_gfx.h" /* G2D ignores most of GFX */
#endif

extern int femUIWriteConfigFile(char *fname);


/* ** PLOTTING AND GRAPHICS ** */


/** re-plots data: "replot"
 * @param cmd command
 * @return status
 */
int func_gui_replot (char *cmd)
{
	int    rv = AF_OK ;

#ifdef _USE_GFX_
	if (femUI_Mode == FEM_UI_MODE_PREP)
	{ 
    femSetPlotTitle(fdbCSysTypeStr()) ;
  }

	/* should be changed later with something more flexible */
	if (femUI_Mode == FEM_UI_MODE_POST)
	{ 
    if ((plotStuff.eres != AF_YES)&&(plotStuff.path != AF_YES))
    {
      femSetPlotTitle(_("Results")) ;
    }
    else
    {
      /* do nothing - label should be set inside "ples" or "plns" */
    }
  }
#endif

#ifdef _USE_GUI_
	rv = femPlot() ;
#endif

#ifdef _USE_GLUT_UI_
	femPlot() ;
#endif

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets viewport mode "vpmode,mode[0|1|2|3]"
 * @param cmd command
 * @return status
 */
int func_gui_vpmode(char *cmd)
{
	int    rv = AF_OK ;
#ifdef _USE_GFX_
  long   set = 0 ;

	if (ciParNum(cmd) > 1) { set = ciGetParInt(cmd, 1) ; }
  rv = femViewPortSet(set) ;
  activePlotView = 0 ;

  if (rv == AF_OK)
  {
    if (plotStuff.autoreplot == AF_YES)
    {
      func_gui_replot(cmd) ;
    }
  }
#endif

	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Sets viewport "vp,[0|1|2|3]"
 * @param cmd command
 * @return status
 */
int func_gui_vp(char *cmd)
{
	int    rv = AF_OK ;
#ifdef _USE_GFX_
  long   set = 0 ;

	if (ciParNum(cmd) > 1) 
  { 
    set = ciGetParInt(cmd, 1) ; 
  }

  if (set < 0) {set = 0 ;}

  if (set >= maxPlotView)
  {
    set = maxPlotView-1 ;
  }

  activePlotView = set ;
#endif

	return ( tuiCmdReact(cmd, rv) ) ;
}


/* Returnes "Yes,No" for AF_YES or "No.Yes" for other values 
 * @param val value
 * @return string "Yes,No" or "No,Yes"
 * */
char *femStrYesNo(long val)
{
  if (val == AF_YES)
  {
    return("Yes,No") ;
  }
  else
  {
    return("No,Yes") ;
  }
}

/** runs gui dialog: "dialog, command"
 * @param cmd command
 * @return status
 */
int func_gui_dialog (char *cmd)
{
	int    rv = AF_OK ;
#ifdef _USE_GUI_
	char  *command = NULL ;
	char  *title[30] ;
	char  *value[30] ;
  long   enlen ;
  long   etpos, etyp ;
	long    i ;

	for (i=0; i<30; i++)
	{
		title[i] = NULL ;
		value[i] = NULL ;
	}

  if (ciParNum(cmd) > 1)
  {
    command = ciGetParStr(cmd,1);
		if (command == NULL)
		{
    	fprintf(msgout, "[E] %s!\n", _("Empty command"));
	  	return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
		}
		ciStrCompr(command);
		if (strlen(command) < 1)
		{
			free(command); command = NULL ;
    	fprintf(msgout, "[E] %s!\n", _("Empty command"));
	  	return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
		}


		if (strcmp(command,"ngen") == 0)
		{
			title[0]=ciAllocStr(_("Number of Copies"));
			value[0]=ciAllocStr("1");
			title[1]=ciAllocStr(_("X Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y Offset"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z Offset"));
			value[3]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Copy Nodes"),
					_("ngen"),
					4, title, value, AF_YES
					) ;
		}
    
    if (strcmp(command,"nmove") == 0)
		{
			title[0]=ciAllocStr(_("X Offset"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Y Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Z Offset"));
			value[2]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Move Nodes"),
					_("nmove"),
					3, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"engen") == 0)
		{
			title[0]=ciAllocStr(_("Number of Copies"));
			value[0]=ciAllocStr("1");
			title[1]=ciAllocStr(_("X Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y Offset"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z Offset"));
			value[3]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Copy Elements"),
					_("engen"),
					4, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"dgen") == 0)
		{
			title[0]=ciAllocStr(_("Number of Copies"));
			value[0]=ciAllocStr("1");
			title[1]=ciAllocStr(_("X Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y Offset"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z Offset"));
			value[3]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Copy Displacements"),
					_("dgen"),
					4, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"fgen") == 0)
		{
			title[0]=ciAllocStr(_("Number of Copies"));
			value[0]=ciAllocStr("1");
			title[1]=ciAllocStr(_("X Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y Offset"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z Offset"));
			value[3]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Copy Forces"),
					_("fgen"),
					4, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"fgen") == 0)
		{
			title[0]=ciAllocStr(_("Number of Copies"));
			value[0]=ciAllocStr("1");
			title[1]=ciAllocStr(_("X Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y Offset"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z Offset"));
			value[3]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Copy Element Loads"),
					_("elgen"),
					4, title, value, AF_YES
					) ;
		}
		/* ---------------------------- */
		if (strcmp(command,"nmirror") == 0)
		{
			title[0]=ciAllocStr(_("Mirror Plane"));
			value[0]=ciAllocStr("YZ,ZX,XY");
			title[1]=ciAllocStr(_("Offset"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Mirror Nodes"),
					_("nmirror"),
					2, title, value, AF_YES
					) ;
		}
		if (strcmp(command,"enmirror") == 0)
		{
			title[0]=ciAllocStr(_("Mirror Plane"));
			value[0]=ciAllocStr("YZ,ZX,XY");
			title[1]=ciAllocStr(_("Offset"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Mirror Elements"),
					_("enmirror"),
					2, title, value, AF_YES
					) ;
		}
		if (strcmp(command,"dmirror") == 0)
		{
			title[0]=ciAllocStr(_("Mirror Plane"));
			value[0]=ciAllocStr("YZ,ZX,XY");
			title[1]=ciAllocStr(_("Offset"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Mirror Displacements"),
					_("dmirror"),
					2, title, value, AF_YES
					) ;
		}
		if (strcmp(command,"fmirror") == 0)
		{
			title[0]=ciAllocStr(_("Mirror Plane"));
			value[0]=ciAllocStr("YZ,ZX,XY");
			title[1]=ciAllocStr(_("Offset"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Mirror Forces"),
					_("fmirror"),
					2, title, value, AF_YES
					) ;
		}
		if (strcmp(command,"elmirror") == 0)
		{
			title[0]=ciAllocStr(_("Mirror Plane"));
			value[0]=ciAllocStr("YZ,ZX,XY");
			title[1]=ciAllocStr(_("Offset"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Mirror Element Loads"),
					_("elmirror"),
					2, title, value, AF_YES
					) ;
		}

		/* ---------------------------- */
		if (strcmp(command,"n") == 0)
		{
			title[0]=ciAllocStr(_("Number"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("X"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z"));
			value[3]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Create Node"),
					_("n"),
					4, title, value, AF_YES
					) ;
		}
		if (strcmp(command,"ndel") == 0)
		{
			title[0]=ciAllocStr(_("Number From"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Number To"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Delete Node"),
					_("ndel"),
					2, title, value, AF_YES
					) ;
		}

		/* ---------------------------- */

		if (strcmp(command,"edef") == 0)
		{
			title[0]=ciAllocStr(_("Element Type"));
			value[0]=femInputStringFromFldInt(ETYPE, ETYPE_ID,NULL);
			title[1]=ciAllocStr(_("Real Set"));
			value[1]=femInputStringFromFldInt(RSET,RSET_ID,NULL);
			title[2]=ciAllocStr(_("Material Type"));
			value[2]=femInputStringFromFldInt(MAT, MAT_ID,NULL);
			femDataDialogSmall(
					_("Default Properties of Elements"),
					_("edef"),
					3, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"e") == 0)
		{
      if (fdbSetInputDefET(0) > 0)
      {
        if (fdbInputCountInt(ETYPE, ETYPE_ID, fdbSetInputDefET(0), &etpos) >= 1)
		    {
		      etyp = fdbInputGetInt(ETYPE, ETYPE_TYPE, etpos);
          enlen = fdbElementType[etyp].nodes ;
          
			    title[0]=ciAllocStr(_("Number"));
			    value[0]=ciAllocStr(" ");
          for (i=1; i<=enlen; i++)
          {
			      title[i]=ciAllocStr(_("Node"));
			      value[i]=ciAllocStr(" ");
          }
			    femDataDialogSmall(
					    _("Create Element"),
					    _("e"),
					    enlen+1, title, value, AF_YES
					    ) ;
		    }
        else
        {
          rv = AF_ERR_EMP ;
          fprintf(msgout,"[E] %s!\n", _("Define element types first"));
        }
      }
      else
      {
        rv = AF_ERR_EMP ;
        fprintf(msgout,"[E] %s!\n",_("No element type found!"));
      }
    }

    if (strcmp(command,"gcreate_e") == 0)
		{
			title[0]=ciAllocStr(_("Element Type"));
			value[0]=femInputStringFromFldInt(ETYPE, ETYPE_ID,NULL);
			title[1]=ciAllocStr(_("Real Set"));
			value[1]=femInputStringFromFldInt(RSET,RSET_ID,NULL);
			title[2]=ciAllocStr(_("Material Type"));
			value[2]=femInputStringFromFldInt(MAT, MAT_ID,NULL);
			femDataDialogSmall(
					_("Properties of Future Elements"),
					_("gcreate,e"),
					3, title, value, AF_NO
					) ;
		}
		
		if (strcmp(command,"edel") == 0)
		{
			title[0]=ciAllocStr(_("Number From"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Number To"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Delete Element"),
					_("edel"),
					2, title, value, AF_YES
					) ;
		}

		/* ---------------------------- */

		if (strcmp(command,"time") == 0)
		{
			title[0]=ciAllocStr(_("Time"));
			value[0]=ciAllocStr(ciInt2Str(fdbSetInputDefEset(0))) ;
			femDataDialogSmall(
					_("Set Set/Time"),
					_("time"),
					1, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"set") == 0)
		{
			title[0]=ciAllocStr(_("Set Number"));
			value[0]=ciAllocStr("next,previous,first,last");
			femDataDialogSmall(
					_("Select Result Set"),
					_("set"),
					1, title, value, AF_YES
					) ;
		}


		/* ---------------------------- */

    if (strcmp(command,"d") == 0)
		{
			title[0]=ciAllocStr(_("Node"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Type"));
			value[1]=fdbFemStrSuppTypListInt();
			title[2]=ciAllocStr(_("Size"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Time"));
			value[3]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Create Displacement"),
					_("d"),
					4, title, value, AF_YES
					) ;
		}

    if (strcmp(command,"gcreate_d") == 0)
		{
			title[0]=ciAllocStr(_("Type mod"));
#if 0
			value[0]=ciListVarsCSV("disp");
#else
			value[0]=fdbFemStrSuppTypListInt();
#endif
			title[1]=ciAllocStr(_("Size"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Time"));
			value[2]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Properties of Future Displacement"),
					_("gcreate,d"),
					3, title, value, AF_NO
					) ;
		}
    
    if (strcmp(command,"ddel") == 0)
		{
			title[0]=ciAllocStr(_("Number From"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Number To"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Delete Element"),
					_("ddel"),
					2, title, value, AF_YES
					) ;
		}

		/* ---------------------------- */

    if (strcmp(command,"f") == 0)
		{
			title[0]=ciAllocStr(_("Node"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Type"));
#if 0
			value[1]=ciListVarsCSV("load");
#else
			value[1]=fdbFemStrNLoadTypListInt();
#endif
			title[2]=ciAllocStr(_("Size"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Time"));
			value[3]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Create Nodal Load"),
					_("f"),
					4, title, value, AF_YES
					) ;
		}

    if (strcmp(command,"gcreate_f") == 0)
		{
			title[0]=ciAllocStr(_("Type"));
			value[0]=fdbFemStrNLoadTypListInt();
			title[1]=ciAllocStr(_("Size"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Time"));
			value[2]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Properties of Future Nodal Load"),
					_("gcreate,f"),
					3, title, value, AF_NO
					) ;
		}
    
    if (strcmp(command,"fdel") == 0)
		{
			title[0]=ciAllocStr(_("Number From"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Number To"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Delete Nodal Load"),
					_("fdel"),
					2, title, value, AF_YES
					) ;
		}
		/* ---------------------------- */

    if (strcmp(command,"accel") == 0)
		{
			title[0]=ciAllocStr(_("Direction"));
			value[0]=ciAllocStr("ux,uy,uz");
			title[1]=ciAllocStr(_("Size"));
			value[2]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Create Acceleration"),
					_("accel"),
					2, title, value, AF_YES
					) ;
		}
    
    if (strcmp(command,"acceldel") == 0)
		{
			title[0]=ciAllocStr(_("Number From"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Number To"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Delete Accelerations"),
					_("acceldel"),
					2, title, value, AF_YES
					) ;
		}


		/* ---------------------------- */
    if (strcmp(command,"pres") == 0)
		{
      for (i=0; i<6; i++)
      {
			  title[i]=ciAllocStr(_("Type"));
			  value[i]=ciListVarsCSV("result");
      }
			femDataDialogSmall(
					_("Print Element Solution"),
					_("pres"),
					6, title, value, AF_NO
					) ;
		}

    if (strcmp(command,"prns") == 0)
		{
      for (i=0; i<6; i++)
      {
			  title[i]=ciAllocStr(_("Type"));
			  value[i]=ciListVarsCSV("result");
      }
			femDataDialogSmall(
					_("Print Nodal Solution"),
					_("prns"),
					6, title, value, AF_NO
					) ;
		}

    if (strcmp(command,"ples") == 0)
		{
			title[0]=ciAllocStr(_("Type"));
			value[0]=fdbFemStrEResListInt();
			femDataDialogSmall(
					_("Plot Element Solution"),
					_("ples"),
					1, title, value, AF_YES
					) ;
		}

    if (strcmp(command,"plns") == 0)
		{
			title[0]=ciAllocStr(_("Type"));
			value[0]=fdbFemStrEResListInt();
			femDataDialogSmall(
					_("Plot Nodal Solution"),
					_("plns"),
					1, title, value, AF_YES
					) ;
		}

		/* ---------------------------- */
		/* ---------------------------- */
		if (strcmp(command,"et") == 0)
		{
			title[0]=ciAllocStr(_("Identifier"));
			value[0]=fdbFemStrFromInt(fdbInputFindMaxInt(ETYPE,ETYPE_ID)+1);
			title[1]=ciAllocStr(_("Type"));
			value[1]=ciListVarsCSV("etype");
			femDataDialogSmall(
					_("Add Element Type"),
					_("et"),
					2, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"etdel") == 0)
		{
			title[0]=ciAllocStr(_("Identifier"));
			value[0]=femInputStringFromFldInt(ETYPE, ETYPE_ID,NULL);;
			femDataDialogSmall(
					_("Delete Element Type"),
					_("etdel"),
					1, title, value, AF_YES
					) ;
		}
    
		if (strcmp(command,"rs") == 0)
		{
			title[0]=ciAllocStr(_("Identifier"));
			value[0]=fdbFemStrFromInt(fdbInputFindMaxInt(RSET,RSET_ID)+1);
			title[1]=ciAllocStr(_("Type"));
			value[1]=ciListVarsCSV("etype");
  		title[2]=ciAllocStr(_("Number of Repeating Data Sets"));
    	value[2]=ciAllocStr("0");
			femDataDialogSmall(
					_("Add Real Set"),
					_("rs"),
					3, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"r") == 0)
		{
			title[0]=ciAllocStr(_("Identifier"));
			value[0]=femInputStringFromFldInt(RSET, RSET_ID,NULL);;
			if (value[0][0] != ' ')
			{
				femDataDialogSmall(
					_("Real Set To Be Edited"),
					_("r_dlg"),
					1, title, value, AF_NO
					) ;
			}
			else
			{
				fprintf(msgout,"[E] %s!\n",_("No real set is defined"));
				rv = AF_ERR_EMP ;
			}
		}
		
		if (strcmp(command,"rsdel") == 0)
		{
			title[0]=ciAllocStr(_("Identifier"));
			value[0]=femInputStringFromFldInt(RSET, RSET_ID,NULL);;
			femDataDialogSmall(
					_("Delete Real Set"),
					_("rsdel"),
					1, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"mat") == 0)
		{
			title[0]=ciAllocStr(_("Identifier"));
			value[0]=fdbFemStrFromInt(fdbInputFindMaxInt(MAT,MAT_ID)+1);
			title[1]=ciAllocStr(_("Type"));
			value[1]=ciListVarsCSV("mattype");
  		title[2]=ciAllocStr(_("Number of Repeating Data Sets"));
    	value[2]=ciAllocStr("0");
			femDataDialogSmall(
					_("Add Material Type"),
					_("mat"),
					3, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"mp") == 0)
		{
			title[0]=ciAllocStr(_("Identifier"));
			value[0]=femInputStringFromFldInt(MAT, MAT_ID,NULL);
			if (value[0][0] != ' ')
			{
				femDataDialogSmall(
					_("Material Type To Be Edited"),
					_("mp_dlg"),
					1, title, value, AF_NO
					) ;
			}
			else
			{
				fprintf(msgout,"[E] %s!\n",_("No material is defined"));
				rv = AF_ERR_EMP ;
			}
		}

		if (strcmp(command,"matdel") == 0)
		{
			title[0]=ciAllocStr(_("Identifier"));
			value[0]=femInputStringFromFldInt(MAT, MAT_ID,NULL);;
			femDataDialogSmall(
					_("Delete Material Type"),
					_("matdel"),
					1, title, value, AF_YES
					) ;
		}

		/* ---------------------------- */
		
		if (strcmp(command,"nsel") == 0)
		{
			title[0]=ciAllocStr(_("Mode"));
			value[0]=ciAllocStr("select,reselect,add,unselect,invert,all,none");
			title[1]=ciAllocStr(_("Item"));
			value[1]=ciAllocStr("id,loc");
			title[2]=ciAllocStr(_("Direction"));
			value[2]=ciAllocStr(" ,X,Y,Z");
			title[3]=ciAllocStr(_("From"));
			value[3]=ciAllocStr(" ");
			title[4]=ciAllocStr(_("To"));
			value[4]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Select Nodes"),
					_("nsel"),
					5, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"esel") == 0)
		{
			title[0]=ciAllocStr(_("Mode"));
			value[0]=ciAllocStr("select,reselect,add,unselect,invert,all,none");
			title[1]=ciAllocStr(_("Item"));
			value[1]=ciAllocStr("id,etype,rset,mat,set,val");
			title[2]=ciAllocStr(_("Direction"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("From"));
			value[3]=ciAllocStr(" ");
			title[4]=ciAllocStr(_("To"));
			value[4]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Select Elements"),
					_("esel"),
					5, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"dsel") == 0)
		{
			title[0]=ciAllocStr(_("Mode"));
			value[0]=ciAllocStr("select,reselect,add,unselect,invert,all,none");
			title[1]=ciAllocStr(_("Item"));
			value[1]=ciAllocStr("id,node,type,set,value");
			title[2]=ciAllocStr(_("Direction"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("From"));
			value[3]=ciAllocStr(" ");
			title[4]=ciAllocStr(_("To"));
			value[4]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Select Displacement"),
					_("dsel"),
					5, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"fsel") == 0)
		{
			title[0]=ciAllocStr(_("Mode"));
			value[0]=ciAllocStr("select,reselect,add,unselect,invert,all,none");
			title[1]=ciAllocStr(_("Item"));
			value[1]=ciAllocStr("id,node,type,set,value");
			title[2]=ciAllocStr(_("Direction"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("From"));
			value[3]=ciAllocStr(" ");
			title[4]=ciAllocStr(_("To"));
			value[4]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Select Nodal Loads"),
					_("fsel"),
					5, title, value, AF_YES
					) ;
		}
		if (strcmp(command,"elsel") == 0)
		{
			title[0]=ciAllocStr(_("Mode"));
			value[0]=ciAllocStr("select,reselect,add,unselect,invert,all,none");
			title[1]=ciAllocStr(_("Item"));
			value[1]=ciAllocStr("id,elem,type,set");
			title[2]=ciAllocStr(_("Direction"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("From"));
			value[3]=ciAllocStr(" ");
			title[4]=ciAllocStr(_("To"));
			value[4]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Select Nodal Loads"),
					_("elsel"),
					5, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"acsel") == 0)
		{
			title[0]=ciAllocStr(_("Mode"));
			value[0]=ciAllocStr("select,reselect,add,unselect,invert,all,none");
			title[1]=ciAllocStr(_("Item"));
			value[1]=ciAllocStr("id,dir,set");
			title[2]=ciAllocStr(_("Direction"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("From"));
			value[3]=ciAllocStr(" ");
			title[4]=ciAllocStr(_("To"));
			value[4]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Select Accelarations"),
					_("acsel"),
					5, title, value, AF_YES
					) ;
		}

    /* mouse selection */
    if (strcmp(command,"gsel") == 0)
		{
			title[0]=ciAllocStr(_("Entity"));
			value[0]=ciAllocStr("Node,Element,Displacement,Force,Keypoint,Geom.Ent.");
			title[1]=ciAllocStr(_("Mode"));
			value[1]=ciAllocStr("Reselect,Unselect");
			femDataDialogSmall(
					_("Select by Mouse"),
					_("gsel"),
					2, title, value, AF_NO
					) ;
		}

		if (strcmp(command,"mcset") == 0)
		{
			title[0]=ciAllocStr(_("Type"));
			value[0]=ciListVarsCSV("monte");
			femDataDialogSmall(
					_("Select Statistic Data"),
					_("set"),
					1, title, value, AF_NO
					) ;
		}

		/* ---------------------------- */
		if (strcmp(command,"k") == 0)
		{
			title[0]=ciAllocStr(_("Number"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("X"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z"));
			value[3]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Create Keypoint"),
					_("k"),
					4, title, value, AF_YES
					) ;
		}
		if (strcmp(command,"kdel") == 0)
		{
			title[0]=ciAllocStr(_("Number From"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Number To"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Delete Node"),
					_("kdel"),
					2, title, value, AF_YES
					) ;
		}

		/* ---------------------------- */
    if (strcmp(command,"echset") == 0)
		{
			title[0]=ciAllocStr(_("Set number"));
			value[0]=ciInt2Str(fdbSetInputDefEset(0));
			femDataDialogSmall(
					_("Number of set"),
					_("pick,e,echset"),
					1, title, value, AF_NO
					) ;
		}
    if (strcmp(command,"gechset") == 0)
		{
			title[0]=ciAllocStr(_("Set number"));
			value[0]=ciInt2Str(fdbSetInputDefEset(0));
			femDataDialogSmall(
					_("Number of set"),
					_("pick,g,gechset"),
					1, title, value, AF_NO
					) ;
		}
    if (strcmp(command,"fchset") == 0)
		{
			title[0]=ciAllocStr(_("Set number"));
			value[0]=ciInt2Str(fdbSetInputLoadSet(0));
			femDataDialogSmall(
					_("Number of set"),
					_("pick,f,fchset"),
					1, title, value, AF_NO
					) ;
		}
    if (strcmp(command,"dchset") == 0)
		{
			title[0]=ciAllocStr(_("Set number"));
			value[0]=ciInt2Str(fdbSetInputLoadSet(0));
			femDataDialogSmall(
					_("Number of set"),
					_("pick,d,dchset"),
					1, title, value, AF_NO
					) ;
		}
#if 0
    if (strcmp(command,"elchset") == 0)
		{
			title[0]=ciAllocStr(_("Set number"));
			value[0]=ciInt2Str(fdbSetInputLoadSet(0));
			femDataDialogSmall(
					_("Number of set"),
					_("pick,el,elchchset"),
					1, title, value, AF_NO
					) ;
		}
#endif

		/* ---------------------------- */

#if 1
    if (strcmp(command,"ddiv") == 0)
		{
			title[0]=ciAllocStr(_("Number of Divisons"));
			value[0]=ciInt2Str(fdbSetInputDefDiv(0));
			femDataDialogSmall(
					_("Default Division"),
					_("ddiv"),
					1, title, value, AF_NO
					) ;
		}
#endif
		
		if (strcmp(command,"helpm") == 0)
		{
			title[0]=ciAllocStr(_("Material Type"));
			value[0]=ciListVarsCSV("mattype");
			femDataDialogSmall(
					_("Help for Material Type"),
					_("help,m"),
					1, title, value, AF_NO
					) ;
		}

		if (strcmp(command,"helpe") == 0)
		{
			title[0]=ciAllocStr(_("Element Type"));
			value[0]=ciListVarsCSV("etype");
			femDataDialogSmall(
					_("Help for Element Type"),
					_("help,e"),
					1, title, value, AF_NO
					) ;
		}

		/* ---------------------------- */
    
#if 0
		if (strcmp(command,"ge") == 0)
		{
      if (fdbSetInputDefET(0) > 0)
      {
        if (fdbInputCountInt(ETYPE, ETYPE_ID, fdbSetInputDefET(0), &etpos) >= 1)
		    {
		      etyp = fdbInputGetInt(ETYPE, ETYPE_TYPE, etpos);
          enlen = fdbElementType[etyp].nodes ;
          
			    title[0]=ciAllocStr(_("Number"));
			    value[0]=ciAllocStr(" ");
          for (i=1; i<=enlen; i++)
          {
			      title[i]=ciAllocStr(_("Keypoint"));
			      value[i]=ciAllocStr(" ");
          }
			    femDataDialogSmall(
					    _("Create Geometric Entity"),
					    _("ge"),
					    enlen+1, title, value, AF_YES
					    ) ;
		    }
        else
        {
          rv = AF_ERR_EMP ;
          fprintf(msgout,"[E] %s!\n", _("Define element types first"));
        }
      }
      else
      {
        rv = AF_ERR_EMP ;
        fprintf(msgout,"[E] %s!\n",_("No element type found!"));
      }
    }
#endif

		if (strcmp(command,"gesize") == 0)
		{
			title[0]=ciAllocStr(_("Entity Type"));
			value[0]=ciAllocStr("rectangle,brick,cvbrick");
			title[1]=ciAllocStr(_("Number"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Start X"));
			value[2]=ciAllocStr("0");
			title[3]=ciAllocStr(_("Start Y"));
			value[3]=ciAllocStr("0");
			title[4]=ciAllocStr(_("Start Z"));
			value[4]=ciAllocStr("0");
			title[5]=ciAllocStr(_("Size X"));
			value[5]=ciAllocStr("0");
			title[6]=ciAllocStr(_("Size Y"));
			value[6]=ciAllocStr("0");
			title[7]=ciAllocStr(_("Size Z"));
			value[7]=ciAllocStr("0");

			femDataDialogSmall(
					_("Properties of Future Entities"),
					_("gesize"),
					8, title, value, AF_NO
					) ;
		}

#if 1
    if (strcmp(command,"gcreate_ge") == 0)
		{
			title[0]=ciAllocStr(_("Entity Type"));
			value[0]=ciListVarsCSV("geom_ent");
			title[1]=ciAllocStr(_("Element Type"));
			value[1]=femInputStringFromFldInt(ETYPE, ETYPE_ID,NULL);
			title[2]=ciAllocStr(_("Real Set"));
			value[2]=femInputStringFromFldInt(RSET,RSET_ID,NULL);
			title[3]=ciAllocStr(_("Material Type"));
			value[3]=femInputStringFromFldInt(MAT, MAT_ID,NULL);
			femDataDialogSmall(
					_("Properties of Future Entities"),
					_("gcreate,ge"),
					4, title, value, AF_NO
					) ;
		}
		
		if (strcmp(command,"gedel") == 0)
		{
			title[0]=ciAllocStr(_("Number From"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Number To"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Delete Geometric Entity"),
					_("gedel"),
					2, title, value, AF_YES
					) ;
		}

    if (strcmp(command,"kgen") == 0)
		{
			title[0]=ciAllocStr(_("Number of Copies"));
			value[0]=ciAllocStr("1");
			title[1]=ciAllocStr(_("X Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y Offset"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z Offset"));
			value[3]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Copy Keypoints"),
					_("kgen"),
					4, title, value, AF_YES
					) ;
		}

    if (strcmp(command,"kmove") == 0)
		{
			title[0]=ciAllocStr(_("X Offset"));
			value[0]=ciAllocStr(" ");
			title[1]=ciAllocStr(_("Y Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Z Offset"));
			value[2]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Move Keypoints"),
					_("kmove"),
					3, title, value, AF_YES
					) ;
		}


		if (strcmp(command,"gekgen") == 0)
		{
			title[0]=ciAllocStr(_("Number of Copies"));
			value[0]=ciAllocStr("1");
			title[1]=ciAllocStr(_("X Offset"));
			value[1]=ciAllocStr(" ");
			title[2]=ciAllocStr(_("Y Offset"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("Z Offset"));
			value[3]=ciAllocStr(" ");

			femDataDialogSmall(
					_("Copy Entities"),
					_("gekgen"),
					4, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"kmirror") == 0)
		{
			title[0]=ciAllocStr(_("Mirror Plane"));
			value[0]=ciAllocStr("YZ,ZX,XY");
			title[1]=ciAllocStr(_("Offset"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Mirror Keypoints"),
					_("kmirror"),
					2, title, value, AF_YES
					) ;
		}
		if (strcmp(command,"gekmirror") == 0)
		{
			title[0]=ciAllocStr(_("Mirror Plane"));
			value[0]=ciAllocStr("YZ,ZX,XY");
			title[1]=ciAllocStr(_("Offset"));
			value[1]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Mirror Entities"),
					_("gekmirror"),
					2, title, value, AF_YES
					) ;
		}

	
		if (strcmp(command,"ksel") == 0)
		{
			title[0]=ciAllocStr(_("Mode"));
			value[0]=ciAllocStr("select,reselect,add,unselect,invert,all,none");
			title[1]=ciAllocStr(_("Item"));
			value[1]=ciAllocStr("id,loc");
			title[2]=ciAllocStr(_("Direction"));
			value[2]=ciAllocStr(" ,X,Y,Z");
			title[3]=ciAllocStr(_("From"));
			value[3]=ciAllocStr(" ");
			title[4]=ciAllocStr(_("To"));
			value[4]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Select Keypoints"),
					_("ksel"),
					5, title, value, AF_YES
					) ;
		}

		if (strcmp(command,"gesel") == 0)
		{
			title[0]=ciAllocStr(_("Mode"));
			value[0]=ciAllocStr("select,reselect,add,unselect,invert,all,none");
			title[1]=ciAllocStr(_("Item"));
			value[1]=ciAllocStr("id,etype,rset,mat,set");
			title[2]=ciAllocStr(_("Direction"));
			value[2]=ciAllocStr(" ");
			title[3]=ciAllocStr(_("From"));
			value[3]=ciAllocStr(" ");
			title[4]=ciAllocStr(_("To"));
			value[4]=ciAllocStr(" ");
			femDataDialogSmall(
					_("Select Geometric Entities"),
					_("gesel"),
					5, title, value, AF_YES
					) ;
		}


		if (strcmp(command,"jobname") == 0)
		{
			title[0]=ciAllocStr(_("JobName"));
			value[0]=ciAllocStr(femGetJobname());
			femDataDialogSmall(
					_("Change JobName"),
					_("jobname"),
					1, title, value, AF_NO
					) ;
		}

		if (strcmp(command,"pgrid") == 0)
		{
			title[0]=ciAllocStr(_("Plane"));
			value[0]=ciListVarsCSVpreDef("pplane",pick_grid_plane);
			title[1]=ciAllocStr(_("Distance X"));
			value[1]=ciAllocStr(ciDbl2Str(pick_grid_dx));
			title[2]=ciAllocStr(_("Distance Y"));
			value[2]=ciAllocStr(ciDbl2Str(pick_grid_dy));
			title[3]=ciAllocStr(_("Number of points in X"));
			value[3]=ciAllocStr(ciInt2Str(pick_grid_siz_x));
			title[4]=ciAllocStr(_("Number of points in Y"));
			value[4]=ciAllocStr(ciInt2Str(pick_grid_siz_y));
			title[5]=ciAllocStr(_(" Origin X"));
			value[5]=ciAllocStr(ciDbl2Str(pick_grid_x0));
			title[6]=ciAllocStr(_(" Origin Y"));
			value[6]=ciAllocStr(ciDbl2Str(pick_grid_y0));
			title[7]=ciAllocStr(_("Origin Z"));
			value[7]=ciAllocStr(ciDbl2Str(pick_grid_z0));
			title[8]=ciAllocStr(_("In-plane angle [deg]"));
			value[8]=ciAllocStr(ciDbl2Str(pick_grid_angle_in));
			title[9]=ciAllocStr(_("Out-plane angle [deg]"));
			value[9]=ciAllocStr(ciDbl2Str(pick_grid_angle_out));
			femDataDialogSmall(
					_("Set Picking Grid Properties"),
					_("pgrid"),
					10, title, value, AF_NO
					) ;
		}


    /* Number of threads/processes*/
		if (strcmp(command,"nproc") == 0)
		{
			title[0]=ciAllocStr(_("Interface"));
			value[0]=ciAllocStr( ciInt2Str(femCmdNumProcUI));
			title[1]=ciAllocStr(_("Solver"));
			value[1]=ciAllocStr( ciInt2Str(femCmdNumProcSolver));
			femDataDialogSmall(
					_("Number of Processes"),
					_("nproc"),
					2, title, value, AF_NO
					) ;
		}




    /* Solver settings iterface */
		if (strcmp(command,"solve") == 0)
		{
			title[0]=ciAllocStr(_("Non-linear solver type"));
			value[0]=ciAllocStr("Linear,NRM,ALM");
			title[1]=ciAllocStr(_("Number of substeps"));
			value[1]=ciAllocStr("20");
			title[2]=ciAllocStr(_("Number of iterations"));
			value[2]=ciAllocStr("333");
			title[3]=ciAllocStr(_("Nth substep to save"));
			value[3]=ciAllocStr("1");
			title[4]=ciAllocStr(_("Node to track"));
			value[4]=ciAllocStr("1");
			title[5]=ciAllocStr(_("Iterative supports"));
			value[5]=ciAllocStr(femStrYesNo(femOneDirSupp) );
			femDataDialogSmall(
					_("Run Solver"),
					_("solve"),
					6, title, value, AF_NO
					) ;
		}


    /* Export settings */
		if (strcmp(command,"export") == 0)
		{
			title[0]=ciAllocStr(_("File type"));
			value[0]=ciAllocStr("FEM,MAC");
			femDataDialogSmall(
					_("Export Selected Data"),
					_("export"),
					1, title, value, AF_NO
					) ;
		}

#endif

		/* ---------------------------- */
		/* ---------------------------- */

		/* we are finished: */
		for (i=0; i<30; i++)
		{
			if (title[i]!=NULL) { free(title[i]); title[i]=NULL; }
			if (value[i]!=NULL) { free(value[i]); value[i]=NULL; }
		}
		free(command);
		command = NULL ;
    if (rv != AF_OK)
    {
	    return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
    }
    else
    {
	    return(rv) ;
    }
  }
  else
  {
    fprintf(msgout, "[E] %s!\n", _("Empty command"));
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }
#endif
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** raw dialog command for RVAL data (do not call it directly!) "r_dlg" */
int func_gui_rset_data_dlg (char *cmd)
{
	int rv = AF_OK ;
#ifdef _USE_GUI_
	long id ;

	if (ciParNum(cmd) < 2)
	{
		rv = AF_ERR_EMP ;
	}
	else
	{
		id = ciGetParInt(cmd, 1);
		rv = femPrepDlg_R_MP("r", id) ;
	}
#endif
	return(tuiCmdReact(cmd, rv));
}

/** raw dialog command for MVAL data (do not call it directly!) "mp_dlg" */
int func_gui_mat_data_dlg (char *cmd)
{
	int rv = AF_OK ;
#ifdef _USE_GUI_
	long id ;

	if (ciParNum(cmd) < 2)
	{
		rv = AF_ERR_EMP ;
	}
	else
	{
		id = ciGetParInt(cmd, 1);
		rv = femPrepDlg_R_MP("mp", id) ;
	}
#endif
	return(tuiCmdReact(cmd, rv));
}

/** runs file dialog: "filedialog, command"
 * @param cmd command
 * @return status
 */
int func_gui_file_dialog (char *cmd)
{
	int    rv = AF_OK ;

#ifdef _USE_GUI_
  if (ciParNum(cmd) > 1)
  {
    gui_file_select_command(ciGetParStr(cmd,1));
  }
  else
  {
    fprintf(msgout, "[E] %s!\n", _("Empty command"));
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }

#endif

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** sets geometry of main window: "ggeom[, x0, y0, width,height]
 * @param cmd command
 * @return status
 */
int func_gui_geom (char *cmd)
{
	int    rv = AF_OK ;
#ifdef _USE_GUI_ 
#ifndef _USE_GLUT_UI_
  long   x0     = -1 ;
  long   y0     = -1 ;
  long   width  =  0 ;
  long   height =  0 ;

  if (ciParNum(cmd) <= 1) 
  { 
    guiProp.none = AF_YES ; 
  }
  else
  {
    guiProp.none = AF_NO ;

    if (ciParNum(cmd) > 1) { x0     = ciGetParInt(cmd, 1); }
    if (ciParNum(cmd) > 2) { y0     = ciGetParInt(cmd, 2); }
    if (ciParNum(cmd) > 3) { width  = ciGetParInt(cmd, 3); }
    if (ciParNum(cmd) > 4) { height = ciGetParInt(cmd, 4); }
  }

    guiProp.x0     = x0 ;
    guiProp.y0     = y0 ;
    guiProp.width  = width ;
    guiProp.height = height ;
#endif
#endif

#ifdef _USE_GUI_
  guiResizeMainWin();
#endif

#ifdef _USE_GUI_
  guiResizeMainWin(); /* why TWO times? */
#endif

	return ( tuiCmdReact(cmd, rv) ) ;
}


/* -------------------------------------- */

/** hide things (prevent from plotting): "hide,(n|e|f|d|r|k|l|a|v)"
 * @param cmd command
 * @return status
 */
int func_gfx_hide(char *cmd)
{
#ifdef _USE_GFX_
	char *str = NULL ;

	if (ciParNum(cmd) <= 1) 
	{
		fprintf(msgout,"[E] %s!\n",_("You must specify what you want to be hidden"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
	
  if (ciGetParStr(cmd,1) != NULL) 
	{ 
		str = ciGetParStr(cmd,1) ; 
	}
	else 
	{
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (strlen(str) >= 1)
	{
		switch (str[0])
		{
			case 'n': 
			case 'N': plotStuff.node = AF_NO ; break ;
			case 'e': 
			case 'E': plotStuff.elem = AF_NO ; break ;
			case 'd': 
			case 'D': plotStuff.disp = AF_NO ; break ;
			case 'f': 
			case 'F': plotStuff.load = AF_NO ; break ;
			case 'r': 
			case 'R': plotStuff.react = AF_NO ; 
			 					plotStuff.eres = AF_NO ; break ;
			case 'k': 
			case 'K': plotStuff.kp = AF_NO ; break ;
			case 'l': 
			case 'L': plotStuff.line = AF_NO ; break ;
			case 'a': 
			case 'A': plotStuff.area = AF_NO ; break ;
			case 'v': 
			case 'V': plotStuff.volume = AF_NO ; break ;
			case 'g': 
			case 'G': plotStuff.line   = AF_NO ; 
                plotStuff.area   = AF_NO ;
                plotStuff.volume = AF_NO ;
                break ;
      case 'p':
      case 'P': 
                plotStuff.grid   = AF_NO ;
                break ;
		}

		free(str);
	}
	else
	{
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
  

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** show things (allow plotting): "show,(n|e|f|d|r|k|l|a|v)"
 * @param cmd command
 * @return status
 */
int func_gfx_show(char *cmd)
{
#ifdef _USE_GFX_
	char *str = NULL ;

	if (ciParNum(cmd) <= 1) 
	{
		fprintf(msgout,"[E] %s!\n",_("You must specify what you want to be hidden"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
	
  if (ciGetParStr(cmd,1) != NULL) 
	{ 
		str = ciGetParStr(cmd,1) ; 
	}
	else 
	{
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (strlen(str) >= 1)
	{
		switch (str[0])
		{
			case 'n': 
			case 'N': plotStuff.node = AF_YES ; break ;
			case 'e': 
			case 'E': plotStuff.elem = AF_YES ; break ;
			case 'd': 
			case 'D': plotStuff.disp = AF_YES ; break ;
			case 'f': 
			case 'F': plotStuff.load = AF_YES ; break ;
			case 'r': 
			case 'R': plotStuff.react = AF_YES ; 
			 					plotStuff.eres = AF_YES ; break ;
			case 'k': 
			case 'K': plotStuff.kp = AF_YES ; break ;
			case 'l': 
			case 'L': plotStuff.line = AF_YES ; break ;
			case 'a': 
			case 'A': plotStuff.area = AF_YES ; break ;
			case 'v': 
			case 'V': plotStuff.volume = AF_YES ; break ;
			case 'g': 
			case 'G': plotStuff.line   = AF_YES ; 
                plotStuff.area   = AF_YES ;
                plotStuff.volume = AF_YES ;
                break ;
		}

		free(str);
	}
	else
	{
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
  

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}


/* -------------------------------------- */


/** plots nodes: "nplot"
 * @param cmd command
 * @return status
 */
int func_gfx_nplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.node = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots elements: "eplot"
 * @param cmd command
 * @return status
 */
int func_gfx_eplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.elem = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots loads: "fplot"
 * @param cmd command
 * @return status
 */
int func_gfx_fplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.load = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots displacements: "dplot"
 * @param cmd command
 * @return status
 */
int func_gfx_dplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.disp = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots keypoints: "kplot"
 * @param cmd command
 * @return status
 */
int func_gfx_kplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.kp = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots lines: "lplot"
 * @param cmd command
 * @return status
 */
int func_gfx_lplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.line = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots areas: "aplot"
 * @param cmd command
 * @return status
 */
int func_gfx_aplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.area = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots volumes: "vplot"
 * @param cmd command
 * @return status
 */
int func_gfx_vplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.volume = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots all geometric entities: "geplot"
 * @param cmd command
 * @return status
 */
int func_gfx_geplot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.line   = AF_YES ;
  plotStuff.area   = AF_YES ;
  plotStuff.volume = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}


/** plots deformed shape (if possible): "pldef[,mult]"
 * @param cmd command
 * @return status
 */
int func_gfx_pldef (char *cmd)
{
#ifdef _USE_GFX_
  double mult = 1.0 ;

  if (femUI_Mode != FEM_UI_MODE_POST)
  {
    fprintf(msgout, "[E] %s!\n", _("Reactions not available") ) ;
	  return ( tuiCmdReact(cmd, AF_ERR) ) ;
  }
  
  plotStuff.def_shape = AF_YES ;

  if (ciParNum(cmd) > 1)
  {
    mult = ciGetParDbl(cmd, 1) ;

    if (mult <= 0)
    {
      plotStuff.def_shape = AF_NO ;
      plotStuff.dof_mult_usr = 1.0 ;
    }
    else
    {
      plotStuff.dof_mult_usr = mult ;
    }
  }

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots reactions (if possible): "plrs"
 * @param cmd command
 * @return status
 */
int func_gfx_plrs (char *cmd)
{
#ifdef _USE_GFX_
  if (femUI_Mode != FEM_UI_MODE_POST)
  {
    fprintf(msgout, "[E] %s!\n", _("Reactions not available") ) ;
	  return ( tuiCmdReact(cmd, AF_ERR) ) ;
  }
  
  plotStuff.react = AF_YES ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots element solution (if possible): "ples,what"
 * @param cmd command
 * @return status
 */
int func_gfx_ples (char *cmd)
{
#ifdef _USE_GFX_
  char tmp[FEM_STR_LEN+1];
  int i;
  for (i=0; i<=FEM_STR_LEN; i++) {tmp[i] = '\0';}

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout, "[E] %s!\n", _("Result type should be specified"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
	
  if (femUI_Mode != FEM_UI_MODE_POST)
  {
    fprintf(msgout, "[E] %s!\n", _("Results not available") ) ;
	  return ( tuiCmdReact(cmd, AF_ERR) ) ;
  }
  
  plotStuff.nres = AF_NO ;

  plotStuff.eres = AF_YES ;
	plotStuff.eresType = ciGetParInt(cmd, 1) ;
	plotStuff.elem = AF_NO ; 

  strncpy (tmp, _("Result"),FEM_STR_LEN);
  strncat (tmp, ": ",FEM_STR_LEN);
  if (ciGetVarNameFromGrp( ciGetParStr(cmd,1), "result") == NULL)
  {
    fprintf(msgout, "[E] %s!\n", _("Unknown result type"));
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  strncat (tmp, ciGetVarNameFromGrp( ciGetParStr(cmd,1), "result"), FEM_STR_LEN);
  femSetPlotTitle( tmp );

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots nodal solution (if possible): "plns,what"
 * @param cmd command
 * @return status
 */
int func_gfx_plns (char *cmd)
{
#ifdef _USE_GFX_
  char tmp[FEM_STR_LEN+1];
  int i;
  for (i=0; i<=FEM_STR_LEN; i++) {tmp[i] = '\0';}

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout, "[E] %s!\n", _("Result type should be specified"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
	
  if (femUI_Mode != FEM_UI_MODE_POST)
  {
    fprintf(msgout, "[E] %s!\n", _("Results not available") ) ;
	  return ( tuiCmdReact(cmd, AF_ERR) ) ;
  }
  
  plotStuff.eres = AF_NO ;

  plotStuff.nres = AF_YES ;
	plotStuff.eresType = ciGetParInt(cmd, 1) ;
	plotStuff.elem = AF_NO ; 

  strncpy (tmp, _("Result"),FEM_STR_LEN);
  strncat (tmp, ": ",FEM_STR_LEN);
  if (ciGetVarNameFromGrp( ciGetParStr(cmd,1), "result") == NULL)
  {
    fprintf(msgout, "[E] %s!\n", _("Unknown result type"));
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  strncat (tmp, ciGetVarNameFromGrp( ciGetParStr(cmd,1), "result"), FEM_STR_LEN);
  femSetPlotTitle( tmp );

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plots nodal solution (if possible): "plpath,what"
 * @param cmd command
 * @return status
 */
int func_gfx_plot_path (char *cmd)
{
#ifdef _USE_GFX_
  char tmp[FEM_STR_LEN+1];
  int i;
  for (i=0; i<=FEM_STR_LEN; i++) {tmp[i] = '\0';}

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout, "[E] %s!\n", _("Result type should be specified"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
	
  if (femUI_Mode != FEM_UI_MODE_POST)
  {
    fprintf(msgout, "[E] %s!\n", _("Results not available") ) ;
	  return ( tuiCmdReact(cmd, AF_ERR) ) ;
  }
  
  plotStuff.eres = AF_NO ;
  plotStuff.nres = AF_NO ;

  plotStuff.path = AF_YES ;
	plotStuff.eresType = ciGetParInt(cmd, 1) ;
	plotStuff.elem = AF_NO ; 

  strncpy (tmp, _("Result"),FEM_STR_LEN);
  strncat (tmp, ": ",FEM_STR_LEN);
  if (ciGetVarNameFromGrp( ciGetParStr(cmd,1), "result") == NULL)
  {
    fprintf(msgout, "[E] %s!\n", _("Unknown result type"));
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  strncat (tmp, ciGetVarNameFromGrp( ciGetParStr(cmd,1), "result"), FEM_STR_LEN);
  femSetPlotTitle( tmp );
printf(">%s<\n", tmp);

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}


/** plots nothing: "noplot"
 * @param cmd command
 * @return status
 */
int func_gfx_no_plot (char *cmd)
{
#ifdef _USE_GFX_
  plotStuff.node   = AF_NO ;
  plotStuff.elem   = AF_NO ;
  plotStuff.disp   = AF_NO ;
  plotStuff.load   = AF_NO ;

  plotStuff.kp     = AF_NO ;
  plotStuff.line   = AF_NO ;
  plotStuff.area   = AF_NO ;
  plotStuff.volume = AF_NO ;

  plotStuff.def_shape = AF_NO ;
  plotStuff.react     = AF_NO ;
  plotStuff.eres      = AF_NO ;
  plotStuff.nres      = AF_NO ;
  plotStuff.path      = AF_NO ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}


/* -------------------------------------- */


/** zoom: "zoom,mult"
 * @param cmd command
 * @return status
 */
int func_gfx_zoom (char *cmd)
{
#ifdef _USE_GFX_
  double mult = 1.0 ;

	if (ciParNum(cmd) > 1) { mult = ciGetParDbl(cmd, 1) ; }
  if (mult == 0.0) {mult = 1.0 ;}
  
  plotTran.zoom += fabs(mult)*plotTran.zoom_step  ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** unzoom: "unzoom,mult"
 * @param cmd command
 * @return status
 */
int func_gfx_unzoom (char *cmd)
{
#ifdef _USE_GFX_
  double mult = 1.0 ;

	if (ciParNum(cmd) > 1) { mult = ciGetParDbl(cmd, 1) ; }
  if (mult == 0.0) {mult = 1.0 ;}
  
  plotTran.zoom -= fabs(mult)*plotTran.zoom_step  ;
  if (plotTran.zoom < 0) { plotTran.zoom = GFX_MIN_ZOOM ; }

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** reset tranformations: "reset"
 * @param cmd command
 * @return status
 */
int func_gfx_reset (char *cmd)
{
#ifdef _USE_GFX_
  femResetTran() ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** move: "move,dir,dist"
 * @param cmd command
 * @return status
 */
int func_gfx_move (char *cmd)
{
#ifdef _USE_GFX_
  double dist = 0.0 ;
  char  *dir = NULL ;

	if (ciParNum(cmd) > 1) { dir = ciGetParStr(cmd, 1) ; }
	else
	{
		fprintf(msgout,"[E] %s!\n", _("At least a move direction is required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
	if (ciParNum(cmd) > 2) { dist = ciGetParDbl(cmd, 2) ; }

  if (strlen(dir) < 1)
  {
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid direction"),dir);
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  if (dist == 0.0) 
	{
#if 0
		dist = plotTran.move_step ; 
#else
		dist = (fabs(plotProp.Max-plotProp.Min)/10.0) ;
		if (plotTran.zoom > 0) { dist = dist / plotTran.zoom ; }
#endif
	}
	else
	{
		dist *= (fabs(plotProp.Max-plotProp.Min)/10.0) ;
		if (plotTran.zoom > 0) { dist = dist / plotTran.zoom ; }
	}
  
  switch (dir[0])
  {
    case 'x':
    case 'X': plotTran.move_x += dist ; 
              break;
    case 'y':
    case 'Y': plotTran.move_y += dist ; 
              break;
    case 'z':
    case 'Z': plotTran.move_z += dist ; 
              break;
    default:
              fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid direction"),dir);
	            return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
              break;
  }


  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** rot: "rot,dir,angle" Angle is in DEGs
 * @param cmd command
 * @return status
 */
int func_gfx_rot (char *cmd)
{
#ifdef _USE_GFX_
  double dist = 0.0 ;
  char  *dir = NULL ;

	if (ciParNum(cmd) > 1) 
	{ 
		dir = ciGetParStr(cmd, 1) ; 
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("At least an axes of rotation is required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	if (ciParNum(cmd) > 2) { dist = ciGetParDbl(cmd, 2) ; }

  if (strlen(dir) < 1)
  {
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid direction"),dir);
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  if (dist == 0.0) { dist = plotTran.move_step ; }
  
  switch (dir[0])
  {
    case 'x':
    case 'X': plotTran.rot_x += dist ; 
              break;
    case 'y':
    case 'Y': plotTran.rot_y += dist ; 
              break;
    case 'z':
    case 'Z': plotTran.rot_z += dist ; 
              break;
    default:
              fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid direction"),dir);
	            return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
              break;
  }


  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** view: "view,(front|back|top|bottom|left|right|iso|aiso)"
 * @param cmd command
 * @return status
 */
int func_gfx_view (char *cmd)
{
#ifdef _USE_GFX_
  char  *dir ;

	if (ciParNum(cmd) > 1) { dir = ciGetParStr(cmd, 1) ; }
  else
  {
    fprintf(msgout, "[E] %s!\n",_("View mode name required"));
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  if (strlen(dir) < 2)
  {
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid view mode"),dir );
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  switch (dir[0])
  {
    case 'l': 
    case 'L': /* left */
              plotTran.rot_x = 0.0 ;
              plotTran.rot_y = 270.0 ;
              plotTran.rot_z = 0.0 ;
              break;
    case 'r':
    case 'R': /* right */
              plotTran.rot_x = 0.0 ;
              plotTran.rot_y = 90.0 ;
              plotTran.rot_z = 0.0 ;
              break;
    case 't':
    case 'T': /* top */
              plotTran.rot_x = 270.0 ;
              plotTran.rot_y = 0.0 ;
              plotTran.rot_z = 0.0 ;
              break;
    case 'b':
    case 'B': if ((dir[1] == 'o')||(dir[1] == 'O'))
              {
                /* bottom*/
                plotTran.rot_x = 90.0 ;
                plotTran.rot_y = 0.0 ;
                plotTran.rot_z = 0.0 ;
              }
              else
              {
                /* back */
                plotTran.rot_x = 0.0 ;
                plotTran.rot_y = 180.0 ;
                plotTran.rot_z = 0.0 ;
              }
              break;
    case 'f':
    case 'F': /* front */
              plotTran.rot_x = 0.0 ;
              plotTran.rot_y = 0.0 ;
              plotTran.rot_z = 0.0 ;
              break;

    case 'i':
    case 'I': /* iso - need some tuning: */
              plotTran.rot_x =  28.0 ;
              plotTran.rot_y =  35.0 ;
              plotTran.rot_z =  9.0 ;
              break;

    case 'a':
    case 'A': /* 2iso - need some tuning: */
              plotTran.rot_x =  30.0 ;
              plotTran.rot_y =  -30.0 ;
              plotTran.rot_z =  -6.0 ;
              break;
    default:
              fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid view mode"),dir );
	            return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
              break;
  }

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** plotcol: "plotcol,(fore|back|text|node|load|disp),r,g,b[,tr]"
 * @param cmd command
 * @return status
 */
int func_gfx_plotcol (char *cmd)
{
#ifdef _USE_GFX_
  char  *dir = NULL ;
  float  r,g,b,tr;

	r = 1 ;
	g = 1 ;
	b = 1 ;
  tr = 1.0 ;

  if (ciParNum(cmd) < 5) 
  {
    fprintf(msgout, "[E] %s!\n",_("Bad number of parameters"));
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }

	if (ciParNum(cmd) > 1) { dir = ciGetParStr(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { r   = fabs(ciGetParDbl(cmd, 2)) ; }
	if (ciParNum(cmd) > 3) { g   = fabs(ciGetParDbl(cmd, 3)) ; }
	if (ciParNum(cmd) > 4) { b   = fabs(ciGetParDbl(cmd, 4)) ; }
	if (ciParNum(cmd) > 5) { tr  = fabs(ciGetParDbl(cmd, 5)) ; }

  if (strlen(dir) < 1)
  {
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),dir );
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  switch (dir[0])
  {
    case 'b': 
    case 'B': /* background */
              plotProp.bgColor[0] = r  ;
              plotProp.bgColor[1] = g  ;
              plotProp.bgColor[2] = b  ;
              plotProp.bgColor[3] = tr ;
              break;
    case 'f':
    case 'F': /* foreground */
              plotProp.fgColor[0] = r  ;
              plotProp.fgColor[1] = g  ;
              plotProp.fgColor[2] = b  ;
              plotProp.fgColor[3] = tr ;
              break;
    case 't':
    case 'T': /* text */
              plotProp.textColor[0] = r  ;
              plotProp.textColor[1] = g  ;
              plotProp.textColor[2] = b  ;
              plotProp.textColor[3] = tr ;
              break;
    case 'n':
    case 'N': /* node */
              plotProp.nodeColor[0] = r  ;
              plotProp.nodeColor[1] = g  ;
              plotProp.nodeColor[2] = b  ;
              plotProp.nodeColor[3] = tr ;
              break;
    case 'l':
    case 'L': /* load */
              plotProp.loadColor[0] = r  ;
              plotProp.loadColor[1] = g  ;
              plotProp.loadColor[2] = b  ;
              plotProp.loadColor[3] = tr ;
              break;
    case 'e':
    case 'E': /* load */
              plotProp.eloadColor[0] = r  ;
              plotProp.eloadColor[1] = g  ;
              plotProp.eloadColor[2] = b  ;
              plotProp.eloadColor[3] = tr ;
              break;

    case 'd':
    case 'D': /* displacement */
              plotProp.dispColor[0] = r  ;
              plotProp.dispColor[1] = g  ;
              plotProp.dispColor[2] = b  ;
              plotProp.dispColor[3] = tr ;
              break;
    case 'h':
    case 'H': /* hilight color */
              plotProp.hilightColor[0] = r  ;
              plotProp.hilightColor[1] = g  ;
              plotProp.hilightColor[2] = b  ;
              plotProp.hilightColor[3] = tr ;
              break;

    default:
              fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),dir );
	            return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
              break;
  }

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}


/** Sets plot properties: "plotprop,something,(yes|no)"
 * @param cmd command
 * @return status
 */
int func_gfx_plot_prop (char *cmd)
{
  int   rv = AF_OK ;
#ifdef _USE_GFX_
  char *what  = NULL ;
  char *mode  = NULL ;
  long  nmode = 0 ;

	if (ciParNum(cmd) <= 1) 
  { 
    fprintf(msgout,"[E] %s!\n", _("Property unspecified"));
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }
	if (ciParNum(cmd) > 1) { what = ciGetParStr(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { 
                            mode  = ciGetParStr(cmd, 2) ; 
                         }

  ciStrCompr(what) ;

  if (mode != NULL)
  {
    ciStrCompr(mode);
    if ((mode[0] == 'y') || (mode[0] == '1'))
    {
      nmode = AF_YES ;
    }
    else
    {
      nmode = AF_NO ;
    }
  }

  rv = AF_ERR_EMP ;

	/** Color palette: */
  if (strcmp(what,"pallen") == 0) {
		if (ciParNum(cmd) > 2)
		{
			nmode = ciGetParInt(cmd, 2);

			if (nmode < 6) { nmode = 6 ; }
			else { nmode = 2*((long)(nmode/2)) ; }

			plotProp.palLen = nmode ;
		}
	}

  if (strcmp(what,"nodenumber") == 0) { plotProp.nodeNumber = nmode ; rv = AF_OK;}
  if (strcmp(what,"elemnumber") == 0) { plotProp.elemNumber = nmode ; rv = AF_OK;}
  if (strcmp(what,"dispnumber") == 0) { plotProp.dispNumber = nmode ; rv = AF_OK;}
  if (strcmp(what,"loadnumber") == 0) { plotProp.loadNumber = nmode ; rv = AF_OK;}

  if (strcmp(what,"smallnode") == 0) { plotProp.smallNode = nmode ; rv = AF_OK;}

  if (strcmp(what,"dispsize") == 0) { plotProp.dispSize = nmode ; rv = AF_OK;}
  if (strcmp(what,"loadsize") == 0) { plotProp.loadSize = nmode ; rv = AF_OK;}

  if (strcmp(what,"kpnumber") == 0) { plotProp.kpNumber = nmode ; rv = AF_OK;}
  if (strcmp(what,"genumber") == 0) { plotProp.geNumber = nmode ; rv = AF_OK;}
  if (strcmp(what,"linenumber") == 0) { plotProp.geNumber = nmode ; rv = AF_OK;}
  if (strcmp(what,"areanumber") == 0) { plotProp.geNumber = nmode ; rv = AF_OK;}
  if (strcmp(what,"volumenumber") == 0) { plotProp.geNumber = nmode ; rv = AF_OK;}
  if (strcmp(what,"divnumber") == 0) 
  { 
    plotProp.divNumber = nmode ; 
    if (nmode == AF_YES) { plotProp.geNumber = AF_NO ; }
    rv = AF_OK;
  }

  if (strcmp(what,"elipticpal") == 0) { plotProp.elipticPal = nmode ; rv = AF_OK;}

  if (strcmp(what,"elemcolet") == 0) 
  { 
    plotProp.elemColET  = nmode ; 
    plotProp.elemColRS  = AF_NO ; 
    plotProp.elemColMat = AF_NO ; 
    plotProp.elemColSet = AF_NO ; 
    rv = AF_OK;
  }
  if (strcmp(what,"elemcolrs") == 0) 
  { 
    plotProp.elemColET  = AF_NO ; 
    plotProp.elemColRS  = nmode ; 
    plotProp.elemColMat = AF_NO ; 
    plotProp.elemColSet = AF_NO ; 
    rv = AF_OK;
  }
  if (strcmp(what,"elemcolmat") == 0) 
  { 
    plotProp.elemColET  = AF_NO ; 
    plotProp.elemColRS  = AF_NO ; 
    plotProp.elemColMat = nmode ; 
    plotProp.elemColSet = AF_NO ; 
    rv = AF_OK;
  }

  if (strcmp(what,"elemcolset") == 0) 
  { 
    plotProp.elemColET  = AF_NO ; 
    plotProp.elemColRS  = AF_NO ; 
    plotProp.elemColMat = AF_NO ; 
    plotProp.elemColSet = nmode ; 
    rv = AF_OK;
  }

  if (strcmp(what,"wireonly") == 0) { plotProp.wireOnly = nmode ; rv = AF_OK;}
  if (strcmp(what,"wireline") == 0) { plotProp.wireLine = nmode ; rv = AF_OK;}
  if (strcmp(what,"wireres") == 0) { plotProp.wireRes = nmode ; rv = AF_OK;}
  if (strcmp(what,"antialias") == 0) { plotProp.antiAlias = nmode ; rv = AF_OK;}
  if (strcmp(what,"jobname") == 0) { plotProp.showJobname = nmode ; rv = AF_OK;}

  if (strcmp(what,"frame") == 0) { plotStuff.frame = nmode ; rv = AF_OK;}
  if (strcmp(what,"autoreplot") == 0) { plotStuff.autoreplot = nmode ; rv = AF_OK;}
  if (strcmp(what,"grid") == 0) { plotStuff.grid = nmode ; rv = AF_OK;}

  if (strcmp(what,"dynView") == 0) { plotProp.ActMot = nmode ; rv = AF_OK;}


	free(what) ;
	free(mode) ;

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, rv) ) ; }
#else
	return ( tuiCmdReact(cmd, rv) ) ;
#endif
}



/** cancels any picking/selecting operation (gedit,pick): "gcancel"
 * @param cmd command
 * @return status
 */
int func_gfx_gcancel (char *cmd)
{
#ifdef _USE_GUI_
#ifdef _USE_GFX_
	femGfxUnSetActiveSelectComman();
  femGfxCancelAllSelects() ;
  fprintf(msgout, "[i] %s.\n",
      _("Interactive operation cancelled"));

  /* replot if possible: */
  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#endif
#endif
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** finishes any picking/selecting operation (gedit,pick): "gfinish"
 * @param cmd command
 * @return status
 */
int func_gfx_gfinish (char *cmd)
{
#ifdef _USE_GUI_
#ifdef _USE_GFX_
	femGfxFinishAllSelects();

  /* replot if possible: */
  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#endif
#endif
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}


/** edits nodes/elems/.. interactivelly: "gedit,(node|elem|disp|force|stop)"
 * @param cmd command
 * @return status
 */
int func_gfx_gedit (char *cmd)
{
#ifdef _USE_GUI_
#ifdef _USE_GFX_
  char  *dir ;

	femGfxSetActiveSelectComman(ciGetParStr(cmd,0));

	if (ciParNum(cmd) > 1) 
  { 
    dir = ciGetParStr(cmd, 1) ; 
    ciStrCompr(dir);
    plotProp.SelAct = GFX_SELE_ACT_EDIT ;
  }
  else
  {
		femGfxUnSetActiveSelectComman();
    plotProp.SelAct = GFX_SELE_ACT_NONE ;
	  return ( tuiCmdReact(cmd, AF_OK) ) ;
  }

  if (strlen(dir) < 1)
  {
		femGfxUnSetActiveSelectComman();
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),dir );
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  switch (dir[0])
  {
    case 'N':
    case 'n': plotProp.SelStuff = GFX_SELE_NODES ;
              break;
    case 'E':
    case 'e': plotProp.SelStuff = GFX_SELE_ELEMS ;
              break;
    case 'D':
    case 'd': plotProp.SelStuff = GFX_SELE_DISPS ;
              break;
    case 'F':
    case 'f': plotProp.SelStuff = GFX_SELE_FORCS ;
              break;
    case 'K':
    case 'k': plotProp.SelStuff = GFX_SELE_KPOINTS ;
              break;
    case 'G':
    case 'g': plotProp.SelStuff = GFX_SELE_ENTS ;

              if (strlen(dir)>=3) /* edit divisions? */
              {
                if ((dir[2] == 'D')||(dir[2] == 'd'))
                {
                  plotProp.SelStuff = GFX_SELE_ENTDIV ;
                }
              }
              break;
    case 'S':
    case 's': plotProp.SelAct = GFX_SELE_ACT_NONE ;
							femGfxUnSetActiveSelectComman();
              break;
    default:  plotProp.SelAct = GFX_SELE_ACT_NONE ;
							femGfxUnSetActiveSelectComman();
              fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),dir );
              free(dir);
	            return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
              break;
  }
  free(dir);

	guiShowPickFrame();
#endif
#endif
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** provide zoom box: "gbox"
 * @param cmd command
 * @return status
 */
int func_gfx_gbox (char *cmd)
{
#ifdef _USE_GUI_
#ifdef _USE_GFX_
	femGfxSetActiveSelectComman(ciGetParStr(cmd,0));

  plotProp.SelStuff = GFX_SELE_SPACE ;
  plotProp.SelAct   = GFX_SELE_PICK_BOX ;
	
	guiShowPickFrame();
#endif
#endif
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** creates something by picking nodes..interactivelly: "gcreate,(n|k|elem|disp|force|stop),[parameters..]" (parameters ignored for "elem")
 * @param cmd command
 * @return status
 */
int func_gfx_gcreate (char *cmd)
{
#ifdef _USE_GUI_
#ifdef _USE_GFX_
  char  *dir = NULL ;
  long   pos = 0 ;

	femGfxSetActiveSelectComman(ciGetParStr(cmd,0));

  femGfxCleanSelCrtStuff();

	if (ciParNum(cmd) > 1) 
  { 
    dir = ciGetParStr(cmd, 1) ; 
    ciStrCompr(dir);
  }
  else
  {
		femGfxUnSetActiveSelectComman();
    femGfxCleanSelCrtStuff();
	  return ( tuiCmdReact(cmd, AF_OK) ) ;
  }

  if (strlen(dir) < 1)
  {
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),dir );
    femGfxCleanSelCrtStuff();
		femGfxUnSetActiveSelectComman();
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

	if (ciParNum(cmd) > 2) 
  { 
    if (femGfxCrtParam != NULL) {free(femGfxCrtParam); femGfxCrtParam = NULL;}

                femGfxCrtParam = ciStrCat5CmdPers(
                    ciGetParStr(cmd,2), 
                    ciGetParStr(cmd,3), 
                    ciGetParStr(cmd,4),
                    ciGetParStr(cmd,5),
                    ciGetParStr(cmd,6)
                    );
  }

  switch (dir[0])
  {
    case 'E':
    case 'e':
							plotProp.SelAct   = GFX_SELE_CRT_ELEM ;
              plotProp.SelStuff = GFX_SELE_NODES ;
	            if (ciParNum(cmd) > 2) 
              { 
                ciRunCmd(ciStrCat2CmdPers("edef",femGfxCrtParam)) ;
              }
              if (fdbInputCountInt(ETYPE, ETYPE_ID,fdbSetInputDefET(0), &pos)< 1)
              {
                fprintf(msgout, "[E] %s!\n", _("Cannot find element type definition"));
                femGfxCleanSelCrtStuff();
                free(dir);
	              return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
              }
              else
              {
                femGfxCrtEnodes = fdbElementType[fdbInputGetInt(ETYPE, ETYPE_TYPE,pos)].nodes ;
                fprintf(msgout,"[ ] %s: %li\n",_("Elements should have this number of nodes"),femGfxCrtEnodes);
              }
              break;

    case 'G':
    case 'g': plotProp.SelAct   = GFX_SELE_CRT_ENTS ;
              plotProp.SelStuff = GFX_SELE_KPOINTS ;

	            if (ciParNum(cmd) > 2) { fdbSetInputDefGEnt(ciGetParInt(cmd,2)) ; }
	            if (ciParNum(cmd) > 3) { fdbSetInputDefET(ciGetParInt(cmd,3)) ; }
	            if (ciParNum(cmd) > 4) { fdbSetInputDefRS(ciGetParInt(cmd,4)) ; }
	            if (ciParNum(cmd) > 5) { fdbSetInputDefMAT(ciGetParInt(cmd,5)) ; }

              if (fdbInputCountInt(ETYPE, ETYPE_ID,fdbSetInputDefET(0), &pos)< 1)
              {
                fprintf(msgout, "[E] %s (%li)!\n", _("Cannot find element type definition"),fdbSetInputDefET(0));
                femGfxCleanSelCrtStuff();
                free(dir);
	              return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
              }

              femGfxCrtEnodes = fdbGeomEnt[fdbSetInputDefGEnt(0)].kps ;
              break;
              
    case 'D':
    case 'd': plotProp.SelAct   = GFX_SELE_CRT_NDISP ;
              plotProp.SelStuff = GFX_SELE_NODES ;
              break;
    case 'F':
    case 'f': plotProp.SelAct   = GFX_SELE_CRT_NLOAD ;
              plotProp.SelStuff = GFX_SELE_NODES ;
              break;

    case 'N':
    case 'n': 
              if (plotStuff.grid != AF_YES){ciRunCmd("dialog,pgrid");}
              plotProp.SelAct   = GFX_SELE_CRT_GR_N ;
              plotProp.SelStuff = GFX_SELE_GRID ;
              plotStuff.grid = AF_YES ;
              break;

    case 'K':
    case 'k': 
              if (plotStuff.grid != AF_YES){ciRunCmd("dialog,pgrid");}
              plotProp.SelAct   = GFX_SELE_CRT_GR_K ;
              plotProp.SelStuff = GFX_SELE_GRID ;
              plotStuff.grid = AF_YES ;
              break;

    case 'S':
    case 's': plotProp.SelAct = GFX_SELE_ACT_NONE ;
							femGfxUnSetActiveSelectComman();
              plotProp.SelStuff = GFX_SELE_NONE ;
              if (femGfxCrtParam != NULL) 
                 {free(femGfxCrtParam); femGfxCrtParam = NULL;}
              break;
    default:  femGfxCleanSelCrtStuff();
							femGfxUnSetActiveSelectComman();
              fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),dir );
              free(dir);
	            return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
              break;
  }
  free(dir);

	guiShowPickFrame();
#endif
#endif
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** modifies picked items by custom function:
 * "pick,item(node|elem|force|disp),function[,parameters]" to end  replace item with "run" or "stop"
 * @param cmd command
 * @return status
 */
int func_gfx_pick (char *cmd)
{
#ifdef _USE_GUI_
#ifdef _USE_GFX_
  int    rv = AF_OK ;
  char  *item  = NULL ;
  long   stuff = 0 ;
  long   i;
  long   len = 0 ;

	femGfxSetActiveSelectComman(ciGetParStr(cmd,0));

	if (ciParNum(cmd) <=2) 
  { 
		femGfxUnSetActiveSelectComman();
    rv = femGfxSeleItemFuncFinish() ;
	  return ( tuiCmdReact(cmd, rv) ) ;
  }

	if (ciParNum(cmd) > 1) { item = ciGetParStr(cmd, 1) ; }

  if (strlen(item) < 1)
  {
		femGfxUnSetActiveSelectComman();
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),item );
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  switch (item[0])
  {
    case 'N':
    case 'n' : stuff = GFX_SELE_NODES ; break ;
    case 'E':
    case 'e' : stuff = GFX_SELE_ELEMS ; break ;
    case 'D':
    case 'd' : stuff = GFX_SELE_DISPS ; break ;
    case 'F':
    case 'f' : stuff = GFX_SELE_FORCS ; break ;
    case 'K':
    case 'k': stuff = GFX_SELE_KPOINTS ; break;
    case 'G':
    case 'g': stuff = GFX_SELE_ENTS ; break;
    case 'S':
    case 's' : 
    case 'R':
    case 'r' : free(item);
							 femGfxUnSetActiveSelectComman();
               rv = femGfxSeleItemFuncFinish() ;
	             return ( tuiCmdReact(cmd, rv) ) ;
               break;
    default: free(item); 
						 femGfxUnSetActiveSelectComman();
             fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),item );
	           return ( tuiCmdReact(cmd, rv) ) ;
             break;
  }

  free(item);

	if (ciParNum(cmd) > 2) {femGfxSelSelectCmd  = ciGetParStr(cmd, 2) ; }

  if (femGfxSelSelectCmd == NULL)
  {
    fprintf(msgout, "[E] %s!\n",_("Invalid command name"));
	  femGfxUnSetActiveSelectComman();
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  if (ciParNum(cmd) > 3)
  {
  
    len = 0 ;

    for (i=3; i<ciParNum(cmd); i++)
    {
      if (ciGetParStr(cmd, i) != NULL) {len += strlen(ciGetParStr(cmd, i));}
    }

    len += ( ciParNum(cmd) - 3 ) ;

    if ((femGfxSelSelectParams =(char *)malloc((len+1)*sizeof(char))) == NULL)
    {
      fprintf(msgout, "[E] %s!\n",_("Out of memory") );
      free(femGfxSelSelectCmd); femGfxSelSelectCmd = NULL ;
	  	femGfxUnSetActiveSelectComman();
	    return ( tuiCmdReact(cmd, AF_ERR_MEM) ) ;
    }

    for (i=0; i<=len; i++) {femGfxSelSelectParams[i] = '\0';}
  
    if (ciGetParStr(cmd,3) != NULL)
    {
      strncpy(femGfxSelSelectParams, ciGetParStr(cmd,3), len+1) ;
    }
  
    for (i=4; i<ciParNum(cmd); i++)
    {
      strncat(femGfxSelSelectParams,",",len+1);
      if (ciGetParStr(cmd,i) != NULL)
      {
        strncat(femGfxSelSelectParams, ciGetParStr(cmd,i), len+1) ;
      }
    }
  }

  plotProp.SelStuff = stuff ;
  plotProp.SelAct   = GFX_SELE_ACT_SELE ;

  fprintf(msgout, "[ ] %s.\n", _("You may start picking items"));

	guiShowPickFrame();
#endif
#endif
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** calls "pick" function to select items:
 * "gsel,what(node|elem|force|disp|stop|run),mode(select,reselect,unselect)" stop with "gsel,(stop|run)"
 * @param cmd command
 * @return status
 */
int func_gfx_pick_sel (char *cmd)
{
#ifdef _USE_GUI_
#ifdef _USE_GFX_
  int    rv = AF_OK ;
  char  *item  = NULL ;
  char  *mode  = NULL ;
  char   sitem[3] ;
  char   smode = 's' ;
  char   ssmode[3] ;
  long   stuff = 0 ;
  long   i;
  long   len = 0 ;

	femGfxSetActiveSelectComman(ciGetParStr(cmd,0));

  sitem[0] = '\0' ;
  sitem[1] = '\0' ;
  sitem[2] = '\0' ;

	if (ciParNum(cmd) <=2) 
  { 
    rv = femGfxSeleItemFuncFinish() ;
	  femGfxUnSetActiveSelectComman();
	  return ( tuiCmdReact(cmd, rv) ) ;
  }

  if (ciParNum(cmd) < 3)
  {
    fprintf(msgout, "[E] %s!\n",_("Mode of selection required"));
	  femGfxUnSetActiveSelectComman();
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

	if (ciParNum(cmd) > 1) { item = ciGetParStr(cmd, 1) ; }

  if (strlen(item) < 1)
  {
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),item );
	  femGfxUnSetActiveSelectComman();
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  switch (item[0])
  {
    case 'N':
    case 'n' : stuff = GFX_SELE_NODES ; sitem[0] = 'n' ; break ;
    case 'E':
    case 'e' : stuff = GFX_SELE_ELEMS ; sitem[0] = 'e' ; break ;
    case 'D':
    case 'd' : stuff = GFX_SELE_DISPS ; sitem[0] = 'd' ; break ;
    case 'F':
    case 'f' : stuff = GFX_SELE_FORCS ; sitem[0] = 'f' ; break ;
    case 'K':
    case 'k': stuff = GFX_SELE_KPOINTS ; sitem[0] = 'k' ; break ; 
    case 'G':
    case 'g': stuff = GFX_SELE_ENTS ;  sitem[0] = 'g' ; sitem[1] = 'e' ; break ;

    case 'S':
    case 's' :
    case 'R':
    case 'r' : free(item);
	  					 femGfxUnSetActiveSelectComman();
               rv = femGfxSeleItemFuncFinish() ;
	             return ( tuiCmdReact(cmd, rv) ) ;
               break;
    default: free(item); 
	  				 femGfxUnSetActiveSelectComman();
             fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid entity"),item );
	           return ( tuiCmdReact(cmd, rv) ) ;
             break;
  }

  free(item);

  if (ciParNum(cmd) > 2) { mode = ciGetParStr(cmd, 2) ; }

  if (mode == NULL)
  {
    fprintf(msgout, "[E] %s!\n",_("Selection mode required"));
		femGfxUnSetActiveSelectComman();
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  if (strlen(mode) < 1)
  {
    fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid mode"),mode );
		femGfxUnSetActiveSelectComman();
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  switch(mode[0])
  {
    case 'S':
    case 's': smode = 'a' ; break ;
    case 'R': 
    case 'r': smode = 'a' ; break ;
    case 'U': 
    case 'u': smode = 'u' ; break ;
    default:  free(mode);
							femGfxUnSetActiveSelectComman();
              fprintf(msgout, "[E] %s: \"%s\"!\n",_("Invalid mode"),item );
	            return ( tuiCmdReact(cmd, rv) ) ;
              break;
              break ;
  }

  free(mode);

  ssmode[0] = smode ;
  if (smode == 'a'){ssmode[1]='d';}
  else {ssmode[1]='\0';}

  ssmode[2]='\0';

  len = strlen(sitem) + strlen("sel,,i,") + 3 ; 

  femGfxSelSelectCmd    = NULL ;
  femGfxSelSelectParams = NULL ;

  if ((femGfxSelSelectCmd =(char *)malloc((len+1)*sizeof(char))) == NULL)
  {
    fprintf(msgout, "[E] %s!\n",_("Out of memory") );
		femGfxUnSetActiveSelectComman();
	  return ( tuiCmdReact(cmd, AF_ERR_MEM) ) ;
  }

  for (i=0; i<=len; i++) {femGfxSelSelectCmd[i] = '\0';}
  
  strncpy(femGfxSelSelectCmd, sitem,len) ;
  strncat(femGfxSelSelectCmd, "sel,",len) ;
  strncat(femGfxSelSelectCmd, ssmode,len) ;
  strncat(femGfxSelSelectCmd, ",i,",len) ; /* command is "i,," so this is OK */

  plotProp.SelStuff = stuff ;
  plotProp.SelAct   = GFX_SELE_ACT_SELE ;

  fprintf(msgout, "[ ] %s.\n", _("You may start picking items"));

	guiShowPickFrame();
#endif
#endif
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** saves plot to file: "gl2ps, filename"
 * @param cmd command
 * @return status
 */
int func_gfx_plot_to_file (char *cmd)
{
  int rv = AF_OK ;
#ifdef _USE_GUI_
#ifdef _USE_GFX_
  if (femPlotFile != NULL)
  {
    free(femPlotFile) ;
    femPlotFile = NULL ;
  }

	if (ciParNum(cmd) > 1) 
  { 
    femPlotFile = ciGetParStr(cmd, 1) ; 
    plotProp.PlotToFile = AF_YES ;
  }
  else
  {
    fprintf(msgout,"[E] %s!\n", _("Filename required") ) ;
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }

  /* replot (to file): */
#ifdef _USE_GUI_
  rv = femPlot() ;
#endif
#ifdef _USE_GLUT_UI_
  rv = femPlot() ;
#endif

  /* cleaning */
  free(femPlotFile) ;
  femPlotFile = NULL ;
  plotProp.PlotToFile = AF_NO ;

  /* replot to normal colors */
#ifdef _USE_GUI_
  femPlot() ;
#endif
#ifdef _USE_GLUT_UI_
  rv = femPlot() ;
#endif

#endif
#endif
	return ( tuiCmdReact(cmd, rv) ) ;
}


/** saves plot to file: "gl2ppm, filename"
 * @param cmd command
 * @return status
 */
int func_gfx_plot_to_ppm_file (char *cmd)
{
  int rv = AF_OK ;
#ifdef _USE_GFX_
  if (femPlotFile != NULL)
  {
    free(femPlotFile) ;
    femPlotFile = NULL ;
  }

	if (ciParNum(cmd) > 1) 
  { 
    femPlotFile = ciGetParStr(cmd, 1) ; 
    plotProp.PlotToFile = AF_YES ;
    plotProp.saveBitmap = AF_YES ;
    plotProp.bitmapType = GFX_PPM ;
  }
  else
  {
    fprintf(msgout,"[E] %s!\n", _("Filename required") ) ;
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }

  /* replot (to file): */
#ifdef _USE_GUI_
  rv = femPlot() ;
#endif
#ifdef _USE_GLUT_UI_
  rv = femPlot() ;
#endif

  /* cleaning */
  free(femPlotFile) ;
  femPlotFile = NULL ;
  plotProp.PlotToFile = AF_NO ;

  /* replot to normal colors */
#ifdef _USE_GUI_
  femPlot() ;
#endif
#ifdef _USE_GLUT_UI_
  rv = femPlot() ;
#endif

#endif
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** saves plot to file: "gl2tiff, filename"
 * @param cmd command
 * @return status
 */
int func_gfx_plot_to_tiff_file (char *cmd)
{
  int rv = AF_OK ;
#ifdef _USE_GFX_
  if (femPlotFile != NULL)
  {
    free(femPlotFile) ;
    femPlotFile = NULL ;
  }

	if (ciParNum(cmd) > 1) 
  { 
    femPlotFile = ciGetParStr(cmd, 1) ; 
    plotProp.PlotToFile = AF_YES ;
    plotProp.saveBitmap = AF_YES ;
    plotProp.bitmapType = GFX_TIFF ;
  }
  else
  {
    fprintf(msgout,"[E] %s!\n", _("Filename required") ) ;
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }

  /* replot (to file): */
#ifdef _USE_GUI_
  rv = femPlot() ;
#endif
#ifdef _USE_GLUT_UI_
  rv = femPlot() ;
#endif

  /* cleaning */
  free(femPlotFile) ;
  femPlotFile = NULL ;
  plotProp.PlotToFile = AF_NO ;

  /* replot to normal colors */
#ifdef _USE_GUI_
  femPlot() ;
#endif
#ifdef _USE_GLUT_UI_
  rv = femPlot() ;
#endif

#endif
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** sets format of file produced by gl2ps: "glformat,(PS|EPS|PDF)"
 * @param cmd command
 * @return status
 */
int func_gfx_plot_format (char *cmd)
{
  int rv = AF_OK ;
#ifdef _USE_GFX_
#ifdef _USE_GL2PS_
  char *form = NULL ;

	if (ciParNum(cmd) > 1) 
  { 
    form  = ciGetParStr(cmd, 1) ; 
    ciStrCompr(form);
    if (strlen(form) < 2)
    {
      fprintf(msgout, "[E] %s!\n", _("At least two characters are required") ); 
      rv = AF_ERR_EMP ;
    }
    else
    {
      switch (form[1])
      {
        case 's' : plotProp. plotFormat = GL2PS_PS  ; break ;
        case 'p' : plotProp. plotFormat = GL2PS_EPS ; break ;
        case 'd' : plotProp. plotFormat = GL2PS_PDF ; break ;
        default:
          fprintf(msgout, "[E] %s!\n", _("Unknown format") ); 
          rv = AF_ERR_VAL ;
          break ;
      }
    }
    free(form);
  }
  else
  {
    fprintf(msgout, "[E] %s!\n", _("Format name required") ); 
    rv = AF_ERR_EMP ;
  }

#endif
#endif
	return ( tuiCmdReact(cmd, rv) ) ;
}


/** hide things (prevent from plotting):
 * "pgrid,plane,dx,dy,lenx,leny[,x0,y0,z0,angle_in,angle_out]
 * @param cmd command
 * @return status
 */
int func_gfx_set_grid(char *cmd)
{
#ifdef _USE_GFX_
  int rv = AF_OK ;
  double dx = 0 ;
  double dy = 0 ;
  double x0 = 0 ;
  double y0 = 0 ;
  double z0 = 0 ;
  double ang_in = 0 ;
  double ang_out = 0 ;
  long plane = PICK_GRID_XY ;
  long nx = 0 ;
  long ny = 0 ;;

	if (ciParNum(cmd) < 6) 
	{
		fprintf(msgout,"[E] %s!\n",_("You must specify at least: plane, dx, dy, number x and number y"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  if (ciParNum(cmd) > 1) { plane  = ciGetParInt(cmd, 1); }
    
  if (ciParNum(cmd) > 2) { dx     = ciGetParDbl(cmd, 2); }
  if (ciParNum(cmd) > 3) { dy     = ciGetParDbl(cmd, 3); }
    
  if (ciParNum(cmd) > 4) { nx = ciGetParInt(cmd, 4); }
  if (ciParNum(cmd) > 5) { ny = ciGetParInt(cmd, 5); }

  if (ciParNum(cmd) > 6) { x0 = ciGetParDbl(cmd, 6); }
  if (ciParNum(cmd) > 7) { y0 = ciGetParDbl(cmd, 7); }
  if (ciParNum(cmd) > 8) { z0 = ciGetParDbl(cmd, 8); }

  if (ciParNum(cmd) > 9) { ang_in = FEM_PI*ciGetParDbl(cmd, 9)/180.0; }
  if (ciParNum(cmd) > 10) { ang_out = FEM_PI*ciGetParDbl(cmd, 10)/180.0; }

  rv = fdb_set_grid( plane, dx, dy, nx, ny, x0, y0, z0, ang_in, ang_out);

  if (rv == AF_OK) { plotStuff.grid = AF_YES ; }

  if (plotStuff.autoreplot == AF_YES)
     { return( func_gui_replot(cmd) ) ; }
  else
     { return ( tuiCmdReact(cmd, AF_OK) ) ; }
#else
	return ( tuiCmdReact(cmd, AF_OK) ) ;
#endif
}

/** Writes current config (basic+gfx) to a selected file: "writegfxconf,file"
 * @param cmd command
 * @return status
 */
int func_var_write_gfx_conf_file (char *cmd)
{
	int    rv    = AF_OK ;

	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Name of file required")) ;
		return(AF_ERR_VAL);
	}

  if (ciGetParStr(cmd,1) == NULL)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty filename not allowed")) ;
		return(AF_ERR_VAL);
  }
  if (strlen(ciGetParStr(cmd,1)) < 1)
  {
		fprintf(msgout,"[E] %s!\n", _("Filename too short")) ;
		return(AF_ERR_VAL);
  }

	rv = femUIWriteConfigFile(ciGetParStr(cmd,1)) ;

#ifdef _USE_GFX_
	if (rv == AF_OK)
	{
		rv = femGfxWriteConfig_data(ciGetParStr(cmd,1),"a") ;
	}
#endif

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Writes current config (basic+gfx+gui) to a selected file: "writeguiconf,file"
 * @param cmd command
 * @return status
 */
int func_var_write_gui_conf_file (char *cmd)
{
	int    rv    = AF_OK ;

	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Name of file required")) ;
		return(AF_ERR_VAL);
	}

  if (ciGetParStr(cmd,1) == NULL)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty filename not allowed")) ;
		return(AF_ERR_VAL);
  }
  if (strlen(ciGetParStr(cmd,1)) < 1)
  {
		fprintf(msgout,"[E] %s!\n", _("Filename too short")) ;
		return(AF_ERR_VAL);
  }

	rv = femUIWriteConfigFile(ciGetParStr(cmd,1)) ;

#ifdef _USE_GFX_
	if (rv == AF_OK)
	{
		rv = femGfxWriteConfig_data(ciGetParStr(cmd,1),"a") ;

		if (rv == AF_OK)
		{
#ifdef _USE_GFX_
#ifndef _USE_GLUT_UI_
		rv = femGUIWriteConfig_data(ciGetParStr(cmd,1),"a") ;
#endif
#endif
		}
	}
#endif

	return ( tuiCmdReact(cmd, rv) ) ;
}
/* end of cmd_plot.c */
