/*
   File name: fdb_mod.c
   Date:      2006/11/15 17:02
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

	 FEM database: modifications of FEM data
*/

#include "fdb_fem.h"
#include <math.h>

double fdb_mod_val(long op, double val, double input)
{
	double result = 0.0 ;

	switch (op)
	{
		case 0: result = input ; break ;
		case 1: result = input + val ; break ; 
		case 2: result = input * val ; break ; 
		case 3: if (val != 0.0) { result = input / val ; } 
						else            { result = input ; }
						break ; 
		case 4: result = pow (input, val) ; break ; 
	}
	
	return(result);
}

/** Modifies sizes of displacements and loads on nodes
 * @param type data type (1=disp, 2=load)
 * @param from id of first modified item
 * @param to if of last modified item
 * @param op operation type (1=+ 2=* 3=/ 4=^)
 * @param val modifier
 */
int fdb_mod_n_l_d(long type, long from, long to, long op, double val)
{
	long i, id;
	long ntype = NODE_X ; /* this default value is never used */
	double vval ;

	switch (type)
	{
		case 1: /* displacements */ 
						for (i=0; i< fdbInputTabLenAll(NDISP); i++)
						{
							id = fdbInputGetInt(NDISP, NDISP_ID, i) ;
							if (id < from) {continue;}
							if (id > to) {break;}
							if (fdbInputTestSelect(NDISP, i) == AF_YES)
							{
								vval = fdbInputGetDbl(NDISP, NDISP_VAL, i);
								fdbInputPutDbl(NDISP, NDISP_VAL, i,
										fdb_mod_val(op, val, vval));
							}
						}
						break;
						
		case 2: /* loads on nodes */
						for (i=0; i< fdbInputTabLenAll(NLOAD); i++)
						{
							id = fdbInputGetInt(NLOAD, NLOAD_ID, i) ;
							if (id < from) {continue;}
							if (id > to) {break;}
							if (fdbInputTestSelect(NLOAD, i) == AF_YES)
							{
								vval = fdbInputGetDbl(NLOAD, NLOAD_VAL, i);
								fdbInputPutDbl(NLOAD, NLOAD_VAL, i,
										fdb_mod_val(op, val, vval));
							}
						}
						break;
    case 3: /* node X */
    case 4: /* node Y */
    case 5: /* node Z */
            if (type == 3) {ntype = NODE_X;}
            if (type == 4) {ntype = NODE_Y;}
            if (type == 5) {ntype = NODE_Z;}

            for (i=0; i< fdbInputTabLenAll(NODE); i++)
						{
							id = fdbInputGetInt(NODE, NODE_ID, i) ;
							if (id < from) {continue;}
							if (id > to) {break;}
							if (fdbInputTestSelect(NODE, i) == AF_YES)
							{
								vval = fdbInputGetDbl(NODE, ntype, i);
								fdbInputPutDbl(NODE, ntype, i,
										fdb_mod_val(op, val, vval));
							}
						}

            break ;

		case 0: 
		default: break ;
	}


	return(AF_OK);
}

/* end of fdb_mod.c */
