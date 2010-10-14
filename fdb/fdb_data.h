/*
   File name: fdb_data.h
   Date:      2003/08/04 15:42
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

	 FEM database - data definitions

	 $Id: fdb_data.h,v 1.7 2004/12/29 19:03:30 jirka Exp $
*/

#include "fdb.h"

#ifndef __FDB_DATA_H__
#define __FDB_DATA_H__


/* TABLES */

#define NODE        0
#define ELEM        1
#define ENODE       2
#define ELOAD       3
#define ELVAL       4
#define NLOAD       5
#define NDISP       6
#define ETYPE       7
#define RSET        8
#define RVAL        9
#define MAT        10
#define MVAL       11
#define GRAV       12

#define KPOINT     13
#define ENTITY     14
#define ENTKP      15
#define ENTDIV     16

#define STEP       17
#define STEPVAL    18

#define RANDVAL    19


/* COLLUMNS: */

#define NODE_SEL    1  /* selection info  */
#define NODE_ID     2  /* node number  */
#define NODE_X      3  /* x coordinate */
#define NODE_Y      4  /* y coordinate */
#define NODE_Z      5  /* z coordinate */

#define ELEM_SEL    6  /* selection info  */
#define ELEM_ID     7  /* elem number  */
#define ELEM_SET    8  /* elememt set number  */
#define ELEM_TYPE   9  /* elem type    */
#define ELEM_RS    10  /* real set     */
#define ELEM_MAT   11  /* material type  */
#define ELEM_NODES 12  /* number of nodes */
#define ELEM_FROM  13  /* starting point in enodes */

#define ENODE_ID   14  /* node number */
#define ENODE_POS  15  /* node number in element (0..n-1) */
#define ENODE_NPOS 16  /* node position in NODE */
#define ENODE_ELEM 17  /* element number  */

#define ELOAD_SEL  18  /* selection info  */
#define ELOAD_ID   19  /* element load number */
#define ELOAD_SET  20  /* load set number */
#define ELOAD_TYPE 21  /* type of element load */
#define ELOAD_ELEM 22  /* element number */
#define ELOAD_NVAL 23  /* number of data */
#define ELOAD_EPOS 24  /* position of element in ELEM */
#define ELOAD_FROM 25  /* starting position in ELVAL */

#define ELVAL_ELID 26  /* ID of ELOAD */
#define ELVAL_VAL  27  /* size of value */

#define NLOAD_SEL  28  /* selection info  */
#define NLOAD_ID   29  /* node load number */
#define NLOAD_SET  30  /* load set  number */
#define NLOAD_TYPE 31  /* type of node load */
#define NLOAD_NODE 32  /* node number */
#define NLOAD_VAL  33  /* value  */
#define NLOAD_NPOS 34  /* node position in NODE */

#define NDISP_SEL  35  /* selection info  */
#define NDISP_ID   36  /* node displacement number */
#define NDISP_SET  37  /* load set  number */
#define NDISP_TYPE 38  /* type of node displacement */
#define NDISP_NODE 39  /* node number */
#define NDISP_VAL  40  /* value  */
#define NDISP_NPOS 41  /* node position in NODE */

#define ETYPE_ID   42  /* element type identifier  */
#define ETYPE_TYPE 43  /* type of element type */

#define RSET_ID    44 /* number of real set */
#define RSET_TYPE  45 /* element type for real set */
#define RSET_NVAL  46 /* number of values */

#define RVAL_RSET  47 /* ID of RSET */
#define RVAL_TYPE  48 /* type of value */
#define RVAL_VAL   49 /* size of value */

#define MAT_ID     50 /* number of material */
#define MAT_TYPE   51 /* type of material */
#define MAT_NVAL   52 /* number of values */

#define MVAL_MAT   53 /* ID of MAT */
#define MVAL_TYPE  54 /* type of value */
#define MVAL_VAL   55 /* size of value */

#define GRAV_SEL   56 /* selection info      */
#define GRAV_ID    57 /* gravity identifier  */
#define GRAV_SET   58 /* load set number     */
#define GRAV_DIR   59 /* direction           */
#define GRAV_VAL   60 /* acceleration        */

/* geometric model: */
#define KPOINT_ID    61 /* keypoint number             */
#define KPOINT_X     62 /* keypoint X                  */
#define KPOINT_Y     63 /* keypoint Y                  */
#define KPOINT_Z     64 /* keypoint Z                  */
#define KPOINT_SEL   65 

#define ENTITY_ID    66 /* geometric entity number     */
#define ENTITY_TYPE  67 /* type of entity              */
#define ENTITY_KPS   68 /* number of keypoints         */
#define ENTITY_DIVS  69 /* number of lines (divisions) */
#define ENTITY_ETYPE 70 /* element type                */
#define ENTITY_RS    71 /* real set                    */
#define ENTITY_MAT   72 /* material                    */
#define ENTITY_SET   73 /* element set                 */
#define ENTITY_KFROM 74 /* start in ENTKP              */
#define ENTITY_DFROM 75 /* start in ENTDIV             */
#define ENTITY_SEL   76 

#define ENTKP_ENT    77 /* entity id                   */
#define ENTKP_POS    78 /* position on entity          */
#define ENTKP_KP     79 /* keypoint number             */
#define ENTKP_KPFROM 80 /* keypoint position           */

#define ENTDIV_ENT   81 /* entity id                   */
#define ENTDIV_POS   82 /* position on entity          */
#define ENTDIV_DIV   83 /* division of the line        */
#define ENTDIV_DRAT  84 /* division ratio              */

/* time/load step tables: */
#define STEP_ID      85 /* step identifier                  */
#define STEP_TIME    86 /* time (in sec or so) if necessary */
#define STEP_NVAL    87 /* number of records in "STEPVAL"   */

#define STEPVAL_STEP 88 /* ID of "STEP" value               */
#define STEPVAL_MULT 89 /* load multiplier (if applicable)  */
#define STEPVAL_SET  90 /* set ID (_SET in other tables)    */

/* random variables */
#define RANDVAL_ID   91 /* random value identifier          */
#define RANDVAL_TYPE 92 /* type of value - node, material.. */
#define RANDVAL_ITM  93 /* item ID                          */
#define RANDVAL_SITM 94 /* sub-time number (x,y,z or type)  */
#define RANDVAL_ITER 95 /* repeating number (if necessary)  */
#endif

/* end of fdb_data.h */
