/*
   File name: cmd_help.c
   Date:      2008/01/05 18:58
   Author:    Jiri Brozovsky

   Copyright (C) 2008 Jiri Brozovsky

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

	 FEM User Interface - help functions
*/

#include "fem_tui.h"
#include "cint.h"
#include "fdb_edef.h"
#include "fdb_mdef.h"
#include "fdb_geom.h"

extern tMT  fdbMatType[] ;
extern long fdbMatTypeLen ;

/** Provides help for selected material type
 * @param type type ID
 */
int tui_help_material(long type)
{
  int rv = AF_OK ;
  long i ;

  if ((type < 1) || (type >= fdbMatTypeLen))
  {
    fprintf(msgout, "[E] %s: %li!\n", _("Material type not known"),type);
    return(AF_ERR_VAL);
  }

  if ((rv=fdbPrnOpenFile(fdbPrnFile)) != AF_OK) { return(rv); }


  fdbPrnBeginTable(fdbPrnFile, 2, _("Material Type Summary")) ;

  fdbPrnTableItemStr(fdbPrnFile,0, _("Type"));
  fdbPrnTableItemInt(fdbPrnFile,1,type);
  fdbPrnTableItemStr(fdbPrnFile,0, _("Name"));
  fdbPrnTableItemStr(fdbPrnFile,1,ciGetVarNameFromGrp(fdbFemStrFromInt(type),"mattype"));
  fdbPrnEndTable(fdbPrnFile);

  fdbPrnBeginTable(fdbPrnFile, 2, _("Values")) ;
  fdbPrnTableItemStr(fdbPrnFile,0, _("Static"));
  fdbPrnTableItemInt(fdbPrnFile,1,fdbMatType[type].vals );
  fdbPrnTableItemStr(fdbPrnFile,0, _("Repeating"));
  fdbPrnTableItemInt(fdbPrnFile,1,fdbMatType[type].vals_rp );
  fdbPrnEndTable(fdbPrnFile);

  fdbPrnBeginTable(fdbPrnFile, 3, _("Static Values")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 2, _("Name")) ;
  for (i=0; i< fdbMatType[type].vals; i++)
  {
    fdbPrnTableItemInt(fdbPrnFile,0, i+1);
    fdbPrnTableItemInt(fdbPrnFile,1, fdbMatType[type].val[i]);
    fdbPrnTableItemStr(fdbPrnFile,2, ciGetVarNameFromGrp(fdbFemStrFromInt(fdbMatType[type].val[i]), "material") );
  }
  fdbPrnEndTable(fdbPrnFile);

  fdbPrnBeginTable(fdbPrnFile, 3, _("Repeating Values")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 2, _("Name")) ;
  for (i=0; i< fdbMatType[type].vals_rp; i++)
  {
    fdbPrnTableItemInt(fdbPrnFile,0, i+1);
    fdbPrnTableItemInt(fdbPrnFile,1, fdbMatType[type].val_rp[i]);
    fdbPrnTableItemStr(fdbPrnFile,2, ciGetVarNameFromGrp(fdbFemStrFromInt(fdbMatType[type].val_rp[i]), "material") );
  }
  fdbPrnEndTable(fdbPrnFile);

  rv = fdbPrnCloseFile(fdbPrnFile);

  return(rv);
}

/** Provides help for selected element type
 * @param type type ID
 */
int tui_help_elem(long type)
{
  int rv = AF_OK ;
  long i ;

  if ((type < 1) || (type >= fdbElementTypeLen))
  {
    fprintf(msgout, "[E] %s: %li!\n", _("Element type not known"), type);
    return(AF_ERR_VAL);
  }

  if ((rv=fdbPrnOpenFile(fdbPrnFile)) != AF_OK) { return(rv); }


  fdbPrnBeginTable(fdbPrnFile, 2, _("Element Type Summary")) ;

  fdbPrnTableItemStr(fdbPrnFile,0, _("Type"));
  fdbPrnTableItemInt(fdbPrnFile,1,type);
  fdbPrnTableItemStr(fdbPrnFile,0, _("Name"));
  fdbPrnTableItemStr(fdbPrnFile,1,ciGetVarNameFromGrp(fdbFemStrFromInt(type),"etype"));
  fdbPrnEndTable(fdbPrnFile);

  fdbPrnBeginTable(fdbPrnFile, 2, _("Values")) ;
  fdbPrnTableItemStr(fdbPrnFile,0, _("DOFs"));
  fdbPrnTableItemInt(fdbPrnFile,1,fdbElementType[type].ndofs );
  fdbPrnTableItemStr(fdbPrnFile,0, _("Static"));
  fdbPrnTableItemInt(fdbPrnFile,1,fdbElementType[type].reals );
  fdbPrnTableItemStr(fdbPrnFile,0, _("Repeating"));
  fdbPrnTableItemInt(fdbPrnFile,1,fdbElementType[type].reals_rep );
  fdbPrnEndTable(fdbPrnFile);

  fdbPrnBeginTable(fdbPrnFile, 3, _("Degrees of Freedom")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 2, _("Name")) ;
  for (i=0; i< fdbElementType[type].ndofs; i++)
  {
    fdbPrnTableItemInt(fdbPrnFile,0, i+1);
    fdbPrnTableItemInt(fdbPrnFile,1, fdbElementType[type].ndof[i]);
    fdbPrnTableItemStr(fdbPrnFile,2, ciGetVarNameFromGrp(fdbFemStrFromInt(fdbElementType[type].ndof[i]), "degree_of_freedom") );
  }
  fdbPrnEndTable(fdbPrnFile);



  fdbPrnBeginTable(fdbPrnFile, 3, _("Static Input Values")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 2, _("Name")) ;
  for (i=0; i< fdbElementType[type].reals; i++)
  {
    fdbPrnTableItemInt(fdbPrnFile,0, i+1);
    fdbPrnTableItemInt(fdbPrnFile,1, fdbElementType[type].r[i]);
    fdbPrnTableItemStr(fdbPrnFile,2, ciGetVarNameFromGrp(fdbFemStrFromInt(fdbElementType[type].r[i]), "real") );
  }
  fdbPrnEndTable(fdbPrnFile);

  fdbPrnBeginTable(fdbPrnFile, 3, _("Repeating Input Values")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 2, _("Name")) ;
  for (i=0; i< fdbElementType[type].reals_rep; i++)
  {
    fdbPrnTableItemInt(fdbPrnFile,0, i+1);
    fdbPrnTableItemInt(fdbPrnFile,1, fdbElementType[type].r_rep[i]);
    fdbPrnTableItemStr(fdbPrnFile,2, ciGetVarNameFromGrp(fdbFemStrFromInt(fdbElementType[type].r_rep[i]), "real") );
  }
  fdbPrnEndTable(fdbPrnFile);


	/* results: */
  fdbPrnBeginTable(fdbPrnFile, 3, _("Static Result Data")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 2, _("Name")) ;
  for (i=0; i< fdbElementType[type].res; i++)
  {
    fdbPrnTableItemInt(fdbPrnFile,0, i+1);
    fdbPrnTableItemInt(fdbPrnFile,1, fdbElementType[type].nres[i]);
    fdbPrnTableItemStr(fdbPrnFile,2,
				ciGetVarNameFromGrp(fdbFemStrFromInt(fdbElementType[type].nres[i]), "result")
				);
  }
  fdbPrnEndTable(fdbPrnFile);

  fdbPrnBeginTable(fdbPrnFile, 3, _("Repeating Result Data")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fdbPrnFile, 2, _("Name")) ;
  for (i=0; i< fdbElementType[type].res_rp; i++)
  {
    fdbPrnTableItemInt(fdbPrnFile,0, i+1);
    fdbPrnTableItemInt(fdbPrnFile,1,
				fdbElementType[type].nres_rp[i]);
    fdbPrnTableItemStr(fdbPrnFile,2, 
				ciGetVarNameFromGrp(fdbFemStrFromInt(fdbElementType[type].nres_rp[i]), "result") 
				);
  }
  fdbPrnEndTable(fdbPrnFile);


  rv = fdbPrnCloseFile(fdbPrnFile);

  return(rv);
}

/** provides help to predefined output: "help,(e|m)[,number]" */
int func_help (char *cmd)
{
  int rv = AF_OK ;
  char *dir = NULL ;
  long  num = 0 ;
  long  to = 0 ;
  long  i ;

	if (ciParNum(cmd) < 3)
  {
    fprintf (msgout,"[I] %s:\n",FEM_TUI_RELEASE);
    fprintf (msgout,"[ ]   %s: %li\n",_("Element types"),fdbElementTypeLen-1);
    fprintf (msgout,"[ ]   %s: %li\n",_("Material types"),fdbMatTypeLen-1);
	  if (femUI_Mode == FEM_UI_MODE_PREP)
	    { fprintf (msgout,"[ ] %s: %s\n",_("Mode"),_("Preprocessing")); }
	  if (femUI_Mode == FEM_UI_MODE_POST)
	    { fprintf (msgout,"[ ] %s: %s\n",_("Mode"),_("Postprocessing")); }

    fprintf (msgout,"[ ]   %s: %s\n",_("Current jobname"),femGetJobname());
	  if (femDataDir != NULL)
	    { fprintf (msgout,"[ ]   %s: %s\n",_("Data directory"),femDataDir); }

    fprintf (msgout,"[i] %s \"%s,(e,m),number\" %s.\n", _("Use"), 
        ciGetParStrNoExpand(cmd,0),_("for element or material type details"));
	  return(tuiCmdReact(cmd, AF_OK));
  }

  if (strlen(dir = ciGetParStrNoExpand(cmd, 1)) < 1) 
	{
    rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Invalid item name"));
		goto memFree;
	}

  if (ciParNum(cmd) > 2) { num = ciGetParInt(cmd,2) ; }
  if (ciTestStringALL(cmd,2) == AF_YES) { to = -1 ; }
  
  switch(dir[0])
  {
    case 'm':
    case 'M':
      if (to == -1) 
      {
        to = fdbMatTypeLen ;
        for (i=1; i<to; i++) { tui_help_material(i); }
      }
      else
      {
        tui_help_material(num); 
      }
      break; 
    case 'e':
    case 'E':
     if (to == -1) 
      {
        to = fdbElementTypeLen ;
        for (i=1; i<to; i++) { tui_help_elem(i); }
      }
      else
      {
        tui_help_elem(num); 
      }
      break;
    case 's':
    case 'S': /* TODO */
    default: 
		        fprintf(msgout,"[E] %s!\n", _("Invalid item type"));
            rv = AF_ERR_VAL ;
            goto memFree ;
            break;
  }

memFree:
  if (dir != NULL) {free(dir); dir = NULL ;}
	return(tuiCmdReact(cmd, rv));
}


/* end of cmd_help.c */
