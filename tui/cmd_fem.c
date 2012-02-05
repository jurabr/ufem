/*
   File name: cmd_fem.c
   Date:      2003/08/31 14:03
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

	 FEM User Interface - commands related to f.e. model

	 $Id: cmd_fem.c,v 1.59 2005/07/11 17:57:09 jirka Exp $
*/

#include "fem_tui.h"
#include "cint.h"
#include "fdb_fem.h"
#include "fdb_geom.h"
#include "fdb_edef.h"
#include "fdb_res.h"

extern FILE *fdbPrnFile ;  /* file pointer */
extern long  fdbOutputFormat ;
extern char *fdbPrnFileName ;
extern long  fdbPrnAutoName ;
extern long  fdbPrnAppendFile ;
extern char *fdbPrnViewCommand;

extern int fdbFromThermalToStruct(void);

/* ** DATA I/O ** */

/** Writes data to file (in current working directory) "save,filename"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_fem_save (char *cmd)
{
	int    rv = AF_OK ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[w] %s: \"%s\".\n",
				_("No filename given - using default"),
				ciSetPath(femGetDataDir(),femGetJobname(),"db")) ;

		rv = fdbInputSaveData(ciSetPath(femGetDataDir(),femGetJobname(),"db")) ;
	}
	else
	{
		rv = fdbInputSaveData(ciGetParStr(cmd, 1)) ;
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Reads data from file (in current working directory) "resume,filename"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_fem_resu (char *cmd)
{
	int    rv = AF_OK ;
#if 0
	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Filename required")) ;
		return(AF_ERR_VAL);
	}
#endif

	femUI_Mode = FEM_UI_MODE_PREP ;

  resPathFreeAll();

	if (ResLen > 0) { fdbFreeResSpacePtrs(); }
	ResLen = 0 ;
  if (aResLen > 0) { fdbAvResFreeSpace(); }
  aResLen = 0 ;

	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[w] %s: \"%s\".\n",
				_("No filename given - using default"),
				ciSetPath(femGetDataDir(),femGetJobname(),"db")) ;

		rv = fdbInputReadData(ciSetPath(femGetDataDir(),femGetJobname(),"db")) ;
	}
	else
	{
		rv = fdbInputReadData(ciGetParStr(cmd, 1)) ;
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Cleans all data: "cleandata"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_fem_clean_data (char *cmd)
{
	int    rv = AF_OK ;

	femUI_Mode = FEM_UI_MODE_PREP ;

	if (ResLen > 0) { fdbFreeResSpacePtrs(); }
	ResLen = 0 ;
  if (aResLen > 0) { fdbAvResFreeSpace(); }
  aResLen = 0 ;

	rv = fdbInputCleanData();

  resPathFreeAll();

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets coordinate system: "csys,type(cart|cylxy|cylyz|cylzx)[,x0,y0,z0]"
 * Empty filename means no viewer
 * @param cmd command
 * @return status
 */
int func_fem_set_csys(char *cmd)
{
	int     rv    = AF_OK ;
  char   *stype = NULL ;
  long    type  = FDB_CSYS_CART  ;
  double  x     = 0.0 ;
  double  y     = 0.0 ;
  double  z     = 0.0 ;

	FEM_TEST_PREPROCESSOR

  if (ciParNum(cmd) <= 1)
  {
    fdbCoodSysReset();
  }
  else
  {
    if (ciParNum(cmd) > 1)
    {
      type = FDB_CSYS_CART ;

      stype = ciGetParStr(cmd, 1) ;
      if (stype != NULL)
      {
        ciStrCompr(stype);

        rv = AF_ERR_VAL ;

        if (strcmp(stype, "cylxy") == 0) { type = FDB_CSYS_CYL_XY ; rv = AF_OK ; }
        if (strcmp(stype, "cylyx") == 0) { type = FDB_CSYS_CYL_XY ; rv = AF_OK ; }
        if (strcmp(stype, "cyl") == 0) { type = FDB_CSYS_CYL_XY ; rv = AF_OK ; }

        if (strcmp(stype, "cylyz") == 0) { type = FDB_CSYS_CYL_YZ ; rv = AF_OK ; }
        if (strcmp(stype, "cylzy") == 0) { type = FDB_CSYS_CYL_YZ ; rv = AF_OK ; }

        if (strcmp(stype, "cylzx") == 0) { type = FDB_CSYS_CYL_ZX ; rv = AF_OK ; }
        if (strcmp(stype, "cylxz") == 0) { type = FDB_CSYS_CYL_ZX ; rv = AF_OK ; }

        if (strcmp(stype, "cart") == 0) { type = FDB_CSYS_CART ; rv = AF_OK ; }

        if (rv != AF_OK)
        {
          fprintf(msgout,"[E] %s!\n", _("Unknown coorditate system"));
          goto memFree;
        }
      }
    }

    if (ciParNum(cmd) > 2) { x = ciGetParDbl(cmd, 2) ; }
    if (ciParNum(cmd) > 3) { y = ciGetParDbl(cmd, 3) ; }
    if (ciParNum(cmd) > 4) { z = ciGetParDbl(cmd, 4) ; }

    rv = fdbCoodSysSet(type, x, y, z);

memFree:
    if (stype != NULL)
    {
      free(stype);
      stype = NULL ;
    }
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/* GEOMETRIC MODEL **************************************** */
/* ------------------------------------------- */
/* ** KEY POINTS ** */

/** Creates node "k,number,x,y,z"
 * @param cmd command
 * @return status
 */
int func_fem_k (char *cmd)
{
	int    rv = AF_OK ;
	long   id = 0 ;   /* node ID      */
	double x  = 0.0 ; /* x coordinate */
	double y  = 0.0 ; /* y coordinate */
	double z  = 0.0 ; /* z coordinate */

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { x  = ciGetParDbl(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { y  = ciGetParDbl(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { z  = ciGetParDbl(cmd, 4) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"k = %li [%e, %e, %e]\n",id,x,y,z);
#endif

	rv = f_k_new_change(id, x, y, z, NULL);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists nodes "klist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_k_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(KPOINT, KPOINT_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_k_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes nodes "kdel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_k_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Keypoint number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(KPOINT, KPOINT_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_k_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[E] %s %li %s.\n", _("Keypoint"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Splits keypoint into two or more: "ksplit,from,to"
 * note: it have to be attached at least to two entities!
 */ 
int func_fem_k_split(char *cmd)
{
  int rv = AF_OK ;
  long i ;
  long from = 0 ;
  long to = 0 ;

	FEM_TEST_PREPROCESSOR

  if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Keypoint number required"));
		goto memFree;
	}

	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (to < from) {to = from ;}
  
  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    to = fdbInputTabLenAll(KPOINT) ;
    for (i=0; i<to; i++) { f_k_split_through_ge_pos(i); }
  }
  else
  {
    for (i=from; i<=to; i++) { f_k_split_through_ge_id(i); }
  }

memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Merges/joins all (selected) coincident KPs "kmerge"
 * @param cmd command
 * @return status
 */
int func_fem_k_merge (char *cmd)
{
	int    rv    = AF_OK ;

	FEM_TEST_PREPROCESSOR

  rv = f_k_join_all() ;

	return ( tuiCmdReact(cmd, rv) ) ;
}


/* **  ENTITIES ** */
/** Sets default division "ddiv,number_of_divisions"
 * using ALL as number you HAVE TO fill all items
 * @param cmd command
 * @return status
 */
int func_fem_default_div (char *cmd)
{
	int    rv  = AF_OK ;
  long   div = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { div  = ciGetParInt(cmd, 1) ; }
  div = fdbSetInputDefDiv(div);
  fprintf(msgout, "[ ] %s: %li\n", _("Default divison set to"),div);

	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Creates element "lp,number,type,real,mat [,set]" Note: if you are
 * using ALL as number you HAVE TO fill all items
 * @param cmd command
 * @return status
 */
int func_fem_entity_prop (char *cmd, long ent_type)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   et  = 0 ;
	long   rs  = 0 ;
	long   mat = 0 ;
	long   set = 0 ;
  long   i ;
	long   et0  = 0 ;
	long   rs0  = 0 ;
	long   mat0 = 0 ;
	long   set0 = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { et  = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { rs  = ciGetParInt(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { mat = ciGetParInt(cmd, 4) ; }
	if (ciParNum(cmd) > 5) { set = ciGetParInt(cmd, 5) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"l = %li: t=%li r=%li m=%li [%li]\n",id,et,rs,mat,set);
#endif


  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
    et0  = et  ;
    rs0  = rs  ;
    mat0 = mat ;
    set0 = set ;

    for (i=0; i<fdbInputTabLenAll(ENTITY); i++)
    {
      if ((fdbInputGetInt(ENTITY, ENTITY_TYPE, i) != ent_type)&&(0 != ent_type)) { continue ; }

      if (fdbInputTestSelect(ENTITY,i) == AF_YES)
      {
        if (et0  == 0) {et  = fdbInputGetInt(ENTITY, ENTITY_ETYPE, i); }
        if (rs0  == 0) {rs  = fdbInputGetInt(ENTITY, ENTITY_RS,   i); }
        if (mat0 == 0) {mat = fdbInputGetInt(ENTITY, ENTITY_MAT,  i); }
        if (set0 == 0) {set = fdbInputGetInt(ENTITY, ENTITY_SET,  i); }

	      rv = f_ent_new_change(fdbInputGetInt(ENTITY, ENTITY_ID, i), ent_type, et, rs, mat, set);

        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else /* change/creation of single element */
  {
	  rv = f_ent_new_change(id, ent_type, et, rs, mat, set);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Changes entity set data "gechset,number,set" Note: if you are
 * @param cmd command
 * @return status
 */
int func_fem_entity_change_set (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   set = 0 ;
  long   i ;
	long   set0 = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { set = ciGetParInt(cmd, 2) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e = %li: s = %li\n",id,set);
#endif


  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
    set0 = set ;

    for (i=0; i<fdbInputTabLenAll(ENTITY); i++)
    {
      if (fdbInputTestSelect(ENTITY,i) == AF_YES)
      {
        if (set0 == 0) {set = fdbInputGetInt(ENTITY, ENTITY_SET,  i); }

	      rv = f_ent_change_set(fdbInputGetInt(ENTITY, ENTITY_ID, i), set);

        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else /* change of single element */
  {
	  rv = f_ent_change_set(id, set);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Creates entity "ge,number,nodes.."
 * @param cmd command
 * @return status
 */
int func_fem_entity_kps (char *cmd, long ent_type)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	int    i   = 0 ;
	long   nnum  = 0 ;
	long  *nodes = NULL ;

	FEM_TEST_PREPROCESSOR

	if ((nnum=ciParNum(cmd)) < 3) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Keypoints should be specified"));
		goto memFree;
	}

	nnum -= 2 ;

	if ((nodes = femIntAlloc(nnum)) == NULL)
	{
		rv = AF_ERR_MEM ;
		fprintf(msgout,"[E] %s!\n", _("Out of memory"));
		goto memFree;
	}

	id  = ciGetParInt(cmd, 1) ; 

	for (i=0; i<nnum; i++) { nodes[i]  = ciGetParInt(cmd, 2+i) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e [%li]: ",id);
	for (i=0; i<nnum; i++) { fprintf(msgout," %li", nodes[i]); }
	fprintf(msgout,"\n");
#endif

	rv = f_entkp_change(id, nodes, nnum, ent_type);

  free(nodes); nodes = NULL ;

memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Creates entity "gesize,type,number,x,y,z,dx,dy,dz."
 * @param cmd command
 * @return status
 */
int func_fem_entity_create_dim (char *cmd)
{
	int    rv  = AF_OK ;
	double x  = 0 ;
	double y  = 0 ;
	double z  = 0 ;
	double dx = 0 ;
	double dy = 0 ;
	double dz = 0 ;
	long   type = 0 ;
	long   id  = -1 ;

	FEM_TEST_PREPROCESSOR
	
	if ((ciParNum(cmd)) < 8) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("All parameters are required: type, id,x,y,z,dx,dy,dz"));
		goto memFree;
	}

	if (ciParNum(cmd) > 1) { type = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { id   = ciGetParInt(cmd, 2) ; }

	if ((type <2) || (type > 5))
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Invalid entity type"));
		goto memFree;
	}

	if (ciParNum(cmd) > 3) { x   = ciGetParDbl(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { y   = ciGetParDbl(cmd, 4) ; }
	if (ciParNum(cmd) > 5) { z   = ciGetParDbl(cmd, 5) ; }

	if (ciParNum(cmd) > 6) { dx   = ciGetParDbl(cmd, 6) ; }
	if (ciParNum(cmd) > 7) { dy   = ciGetParDbl(cmd, 7) ; }
	if (ciParNum(cmd) > 8) { dz   = ciGetParDbl(cmd, 8) ; }
	
	 rv = f_ent_create_dim(
    type,
    id, 
    x, 
    y, 
    z, 
    dx, 
    dy, 
    dz);

memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets entity divisions "ed,number,nodes.."
 * @param cmd command
 * @return status
 */
int func_fem_entity_divs (char *cmd, long ent_type)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	int    i   = 0 ;
	long   nnum  = 0 ;
	long  *nodes = NULL ;

	FEM_TEST_PREPROCESSOR

	if ((nnum=ciParNum(cmd)) < 3) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Keypoints should be specified"));
		goto memFree;
	}

	nnum -= 2 ;

	if ((nodes = femIntAlloc(nnum)) == NULL)
	{
		rv = AF_ERR_MEM ;
		fprintf(msgout,"[E] %s!\n", _("Out of memory"));
		goto memFree;
	}

	id  = ciGetParInt(cmd, 1) ; 

	for (i=0; i<nnum; i++) { nodes[i]  = ciGetParInt(cmd, 2+i) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e [%li]: ",id);
	for (i=0; i<nnum; i++) { fprintf(msgout," %li", nodes[i]); }
	fprintf(msgout,"\n");
#endif

  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
    for (i=0; i<fdbInputTabLenAll(ENTITY); i++)
    {
      if ((fdbInputGetInt(ENTITY, ENTITY_TYPE, i) != ent_type)&&(0 != ent_type)) { continue ; }

      if (fdbInputTestSelect(ENTITY,i) == AF_YES)
			{
				rv = f_entkp_div_change(fdbInputGetInt(ENTITY,ENTITY_ID, i), nodes, nnum);
				if (rv != AF_OK) {goto memFree;}
			}
		}
	}
	else
	{
		rv = f_entkp_div_change(id, nodes, nnum);
	}

  free(nodes); nodes = NULL ;

memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists entities "elist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_entity_list (char *cmd, long ent_type)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(ENTITY, ENTITY_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_ent_list_prn(fdbPrnFile, from, to, ent_type);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes entities "edel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_entity_del (char *cmd, long ent_type)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Element number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(ENTITY, ENTITY_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_ent_delete(i, ent_type) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Element"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* ----- */

int func_fem_line_prop (char *cmd)
    { return( func_fem_entity_prop (cmd, 1)); }

int func_fem_line_kps (char *cmd)
    { return( func_fem_entity_kps (cmd, 1)); }

int func_fem_line_list (char *cmd)
    { return( func_fem_entity_list (cmd, 1)); }

int func_fem_line_divs (char *cmd)
    { return( func_fem_entity_divs (cmd, 1)); }

int func_fem_line_del (char *cmd)
    { return( func_fem_entity_del (cmd, 1)); }
    
/* ----- */

int func_fem_area_prop (char *cmd)
    { return( func_fem_entity_prop (cmd, 2)); }

int func_fem_area_kps (char *cmd)
    { return( func_fem_entity_kps (cmd, 2)); }

int func_fem_area_list (char *cmd)
    { return( func_fem_entity_list (cmd, 2)); }

int func_fem_area_divs (char *cmd)
    { return( func_fem_entity_divs (cmd, 2)); }

int func_fem_area_del (char *cmd)
    { return( func_fem_entity_del (cmd, 2)); }
    
/* ----- */

int func_fem_volume_prop (char *cmd)
    { return( func_fem_entity_prop (cmd, 3)); }

int func_fem_volume_kps (char *cmd)
    { return( func_fem_entity_kps (cmd, 3)); }

int func_fem_volume_list (char *cmd)
    { return( func_fem_entity_list (cmd, 3)); }

int func_fem_volume_divs (char *cmd)
    { return( func_fem_entity_divs (cmd, 3)); }

int func_fem_volume_del (char *cmd)
    { return( func_fem_entity_del (cmd, 3)); }

/* ------------------------ */

/** Creates entity "gep,entity_type,number,etype,real,mat [,set]" Note: if you are
 * using ALL as number you HAVE TO fill all items
 * @param cmd command
 * @return status
 */
int func_fem_ge_prop (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   et  = 0 ;
	long   rs  = 0 ;
	long   mat = 0 ;
	long   set = 0 ;
  long   i ;
	long   et0  = 0 ;
	long   rs0  = 0 ;
	long   mat0 = 0 ;
	long   set0 = 0 ;
  long ent_type = 0 ;

	FEM_TEST_PREPROCESSOR

  if (ciParNum(cmd) < 2)
  {
    fprintf(msgout,"[E] %s!\n", _("Type of geometric entity must be specified"));
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }

	if (ciParNum(cmd) > 1) { ent_type  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { id  = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { et  = ciGetParInt(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { rs  = ciGetParInt(cmd, 4) ; }
	if (ciParNum(cmd) > 5) { mat = ciGetParInt(cmd, 5) ; }
	if (ciParNum(cmd) > 6) { set = ciGetParInt(cmd, 6) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"g[%li] = %li: t=%li r=%li m=%li [%li]\n",ent_type,id,et,rs,mat,set);
#endif


  if (ciTestStringALL(cmd,2) == AF_YES) /* changing all selected elements  */
  {
    et0  = et  ;
    rs0  = rs  ;
    mat0 = mat ;
    set0 = set ;

    for (i=0; i<fdbInputTabLenAll(ENTITY); i++)
    {
      if (fdbInputGetInt(ENTITY, ENTITY_TYPE, i) != ent_type) { continue ; }

      if (fdbInputTestSelect(ENTITY,i) == AF_YES)
      {
        if (et0  == 0) {et  = fdbInputGetInt(ENTITY, ENTITY_ETYPE, i); }
        if (rs0  == 0) {rs  = fdbInputGetInt(ENTITY, ENTITY_RS,   i); }
        if (mat0 == 0) {mat = fdbInputGetInt(ENTITY, ENTITY_MAT,  i); }
        if (set0 == 0) {set = fdbInputGetInt(ENTITY, ENTITY_SET,  i); }

	      rv = f_ent_new_change(fdbInputGetInt(ENTITY, ENTITY_ID, i), ent_type, et, rs, mat, set);

        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else /* change/creation of single element */
  {
	  rv = f_ent_new_change(id, ent_type, et, rs, mat, set);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Creates element "ge,type,number,keypoints.."
 * @param cmd command
 * @return status
 */
int func_fem_ge_kps (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	int    i   = 0 ;
	long   nnum  = 0 ;
	long  *nodes = NULL ;
  long   ent_type = -1 ;

	FEM_TEST_PREPROCESSOR

	if ((nnum=ciParNum(cmd)) < 4) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Keypoints should be specified"));
		goto memFree;
	}

	nnum -= 3 ;

	if ((nodes = femIntAlloc(nnum)) == NULL)
	{
		rv = AF_ERR_MEM ;
		fprintf(msgout,"[E] %s!\n", _("Out of memory"));
		goto memFree;
	}

	ent_type = ciGetParInt(cmd, 1) ; 
	id       = ciGetParInt(cmd, 2) ; 

	for (i=0; i<nnum; i++) { nodes[i]  = ciGetParInt(cmd, 3+i) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e [%li]: ",id);
	for (i=0; i<nnum; i++) { fprintf(msgout," %li", nodes[i]); }
	fprintf(msgout,"\n");
#endif

	rv = f_entkp_change(id, nodes, nnum, ent_type);

  free(nodes); nodes = NULL ;

memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets entity divisions "gediv,number,divisions.."
 * @param cmd command
 * @return status
 */
int func_fem_ge_divs (char *cmd)
    { return( func_fem_entity_divs (cmd, 0)); }

/** Lists elements "glist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_ge_list (char *cmd, long ent_type)
    { return( func_fem_entity_list (cmd, 0)); }

/** Deletes elements "gedel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_ge_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
  long   ent_type = 0 ;
	long   i,pos ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Entity number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(ENTITY, ENTITY_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
    if (fdbInputCountInt(ENTITY, ENTITY_ID, i, &pos) < 1) {continue;}
    ent_type = fdbInputGetInt(ENTITY, ENTITY_TYPE, pos);
		if (f_ent_delete(i, ent_type) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Entity"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* ------------------------------------------------------ */

/** Mirror keypoint(s): "kmirror,dir,pos"
 * @param cmd command
 * @return status
 */
int func_fem_kmirror(char *cmd)
{
	int rv = AF_OK ;
  long dir = 0 ;
	double ddist, dx, dy,dz ;

	FEM_TEST_PREPROCESSOR

	ddist = 0.0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Mirror plane direction required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
    dir = ciGetParInt(cmd, 1) ;
		if ( (dir < U_X) || (dir > U_Z) )
		{
			fprintf(msgout,"[E] %s!\n", _("Invalid plane specification"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2)
  {
    ddist = ciGetParDbl(cmd, 2) ;

    switch (dir)
    {
      case 1:  dx = ddist ; break ;
      case 2:  dy = ddist ; break ;
      case 3:  dz = ddist ; break ;
    }
  }

	rv = f_k_gen_1d(1, FDB_MIRROR, dir, dx, dy, dz, 0,0,0) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Mirror wmtity(s) and node: "gekmirror,dir,pos"
 * @param cmd command
 * @return status
 */
int func_fem_entkpmirror(char *cmd)
{
	int rv = AF_OK ;
  long dir = 0 ;
	double ddist, dx, dy,dz ;

	FEM_TEST_PREPROCESSOR

	ddist = 0.0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Mirror plane direction required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
    dir = ciGetParInt(cmd, 1) ;
		if ( (dir < U_X) || (dir > U_Z) )
		{
			fprintf(msgout,"[E] %s!\n", _("Invalid plane specification"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2)
  {
    ddist = ciGetParDbl(cmd, 2) ;

    switch (dir)
    {
      case 1:  dx = ddist ; break ;
      case 2:  dy = ddist ; break ;
      case 3:  dz = ddist ; break ;
    }
  }

	rv = f_ent_k_gen_1d(1, FDB_MIRROR, dir, dx, dy, dz, 0,0,0) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}
/* ------------------------------- */

/** Copy keypoint(s): "kgen,number_of_copies,dx,dy,dz,dx1,dy1,dz1
 * @param cmd command
 * @return status
 */
int func_fem_kgen(char *cmd)
{
	int rv = AF_OK ;
	long ncopy ;
	double dx, dy,dz ;
	double dx1, dy1,dz1 ;

	FEM_TEST_PREPROCESSOR

	ncopy = 0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;
	dx1 = 0.0 ;
	dy1 = 0.0 ;
	dz1 = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Number of copies required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
		if ((ncopy = ciGetParInt(cmd, 1)) < 1) 
		{
			fprintf(msgout,"[E] %s!\n", _("Number of copies should be one or more"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2) { dx = ciGetParDbl(cmd, 2); }
	if (ciParNum(cmd) > 3) { dy = ciGetParDbl(cmd, 3); }
	if (ciParNum(cmd) > 4) { dz = ciGetParDbl(cmd, 4); }

	if (ciParNum(cmd) > 5) { dx1 = ciGetParDbl(cmd, 5); }
	if (ciParNum(cmd) > 6) { dy1 = ciGetParDbl(cmd, 6); }
	if (ciParNum(cmd) > 7) { dz1 = ciGetParDbl(cmd, 7); }

	rv = f_k_gen_1d(ncopy, FDB_COPY, 0, dx, dy, dz, dx1,dy1,dz1) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Moves keypoints "kmove,dx,dy,dz"
 * @param cmd command
 * @return status
 */
int func_fem_k_move (char *cmd)
{
	int    rv    = AF_OK ;
  double dx,dy,dz ;
	long   i ;

	FEM_TEST_PREPROCESSOR

  dx = 0.0 ;
  dy = 0.0 ;
  dz = 0.0 ;
	
	if (ciParNum(cmd) > 1) { dx = ciGetParDbl(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { dy = ciGetParDbl(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { dz = ciGetParDbl(cmd, 3) ; }


  for (i=0; i<fdbInputTabLenAll(KPOINT); i++)
  {
    rv = k_move_coord(i, dx, dy, dz) ;
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Moves keypoints "krotate,plane(xy|yz|yz),angle,x0,y0,z0"
 * @param cmd command
 * @return status
 */
int func_fem_k_rotate (char *cmd)
{
	int    rv    = AF_OK ;
  double dx,dy,dz, angle ;
	long   i, plane ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 2)
	{
		fprintf(msgout,"[E] %s!\n", _("plane and angle are required"));
		return(AF_ERR_EMP);
	}

	plane = 0 ;
	angle = 0.0 ;
  dx = 0.0 ;
  dy = 0.0 ;
  dz = 0.0 ;
	
	if (ciParNum(cmd) > 1) { plane = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { angle = ciGetParDbl(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { dx    = ciGetParDbl(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { dy    = ciGetParDbl(cmd, 4) ; }
	if (ciParNum(cmd) > 5) { dz    = ciGetParDbl(cmd, 5) ; }


  for (i=0; i<fdbInputTabLenAll(KPOINT); i++)
  {
    rv = k_rotate_coord(i, plane, angle, dx, dy, dz) ;
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Copy elemen(s) and create nodes (if required): "gekgen,number_of_copies,dx,dy,dz,dx1,dy1,dz1
 * @param cmd command
 * @return status
 */
int func_fem_gekgen(char *cmd)
{
	int rv = AF_OK ;
	long ncopy ;
	double dx, dy,dz ;
	double dx1, dy1,dz1 ;

	FEM_TEST_PREPROCESSOR

	ncopy = 0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;
	dx1 = 0.0 ;
	dy1 = 0.0 ;
	dz1 = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Number of copies required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
		if ((ncopy = ciGetParInt(cmd, 1)) < 1) 
		{
			fprintf(msgout,"[E] %s!\n", _("Number of copies should be one or more"));
			return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
		}
	}

	if (ciParNum(cmd) > 2) { dx = ciGetParDbl(cmd, 2); }
	if (ciParNum(cmd) > 3) { dy = ciGetParDbl(cmd, 3); }
	if (ciParNum(cmd) > 4) { dz = ciGetParDbl(cmd, 4); }

	if (ciParNum(cmd) > 5) { dx1 = ciGetParDbl(cmd, 5); }
	if (ciParNum(cmd) > 6) { dy1 = ciGetParDbl(cmd, 6); }
	if (ciParNum(cmd) > 7) { dz1 = ciGetParDbl(cmd, 7); }

	rv = f_ent_k_gen_1d(ncopy, FDB_COPY, 0, dx, dy, dz, dx1,dy1,dz1) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Creates FE mesh: "mesh"
 * @param cmd command
 * @return status
 */
int func_fem_create_mesh (char *cmd)
{
	int    rv    = AF_OK ;

	FEM_TEST_PREPROCESSOR

  rv = fdbGeomCreateMesh() ;
  
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* ----------------------------------------- */

/* LOAD STEPS:  */
/** Create/change the load step: "step,id,time,values.."
 * @param cmd command
 * @return status
 */
int func_fem_step_new_change (char *cmd)
{
	int    rv      = AF_OK ;
  long  *values  = NULL ;
  long   nval    = 0 ;
  long   id      = 0 ;
  double time ;
  int    i ;

	FEM_TEST_PREPROCESSOR

  if ((nval=ciParNum(cmd)) < 3)
	{
		fprintf(msgout,"[E] %s!\n",
			 	_("Step identifier and time information are required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  id   = ciGetParInt(cmd, 1) ;
  time = ciGetParDbl(cmd, 2);

  nval -= 3 ;

  if ((nval == 0) && (id <= 0))
  {
		fprintf(msgout,"[E] %s!\n",
			 	_("Can not edit unspecified step"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }
  else
  {
    rv = f_step_new_change(id, time, 0, NULL);
	  return ( tuiCmdReact(cmd, rv) ) ;
  }

  if ((values=femIntAlloc(nval)) == NULL)
  {
		fprintf(msgout,"[E] %s!\n",
			 	_("Out of memory"));
		return ( tuiCmdReact(cmd, AF_ERR_MEM) ) ;
  }

  for (i=0; i<nval; i++)
  {
    values[i] = ciGetParInt(cmd, i+3) ;
    if (values[i] <= 0) 
    {
      nval = i ;
      break ;
    }
  }

  if (nval <= 0)
  {
		fprintf(msgout,"[E] %s!\n", _("No Set/Time data given"));
    rv = AF_ERR_EMP ;
  }
  else
  {
    rv = f_step_new_change(id, time, nval, values);
  }

  femIntFree(values) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Set load case (set/time) multipier: "ssmult,step,set,multiplier"
 * @param cmd command
 * @return status
 */
int func_fem_step_val_change (char *cmd)
{
	int    rv      = AF_OK ;
  long   step    = 0 ;
  long   set     = 0 ;
  double mult    = 1.0 ;

	FEM_TEST_PREPROCESSOR

  if ((ciParNum(cmd)) < 4)
	{
		fprintf(msgout,"[E] %s!\n",
			 	_("Step, set identifiers and multiplier size are all required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  step = ciGetParInt(cmd, 1) ;
  set  = ciGetParInt(cmd, 2) ;
  mult = ciGetParDbl(cmd, 3);

  if ((step <=0)||(set<=0))
  {
		fprintf(msgout,"[E] %s!\n",
			 	_("Both step and set identifiers have to be given"));
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  rv = f_stepval_change(step, set, mult) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Delete load step: "stepdel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_step_delete (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Step number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(STEP, STEP_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_step_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Step"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists elements "steplist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_step_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(ELEM, ELEM_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_step_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}



/* ------------------------------------------- */
/* ** DATA SUMMARY ** */

/** Prints f.e. data summary to output: "summ"
 * @param cmd command
 * @return status
 */
int func_fem_summ (char *cmd)
{
	fdbInputListSumm(AF_YES, stdout);

	return(AF_OK);
}

/** Prints f.e. data summary to file: "prsumm"
 * @param cmd command
 * @return status
 */
int func_fem_prsumm (char *cmd)
{
  fdbInputListSummPrn(fdbPrnFile) ;

	return(AF_OK);
}

/** Prints active coordinate system: "prcs"
 * @param cmd command
 * @return status
 */
int func_fem_prcs (char *cmd)
{
  fdbCSysTypePrn(fdbPrnFile);

	return(AF_OK);
}

/** Prints active coordinate system: "cslist"
 * @param cmd command
 * @return status
 */
int func_fem_cslist (char *cmd)
{
  fdbCSysTypeList(stdout);

	return(AF_OK);
}



/* ------------------------------------------- */
/** Sets preprocessor preferences: "prepset,(fsumm|dsumm),(yes|no)"
 * @param cmd command
 * @return status
 */
int func_fem_prepset(char *cmd)
{
	int rv = AF_OK ;
  char *what  = NULL ;
  char *mode  = NULL ;
  long  nmode = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
  { 
    fprintf(msgout,"[E] %s!\n", _("Property unspecified"));
	  return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
  }
	if (ciParNum(cmd) > 1) { what = ciGetParStr(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { mode  = ciGetParStr(cmd, 2) ; }

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

  if (strcmp(what,"dsumm") == 0) { fdbSummNodalDisps = nmode ; rv = AF_OK;}
  if (strcmp(what,"fsumm") == 0) { fdbSummNodalLoads = nmode ; rv = AF_OK;}

	free(what) ;
	free(mode) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/* ------------------------------------------- */
/* ** NODES ** */

/** Creates node "n,number,x,y,z"
 * @param cmd command
 * @return status
 */
int func_fem_n (char *cmd)
{
	int    rv = AF_OK ;
	long   id = 0 ;   /* node ID      */
	double x  = 0.0 ; /* x coordinate */
	double y  = 0.0 ; /* y coordinate */
	double z  = 0.0 ; /* z coordinate */

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { x  = ciGetParDbl(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { y  = ciGetParDbl(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { z  = ciGetParDbl(cmd, 4) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"n = %li [%e, %e, %e]\n",id,x,y,z);
#endif

	rv = f_n_new_change(id, x, y, z);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists nodes "nlist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_n_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(NODE, NODE_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_n_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes nodes "ndel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_n_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Node number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(NODE, NODE_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_n_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Node"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Merges/joins all (selected) coincident nodes "nmerge"
 * @param cmd command
 * @return status
 */
int func_fem_n_merge (char *cmd)
{
	int    rv    = AF_OK ;

	FEM_TEST_PREPROCESSOR

  rv = f_n_join_all() ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Splits node into two or more: "nsplit,from,to"
 * note: it have to be attached at least to two elements!
 */ 
int func_fem_n_split(char *cmd)
{
  int rv = AF_OK ;
  long i ;
  long from = 0 ;
	long to   = 0 ;

	FEM_TEST_PREPROCESSOR

  if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Node number required"));
		goto memFree;
	}

	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (to < from) {to = from ;}
  
  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    to = fdbInputTabLenAll(NODE) ;
    for (i=0; i<to; i++) { f_n_split_through_e_pos(i); }
  }
  else
  {
    for (i=from; i<=to; i++) { f_n_split_through_e_id(i); }
  }

memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ** ELEMENTS ** */

/** Sets default properties of elements "eprop,et,rs,mat,echset"
 * @param cmd command
 * @return status
 */
int func_def_e_prop(char *cmd)
{
	int    rv  = AF_OK ;
  long   et = 0 ;
  long   rs = 0 ;
  long   mp = 0 ;
  long   es = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { et  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { rs  = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { mp  = ciGetParInt(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { es  = ciGetParInt(cmd, 4) ; }

  et = fdbSetInputDefET(et) ;
  rs = fdbSetInputDefRS(rs) ;
  mp = fdbSetInputDefMAT(mp) ;
  es = fdbSetInputDefEset(es) ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout," et=%li rs=%li mp=%li eset=%li\n", et,rs,mp,es);
#endif

  return(rv);
}

/** Creates element "ep,number,type,real,mat [,set]" Note: if you are
 * using ALL as number you HAVE TO fill all items
 * @param cmd command
 * @return status
 */
int func_fem_e_prop (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   et  = 0 ;
	long   rs  = 0 ;
	long   mat = 0 ;
	long   set = 0 ;
  long   i ;
	long   et0  = 0 ;
	long   rs0  = 0 ;
	long   mat0 = 0 ;
	long   set0 = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { et  = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { rs  = ciGetParInt(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { mat = ciGetParInt(cmd, 4) ; }
	if (ciParNum(cmd) > 5) { set = ciGetParInt(cmd, 5) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e = %li: t=%li r=%li m=%li [%li]\n",id,et,rs,mat,set);
#endif


  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
    et0  = et  ;
    rs0  = rs  ;
    mat0 = mat ;
    set0 = set ;

    for (i=0; i<fdbInputTabLenAll(ELEM); i++)
    {
      if (fdbInputTestSelect(ELEM,i) == AF_YES)
      {
        if (et0  == 0) {et  = fdbInputGetInt(ELEM, ELEM_TYPE, i); }
        if (rs0  == 0) {rs  = fdbInputGetInt(ELEM, ELEM_RS,   i); }
        if (mat0 == 0) {mat = fdbInputGetInt(ELEM, ELEM_MAT,  i); }
        if (set0 == 0) {set = fdbInputGetInt(ELEM, ELEM_SET,  i); }

	      rv = f_e_new_change(fdbInputGetInt(ELEM, ELEM_ID, i), et, rs, mat, set);

        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else /* change/creation of single element */
  {
	  rv = f_e_new_change(id, et, rs, mat, set);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Changes element set data "fchset,number,set" Note: if you are
 * @param cmd command
 * @return status
 */
int func_fem_e_change_set (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   set = 0 ;
  long   i ;
	long   set0 = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { set = ciGetParInt(cmd, 2) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e = %li: s = %li\n",id,set);
#endif


  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
    set0 = set ;

    for (i=0; i<fdbInputTabLenAll(ELEM); i++)
    {
      if (fdbInputTestSelect(ELEM,i) == AF_YES)
      {
        if (set0 == 0) {set = fdbInputGetInt(ELEM, ELEM_SET,  i); }

	      rv = f_e_change_set(fdbInputGetInt(ELEM, ELEM_ID, i), set);

        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else /* change of single element */
  {
	  rv = f_e_change_set(id, set);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Creates element "e,number,nodes.."
 * @param cmd command
 * @return status
 */
int func_fem_e_nodes (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	int    i   = 0 ;
	long   nnum  = 0 ;
	long  *nodes = NULL ;

	FEM_TEST_PREPROCESSOR

	if ((nnum=ciParNum(cmd)) < 3) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Nodes should be specified"));
		goto memFree;
	}

	nnum -= 2 ;

	if ((nodes = femIntAlloc(nnum)) == NULL)
	{
		rv = AF_ERR_MEM ;
		fprintf(msgout,"[E] %s!\n", _("Out of memory"));
		goto memFree;
	}

	id  = ciGetParInt(cmd, 1) ; 

	for (i=0; i<nnum; i++) { nodes[i]  = ciGetParInt(cmd, 2+i) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e [%li]: ",id);
	for (i=0; i<nnum; i++) { fprintf(msgout," %li", nodes[i]); }
	fprintf(msgout,"\n");
#endif

	rv = f_en_change(id, nodes, nnum);

  free(nodes); nodes = NULL ;

memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists elements "elist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_e_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(ELEM, ELEM_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_e_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes elements "edel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_e_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Element number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(ELEM, ELEM_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_e_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Element"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* ------------------------------------------- */

/* ** ELEMENT TYPES ** */

/** Creates element type "et,number,type"
 * @param cmd command
 * @return status
 */
int func_fem_et_new (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   et  = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 3) { return(AF_ERR_VAL) ; }
	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { et  = ciGetParInt(cmd, 2) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"id = %li: t=%li\n",id,et);
#endif

	rv = f_et_new_change(id, et);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists element types "etlist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_et_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(ETYPE, ETYPE_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_et_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes element types "etdel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_et_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Element type number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(ETYPE, ETYPE_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_et_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Element type"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* ------------------------------------------- */
/* ** REAL SETS ** */

/** Creates element "rs,number,type[,rep_num]"
 * @param cmd command
 * @return status
 */
int func_fem_rs_prop (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   et  = 0 ;
	long   rp  = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 3) { return(AF_ERR_EMP); }

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { et  = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { rp  = ciGetParInt(cmd, 3) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"rs = %li: t=%li (rp=%li)\n",id,et,rp);
#endif

	rv = f_rset_new_change(id, et, rp);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Changes felue in real set  "r,valtype,number,value[,rep_index"
 * @param cmd command
 * @return status
 */
int func_fem_r_val (char *cmd)
{
	int    rv  = AF_OK ;
	long   id         = 0 ;   /* node ID      */
	long   et         = 0 ;
	double val        = 0.0 ;
	long   rep_index  = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 4) { return(AF_ERR_EMP); }

	if (ciParNum(cmd) > 1) { et  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { id  = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { val  = ciGetParDbl(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { rep_index = ciGetParInt(cmd, 4) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"rval: rs = %li (t=%li): v=%e (rp=%li)\n",id,et,val,rep_index);
#endif

	rv = f_rs_change(id, et, val, rep_index);

	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Lists elements "rlist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_rset_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(RSET, RSET_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_rs_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes elements "rdel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_rset_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Real set number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(RSET, RSET_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_rset_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Real set"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* ------------------------------------------- */
/* ** MATERIALS ** */

/** Creates element "mat,number,type[,rep_num]"
 * @param cmd command
 * @return status
 */
int func_fem_mat(char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   et  = 0 ;
	long   rp  = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 3) { return(AF_ERR_EMP); }

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { et  = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { rp  = ciGetParInt(cmd, 3) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"mat = %li: t=%li (rp=%li)\n",id,et,rp);
#endif

	rv = f_mat_new_change(id, et, rp);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Changes value in real set  "mp,valtype,number,value[,rep_index]"
 * @param cmd command
 * @return status
 */
int func_fem_mp_val (char *cmd)
{
	int    rv  = AF_OK ;
	long   id         = 0 ;   /* node ID      */
	long   et         = 0 ;
	double val        = 0.0 ;
	long   rep_index  = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 4) { return(AF_ERR_EMP); }

	if (ciParNum(cmd) > 1) { et  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { id  = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { val = ciGetParDbl(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { rep_index = ciGetParInt(cmd, 4) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"mp = %li (t=%li): v=%e (rp=%li)\n",id,et,val,rep_index);
#endif

	rv = f_mp_change(id, et, val, rep_index);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists elements "mplist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_mat_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(MAT, MAT_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_mat_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes elements "mpdel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_mat_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Material number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(MAT, MAT_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_mat_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Material"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ------------------------------------------- */
/* ** LOADS SETS ** */

/** Sets default load set  "time,id"
 * @param cmd command
 * @return status
 */
int func_fem_time(char *cmd)
{
	int    rv    = AF_OK ;
	long   id    = 0 ;
	long   newid = 0 ;

	if (ciParNum(cmd) < 1) { return(AF_ERR_EMP); }

	if (ciParNum(cmd) > 1) { id = ciGetParInt(cmd, 1) ; }

	if (id < 1) 
	{
		fprintf(msgout, "[E] %s: %li!\n", _("Invalid load set number"), id) ;
		return(AF_ERR_VAL) ;
	}

	newid = fdbSetInputLoadSet(id) ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"set = %li\n",newid);
#endif
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* ------------------------------------------- */
/* ** DISPLACEMENTS ** */

/** Creates displacement "d,node,type,val[,set,id]"
 * @param cmd command
 * @return status
 */
int func_fem_disp(char *cmd)
{
	int    rv   = AF_OK ;
	long   id   = 0 ;
	long   node = 0 ;
	long   set  = 0 ;
	long   type = 1 ;
	double val  = 0.0 ;
  long   i;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 3) { return(tuiCmdReact(cmd, AF_ERR_EMP)); }

	if (ciParNum(cmd) > 1) { node = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { type = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { val  = ciGetParDbl(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { set  = ciGetParInt(cmd, 4) ; }
	if (ciParNum(cmd) > 5) { id   = ciGetParInt(cmd, 5) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"d(%li)[n=%li] t = %li, v = %e; s=%li\n",id,node,type,val, set);
#endif


  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    for (i=0; i<fdbInputTabLenAll(NODE); i++)
    {
      if (fdbInputTestSelect(NODE,i) == AF_YES)
      {
	      rv = f_nd_new_change(0, fdbInputGetInt(NODE, NODE_ID, i), type, set, val);
        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else
  {
	  rv = f_nd_new_change(id, node, type, set, val);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Changes element set data "dchset,number,set" Note: if you are
 * @param cmd command
 * @return status
 */
int func_fem_disp_change_set (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   set = 0 ;
  long   i ;
	long   set0 = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { set = ciGetParInt(cmd, 2) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e = %li: s = %li\n",id,set);
#endif


  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
    set0 = set ;

    for (i=0; i<fdbInputTabLenAll(NDISP); i++)
    {
      if (fdbInputTestSelect(NDISP,i) == AF_YES)
      {
        if (set0 == 0) {set = fdbInputGetInt(NDISP, NDISP_SET,  i); }

	      rv = f_nd_change_set(fdbInputGetInt(NDISP, NDISP_ID, i), set);

        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else /* change of single element */
  {
	  rv = f_nd_change_set(id, set);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists displacement "dlist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_disp_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(NDISP, NDISP_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_nd_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes displacement "ddel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_disp_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Displacement number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }


  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(NDISP, NDISP_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_nd_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Displacement"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ------------------------------------------- */
/* ** NODAL LOADS ** */

/** Creates nodal load "f,node,type,val[,set,id]"
 * @param cmd command
 * @return status
 */
int func_fem_nl(char *cmd)
{
	int    rv   = AF_OK ;
	long   id   = 0 ;
	long   node = 0 ;
	long   set  = 0 ;
	long   type = 1 ;
	double val  = 0.0 ;
  long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 3) { return(AF_ERR_EMP); }

	if (ciParNum(cmd) > 1) { node = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { type = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { val = ciGetParDbl(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { set  = ciGetParInt(cmd, 4) ; }
	if (ciParNum(cmd) > 5) { id  = ciGetParInt(cmd, 5) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"f(%li)[n=%li] t = %li, v = %e; s=%li\n",id,node,type,val, set);
#endif

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    for (i=0; i<fdbInputTabLenAll(NODE); i++)
    {
      if (fdbInputTestSelect(NODE,i) == AF_YES)
      {
	      rv = f_nl_new_change(0, fdbInputGetInt(NODE, NODE_ID, i), type, set, val);
        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else
  {
	  rv = f_nl_new_change(id, node, type, set, val);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Changes element set data "eset,number,set" Note: if you are
 * @param cmd command
 * @return status
 */
int func_fem_nl_change_set (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   set = 0 ;
  long   i ;
	long   set0 = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { set = ciGetParInt(cmd, 2) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e = %li: s = %li\n",id,set);
#endif


  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
    set0 = set ;

    for (i=0; i<fdbInputTabLenAll(NLOAD); i++)
    {
      if (fdbInputTestSelect(NLOAD,i) == AF_YES)
      {
        if (set0 == 0) {set = fdbInputGetInt(NLOAD, NLOAD_SET,  i); }

	      rv = f_nl_change_set(fdbInputGetInt(NLOAD, NLOAD_ID, i), set);

        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else /* change of single element */
  {
	  rv = f_nl_change_set(id, set);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists nodal load  "flist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_nl_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(NLOAD, NLOAD_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_nl_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes nodal load "fdel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_nl_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Nodal load number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(NLOAD, NLOAD_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_nl_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Nodal load"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ------------------------------------------- */
/* ** ELEMENT LOADS ** */

/** Creates nodal load "el,element,type,val1,..,valn[,set,id]"
 * @param cmd command
 * @return status
 */
int func_fem_el(char *cmd)
{
	int     rv   = AF_OK ;
	long    id   = 0 ;
	long    elem = 0 ;
	long    set  = 0 ;
	long    type = 1 ;
	double *val  = NULL ;
	long    vals = 0 ;
	long    i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 4) { return(AF_ERR_EMP); }

	if (ciParNum(cmd) > 1) { elem = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { type = ciGetParInt(cmd, 2) ; }
	/*if (ciParNum(cmd) > 3) { type = ciGetParInt(cmd, 3) ; }*/

	if (type >= fdbElementLoadTypeLen) 
	{
		fprintf(msgout,"[E] %s: %li!\n", _("Incorrect load type"),type);
		return(AF_ERR_VAL);
	}

	vals = fdbElementLoadType[type].vals ;

	if ((val = (double *)malloc(sizeof(double)*vals)) == NULL)
	{
		fprintf(msgout,"[E] %s !\n", _("Out of memory"));
		return(AF_ERR_MEM);
	}

	for (i=3; i<3+vals; i++)
	{
		if (ciParNum(cmd) > i) { val[i-3]  = ciGetParDbl(cmd, i) ; }
	}
	
	if (ciParNum(cmd) > (4+vals)) { set = ciGetParInt(cmd, (4+vals)) ; }
	if (ciParNum(cmd) > (5+vals)) { id  = ciGetParInt(cmd, (5+vals)) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"d(%li)[n=%li] t = %li s=%li n=%li\n",id,elem,type, set, vals);
#endif

  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
		id = 0 ;
    for (i=0; i<fdbInputTabLenAll(ELEM); i++)
    {
      if (fdbInputTestSelect(ELEM,i) == AF_YES)
      {
				elem = fdbInputGetInt(ELEM, ELEM_ID, i) ;
				rv = f_el_new_change(id, elem, type, set, vals, val);
        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
	}
	else
	{
		rv = f_el_new_change(id, elem, type, set, vals, val);
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Changes element set data "elchset,number,set" Note: if you are
 * @param cmd command
 * @return status
 */
int func_fem_el_change_set (char *cmd)
{
	int    rv  = AF_OK ;
	long   id  = 0 ;   /* node ID      */
	long   set = 0 ;
  long   i ;
	long   set0 = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id  = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { set = ciGetParInt(cmd, 2) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"e = %li: s = %li\n",id,set);
#endif


  if (ciTestStringALL(cmd,1) == AF_YES) /* changing all selected elements  */
  {
    set0 = set ;

    for (i=0; i<fdbInputTabLenAll(ELEM); i++)
    {
      if (fdbInputTestSelect(ELEM,i) == AF_YES)
      {
        if (set0 == 0) {set = fdbInputGetInt(ELEM, ELEM_SET,  i); }

	      rv = f_el_change_set(fdbInputGetInt(ELEM, ELEM_ID, i), set);

        if (rv != AF_OK) { return ( tuiCmdReact(cmd, rv) ) ; }
      }
    }
  }
  else /* change of single element */
  {
	  rv = f_el_change_set(id, set);
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists element load  "ellist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_el_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;
	int    header = AF_NO ;
	long   i ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(ELOAD, ELOAD_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

	for (i=from; i<=to; i++) 
	{ 
		if (i == from) {header = AF_YES;}
		else           {header = AF_NO; }
		f_el_list(i, header, stdout); 
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes element load "eldel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_el_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Element load number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(ELOAD, ELOAD_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_el_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Element load"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ------------------------------------------- */
/* ** GRAVITATION/ACCELERATION ** */

/** Creates nodal load "accel,dir,val[,set,id]"
 * @param cmd command
 * @return status
 */
int func_fem_gl(char *cmd)
{
	int    rv   = AF_OK ;
	long   id   = 0 ;
	long   set  = 0 ;
	long   dir  = 1 ;
	double val  = 0.0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 3) { return(AF_ERR_EMP); }

	if (ciParNum(cmd) > 1) { dir = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { val = ciGetParDbl(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { set = ciGetParInt(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { id  = ciGetParInt(cmd, 4) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"acc(%li) d = %li, v = %e; s=%li\n",id,dir,val, set);
#endif

	rv = f_gl_new_change(id, set,  dir, val);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists nodal load  "accellist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_gl_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(GRAV, GRAV_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = f_gl_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes nodal load "acceldel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_gl_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Gravitation number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }


  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(GRAV, GRAV_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (f_gl_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Gravitation"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}



/* ------------------------------- */

/** Copy node(s): "ngen,number_of_copies,dx,dy,dz,dx1,dy1,dz1
 * @param cmd command
 * @return status
 */
int func_fem_ngen(char *cmd)
{
	int rv = AF_OK ;
	long ncopy ;
	double dx, dy,dz ;
	double dx1, dy1,dz1 ;

	FEM_TEST_PREPROCESSOR

	ncopy = 0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;
	dx1 = 0.0 ;
	dy1 = 0.0 ;
	dz1 = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Number of copies required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
		if ((ncopy = ciGetParInt(cmd, 1)) < 1) 
		{
			fprintf(msgout,"[E] %s!\n", _("Number of copies should be one or more"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2) { dx = ciGetParDbl(cmd, 2); }
	if (ciParNum(cmd) > 3) { dy = ciGetParDbl(cmd, 3); }
	if (ciParNum(cmd) > 4) { dz = ciGetParDbl(cmd, 4); }

	if (ciParNum(cmd) > 5) { dx1 = ciGetParDbl(cmd, 5); }
	if (ciParNum(cmd) > 6) { dy1 = ciGetParDbl(cmd, 6); }
	if (ciParNum(cmd) > 7) { dz1 = ciGetParDbl(cmd, 7); }

	rv = f_n_gen_1d(ncopy, FDB_COPY, 0, dx, dy, dz, dx1,dy1,dz1) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Moves nodes "nmove,dx,dy,dz"
 * @param cmd command
 * @return status
 */
int func_fem_n_move (char *cmd)
{
	int    rv    = AF_OK ;
  double dx,dy,dz ;
	long   i ;

	FEM_TEST_PREPROCESSOR

  dx = 0.0 ;
  dy = 0.0 ;
  dz = 0.0 ;
	
	if (ciParNum(cmd) > 1) { dx = ciGetParDbl(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { dy = ciGetParDbl(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { dz = ciGetParDbl(cmd, 3) ; }

  for (i=0; i<fdbInputTabLenAll(NODE); i++)
  {
    n_move_coord(i, dx, dy, dz) ;
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Copy element(s) and create nodes (if required): "engen,number_of_copies,dx,dy,dz,dx1,dy1,dz1
 * @param cmd command
 * @return status
 */
int func_fem_engen(char *cmd)
{
	int rv = AF_OK ;
	long ncopy ;
	double dx, dy,dz ;
	double dx1, dy1,dz1 ;

	FEM_TEST_PREPROCESSOR

	ncopy = 0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;
	dx1 = 0.0 ;
	dy1 = 0.0 ;
	dz1 = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Number of copies required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
		if ((ncopy = ciGetParInt(cmd, 1)) < 1) 
		{
			fprintf(msgout,"[E] %s!\n", _("Number of copies should be one or more"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2) { dx = ciGetParDbl(cmd, 2); }
	if (ciParNum(cmd) > 3) { dy = ciGetParDbl(cmd, 3); }
	if (ciParNum(cmd) > 4) { dz = ciGetParDbl(cmd, 4); }

	if (ciParNum(cmd) > 5) { dx1 = ciGetParDbl(cmd, 5); }
	if (ciParNum(cmd) > 6) { dy1 = ciGetParDbl(cmd, 6); }
	if (ciParNum(cmd) > 7) { dz1 = ciGetParDbl(cmd, 7); }

	rv = f_e_n_gen_1d(ncopy, FDB_COPY, 0, dx, dy, dz, dx1,dy1,dz1) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Copy ndisplacement(s) on node(s): "dgen,number_of_copies,dx,dy,dz
 * @param cmd command
 * @return status
 */
int func_fem_dgen(char *cmd)
{
	int rv = AF_OK ;
	long ncopy ;
	double dx, dy,dz ;
	double dx1, dy1,dz1 ;

	FEM_TEST_PREPROCESSOR

	ncopy = 0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;
	dx1 = 0.0 ;
	dy1 = 0.0 ;
	dz1 = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Number of copies required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
		if ((ncopy = ciGetParInt(cmd, 1)) < 1) 
		{
			fprintf(msgout,"[E] %s!\n", _("Number of copies should be one or more"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2) { dx = ciGetParDbl(cmd, 2); }
	if (ciParNum(cmd) > 3) { dy = ciGetParDbl(cmd, 3); }
	if (ciParNum(cmd) > 4) { dz = ciGetParDbl(cmd, 4); }

	if (ciParNum(cmd) > 5) { dx1 = ciGetParDbl(cmd, 5); }
	if (ciParNum(cmd) > 6) { dy1 = ciGetParDbl(cmd, 6); }
	if (ciParNum(cmd) > 7) { dz1 = ciGetParDbl(cmd, 7); }

	rv = f_nd_gen_1d(ncopy, FDB_COPY, 0, dx, dy, dz, dx1,dy1,dz1) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Copy loads(s) on node(s): "fgen,number_of_copies,dx,dy,dz
 * @param cmd command
 * @return status
 */
int func_fem_fgen(char *cmd)
{
	int rv = AF_OK ;
	long ncopy ;
	double dx, dy,dz ;
	double dx1, dy1,dz1 ;

	FEM_TEST_PREPROCESSOR

	ncopy = 0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;
	dx1 = 0.0 ;
	dy1 = 0.0 ;
	dz1 = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Number of copies required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
		if ((ncopy = ciGetParInt(cmd, 1)) < 1) 
		{
			fprintf(msgout,"[E] %s!\n", _("Number of copies should be one or more"));
			return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
		}
	}

	if (ciParNum(cmd) > 2) { dx = ciGetParDbl(cmd, 2); }
	if (ciParNum(cmd) > 3) { dy = ciGetParDbl(cmd, 3); }
	if (ciParNum(cmd) > 4) { dz = ciGetParDbl(cmd, 4); }

	if (ciParNum(cmd) > 5) { dx1 = ciGetParDbl(cmd, 5); }
	if (ciParNum(cmd) > 6) { dy1 = ciGetParDbl(cmd, 6); }
	if (ciParNum(cmd) > 7) { dz1 = ciGetParDbl(cmd, 7); }

	rv = f_nl_gen_1d(ncopy, FDB_COPY, 0, dx, dy, dz, dx1,dy1,dz1) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ------------------------------------------------------ */

/** Mirror node(s): "nmirror,dir,pos"
 * @param cmd command
 * @return status
 */
int func_fem_nmirror(char *cmd)
{
	int rv = AF_OK ;
  long dir = 1 ;
	double ddist, dx, dy,dz ;

	FEM_TEST_PREPROCESSOR

	ddist = 0.0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Mirror plane direction required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
    dir = ciGetParInt(cmd, 1) ;
		if ( (dir < U_X) || (dir > U_Z) )
		{
			fprintf(msgout,"[E] %s!\n", _("Invalid plane specification"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2)
  {
    ddist = ciGetParDbl(cmd, 2) ;

    switch (dir)
    {
      case 1:  dx = ddist ; break ;
      case 2:  dy = ddist ; break ;
      case 3:  dz = ddist ; break ;
    }
  }

	rv = f_n_gen_1d(1, FDB_MIRROR, dir, dx, dy, dz, 0,0,0) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Mirror element(s) and node: "enmirror,dir,pos"
 * @param cmd command
 * @return status
 */
int func_fem_enmirror(char *cmd)
{
	int rv = AF_OK ;
  long dir ;
	double ddist, dx, dy,dz ;

	FEM_TEST_PREPROCESSOR

	dir = 0 ;

	ddist = 0.0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Mirror plane direction required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
    dir = ciGetParInt(cmd, 1) ;
		if ( (dir < U_X) || (dir > U_Z) )
		{
			fprintf(msgout,"[E] %s!\n", _("Invalid plane specification"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2)
  {
    ddist = ciGetParDbl(cmd, 2) ;

    switch (dir)
    {
      case 1:  dx = ddist ; break ;
      case 2:  dy = ddist ; break ;
      case 3:  dz = ddist ; break ;
    }
  }

	rv = f_e_n_gen_1d(1, FDB_MIRROR, dir, dx, dy, dz, 0,0,0) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Mirror displacements: "dmirror,dir,pos"
 * @param cmd command
 * @return status
 */
int func_fem_dmirror(char *cmd)
{
	int rv = AF_OK ;
  long dir = 0 ;
	double ddist, dx, dy,dz ;

	FEM_TEST_PREPROCESSOR

	ddist = 0.0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Mirror plane direction required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
    dir = ciGetParInt(cmd, 1) ;
		if ( (dir < U_X) || (dir > U_Z) )
		{
			fprintf(msgout,"[E] %s!\n", _("Invalid plane specification"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2)
  {
    ddist = ciGetParDbl(cmd, 2) ;

    switch (dir)
    {
      case 1:  dx = ddist ; break ;
      case 2:  dy = ddist ; break ;
      case 3:  dz = ddist ; break ;
    }
  }

	rv = f_nd_gen_1d(1, FDB_MIRROR, dir, dx, dy, dz, 0,0,0) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Mirror forces: "fmirror,dir,pos"
 * @param cmd command
 * @return status
 */
int func_fem_fmirror(char *cmd)
{
	int rv = AF_OK ;
  long dir = 0 ;
	double ddist, dx, dy,dz ;

	FEM_TEST_PREPROCESSOR

	ddist = 0.0  ;
	dx = 0.0 ;
	dy = 0.0 ;
	dz = 0.0 ;

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Mirror plane direction required"));
		return(AF_ERR_EMP);
	}

	if (ciParNum(cmd) > 1)
	{
    dir = ciGetParInt(cmd, 1) ;
		if ( (dir < U_X) || (dir > U_Z) )
		{
			fprintf(msgout,"[E] %s!\n", _("Invalid plane specification"));
			return(AF_ERR_EMP);
		}
	}

	if (ciParNum(cmd) > 2)
  {
    ddist = ciGetParDbl(cmd, 2) ;

    switch (dir)
    {
      case 1:  dx = ddist ; break ;
      case 2:  dy = ddist ; break ;
      case 3:  dz = ddist ; break ;
    }
  }

	rv = f_nl_gen_1d(1, FDB_MIRROR, dir, dx, dy, dz, 0,0,0) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Modification of load/displacement data: "modify,(d|f),id,(+|*|/|^), value"
 * @param cmd command
 * @return status
 */
int func_fem_mod_nl_nd (char *cmd)
{
	int   rv = AF_OK ;
	char *str = NULL ;
	long  entity = 0  ;
	long  op = 0 ;
	long  id = 0 ;
	long  len = 0 ;
	double val = 0.0 ;

	FEM_TEST_PREPROCESSOR
	
	if (ciParNum(cmd) < 4)
	{
		fprintf(msgout,"[E] %s!\n", _("entity type, identifier, operation and modifier are all required"));
		return(AF_ERR_EMP);
	}


	if ((str = ciGetParStrNoExpand(cmd, 1)) == NULL) { return(tuiCmdReact(cmd, AF_ERR_VAL)); }
	if (strlen(str) < 1) { return(tuiCmdReact(cmd, AF_ERR_VAL)); }

	switch (str[0])
	{
		case 'd':
		case 'D':	entity = 1 ;
							break ;
		case 'f':
		case 'F': entity = 2 ;
							break ;

		case 'x':
		case 'X': entity = 3 ;
							break ;
		case 'y':
		case 'Y': entity = 4 ;
							break ;
		case 'z':
		case 'Z': entity = 5 ;
							break ;
	}

	free(str); str = NULL ;

	val = ciGetParDbl(cmd, 4) ;

	if ((str = ciGetParStr(cmd, 3)) == NULL) { return(tuiCmdReact(cmd, AF_ERR_VAL)); }
	if (strlen(str) < 1) { return(tuiCmdReact(cmd, AF_ERR_VAL)); }

	switch (str[0])
	{
		case '+':	op = 1 ; break ;
		case '-':	op = 1 ; val *= (-1.0); break ;
		case '*':
		case 'x': op = 2 ; break ;
		case '/':
		case '%':
							op = 3 ; break ;
		case '^': op = 4 ; break ;
	}

	free(str); str = NULL ;

	if ((op == 0) || (entity == 0)) { return(tuiCmdReact(cmd, AF_ERR_VAL)); }


  if (ciTestStringALL(cmd,2) == AF_YES)
	{
		switch (entity)
		{
			case 1: len = fdbInputFindMaxInt(NDISP, NDISP_ID) ; break ;
			case 2: len = fdbInputFindMaxInt(NLOAD, NLOAD_ID) ; break ;

			case 3: 
			case 4: 
			case 5: len = fdbInputFindMaxInt(NODE, NODE_ID) ; break ;
		}

		rv = fdb_mod_n_l_d(entity, 0, len, op, val) ;
	}
	else
	{
		id = ciGetParInt(cmd, 2) ;
		rv = fdb_mod_n_l_d(entity, id, id, op, val) ;
	}

	return(tuiCmdReact(cmd, rv));
}



/* ------------------------------------------------------ */

/** Initialization of preprocessor in the simplest way: "prep"
 * @param cmd command
 * @return status
 */
int func_fem_prep(char *cmd)
{
	/* DO NOT TOUCH cmd (it might be NULL!) */
	if (ResLen > 0) { fdbFreeResSpacePtrs(); }
	ResLen = 0 ;
  if (aResLen > 0) { fdbAvResFreeSpace(); }
  aResLen = 0 ;

  resPathFreeAll();

	femUI_Mode = FEM_UI_MODE_PREP ;
  if (cmd != NULL)
  {
	  return ( tuiCmdReact(cmd, AF_OK) ) ;
  }
  else
  {
    return(AF_OK);
  }
}

/** Computes data from monte carlo solution "mcres,summary_file_name"
 * @param cmd command
 * @return status
 */
int func_fem_res_compute_mc(char *cmd)
{
	int rv = AF_OK ;
	
	femUI_Mode = FEM_UI_MODE_POST ;

  ciRunCmd("nsel,all");
  ciRunCmd("esel,all");
  ciRunCmd("fsel,all");
  ciRunCmd("dsel,all");
  ciRunCmd("gsel,all");
	
	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Name of summary data file required "));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}
	else
	{
		rv = fdb_read_monte_res(ciGetParStr(cmd, 1)) ;
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}



/** Sets solver path: "setsolver,name"
 * @param cmd command
 * @return status
 */
int func_fem_set_solver(char *cmd)
{
	int     rv  = AF_OK ;

  if (ciParNum(cmd) <=  1)
  {
    fprintf(msgout,"[E] %s!\n", _("Path to solver required"));
    rv = AF_ERR_EMP ;
  }
  else
  {
    rv = femSetSolverPath(ciGetParStr(cmd,1)) ;
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets datadir: "datadir,dirname"
 * @param cmd command
 * @return status
 */
int func_fem_set_datadir(char *cmd)
{
	int     rv  = AF_OK ;

  if (ciParNum(cmd) <=  1)
  {
    fprintf(msgout,"[E] %s!\n", _("Directory name required"));
    rv = AF_ERR_EMP ;
  }
  else
  {
    rv = femSetDataDir(ciGetParStr(cmd,1)) ;
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets jobname: "jobname,name"
 * @param cmd command
 * @return status
 */
int func_fem_set_jobname(char *cmd)
{
	int     rv  = AF_OK ;

	FEM_TEST_PREPROCESSOR

  if (ciParNum(cmd) <=  1)
  {
    fprintf(msgout,"[E] %s!\n", _("Name required"));
    rv = AF_ERR_EMP ;
  }
  else
  {
    rv = femSetJobname(ciGetParStr(cmd,1)) ;
  }

	if (rv == AF_OK)
	{
		fprintf(msgout,"[ ] %s: \"%s\"\n",_("Job name set to"), ciGetParStr(cmd,1) );
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Returns ide number of first found item "fillvarfirstsel,[n|e|f|d]" */
int func_fill_var_find_first(char *cmd)
{
  int rv  = AF_OK ;
  char *var = NULL ;
  char *dir = NULL ;
	long  i ;
	char  value[CI_STR_LEN];
  long  id, type, type_id ;

	if (ciParNum(cmd) < 2)
	{
		fprintf(msgout,"[E] %s: \"%s,%s\"!\n",
			 	_("Parameters is required:"),
				ciGetParStrNoExpand(cmd, 0),
        "variable, type(n|e|d|f)");
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

	for (i=0; i<CI_STR_LEN; i++) { value[i] = '\0' ; }

	if (strlen(var = ciGetParStrNoExpand(cmd, 1)) < 1) 
	{
		free(var);
		fprintf(msgout,"[E] %s!\n", 
				_("Invalid variable"));
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}
  
  if (strlen(dir = ciGetParStrNoExpand(cmd, 2)) < 1) 
	{
		free(dir);
    rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Invalid item name"));
		goto memFree;
	}

	switch(dir[0])
  {
    case 'n': type = NODE ; type_id = NODE_ID ; break ;
    case 'e': type = ELEM ; type_id = ELEM_ID ;
              if (strlen(dir) > 1)
              {
                if (dir[1] == 'l') 
                { 
                  type = ELOAD ; 
                  type_id = ELOAD_ID ; 
                }
              }
              break ;
    case 'd': type = NDISP ; type_id = NDISP_ID ; break ;
    case 'f': type = NLOAD ; type_id = NLOAD_ID ; break ;
    case 'l': 
    case 'a': 
    case 'v': 
    case 'g': type = ENTITY ; type_id = ENTITY_ID ; break ;
    case 'k': type = KPOINT ; type_id = KPOINT_ID ; break ;
    default: 
            rv = AF_ERR_VAL ;
		        fprintf(msgout,"[E] %s!\n", _("Invalid date type"));
            goto memFree ;
            break;
  }
	
	id = fdb_user_get_first_selected(type, type_id) ;

	if (id > 0)
	{
		sprintf(value,"%li",id );
		rv = ciAddVar(var, value) ;
	}
	else
	{
		rv = AF_ERR_EMP ;
	}
memFree:
  free(dir); dir = NULL ;
  free(var); var = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ----------------------------------------------- */
/* RANDOM VARIABLES */

/** Creates random variable "rval,number,type,item,subitem[,repeat_number]"
 * @param cmd command
 * @return status
 */
int func_fem_rv (char *cmd)
{
	int    rv   = AF_OK ;
	long   id   = 0 ;   /* node ID      */
  long   type = 0 ;
  long   itm  = 0 ;
  long   sitm = 0 ;
  long   iter = 0 ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) > 1) { id   = ciGetParInt(cmd, 1) ; }
	if (ciParNum(cmd) > 2) { type = ciGetParInt(cmd, 2) ; }
	if (ciParNum(cmd) > 3) { itm  = ciGetParInt(cmd, 3) ; }
	if (ciParNum(cmd) > 4) { sitm = ciGetParInt(cmd, 4) ; }
	if (ciParNum(cmd) > 5) { iter = ciGetParInt(cmd, 5) ; }

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"rv = %li [%li, %li, %li, %li]\n", id,type,itm,sitm,iter);
#endif

	rv = fdb_rand_new(id, type, itm, sitm, iter);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Lists random variables "rvlist,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_rv_list (char *cmd)
{
	int    rv = AF_OK ;
	long   from = 0 ;
	long   to   = 0 ;

	if (ciParNum(cmd) <= 1) { from = 1; to = fdbInputFindMaxInt(RANDVAL, RANDVAL_ID); }
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  rv = fdb_rand_list_prn(fdbPrnFile, from, to);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Deletes random variables "rvdel,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_rv_del (char *cmd)
{
	int    rv    = AF_OK ;
	long   from  = 0 ;
	long   to    = 0 ;
	int    count = 0 ;
	long   i ;

  /* TODO */

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) <= 1) 
	{
		rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n",_("Node number required"));
		goto memFree;
	}
	
	if (ciParNum(cmd) > 1) { from = ciGetParInt(cmd, 1) ; to = from ; }
	if (ciParNum(cmd) > 2) { to   = ciGetParInt(cmd, 2) ; }

  if (ciTestStringALL(cmd,1) == AF_YES)
  {
    from = 1; 
    to = fdbInputFindMaxInt(RANDVAL, RANDVAL_ID) ;
  }

	for (i=from; i<=to; i++) 
	{ 
		if (fdb_rand_delete(i) == AF_OK) 
		{ 
			count++ ; 
		}
		else
		{
			fprintf(msgout,"[w] %s %li %s.\n", _("Node"), i, _("not deleted"));
		}
	}

memFree:
	if (count < 1) {rv = AF_ERR_VAL ;}
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* end of RANDOM variables */
/* ----------------------------------------------- */



/** Do a data consistency test: "datatest"
 * @param cmd command
 * @return status
 */
int func_data_consist_test (char *cmd)
{
  return ( tuiCmdReact(cmd, fdb_test_data() ) ) ;
}

/* ** Results summary ** */ 

/** Prints f.e. data summary to output: "slist"
 * @param cmd command
 * @return status
 */
int func_fem_res_set_list (char *cmd)
{
	if (femUI_Mode != FEM_UI_MODE_POST) {return(AF_ERR);}
		
	fdbResListSets(AF_YES, stdout);

	return(tuiCmdReact(cmd, AF_OK));
}

/** Prints f.e. data summary to output: "prslist"
 * @param cmd command
 * @return status
 */
int func_fem_res_set_print (char *cmd)
{
	if (femUI_Mode != FEM_UI_MODE_POST) {return(AF_ERR);}

	fdbResListSetsPrn(fdbPrnFile);

	return(tuiCmdReact(cmd, AF_OK));
}

/** Fill variable with node data: "fillvarnode,var,id,(x|y|z)"
 * @param cmd command
 * @return status
 */
int func_fill_var_node(char *cmd)
{
  int rv  = AF_OK ;
  int i ;
  char *var = NULL ;
  char *dir = NULL ;
  char  idir ;
  long id  = -1 ;
	char   value[CI_STR_LEN];

	for (i=0; i<CI_STR_LEN; i++) { value[i] = '\0' ; }

  if (ciParNum(cmd) < 4)
	{
		fprintf(msgout,"[E] %s: \"%s,%s\"!\n",
			 	_("All parameters are required:"),
				ciGetParStrNoExpand(cmd, 0),
        "variable,node_number,direction");
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  if (strlen(var = ciGetParStrNoExpand(cmd, 1)) < 1) 
	{
		free(var);
		fprintf(msgout,"[E] %s!\n", 
				_("Invalid variable"));
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

  if ((id = ciGetParInt(cmd,2)) <1)
  {
    rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Invalid node"));
		goto memFree;
  }
  
  if (strlen(dir = ciGetParStrNoExpand(cmd, 3)) < 1) 
	{
		free(dir);
    rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Invalid direction"));
		goto memFree;
	}

  switch(dir[0])
  {
    case 'x': idir = NODE_X ; break ;
    case 'y': idir = NODE_Y ; break ;
    case 'z': idir = NODE_Z ; break ;
    default: 
            rv = AF_ERR_VAL ;
		        fprintf(msgout,"[E] %s!\n", _("Invalid direction"));
            goto memFree ;
            break;
  }

  
	sprintf(value,"%e",fdb_user_get_node_xyz(id, idir) );
	rv = ciAddVar(var, value) ;
  
memFree:
  free(dir); dir = NULL ;
  free(var); var = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Fill variable with keypoint data: "fillvarkp,var,id,(x|y|z)"
 * @param cmd command
 * @return status
 */
int func_fill_var_kp(char *cmd)
{
  int rv  = AF_OK ;
  int i ;
  char *var = NULL ;
  char *dir = NULL ;
  char  idir ;
  long id  = -1 ;
	char   value[CI_STR_LEN];

	for (i=0; i<CI_STR_LEN; i++) { value[i] = '\0' ; }

  if (ciParNum(cmd) < 4)
	{
		fprintf(msgout,"[E] %s: \"%s,%s\"!\n",
			 	_("All parameters are required:"),
				ciGetParStrNoExpand(cmd, 0),
        "variable,node_number,direction");
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  if (strlen(var = ciGetParStrNoExpand(cmd, 1)) < 1) 
	{
		free(var);
		fprintf(msgout,"[E] %s!\n", 
				_("Invalid variable"));
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

  if ((id = ciGetParInt(cmd,2)) <1)
  {
    rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Invalid keypoint"));
		goto memFree;
  }
  
  if (strlen(dir = ciGetParStrNoExpand(cmd, 3)) < 1) 
	{
		free(dir);
    rv = AF_ERR_VAL ;
		fprintf(msgout,"[E] %s!\n", _("Invalid direction"));
		goto memFree;
	}

  switch(dir[0])
  {
    case 'x': idir = KPOINT_X ; break ;
    case 'y': idir = KPOINT_Y ; break ;
    case 'z': idir = KPOINT_Z ; break ;
    default: 
            rv = AF_ERR_VAL ;
		        fprintf(msgout,"[E] %s!\n", _("Invalid direction"));
            goto memFree ;
            break;
  }

  
	sprintf(value,"%e",fdb_user_get_kp_xyz(id, idir) );
	rv = ciAddVar(var, value) ;
  
memFree:
  free(dir); dir = NULL ;
  free(var); var = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ** MULTIPHUSICS (THERMAL->STRUCTURAL ATM) ** */

/** Converts data from thermal analysis to structural: "therm2struct[,newjobname]"
 * @param cmd command
 * @return status
 */
int func_therm_to_struct(char *cmd)
{
  int rv  = AF_OK ;
  char *jobname = NULL ;
  long newjobname = AF_NO ;

	FEM_TEST_POSTPROCESSOR

  if (ciParNum(cmd) > 1)
  {
    (jobname = ciGetParStrNoExpand(cmd, 1));
  }

  rv = fdbFromThermalToStruct();

	FEM_TEST_PREPROCESSOR

  if (jobname != NULL)
  {
    if (strlen(jobname) >= 1)
    {
      ciStrCompr(jobname);
      if (strlen(jobname) >= 1)
      {
        if (femSetJobname(jobname) == AF_OK)
        {
          newjobname = AF_YES ;
        }
      }
    }
  }

  if (newjobname != AF_YES) { femSetJobname("femtherm") ; }
  
  if (jobname != NULL) { free(jobname); jobname = NULL ; }
	return ( tuiCmdReact(cmd, rv) ) ;
}

/* end of cmd_fem.c */
