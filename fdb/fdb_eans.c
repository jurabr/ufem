/*
   File name: fdb_eans.c
   Date:      2006/12/06 12:20
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

	 Database for FEM: export of (some) data to an APDL-like language
*/

#include "fdb_fem.h"
#include "cint.h"

/** Converts element type number: uFEM -> APDL */
long fdb_exp_ans_et(long fem_type)
{
	switch (fem_type)
	{
		case  1: return( 1); break ;
		case  2: return(42); break ;
		case  3: return( 3); break ;
		case  4: return(45); break ; /* hopefully it will work... */
		case  5: return(63); break ; /* hopefully it will work... */
		case  6: return( 1); break ;
		case  7: return( 6); break ; 
		case  8: return(95); break ;
		case  9: return(45); break ;
		case 10: return(45); break ;
		case 11: return(42); break ; 
						
		default: return(1);
	}
}

/** Converts element type number: uFEM -> APDL */
long fdb_exp_ans_rs(long fem_type, FILE *fw)
{
#if 0
	switch (fem_type)
	{
		case  1: return( 1); break ;
		case  2: return(42); break ;
		case  3: return( 3); break ;
		case  4: return(45); break ; /* hopefully it will work... */
		case  5: return(63); break ; /* hopefully it will work... */
		case  6: return( 1); break ;
		case  7: return( 6); break ; 
		case  8: return(95); break ;
		case  9: return(45); break ;
		case 10: return(45); break ;
		case 11: return(42); break ; 
						
		default: return(1);
	}
#endif
  return(0);
}



/** Converts element type number: uFEM -> APDL */
void fdb_exp_ans_mp(FILE *fw, long mp_type, long mp_id, double val)
{
	switch (mp_type)
	{
		case  1: fprintf(fw,"mp,DENS,%li,%li,%e\n",mp_type,mp_id,val); break;
		case  2: fprintf(fw,"mp,EX,%li,%li,%e\n",mp_type,mp_id,val); break;
		case  5: fprintf(fw,"mp,PRXY,%li,%li,%e\n",mp_type,mp_id,val); break;
		default: return;
	}
}


int fdb_export_to_ans(FILE *fw, long *opts, long optlen)
{
	int  rv = AF_OK ;
	long id, type, pos ;
	long nlen ;
	double val ;
	double x, y, z ;
	long i, j ;

	/* initialization: */
	fprintf(fw,"/batch\n");
	if (ciGetVarVal("jobname") != NULL)
	{
		fprintf(fw,"/title,%s - from the uFEM with love... \n",
				ciGetVarVal("jobname"));
	}
	else
	{
		fprintf(fw,"/title,From the uFEM with love... \n");
	}
	
	/* element types */
	for (i=0; i<fdbInputTabLenSel(ETYPE); i++)
	{
		fprintf(fw,"et,%li,%li\n",
			fdbInputGetInt(ETYPE, ETYPE_ID, i),
			fdb_exp_ans_et(fdbInputGetInt(ETYPE, ETYPE_TYPE, i))
		);
	}

	/* real sets - ignored right now... */

	/* TODO */
	for (i=0; i<fdbInputTabLenSel(RSET); i++)
	{
		id   = fdbInputGetInt(RSET, RSET_ID, i);
		if (fdbInputCountIntAll(RVAL, RVAL_RSET, id, &pos) < 1) {continue;}

		for (j=0; j<fdbInputGetInt(RSET, RSET_NVAL, i); j++)
		{
#if 0
			type = fdbInputGetInt(MVAL, MVAL_TYPE, j+pos);
			val  = fdbInputGetDbl(MVAL, MVAL_VAL,  j+pos);
			fdb_exp_ans_mp(fw, type, id, val) ;
#endif
		}
	}

	/* materials */
	for (i=0; i<fdbInputTabLenSel(MAT); i++)
	{
		id   = fdbInputGetInt(MAT, MAT_ID, i);
		if (fdbInputCountIntAll(MVAL, MVAL_MAT, id, &pos) < 1) {continue;}

		for (j=0; j<fdbInputGetInt(MAT, MAT_NVAL, i); j++)
		{
			type = fdbInputGetInt(MVAL, MVAL_TYPE, j+pos);
			val  = fdbInputGetDbl(MVAL, MVAL_VAL,  j+pos);
			fdb_exp_ans_mp(fw, type, id, val) ;
		}
	}

	/* nodes */
	for (i=0; i<fdbInputTabLenSel(NODE); i++)
	{
		id = fdbInputGetInt(NODE, NODE_ID, i) ;
		
		x = fdbInputGetDbl(NODE, NODE_X, i) ;
		y = fdbInputGetDbl(NODE, NODE_Y, i) ;
		z = fdbInputGetDbl(NODE, NODE_Z, i) ;

		fprintf(fw,"n,%li,%e,%e,%e\n", id,x,y,z);
	}

	/* elements */
	for (i=0; i<fdbInputTabLenSel(ELEM); i++)
	{
		id   = fdbInputGetInt(ELEM, ELEM_ID, i) ;
		nlen = fdbInputGetInt(ELEM, ELEM_NODES, i) ;

		if (fdbInputCountIntAll(ENODE, ENODE_ELEM, id, &pos) < 1) {continue;}

		/* TODO: set element type, material and real set !!! */


		fprintf(fw,"e");

		for (j=0; j<nlen; j++)
		{
			fprintf(fw,",%li",fdbInputGetInt(ENODE,ENODE_ID,j+pos));
		}
		fprintf(fw,"\n");
		
	}


	/* displacements and loads on nodes */

	/* TODO */

	return(rv);
}


/* end of fdb_eans.c */
