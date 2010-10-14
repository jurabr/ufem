/*
   File name: fdb_tdef.c
   Date:      2003/08/04 16:32
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

	 FEM database - defnitions of tables

	 $Id: fdb_tdef.c,v 1.9 2004/12/29 19:03:35 jirka Exp $
*/

#include "fdb.h"
#include "fdb_data.h"

long InputTabLen = 20 ; /* when changed it makes old input data useless */
tTab InputTab[20];

long ResTabLen = 1 ;
tTab ResTab[1];

int fdbCreateInputTabHdr(void)
{
	int     rv = AF_OK ;

	/* NODES */
	InputTab[0].id = 0 ;
	InputTab[0].cols = 5 ;
	InputTab[0].selcolnum = NODE_SEL ;

	if ((InputTab[0].cid  = femIntAlloc(5)) == NULL) {goto memFree;}
	if ((InputTab[0].col  = femIntAlloc(5)) == NULL) {goto memFree;}
	if ((InputTab[0].type  = femIntAlloc(5)) == NULL) {goto memFree;}

	InputTab[0].cid[0]  = NODE_SEL ;
	InputTab[0].type[0] = FDB_INT  ;

	InputTab[0].cid[1]  = NODE_ID ;
	InputTab[0].type[1] = FDB_INT  ;

	InputTab[0].cid[2]  = NODE_X ;
	InputTab[0].type[2] = FDB_DBL  ;

	InputTab[0].cid[3]  = NODE_Y ;
	InputTab[0].type[3] = FDB_DBL  ;

	InputTab[0].cid[4]  = NODE_Z ;
	InputTab[0].type[4] = FDB_DBL  ;

	/* ELEMENTS */
	InputTab[1].id = 1 ;
	InputTab[1].cols = 8 ;
	InputTab[1].selcolnum = ELEM_SEL ;

	if ((InputTab[1].cid  = femIntAlloc(8)) == NULL) {goto memFree;}
	if ((InputTab[1].col  = femIntAlloc(8)) == NULL) {goto memFree;}
	if ((InputTab[1].type  = femIntAlloc(8)) == NULL) {goto memFree;}

	InputTab[1].cid[0]  = ELEM_SEL ;
	InputTab[1].type[0] = FDB_INT  ;

	InputTab[1].cid[1]  = ELEM_ID ;
	InputTab[1].type[1] = FDB_INT  ;

	InputTab[1].cid[2]  = ELEM_SET ;
	InputTab[1].type[2] = FDB_INT  ;

	InputTab[1].cid[3]  = ELEM_TYPE ;
	InputTab[1].type[3] = FDB_INT  ;

	InputTab[1].cid[4]  = ELEM_RS ;
	InputTab[1].type[4] = FDB_INT  ;

	InputTab[1].cid[5]  = ELEM_MAT ;
	InputTab[1].type[5] = FDB_INT  ;

	InputTab[1].cid[6]  = ELEM_NODES ;
	InputTab[1].type[6] = FDB_INT  ;

	InputTab[1].cid[7]  = ELEM_FROM ;
	InputTab[1].type[7] = FDB_INT  ;

	/* ELEMENT NODES */
	InputTab[2].id = 2 ;
	InputTab[2].cols = 4 ;
	InputTab[2].selcolnum = FDB_NONE ;

	if ((InputTab[2].cid  = femIntAlloc(4)) == NULL) {goto memFree;}
	if ((InputTab[2].col  = femIntAlloc(4)) == NULL) {goto memFree;}
	if ((InputTab[2].type  = femIntAlloc(4)) == NULL) {goto memFree;}

	InputTab[2].cid[0]  = ENODE_ID ;
	InputTab[2].type[0] = FDB_INT  ;

	InputTab[2].cid[1]  = ENODE_POS ;
	InputTab[2].type[1] = FDB_INT  ;

	InputTab[2].cid[2]  = ENODE_NPOS ;
	InputTab[2].type[2] = FDB_INT  ;

	InputTab[2].cid[3]  = ENODE_ELEM ;
	InputTab[2].type[3] = FDB_INT  ;

	/* ELEMENT LOADS */
	InputTab[3].id = 3 ;
	InputTab[3].cols = 8 ;
	InputTab[3].selcolnum = ELOAD_SEL ;

	if ((InputTab[3].cid  = femIntAlloc(8)) == NULL) {goto memFree;}
	if ((InputTab[3].col  = femIntAlloc(8)) == NULL) {goto memFree;}
	if ((InputTab[3].type  = femIntAlloc(8)) == NULL) {goto memFree;}

	InputTab[3].cid[0]  = ELOAD_SEL ;
	InputTab[3].type[0] = FDB_INT  ;

	InputTab[3].cid[1]  = ELOAD_ID ;
	InputTab[3].type[1] = FDB_INT  ;

	InputTab[3].cid[2]  = ELOAD_SET ;
	InputTab[3].type[2] = FDB_INT  ;

	InputTab[3].cid[3]  = ELOAD_TYPE ;
	InputTab[3].type[3] = FDB_INT  ;

	InputTab[3].cid[4]  = ELOAD_ELEM ;
	InputTab[3].type[4] = FDB_INT  ;

	InputTab[3].cid[5]  = ELOAD_NVAL ;
	InputTab[3].type[5] = FDB_INT  ;

	InputTab[3].cid[6]  = ELOAD_EPOS ;
	InputTab[3].type[6] = FDB_INT  ;

	InputTab[3].cid[7]  = ELOAD_FROM ;
	InputTab[3].type[7] = FDB_INT  ;

	/* ELEMENT LOADS VALUES */
	InputTab[4].id = 4 ;
	InputTab[4].cols = 2 ;
	InputTab[4].selcolnum = FDB_NONE ;

	if ((InputTab[4].cid  = femIntAlloc(2)) == NULL) {goto memFree;}
	if ((InputTab[4].col  = femIntAlloc(2)) == NULL) {goto memFree;}
	if ((InputTab[4].type  = femIntAlloc(2)) == NULL) {goto memFree;}

	InputTab[4].cid[0]  = ELVAL_ELID ;
	InputTab[4].type[0] = FDB_INT  ;

	InputTab[4].cid[1]  = ELVAL_VAL ;
	InputTab[4].type[1] = FDB_DBL  ;

	/* NODE LOADS */
	InputTab[5].id = 5 ;
	InputTab[5].cols = 7 ;
	InputTab[5].selcolnum = NLOAD_SEL ;

	if ((InputTab[5].cid  = femIntAlloc(7)) == NULL) {goto memFree;}
	if ((InputTab[5].col  = femIntAlloc(7)) == NULL) {goto memFree;}
	if ((InputTab[5].type  = femIntAlloc(7)) == NULL) {goto memFree;}

	InputTab[5].cid[0]  = NLOAD_SEL ;
	InputTab[5].type[0] = FDB_INT  ;

	InputTab[5].cid[1]  = NLOAD_ID ;
	InputTab[5].type[1] = FDB_INT  ;

	InputTab[5].cid[2]  = NLOAD_SET ;
	InputTab[5].type[2] = FDB_INT  ;

	InputTab[5].cid[3]  = NLOAD_TYPE ;
	InputTab[5].type[3] = FDB_INT  ;

	InputTab[5].cid[4]  = NLOAD_NODE ;
	InputTab[5].type[4] = FDB_INT  ;

	InputTab[5].cid[5]  = NLOAD_VAL ;
	InputTab[5].type[5] = FDB_DBL  ;

	InputTab[5].cid[6]  = NLOAD_NPOS ;
	InputTab[5].type[6] = FDB_INT  ;

	/* NODE DISPLACEMENTS */
	InputTab[6].id = 6 ;
	InputTab[6].cols = 7 ;
	InputTab[6].selcolnum = NDISP_SEL ;

	if ((InputTab[6].cid  = femIntAlloc(7)) == NULL) {goto memFree;}
	if ((InputTab[6].col  = femIntAlloc(7)) == NULL) {goto memFree;}
	if ((InputTab[6].type  = femIntAlloc(7)) == NULL) {goto memFree;}

	InputTab[6].cid[0]  = NDISP_SEL ;
	InputTab[6].type[0] = FDB_INT  ;

	InputTab[6].cid[1]  = NDISP_ID ;
	InputTab[6].type[1] = FDB_INT  ;

	InputTab[6].cid[2]  = NDISP_SET ;
	InputTab[6].type[2] = FDB_INT  ;

	InputTab[6].cid[3]  = NDISP_TYPE ;
	InputTab[6].type[3] = FDB_INT  ;

	InputTab[6].cid[4]  = NDISP_NODE ;
	InputTab[6].type[4] = FDB_INT  ;

	InputTab[6].cid[5]  = NDISP_VAL ;
	InputTab[6].type[5] = FDB_DBL  ;

	InputTab[6].cid[6]  = NDISP_NPOS ;
	InputTab[6].type[6] = FDB_INT  ;

	/* ELEMENT TYPE */
	InputTab[7].id = 7 ;
	InputTab[7].cols = 2 ;
	InputTab[7].selcolnum = FDB_NONE ;

	if ((InputTab[7].cid  = femIntAlloc(2)) == NULL) {goto memFree;}
	if ((InputTab[7].col  = femIntAlloc(2)) == NULL) {goto memFree;}
	if ((InputTab[7].type  = femIntAlloc(2)) == NULL) {goto memFree;}

	InputTab[7].cid[0]  = ETYPE_ID ;
	InputTab[7].type[0] = FDB_INT  ;

	InputTab[7].cid[1]  = ETYPE_TYPE ;
	InputTab[7].type[1] = FDB_INT  ;

	/* REAL SETS */
	InputTab[8].id = 8 ;
	InputTab[8].cols = 3 ;
	InputTab[8].selcolnum = FDB_NONE ;

	if ((InputTab[8].cid  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[8].col  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[8].type  = femIntAlloc(3)) == NULL) {goto memFree;}

	InputTab[8].cid[0]  = RSET_ID ;
	InputTab[8].type[0] = FDB_INT  ;

	InputTab[8].cid[1]  = RSET_TYPE ;
	InputTab[8].type[1] = FDB_INT  ;

	InputTab[8].cid[2]  = RSET_NVAL ;
	InputTab[8].type[2] = FDB_INT  ;

	/* REAL SET DATA */
	InputTab[9].id = 9 ;
	InputTab[9].cols = 3 ;
	InputTab[9].selcolnum = FDB_NONE ;

	if ((InputTab[9].cid  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[9].col  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[9].type  = femIntAlloc(3)) == NULL) {goto memFree;}

	InputTab[9].cid[0]  = RVAL_RSET ;
	InputTab[9].type[0] = FDB_INT  ;

	InputTab[9].cid[1]  = RVAL_TYPE ;
	InputTab[9].type[1] = FDB_INT  ;

	InputTab[9].cid[2]  = RVAL_VAL ;
	InputTab[9].type[2] = FDB_DBL  ;

	/* MATERIAL TYPES */
	InputTab[10].id = 10 ;
	InputTab[10].cols = 3 ;
	InputTab[10].selcolnum = FDB_NONE ;

	if ((InputTab[10].cid  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[10].col  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[10].type  = femIntAlloc(3)) == NULL) {goto memFree;}

	InputTab[10].cid[0]  = MAT_ID ;
	InputTab[10].type[0] = FDB_INT  ;

	InputTab[10].cid[1]  = MAT_TYPE ;
	InputTab[10].type[1] = FDB_INT  ;

	InputTab[10].cid[2]  = MAT_NVAL ;
	InputTab[10].type[2] = FDB_INT  ;

	/* MATERIAL DATA */
	InputTab[11].id = 11 ;
	InputTab[11].cols = 3 ;
	InputTab[11].selcolnum = FDB_NONE ;

	if ((InputTab[11].cid  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[11].col  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[11].type  = femIntAlloc(3)) == NULL) {goto memFree;}

	InputTab[11].cid[0]  = MVAL_MAT ;
	InputTab[11].type[0] = FDB_INT  ;

	InputTab[11].cid[1]  = MVAL_TYPE ;
	InputTab[11].type[1] = FDB_INT  ;

	InputTab[11].cid[2]  = MVAL_VAL ;
	InputTab[11].type[2] = FDB_DBL  ;


	/* GRAVITATION */
	InputTab[12].id = 12 ;
	InputTab[12].cols = 5 ;
	InputTab[12].selcolnum = GRAV_SEL ;

	if ((InputTab[12].cid  = femIntAlloc(5)) == NULL) {goto memFree;}
	if ((InputTab[12].col  = femIntAlloc(5)) == NULL) {goto memFree;}
	if ((InputTab[12].type  = femIntAlloc(5)) == NULL) {goto memFree;}

	InputTab[12].cid[0]  = GRAV_SEL ;
	InputTab[12].type[0] = FDB_INT  ;

	InputTab[12].cid[1]  = GRAV_ID ;
	InputTab[12].type[1] = FDB_INT  ;

	InputTab[12].cid[2]  = GRAV_SET ;
	InputTab[12].type[2] = FDB_INT  ;

	InputTab[12].cid[3]  = GRAV_DIR ;
	InputTab[12].type[3] = FDB_INT  ;

	InputTab[12].cid[4]  = GRAV_VAL ;
	InputTab[12].type[4] = FDB_DBL  ;


	/* to be continued (geometry, load cases,...) */

	/* KEYPOINTS  */
	InputTab[13].id = 13 ;
	InputTab[13].cols = 5 ;
	InputTab[13].selcolnum = KPOINT_SEL ;

	if ((InputTab[13].cid  = femIntAlloc(5)) == NULL) {goto memFree;}
	if ((InputTab[13].col  = femIntAlloc(5)) == NULL) {goto memFree;}
	if ((InputTab[13].type  = femIntAlloc(5)) == NULL) {goto memFree;}

	InputTab[13].cid[0]  = KPOINT_SEL ;
	InputTab[13].type[0] = FDB_INT  ;

	InputTab[13].cid[1]  = KPOINT_ID ;
	InputTab[13].type[1] = FDB_INT  ;

	InputTab[13].cid[2]  = KPOINT_X ;
	InputTab[13].type[2] = FDB_DBL  ;

	InputTab[13].cid[3]  = KPOINT_Y ;
	InputTab[13].type[3] = FDB_DBL  ;

	InputTab[13].cid[4]  = KPOINT_Z ;
	InputTab[13].type[4] = FDB_DBL  ;


	/* GEOMETRY ENTITIES  */
	InputTab[14].id = 14 ;
	InputTab[14].cols = 11 ;
	InputTab[14].selcolnum = ENTITY_SEL ;

	if ((InputTab[14].cid  = femIntAlloc(11)) == NULL) {goto memFree;}
	if ((InputTab[14].col  = femIntAlloc(11)) == NULL) {goto memFree;}
	if ((InputTab[14].type  = femIntAlloc(11)) == NULL) {goto memFree;}

	InputTab[14].cid[0]  = ENTITY_ID ;
	InputTab[14].type[0] = FDB_INT  ;

	InputTab[14].cid[1]  = ENTITY_TYPE ;
	InputTab[14].type[1] = FDB_INT  ;

	InputTab[14].cid[2]  = ENTITY_KPS ;
	InputTab[14].type[2] = FDB_INT  ;

	InputTab[14].cid[3]  = ENTITY_DIVS ;
	InputTab[14].type[3] = FDB_INT  ;

	InputTab[14].cid[4]  = ENTITY_ETYPE ;
	InputTab[14].type[4] = FDB_INT  ;

	InputTab[14].cid[5]  = ENTITY_RS;
	InputTab[14].type[5] = FDB_INT  ;

	InputTab[14].cid[6]  = ENTITY_MAT ;
	InputTab[14].type[6] = FDB_INT  ;

	InputTab[14].cid[7]  = ENTITY_SET ;
	InputTab[14].type[7] = FDB_INT  ;

	InputTab[14].cid[8]  = ENTITY_KFROM ;
	InputTab[14].type[8] = FDB_INT  ;

	InputTab[14].cid[9]  = ENTITY_DFROM ;
	InputTab[14].type[9] = FDB_INT  ;

	InputTab[14].cid[10]  = ENTITY_SEL ;
	InputTab[14].type[10] = FDB_INT  ;


	/* ENTITY KEYPOINTS  */
	InputTab[15].id = 15 ;
	InputTab[15].cols = 4 ;
	InputTab[15].selcolnum = FDB_NONE ;

	if ((InputTab[15].cid  = femIntAlloc(4)) == NULL) {goto memFree;}
	if ((InputTab[15].col  = femIntAlloc(4)) == NULL) {goto memFree;}
	if ((InputTab[15].type  = femIntAlloc(4)) == NULL) {goto memFree;}

	InputTab[15].cid[0]  = ENTKP_ENT ;
	InputTab[15].type[0] = FDB_INT  ;

	InputTab[15].cid[1]  = ENTKP_POS ;
	InputTab[15].type[1] = FDB_INT  ;

	InputTab[15].cid[2]  = ENTKP_KP ;
	InputTab[15].type[2] = FDB_INT  ;

	InputTab[15].cid[3]  = ENTKP_KPFROM ;
	InputTab[15].type[3] = FDB_INT  ;


	/* LINE DIVISIONS  */
	InputTab[16].id = 16 ;
	InputTab[16].cols = 4 ;
	InputTab[16].selcolnum = FDB_NONE ;

	if ((InputTab[16].cid  = femIntAlloc(4)) == NULL) {goto memFree;}
	if ((InputTab[16].col  = femIntAlloc(4)) == NULL) {goto memFree;}
	if ((InputTab[16].type  = femIntAlloc(4)) == NULL) {goto memFree;}

	InputTab[16].cid[0]  = ENTDIV_ENT ;
	InputTab[16].type[0] = FDB_INT  ;

	InputTab[16].cid[1]  = ENTDIV_POS ;
	InputTab[16].type[1] = FDB_INT  ;

	InputTab[16].cid[2]  = ENTDIV_DIV ;
	InputTab[16].type[2] = FDB_INT  ;

	InputTab[16].cid[3]  = ENTDIV_DRAT ;
	InputTab[16].type[3] = FDB_DBL  ;

  /* TIME DEFINITIONS */
  InputTab[17].id = 17 ;
	InputTab[17].cols = 3 ;
	InputTab[17].selcolnum = FDB_NONE ;

	if ((InputTab[17].cid  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[17].col  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[17].type  = femIntAlloc(3)) == NULL) {goto memFree;}

	InputTab[17].cid[0]  = STEP_ID ;
	InputTab[17].type[0] = FDB_INT  ;

	InputTab[17].cid[1]  = STEP_TIME ;
	InputTab[17].type[1] = FDB_DBL  ;

	InputTab[17].cid[2]  = STEP_NVAL ;
	InputTab[17].type[2] = FDB_INT  ;


  /* TIME CONTENTS */
  InputTab[18].id = 18 ;
	InputTab[18].cols = 3 ;
	InputTab[18].selcolnum = FDB_NONE ;

	if ((InputTab[18].cid  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[18].col  = femIntAlloc(3)) == NULL) {goto memFree;}
	if ((InputTab[18].type  = femIntAlloc(3)) == NULL) {goto memFree;}

	InputTab[18].cid[0]  = STEPVAL_STEP ;
	InputTab[18].type[0] = FDB_INT  ;

	InputTab[18].cid[1]  = STEPVAL_MULT ;
	InputTab[18].type[1] = FDB_DBL  ;

	InputTab[18].cid[2]  = STEPVAL_SET ;
	InputTab[18].type[2] = FDB_INT  ;


  /* RANDOM VARIABLSE */
  InputTab[19].id = 19 ;
	InputTab[19].cols = 5 ;
	InputTab[19].selcolnum = FDB_NONE ;

	if ((InputTab[19].cid  = femIntAlloc(5)) == NULL) {goto memFree;}
	if ((InputTab[19].col  = femIntAlloc(5)) == NULL) {goto memFree;}
	if ((InputTab[19].type  = femIntAlloc(5)) == NULL) {goto memFree;}

	InputTab[19].cid[0]  = RANDVAL_ID ;
	InputTab[19].type[0] = FDB_INT  ;

	InputTab[19].cid[1]  = RANDVAL_TYPE ;
	InputTab[19].type[1] = FDB_INT  ;

	InputTab[19].cid[2]  = RANDVAL_ITM ;
	InputTab[19].type[2] = FDB_INT  ;

	InputTab[19].cid[3]  = RANDVAL_SITM ;
	InputTab[19].type[3] = FDB_INT  ;

	InputTab[19].cid[4]  = RANDVAL_ITER ;
	InputTab[19].type[4] = FDB_INT  ;

	return(rv);
memFree:
#ifdef DEVEL_VERNOSE
	fprintf(msgout,"Error - fdbCreateInputTabHdr - out of memory!\n");
#endif
	exit(rv);
}

/* end of fdb_tdef.c */
