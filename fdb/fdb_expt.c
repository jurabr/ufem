/*
   File name: fdb_expt.c
   Date:      2003/11/16 21:24
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

	 Database for FEM - export to various formats 

	 $Id: fdb_expt.c,v 1.6 2004/02/24 19:24:21 jirka Exp $
*/

#include "fdb_fem.h"
#include "cint.h"
#include "fdb_edef.h"
#include "fdb_mdef.h"

extern long f_rs_check_stat_type(long mat_id, long type) ;
extern long f_mat_check_stat_type(long mat_id, long type);

extern int fdb_export_to_ans(FILE *fw, long *opts, long optlen);

extern int fdb_eload_expt_fem(FILE *fw, long eLPos, long *node_index, long preparsed); /* fdb_exel.c */

void fdb_to_fem_nd_type(long type_fdb, long *type, long *dir)
{
	switch (type_fdb)
	{
		case 0: *type = 0 ;
						*dir  = 0 ;
						break ;
		case 1: *type = 1 ;
						*dir  = 1 ;
						break ;
		case 2: *type = 1 ;
						*dir  = 2 ;
						break ;
		case 3: *type = 1 ;
						*dir  = 3 ;
						break ;
		case 4: *type = 1 ;
						*dir  = 4 ;
						break ;
		case 5: *type = 1 ;
						*dir  = 5 ;
						break ;
		case 6: *type = 1 ;
						*dir  = 6 ;
						break ;

		/* temperature */
		case 7: *type = 8 ;
					 *dir  = 7 ;
						break ;


    /* stiffness: */
		case 8: *type = 2 ;
						*dir  = 1 ;
						break ;
		case 9: *type = 2 ;
						*dir  = 2 ;
						break ;
		case 10: *type = 2 ;
						*dir  = 3 ;
						break ;
		case 11: *type = 2 ;
						*dir  = 4 ;
						break ;
		case 12: *type = 2 ;
						*dir  = 5 ;
						break ;
		case 13: *type = 2 ;
						*dir  = 6 ;
						break ;

		/* + only displacements */
		case 15: *type = 6 ;
						*dir  = 1 ;
						break ;
		case 16: *type = 5 ;
						*dir  = 2 ;
						break ;
		case 17: *type = 5 ;
						*dir  = 3 ;
						break ;
		case 18: *type = 5 ;
						*dir  = 4 ;
						break ;
		case 19: *type = 5 ;
						*dir  = 5 ;
						break ;
		case 20: *type = 5 ;
						*dir  = 6 ;
						break ;

		/* + only stiffnesses */
		case 22: *type = 7 ;
						*dir  = 1 ;
						break ;
		case 23: *type = 7 ;
						*dir  = 2 ;
						break ;
		case 24: *type = 7 ;
						*dir  = 3 ;
						break ;
		case 25: *type = 7 ;
						*dir  = 4 ;
						break ;
		case 26: *type = 7 ;
						*dir  = 5 ;
						break ;
		case 27: *type = 7 ;
						*dir  = 6 ;
						break ;

		/* - only displacements */
		case 29: *type = 4 ;
						*dir  = 1 ;
						break ;
		case 30: *type = 4 ;
						*dir  = 2 ;
						break ;
		case 31: *type = 4 ;
						*dir  = 3 ;
						break ;
		case 32: *type = 4 ;
						*dir  = 4 ;
						break ;
		case 33: *type = 4 ;
						*dir  = 5 ;
						break ;
		case 34: *type = 4 ;
						*dir  = 6 ;
						break ;

		/* - only stiffnesses */
		case 36: *type = 6 ;
						*dir  = 1 ;
						break ;
		case 37: *type = 6 ;
						*dir  = 2 ;
						break ;
		case 38: *type = 6 ;
						*dir  = 3 ;
						break ;
		case 39: *type = 6 ;
						*dir  = 4 ;
						break ;
		case 40: *type = 6 ;
						*dir  = 5 ;
						break ;
		case 41: *type = 6 ;
						*dir  = 6 ;
						break ;


		default:
				    *type = 0 ;
						*dir  = 0 ;
						break ;
	}
}


void fdb_to_fem_nl_type(long type_fdb, long *type, long *dir)
{
	switch (type_fdb)
	{
		case 0: *type = 0 ;
						*dir  = 0 ;
						break ;
		case 1: *type = 3 ;
						*dir  = 1 ;
						break ;
		case 2: *type = 3 ;
						*dir  = 2 ;
						break ;
		case 3: *type = 3 ;
						*dir  = 3 ;
						break ;
		case 4: *type = 3 ;
						*dir  = 4 ;
						break ;
		case 5: *type = 3 ;
						*dir  = 5 ;
						break ;
		case 6: *type = 3 ;
						*dir  = 6 ;
						break ;
		case 7: *type = 9 ; /* temperature */
						*dir  = 1 ;
						break ;

		default:
				    *type = 0 ;
						*dir  = 0 ;
						break ;
	}
}

/* writes random variable data */
int fdb_to_fem_rand(FILE *fw)
{
  int   rv = AF_OK ;
	long  num_ivar, num_ovar, n, pos;
	long  type, item, sitem, iter ;
	long  i ;

  fdb_rand_check();

	n = fdbInputTabLenAll(RANDVAL) ;

	if (n == 0) { fprintf(fw, "0 0 \n"); return(rv); }

	/* Compute and write number of variables */
	num_ivar = 0  ;
	num_ovar = 0  ;

	for (i=0; i<n; i++)
	{
		type = fdbInputGetInt(RANDVAL, RANDVAL_TYPE, i) ;
		if (type < MONTE_VTYPE_RES_D) { num_ivar++ ; }
		else          { num_ovar++ ; }
	}

	fprintf(fw, "%li %li\n",num_ivar,num_ovar);

	/* Writes variables - data renumbering is done of the fly... */

	for (i=0; i<n; i++)
	{
		type = fdbInputGetInt(RANDVAL, RANDVAL_TYPE, i) ;
		item = fdbInputGetInt(RANDVAL, RANDVAL_ITM, i) ;

		sitem = fdbInputGetInt(RANDVAL, RANDVAL_SITM, i) ;
		iter  = fdbInputGetInt(RANDVAL, RANDVAL_ITER, i) ;

		switch (type)
		{
			case MONTE_VTYPE_RS:    fdbInputCountInt(RSET, RSET_ID, item, &pos ); break;
			case MONTE_VTYPE_MAT:   fdbInputCountInt(MAT,  MAT_ID,  item, &pos ); break;
			case MONTE_VTYPE_N:     fdbInputCountInt(NODE, NODE_ID, item, &pos ); break;
			case MONTE_VTYPE_NLD:   /* TODO */
                              if (sitem >=3)
                              {
                                fdbInputCountTwoInt(NLOAD, 
                                    NLOAD_NODE, item,
                                    NLOAD_TYPE, iter,
                                    &pos );
                                pos += fdbInputTabLenSel(NDISP) ;
                              }
                              else
                              {
                                fdbInputCountTwoInt(NDISP, 
                                    NDISP_NODE, item,
                                    NDISP_TYPE, iter,
                                    &pos );
                              }
                              break;
			case MONTE_VTYPE_NLPOS: 
                              fdbInputCountInt(NLOAD, NLOAD_ID, item, &pos ); 
                              pos += fdbInputTabLenSel(NDISP);
                              break;
			case MONTE_VTYPE_EL:    fdbInputCountInt(ELOAD, ELOAD_ELEM, item, &pos ); break;
			case MONTE_VTYPE_RES_D:
			case MONTE_VTYPE_RES_D_MAX:
			case MONTE_VTYPE_RES_D_MIN:
			case MONTE_VTYPE_RES_D_SUM:
			case MONTE_VTYPE_RES_A_MAX:
                              fdbInputCountInt(NODE, NODE_ID, item, &pos ); break;
			case MONTE_VTYPE_RES_R: fdbInputCountInt(NODE, NODE_ID, item, &pos ); break;
			case MONTE_VTYPE_RES_MAX_E:
			case MONTE_VTYPE_RES_MIN_E:
			case MONTE_VTYPE_RES_FAIL_E:
			case MONTE_VTYPE_RES_SUM_E:
			case MONTE_VTYPE_RES_E:
                              fdbInputCountInt(ELEM, ELEM_ID, item, &pos ); break;
			case MONTE_VTYPE_DAMP:  pos = item ; break;

		}

    if (pos == -1) {pos = 0 ;}

		fprintf(fw, "%li %li %li %li %li\n", i, type, pos, sitem, iter);
	}

  return(rv);
}

/** Creates data for the "fem" solver -- non-optimized mode
 *  Note that element/disp/load groups are still ignored
 * @param fw pointer to output file
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdb_export_to_fem(FILE *fw, long *opts, long optlen)
{
  int  rv = AF_OK ;
	long i, j ;
	long len, len_all ;
	long size ;
	long isize ;
	long pos;
	long etype_id, etype_type;
  long rtype, rtype_pos ;
  long mtype, mtype_pos ;
  long nload_pos, ndisp_pos;
	long len_nl, len_nd, len_el ;
	long type, dir;
  long preparsed = AF_NO ;
  long *node_index = NULL ;
  long n_sum = 0 ;

#if 1
  if (optlen > 0)
  {
    if (opts[0] == AF_YES) {preparsed = AF_YES ;}
  }
#endif

  /* Setting up indexes: */
  fdbInputSyncStats();

  fdbInputRenumFromFlds(ELEM, ELEM_FROM, ELEM_ID, ENODE, ENODE_ELEM);  
  fdbInputRenumFromFlds(ELOAD, ELOAD_FROM, ELOAD_ID, ELVAL, ELVAL_ELID);


	/* ** General information */

	/* step/time number, depends on number (-1 means on nothing) */
	fprintf(fw,"%li %li\n", (long)1, (long)-1) ;

	/* linear solver, non-linear solver, number of steps for non-lin */
	fprintf(fw,"%li %li %li\n", (long)1, (long)0, (long)0);

	/* ** Real sets  ---------------------------------------  */

	/* number of real sets */
	len = fdbInputTabLenSel(RSET) ;

	fprintf(fw,"%li\n", len);

	if (len > 0)
	{
		size = 0 ; /* ! */

		/* description of real sets: */
		for (i=0; i<len; i++)
		{
    	if (fdbTestSelect(&InputTab[RSET], &inputData.intfld, i) != AF_YES) 
		   	{ continue ; }

			size += fdbInputGetInt(RSET, RSET_NVAL, i) ;

			fprintf(fw,"%li %li %li\n",
					fdbInputGetInt(RSET, RSET_ID, i),
					fdbInputGetInt(RSET, RSET_TYPE, i),
					fdbInputGetInt(RSET, RSET_NVAL, i)
					);
		}

		/* number of properties */
		fprintf(fw,"%li\n", size);

		/* list of properties */
		for (i=0; i<len; i++)
		{
    	if (fdbTestSelect(&InputTab[RSET], &inputData.intfld, i) != AF_YES) 
		   	{ continue ; }

			isize = fdbInputGetInt(RSET, RSET_NVAL, i) ;
		
			fdbInputCountInt(RVAL, RVAL_RSET, 
				             	fdbInputGetInt(RSET, RSET_ID, i), 
										 	&pos);

			for (j=0; j<isize; j++)
			{
				fprintf(fw,"%e ",fdbInputGetDbl(RVAL, RVAL_VAL,pos+j));
			}
			fprintf(fw,"\n");
		}
  }

	/* ** Material properties  -----------------------------  */

	/* number of material sets */
	len = fdbInputTabLenSel(MAT) ;

	fprintf(fw,"%li\n", len);

	size = 0 ; /* ! */

	/* description of materia sets: */
	for (i=0; i<len; i++)
	{
    if (fdbTestSelect(&InputTab[MAT], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		size += fdbInputGetInt(MAT, MAT_NVAL, i) ;

		fprintf(fw,"%li %li %li\n",
				fdbInputGetInt(MAT, MAT_ID, i),
				fdbInputGetInt(MAT, MAT_TYPE, i),
				fdbInputGetInt(MAT, MAT_NVAL, i)
				);
	}

	/* number of properties */
	fprintf(fw,"%li\n", size);

	/* list of properties */
	for (i=0; i<len; i++)
	{
    if (fdbTestSelect(&InputTab[MAT], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(MAT, MAT_NVAL, i) ;
		
		fdbInputCountInt(MVAL, MVAL_MAT, 
				             fdbInputGetInt(MAT, MAT_ID, i), 
										 &pos);

		for (j=0; j<isize; j++)
		{
			fprintf(fw,"%e ",fdbInputGetDbl(MVAL, MVAL_VAL,pos+j));
		}
		fprintf(fw,"\n");
	}


	/* ** Nodes  -------------------------------------------  */

	/* Number of nodes */
	len = fdbInputTabLenSel(NODE) ;
	fprintf(fw,"%li\n", len);

	len_all = fdbInputTabLenAll(NODE) ;

  if (preparsed == AF_YES)
  {
    /* indexes: */
    if ((node_index=(long *)malloc(sizeof(long)*len_all)) == NULL) 
       {return (AF_ERR_MEM);}
    for (i=0; i<len_all; i++) {node_index[i] = -1 ;}
    n_sum = 0 ;
  }

	for (i=0; i<len_all; i++)
	{
    if (fdbInputTestSelect(NODE,i) == AF_YES)
    {
      if (preparsed == AF_YES)
      {
        node_index[i] = n_sum;
        n_sum++;
      }
	  	fprintf(fw, "%li %e %e %e\n", 
				fdbInputGetInt(NODE,NODE_ID, i),
				fdbInputGetDbl(NODE,NODE_X, i),
				fdbInputGetDbl(NODE,NODE_Y, i),
				fdbInputGetDbl(NODE,NODE_Z, i)
			);
    }
	}


	/* ** Elements  ----------------------------------------  */

	/* number of elements */
	len = fdbInputTabLenSel(ELEM) ;

	len_all = fdbInputTabLenAll(ELEM) ;

	fprintf(fw,"%li\n", len);

	size = 0 ; /* ! */

	/* description of elements: */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		size += fdbInputGetInt(ELEM, ELEM_NODES, i) ;
		etype_id = fdbInputGetInt(ELEM, ELEM_TYPE, i) ;
		fdbInputCountInt(ETYPE, ETYPE_ID, etype_id, &pos) ;
		etype_type = fdbInputGetInt(ETYPE, ETYPE_TYPE, pos) ;


    if (preparsed == AF_YES)
    {
      rtype = fdbInputGetInt(ELEM, ELEM_RS, i) ;
		  fdbInputCountInt(RSET, RSET_ID, rtype, &rtype_pos) ;

      mtype = fdbInputGetInt(ELEM, ELEM_MAT, i) ;
		  fdbInputCountInt(MAT, MAT_ID, mtype, &mtype_pos) ;
    }
    else
    {
		  rtype_pos =	fdbInputGetInt(ELEM, ELEM_RS, i);
			mtype_pos =	fdbInputGetInt(ELEM, ELEM_MAT, i);
    }

    if (rtype_pos < 0) {rtype_pos = 0 ;}
    if (mtype_pos < 0) {mtype_pos = 0 ;}

		fprintf(fw,"%li %li %li %li %li\n",
				fdbInputGetInt(ELEM, ELEM_ID, i),
				etype_type,
				rtype_pos,
				mtype_pos,
				fdbInputGetInt(ELEM, ELEM_NODES, i)
				);
	}

	/* number of nodes */
	fprintf(fw,"%li\n", size);

	/* list of nodes */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(ELEM, ELEM_NODES, i) ;
		
#if 0
		fdbInputCountInt(ENODE, ENODE_ELEM, 
				             fdbInputGetInt(ELEM, ELEM_ID, i), 
										 &pos);
#else
    if (preparsed != AF_YES)
    {
    	pos = fdbInputGetInt(ELEM, ELEM_FROM, i) ;
		}
#endif

		for (j=0; j<isize; j++)
		{
      if (preparsed == AF_YES)
      {
			  fprintf(fw,"%li ",node_index[fdbEnodePos(i, j)]);
      }
      else
      {
			  fprintf(fw,"%li ",fdbInputGetInt(ENODE,ENODE_ID,pos+j));
      }
		}
		fprintf(fw,"\n");
	}

	/*  ** Gravitation  ------------------------------------  */

	if (fdbInputTabLenSel(GRAV) >= 1)
	{
		i = 0 ;

    if (fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
		{
			fprintf(fw,"0 0.0\n");
		}
		else
		{
		fprintf(fw,"%li %e\n",
				fdbInputGetInt(GRAV,GRAV_DIR,i),
				fdbInputGetDbl(GRAV,GRAV_VAL,i)
				);
		}
	}
	else
	{
		fprintf(fw,"0 0\n");
	}

	/* nodal disp/load data length */
	len_nd = fdbInputTabLenSel(NDISP) ;
	len_nl = fdbInputTabLenSel(NLOAD) ;
	len_el = fdbInputTabLenSel(ELOAD) ;
	
	fprintf(fw,"%li\n", len_nd + len_nl + 4*len_el) ;  /* must be fixed later !!! */

	/* nodal disps. + nodal load should be here..  */
	for (i=0; i<len_nd; i++)
	{
    if (fdbTestSelect(&InputTab[NDISP], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		fdb_to_fem_nd_type(fdbInputGetInt(NDISP, NDISP_TYPE, i), &type, &dir);

    if (preparsed == AF_YES)
    {
      ndisp_pos = node_index[fdbNdispNodePos(i)] ;
    }
    else
    {
      ndisp_pos = fdbInputGetInt(NDISP, NDISP_NODE, i);
    }

		fprintf(fw,"%li %li %li %e\n",
				ndisp_pos,
				type,
				dir,
				fdbInputGetDbl(NDISP, NDISP_VAL, i)
				); 
	}

	for (i=0; i<len_nl; i++)
	{
		if (fdbTestSelect(&InputTab[NLOAD], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		fdb_to_fem_nl_type(fdbInputGetInt(NLOAD, NLOAD_TYPE, i), &type, &dir);

    if (preparsed == AF_YES)
    {
      nload_pos = node_index[fdbNloadNodePos(i)] ;
    }
    else
    {
      nload_pos = fdbInputGetInt(NLOAD, NLOAD_NODE, i);
    }

		fprintf(fw,"%li %li %li %e\n",
				nload_pos,
				type,
				dir,
				fdbInputGetDbl(NLOAD, NLOAD_VAL, i)
				);
	}


  /* quick and dirty simulation of element loads: */
  for (i=0; i<len_el; i++)
	{
    fdb_eload_expt_fem(fw, i, node_index, preparsed) ;
  }

	/* element load  should be here... maybe later  */
	fprintf(fw," 0\n");

  /* dynamics loads should be here... or in an extra file */
	fprintf(fw," 0\n");

  /* random data ("monte") */
  rv = fdb_to_fem_rand(fw) ;

  return(rv);
}

/** Creates batch input file for uFEM
 *  Note that element/disp/load groups are still ignored
 * @param fw pointer to output file
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdb_export_to_mac(FILE *fw, long *opts, long optlen)
{
  int  rv = AF_OK ;
	long i, j ;
	long len, len_all ;
	long size ;
	long isize ;
	long pos;
	long rep_i ;
	long etype_id, etype_type;
  long rtype_pos ;
  long mtype_pos ;
  long nload_pos, ndisp_pos;
	long len_nl, len_nd ;
	long type, dir;
	long rset_rep = 0 ;
	long rset_num = 0 ;

  /* Setting up indexes: */
  fdbInputSyncStats();

  fdbInputRenumFromFlds(ELEM, ELEM_FROM, ELEM_ID, ENODE, ENODE_ELEM);  
  fdbInputRenumFromFlds(ELOAD, ELOAD_FROM, ELOAD_ID, ELVAL, ELVAL_ELID);

	/* ** Element types  ---------------------------------------  */

	len = fdbInputTabLenSel(ETYPE) ;

	/* number of element types */
	if (len > 0)
	{
		/* description of element types: */
		for (i=0; i<len; i++)
		{
    	if (fdbTestSelect(&InputTab[ETYPE], &inputData.intfld, i) != AF_YES) 
		   	{ continue ; }

			fprintf(fw,"et,%li,%li\n",
					fdbInputGetInt(ETYPE, ETYPE_ID, i),
					fdbInputGetInt(ETYPE, ETYPE_TYPE, i)
					);
		}
  }
	fprintf(fw,"\n");

	/* ** Real sets  ---------------------------------------  */

	/* number of real sets */
	len = fdbInputTabLenSel(RSET) ;

	if (len > 0)
	{
		size = 0 ; /* ! */

		/* description of real sets: */
		for (i=0; i<len; i++)
		{
    	if (fdbTestSelect(&InputTab[RSET], &inputData.intfld, i) != AF_YES) 
		   	{ continue ; }

			size += fdbInputGetInt(RSET, RSET_NVAL, i) ;

			rset_num = fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals_rep ;

			if (rset_num > 0)
			{
			 rset_rep = (long)
				(
				fdbInputGetInt(RSET, RSET_NVAL, i)
				-
				fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals
				) / rset_num ;
			}
			if ((rset_rep * rset_num + 
				fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals)
					<
				fdbInputGetInt(RSET, RSET_NVAL, i))
			{
				rset_rep++ ;
			}

			fprintf(fw,"rs,%li,%li,%li\n",
					fdbInputGetInt(RSET, RSET_ID, i),
					fdbInputGetInt(RSET, RSET_TYPE, i),
					rset_rep
					);
		}

		/* list of properties */
		for (i=0; i<len; i++)
		{
    	if (fdbTestSelect(&InputTab[RSET], &inputData.intfld, i) != AF_YES) 
		   	{ continue ; }

			isize = fdbInputGetInt(RSET, RSET_NVAL, i) ;
		
			fdbInputCountInt(RVAL, RVAL_RSET, 
				             	fdbInputGetInt(RSET, RSET_ID, i), 
										 	&pos);


			rep_i = 0;
			for (j=0; j<isize; j++)
			{
				if (j < fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals)
				{
					rep_i = 0 ;
				}
				else
				{
					rep_i = (long)((j - fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals)
						/ fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals_rep + 1) ;

					if (rep_i <= 0) {continue;}
				}

				fprintf(fw,"r,%s,%li,%e,%li\n",
            ciGetVarNameFromGrp(
            fdbFemStrFromInt(fdbInputGetInt(RVAL,RVAL_TYPE,pos+j)),"real"
						),
						fdbInputGetInt(RVAL, RVAL_RSET,pos+j),
						fdbInputGetDbl(RVAL, RVAL_VAL,pos+j),
						rep_i
						);
			}
		}
		fprintf(fw,"\n");
  }

	/* ** Material properties  -----------------------------  */

	/* number of material sets */
	len = fdbInputTabLenSel(MAT) ;

	size = 0 ; /* ! */

	/* description of materia sets: */
	for (i=0; i<len; i++)
	{
    if (fdbTestSelect(&InputTab[MAT], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		size += fdbInputGetInt(MAT, MAT_NVAL, i) ;

		rset_num = fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals_rp ;

			if (rset_num > 0)
			{
			 rset_rep = (long)
				(
				fdbInputGetInt(MAT, MAT_NVAL, i)
				-
				fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals
				) / rset_num ;
			}
			if ((rset_rep * rset_num + 
				fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals)
					<
				fdbInputGetInt(MAT, MAT_NVAL, i))
			{
				rset_rep++ ;
			}

		fprintf(fw,"mat,%li,%li,%li\n",
				fdbInputGetInt(MAT, MAT_ID, i),
				fdbInputGetInt(MAT, MAT_TYPE, i),
				rset_rep
				);
	}

	/* list of properties */
	for (i=0; i<len; i++)
	{
    if (fdbTestSelect(&InputTab[MAT], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(MAT, MAT_NVAL, i) ;
		
		fdbInputCountInt(MVAL, MVAL_MAT, 
				             fdbInputGetInt(MAT, MAT_ID, i), 
										 &pos);

		rep_i = 0;
		for (j=0; j<isize; j++)
		{
			if (j < fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals)
			{
				rep_i = 0 ;
			}
			else
			{
				rep_i = (long)((j - fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals)
					/ fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals_rp + 1) ;

				if (rep_i <= 0) {continue;}
			}

				fprintf(fw,"mp,%s,%li,%e,%li\n",
            ciGetVarNameFromGrp(
            fdbFemStrFromInt(fdbInputGetInt(MVAL,MVAL_TYPE,pos+j)),"material"
						),
						fdbInputGetInt(MVAL, MVAL_MAT,pos+j),
						fdbInputGetDbl(MVAL, MVAL_VAL,pos+j),
						rep_i
						);
		}
	}
	fprintf(fw,"\n");

	/* ** KPs  -------------------------------------------  */

	len_all = fdbInputTabLenAll(KPOINT) ;

	for (i=0; i<len_all; i++)
	{
    if (fdbInputTestSelect(KPOINT,i) == AF_YES)
    {
	  	fprintf(fw, "k,%li,%e,%e,%e\n", 
				fdbInputGetInt(KPOINT,KPOINT_ID, i),
				fdbInputGetDbl(KPOINT,KPOINT_X, i),
				fdbInputGetDbl(KPOINT,KPOINT_Y, i),
				fdbInputGetDbl(KPOINT,KPOINT_Z, i)
			);
    }
	}

	fprintf(fw,"\n");


	/* ** Geometric entities  ------------------------------  */

	/* number of elements */
	len = fdbInputTabLenSel(ENTITY) ;

	len_all = fdbInputTabLenAll(ENTITY) ;

	size = 0 ; /* ! */

	/* description of entities: */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ENTITY], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		size += fdbInputGetInt(ENTITY, ENTITY_KPS, i) ;
    
		rtype_pos =	fdbInputGetInt(ENTITY, ENTITY_RS, i);
		mtype_pos =	fdbInputGetInt(ENTITY, ENTITY_MAT, i);

    if (rtype_pos < 0) {rtype_pos = 0 ;}
    if (mtype_pos < 0) {mtype_pos = 0 ;}

		fprintf(fw,"gep,%s,%li,%li,%li,%li,%li\n",
        ciGetVarNameFromGrp( fdbFemStrFromInt(
				fdbInputGetInt(ENTITY, ENTITY_TYPE, i))
					,"geom_ent"),
				fdbInputGetInt(ENTITY, ENTITY_ID, i),
				fdbInputGetInt(ENTITY, ENTITY_ETYPE, i),
				rtype_pos,
				mtype_pos,
				fdbInputGetInt(ENTITY, ENTITY_SET, i)
				);
	}

	/* list of keypoints */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ENTITY], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(ENTITY, ENTITY_KPS, i) ;

		pos = fdbInputGetInt(ENTITY, ENTITY_KFROM, i) ;
		
		if (isize > 0)
		{
			fprintf(fw,"ge,%s,%li",
        ciGetVarNameFromGrp( fdbFemStrFromInt(
				fdbInputGetInt(ENTITY, ENTITY_TYPE, i))
					,"geom_ent"),
					fdbInputGetInt(ENTKP,ENTKP_ENT, pos)
					);
			for (j=0; j<isize; j++)
			{
				fprintf(fw,",%li",fdbInputGetInt(ENTKP,ENTKP_KP,pos+j));
			}
		}
		fprintf(fw,"\n");
	}

	/* list of divisions */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ENTITY], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputCountInt(ENTDIV, ENTDIV_ENT,
				fdbInputGetInt(ENTITY,ENTITY_ID, i), &pos) ;
		
		if (isize > 0)
		{
			fprintf(fw,"gediv,%li",
					fdbInputGetInt(ENTDIV,ENTDIV_ENT, pos)
					);
			for (j=0; j<isize; j++)
			{
				fprintf(fw,",%li",fdbInputGetInt(ENTDIV,ENTDIV_DIV,pos+j));
			}
		}
		fprintf(fw,"\n");
	}

	fprintf(fw,"\n");




	/* ** Nodes  -------------------------------------------  */

	len_all = fdbInputTabLenAll(NODE) ;

	for (i=0; i<len_all; i++)
	{
    if (fdbInputTestSelect(NODE,i) == AF_YES)
    {
	  	fprintf(fw, "n,%li,%e,%e,%e\n", 
				fdbInputGetInt(NODE,NODE_ID, i),
				fdbInputGetDbl(NODE,NODE_X, i),
				fdbInputGetDbl(NODE,NODE_Y, i),
				fdbInputGetDbl(NODE,NODE_Z, i)
			);
    }
	}

	fprintf(fw,"\n");

	/* ** Elements  ----------------------------------------  */

	/* number of elements */
	len = fdbInputTabLenSel(ELEM) ;

	len_all = fdbInputTabLenAll(ELEM) ;

	size = 0 ; /* ! */

	/* description of elements: */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		size += fdbInputGetInt(ELEM, ELEM_NODES, i) ;
		etype_id = fdbInputGetInt(ELEM, ELEM_TYPE, i) ;
		fdbInputCountInt(ETYPE, ETYPE_ID, etype_id, &pos) ;
		etype_type = fdbInputGetInt(ETYPE, ETYPE_TYPE, pos) ;
    
		rtype_pos =	fdbInputGetInt(ELEM, ELEM_RS, i);
		mtype_pos =	fdbInputGetInt(ELEM, ELEM_MAT, i);

    if (rtype_pos < 0) {rtype_pos = 0 ;}
    if (mtype_pos < 0) {mtype_pos = 0 ;}

		fprintf(fw,"ep,%li,%li,%li,%li,%li\n",
				fdbInputGetInt(ELEM, ELEM_ID, i),
				etype_id,
				rtype_pos,
				mtype_pos,
				fdbInputGetInt(ELEM, ELEM_SET, i)
				);
	}

	/* list of nodes */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(ELEM, ELEM_NODES, i) ;

		pos = fdbInputGetInt(ELEM, ELEM_FROM, i) ;
		
		if (isize > 0)
		{
			fprintf(fw,"e,%li", fdbInputGetInt(ENODE,ENODE_ELEM, pos));
			for (j=0; j<isize; j++)
			{
				fprintf(fw,",%li",fdbInputGetInt(ENODE,ENODE_ID,pos+j));
			}
		}
		fprintf(fw,"\n");
	}

	fprintf(fw,"\n");

	/*  ** Gravitation  ------------------------------------  */

	if (fdbInputTabLenSel(GRAV) >= 1)
	{
		len_all = fdbInputTabLenAll(GRAV) ;

		for (i=0; i<len_all; i++)
		{
    if (fdbTestSelect(&InputTab[GRAV], &inputData.intfld, i) != AF_YES) 
			 { continue ; }
		
		fprintf(fw,"accel,%li,%e,%li,%li\n",
				fdbInputGetInt(GRAV,GRAV_DIR,i),
				fdbInputGetDbl(GRAV,GRAV_VAL,i),
				fdbInputGetInt(GRAV,GRAV_SET,i),
				fdbInputGetInt(GRAV,GRAV_ID,i)
				);
		}
	}

	fprintf(fw,"\n");

	/* nodal disp/load data length */
	len_nd = fdbInputTabLenSel(NDISP) ;
	len_nl = fdbInputTabLenSel(NLOAD) ;
	
	/* nodal disps. + nodal load should be here..  */
	for (i=0; i<len_nd; i++)
	{
    if (fdbTestSelect(&InputTab[NDISP], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

    ndisp_pos = fdbInputGetInt(NDISP, NDISP_NODE, i);

		fprintf(fw,"d,%li,%s,%e,%li\n",
				ndisp_pos,
        ciGetVarNameFromGrp(
        fdbFemStrFromInt(
					fdbInputGetInt(NDISP, NDISP_TYPE, i)
					),"disp"),
				fdbInputGetDbl(NDISP, NDISP_VAL, i),
				fdbInputGetInt(NDISP, NDISP_SET, i)
				); 
	}

	fprintf(fw,"\n");

	for (i=0; i<len_nl; i++)
	{
		if (fdbTestSelect(&InputTab[NLOAD], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		fdb_to_fem_nl_type(fdbInputGetInt(NLOAD, NLOAD_TYPE, i), &type, &dir);

    nload_pos = fdbInputGetInt(NLOAD, NLOAD_NODE, i);
			
		fprintf(fw,"f,%li,%s,%e,%li\n",
				nload_pos,
        ciGetVarNameFromGrp(
        fdbFemStrFromInt(
					fdbInputGetInt(NLOAD, NLOAD_TYPE, i)
					),"load"),
				fdbInputGetDbl(NLOAD, NLOAD_VAL, i),
				fdbInputGetInt(NLOAD, NLOAD_SET, i)
				); 
	}

	fprintf(fw,"\n");

	/* TODO:  element load  should be here... maybe later  */
	for (i=0; i<fdbInputTabLenAll(ELOAD); i++)
	{
    if (fdbTestSelect(&InputTab[ELOAD], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(ELOAD, ELOAD_NVAL, i) ;

		pos = fdbInputGetInt(ELOAD, ELOAD_FROM, i) ;
		
		if (isize > 0)
		{
			fprintf(fw,"el,%li,%li", 
					fdbInputGetInt(ELOAD,ELOAD_ELEM, i),
					fdbInputGetInt(ELOAD,ELOAD_TYPE, i)
					);
			for (j=0; j<isize; j++)
			{
				fprintf(fw,",%e",fdbInputGetDbl(ELVAL,ELVAL_VAL,pos+j));
			}
		}
		fprintf(fw,"\n");
	}

	fprintf(fw,"\n");

	/* LOAD STEP data */
	for (i=0; i<fdbInputTabLenAll(STEP); i++)
	{
		fprintf(fw,"step,%li,%li,%li\n",
				fdbInputGetInt(STEP,STEP_ID,i),
				fdbInputGetInt(STEP,STEP_TIME,i),
				fdbInputGetInt(STEP,STEP_NVAL,i));
	}
	
	for (i=0; i<fdbInputTabLenAll(STEPVAL); i++)
	{
		fprintf(fw,"step,%li,%li,%e\n",
				fdbInputGetInt(STEPVAL,STEPVAL_STEP,i),
				fdbInputGetInt(STEPVAL,STEPVAL_SET,i),
				fdbInputGetDbl(STEPVAL,STEPVAL_MULT,i));
	}

	fprintf(fw,"\n");

  /* random data ("monte") */
	for (i=0; i<fdbInputTabLenAll(RANDVAL); i++)
	{
		fprintf(fw,"rval,%li,%li,%li,%li,%li\n",
				fdbInputGetInt(RANDVAL,RANDVAL_ID,i),
				fdbInputGetInt(RANDVAL,RANDVAL_TYPE,i),
				fdbInputGetInt(RANDVAL,RANDVAL_ITM,i),
				fdbInputGetInt(RANDVAL,RANDVAL_SITM,i),
				fdbInputGetInt(RANDVAL,RANDVAL_ITER,i)
				);
	}

  return(rv);
}


void fdb_to_l3d_nl_type(long type_fdb, long *type)
{
	switch (type_fdb)
	{
		case 0: *type = 0 ;
						break ;
		case 1: *type = 7 ;
						break ;
		case 2: *type = 8 ;
						break ;
		case 3: *type = 9 ;
						break ;
		default:
				    *type = 0 ;
						break ;
	}
}


/** Creates data for the link3d solver 
 * @param fw pointer to output file
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdb_export_to_link3d(FILE *fw, long *opts, long optlen)
{
  int  rv = AF_OK ;
	long i ;
	long len, len_all ;
	long size ;
	long pos;
  long nload_pos, ndisp_pos;
	long len_nl, len_nd ;
	long type ;
  double E, A ;
  long n1, n2, id ;

  /* No testing is provided!!!! */

  /* Setting up indexes: */
  fdbInputSyncStats();

  fdbInputRenumFromFlds(ELEM, ELEM_FROM, ELEM_ID, ENODE, ENODE_ELEM);  
  fdbInputRenumFromFlds(ELOAD, ELOAD_FROM, ELOAD_ID, ELVAL, ELVAL_ELID);

	
	/* ** Nodes  -------------------------------------------  */

	/* Number of nodes */
	len = fdbInputTabLenSel(NODE) ;
	fprintf(fw,"%li\n", len);

	len_all = fdbInputTabLenAll(NODE) ;

	for (i=0; i<len_all; i++)
	{
    if (fdbInputTestSelect(NODE,i) == AF_YES)
    {
	  	fprintf(fw, "%li %e %e %e\n", 
				fdbInputGetInt(NODE,NODE_ID, i),
				fdbInputGetDbl(NODE,NODE_X, i),
				fdbInputGetDbl(NODE,NODE_Y, i),
				fdbInputGetDbl(NODE,NODE_Z, i)
			);
    }
	}


	/* ** Elements  ----------------------------------------  */

	/* number of elements */
	len = fdbInputTabLenSel(ELEM) ;

	len_all = fdbInputTabLenAll(ELEM) ;

	fprintf(fw,"%li\n", len);

	size = 0 ; /* ! */

	/* description of elements: */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

    id = fdbInputGetInt(MAT,MAT_ID,i) ;
    fdbInputCountInt(MVAL, MVAL_MAT, id, &pos) ;
		E =	fdbInputGetDbl(MVAL, MVAL_VAL, pos+0);


    id = fdbInputGetInt(RSET,RSET_ID,i) ;
    fdbInputCountInt(RVAL, RVAL_RSET, id, &pos) ;
		A =	fdbInputGetDbl(RVAL, RVAL_VAL, pos+0);

    pos = fdbInputGetInt(ELEM, ELEM_FROM, i) ;
    n1 = fdbInputGetInt(ENODE,ENODE_ID,pos+0) ;
    n2 = fdbInputGetInt(ENODE,ENODE_ID,pos+1) ;

		fprintf(fw,"%li %li %li %e %e 0 0 0 0 0\n",
				fdbInputGetInt(ELEM, ELEM_ID, i),
				n1,
				n2,
				E,
				A
				);
	}


	/* nodal disp/load data length */
	len_nd = fdbInputTabLenSel(NDISP) ;
	len_nl = fdbInputTabLenSel(NLOAD) ;
	
	fprintf(fw,"%li\n", len_nd + len_nl) ;

	/* nodal disps. + nodal load should be here..  */
	for (i=0; i<len_nd; i++)
	{
    if (fdbTestSelect(&InputTab[NDISP], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		type = fdbInputGetInt(NDISP, NDISP_TYPE, i);

    ndisp_pos = fdbInputGetInt(NDISP, NDISP_NODE, i);

		fprintf(fw,"%li %li %e\n",
				ndisp_pos,
				type,
				fdbInputGetDbl(NDISP, NDISP_VAL, i)
				); 
	}

	for (i=0; i<len_nl; i++)
	{
		if (fdbTestSelect(&InputTab[NLOAD], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		fdb_to_l3d_nl_type(fdbInputGetInt(NLOAD, NLOAD_TYPE, i), &type);

    nload_pos = fdbInputGetInt(NLOAD, NLOAD_NODE, i);

		fprintf(fw,"%li %li %e\n",
				nload_pos,
				type,
				fdbInputGetDbl(NLOAD, NLOAD_VAL, i)
				);
	}

  return(rv);
}


/** Calls export function
 * @param fname filename (including path and extension, if required)
 * @param format file format (1=fem solver input,...)
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdbExport(char *fname, long format, long *opts, long optlen)
{
  int     rv = AF_OK ;
  FILE   *fw = NULL ;

  if ((fw = fopen(fname, "w")) == NULL)
  {
    fprintf(msgout, "[E] %s!\n", _("Cannot open export file"));
    return(AF_ERR_IO);
  }
  else
  {
    /* export */
    switch (format)
    {
      case 1 : /* "fem" solver format */
               if ((rv = fdb_export_to_fem(fw, opts, optlen)) != AF_OK)
                  { fprintf(msgout, "[E] %s!\n", _("Export failed")); }
               break ;
      case 2 : /* "link3d" solver format */
               if ((rv = fdb_export_to_link3d(fw, opts, optlen)) != AF_OK)
                  { fprintf(msgout, "[E] %s!\n", _("Export failed")); }
               break ;
      case 3 : /* APDL format */
               if ((rv = fdb_export_to_ans(fw, opts, optlen)) != AF_OK)
                  { fprintf(msgout, "[E] %s!\n", _("Export failed")); }
               break ;
      case 4 : /* "fem" solver format */
               if ((rv = fdb_export_to_mac(fw, opts, optlen)) != AF_OK)
                  { fprintf(msgout, "[E] %s!\n", _("Export failed")); }
               break ;

      default: /* unknown format */
               fprintf(msgout, "[E] %s!\n", _("Unknown format"));
               rv = AF_ERR_VAL ;
               break ;
    }

    if (fclose(fw) != 0)
    {
      fprintf(msgout, "[E] %s!\n", _("Cannot close export file"));
      return(AF_ERR_IO);
    }
    else
    {
      return(rv) ;
    }
  }
}

/** Converts uFEM element type to AN**S one 
 * @param type ufem type number
 * @return type ans type number
 */
long fdb_ufem_et_to_ans(long type)
{
  switch (type)
	{
		case 1: return(1);  break ;
		case 2: return(42);  break ;
		case 3: return(3);  break ;
		case 4: return(45);  break ;
		case 5: return(63);  break ;
		case 7: return(8);  break ; /* TODO: check - link8 ? */
		case 6: return(1);  break ;
		case 8: return(195);  break ; /* TODO: check */
		case 9: return(45);  break ; 
		case 10: return(45);  break ;
		case 11: return(42);  break ;
		case 12: return(195);  break ; /* TODO: check */
		case 14: return(4);  break ;  /* TODO: check */
		case 17: return(4);  break ; /* TODO: check */

		default: return(1);  break;
	}
}

char *fdb_ufem_mat_to_ans(long matprop)
{
	switch (matprop)
	{
		case 2 : return("ex"); break ;
		case 5 : return("nuxy"); break ;
		case 1 : return("dens"); break ;
		default: return("x"); break;
	}
}

/** Creates batch input file for AN**S software
 * TODO: finish this!
 *  Note that element/disp/load groups are still ignored
 * @param fw pointer to output file
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdb_export_to_ans(FILE *fw, long *opts, long optlen)
{
  int  rv = AF_OK ;
	long i, j ;
	long len, len_all ;
	long size ;
	long isize ;
	long pos;
	long rep_i ;
	long etype_id, etype_type;
  long rtype_pos ;
  long mtype_pos ;
  long nload_pos, ndisp_pos;
	long len_nl, len_nd ;
	long type, dir;
	long rset_rep = 0 ;
	long rset_num = 0 ;
	char *varstr = NULL ;

  /* Setting up indexes: */
  fdbInputSyncStats();

  fdbInputRenumFromFlds(ELEM, ELEM_FROM, ELEM_ID, ENODE, ENODE_ELEM);  
  fdbInputRenumFromFlds(ELOAD, ELOAD_FROM, ELOAD_ID, ELVAL, ELVAL_ELID);

	/* ** Element types  ---------------------------------------  */

	len = fdbInputTabLenSel(ETYPE) ;

	/* number of element types */
	if (len > 0)
	{
		/* description of element types: */
		for (i=0; i<len; i++)
		{
    	if (fdbTestSelect(&InputTab[ETYPE], &inputData.intfld, i) != AF_YES) 
		   	{ continue ; }

			fprintf(fw,"et,%li,%li\n",
					fdbInputGetInt(ETYPE, ETYPE_ID, i),
					fdb_ufem_et_to_ans( fdbInputGetInt(ETYPE, ETYPE_TYPE, i) )
					);
		}
  }
	fprintf(fw,"\n");

	/* ** Real sets  ---------------------------------------  */

	/* number of real sets */
#if 0 /* TODO fix this */
	len = fdbInputTabLenSel(RSET) ;

	if (len > 0)
	{
		size = 0 ; /* ! */

		/* description of real sets: */
		for (i=0; i<len; i++)
		{
    	if (fdbTestSelect(&InputTab[RSET], &inputData.intfld, i) != AF_YES) 
		   	{ continue ; }

			size += fdbInputGetInt(RSET, RSET_NVAL, i) ;

			rset_num = fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals_rep ;

			if (rset_num > 0)
			{
			 rset_rep = (long)
				(
				fdbInputGetInt(RSET, RSET_NVAL, i)
				-
				fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals
				) / rset_num ;
			}
			if ((rset_rep * rset_num + 
				fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals)
					<
				fdbInputGetInt(RSET, RSET_NVAL, i))
			{
				rset_rep++ ;
			}

			fprintf(fw,"rs,%li,%li,%li\n",
					fdbInputGetInt(RSET, RSET_ID, i),
					fdbInputGetInt(RSET, RSET_TYPE, i),
					rset_rep
					);
		}

		/* list of properties */
		for (i=0; i<len; i++)
		{
    	if (fdbTestSelect(&InputTab[RSET], &inputData.intfld, i) != AF_YES) 
		   	{ continue ; }

			isize = fdbInputGetInt(RSET, RSET_NVAL, i) ;
		
			fdbInputCountInt(RVAL, RVAL_RSET, 
				             	fdbInputGetInt(RSET, RSET_ID, i), 
										 	&pos);


			rep_i = 0;
			for (j=0; j<isize; j++)
			{
				if (j < fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals)
				{
					rep_i = 0 ;
				}
				else
				{
					rep_i = (long)((j - fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals)
						/ fdbElementType[fdbInputGetInt(RSET, RSET_TYPE, i)].reals_rep + 1) ;

					if (rep_i <= 0) {continue;}
				}

				fprintf(fw,"r,%s,%li,%e,%li\n",
            ciGetVarNameFromGrp(
            fdbFemStrFromInt(fdbInputGetInt(RVAL,RVAL_TYPE,pos+j)),"real"
						),
						fdbInputGetInt(RVAL, RVAL_RSET,pos+j),
						fdbInputGetDbl(RVAL, RVAL_VAL,pos+j),
						rep_i
						);
			}
		}
		fprintf(fw,"\n");
  }
#endif

#if 0
	/* ** Material properties  -----------------------------  */

	/* number of material sets */
	len = fdbInputTabLenSel(MAT) ;

	size = 0 ; /* ! */

	/* description of materia sets: */
	for (i=0; i<len; i++)
	{
    if (fdbTestSelect(&InputTab[MAT], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		size += fdbInputGetInt(MAT, MAT_NVAL, i) ;

		rset_num = fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals_rp ;

			if (rset_num > 0)
			{
			 rset_rep = (long)
				(
				fdbInputGetInt(MAT, MAT_NVAL, i)
				-
				fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals
				) / rset_num ;
			}
			if ((rset_rep * rset_num + 
				fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals)
					<
				fdbInputGetInt(MAT, MAT_NVAL, i))
			{
				rset_rep++ ;
			}

	/*	fprintf(fw,"mat,%li,%li,%li\n", fdbInputGetInt(MAT, MAT_ID, i), fdbInputGetInt(MAT, MAT_TYPE, i), rset_rep);*/
	}

	/* list of properties */
	for (i=0; i<len; i++)
	{
    if (fdbTestSelect(&InputTab[MAT], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(MAT, MAT_NVAL, i) ;
		
		fdbInputCountInt(MVAL, MVAL_MAT, 
				             fdbInputGetInt(MAT, MAT_ID, i), 
										 &pos);

		rep_i = 0;
		for (j=0; j<isize; j++)
		{
			if (j < fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals)
			{
				rep_i = 0 ;
			}
			else
			{
				rep_i = (long)((j - fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals)
					/ fdbMatType[fdbInputGetInt(MAT, MAT_TYPE, i)].vals_rp + 1) ;

				if (rep_i <= 0) {continue;}
			}

			if ((varstr=fdb_ufem_mat_to_ans(fdbInputGetDbl(MVAL), MVAL_VAL,pos+j)) != "x")
			{

				fprintf(fw,"mp,%s,%li,%e\n",
            ciGetVarNameFromGrp(
            fdbFemStrFromInt(fdbInputGetInt(MVAL,MVAL_TYPE,pos+j)),"material"
						),
						fdbInputGetInt(MVAL, MVAL_MAT,pos+j),
						varstr
						);
			}
			free(varstr); varstr=NULL;
		}
	}
	fprintf(fw,"\n");
#endif

#if 0
	/* ** KPs  -------------------------------------------  */

	len_all = fdbInputTabLenAll(KPOINT) ;

	for (i=0; i<len_all; i++)
	{
    if (fdbInputTestSelect(KPOINT,i) == AF_YES)
    {
	  	fprintf(fw, "k,%li,%e,%e,%e\n", 
				fdbInputGetInt(KPOINT,KPOINT_ID, i),
				fdbInputGetDbl(KPOINT,KPOINT_X, i),
				fdbInputGetDbl(KPOINT,KPOINT_Y, i),
				fdbInputGetDbl(KPOINT,KPOINT_Z, i)
			);
    }
	}

	fprintf(fw,"\n");


	/* ** Geometric entities  ------------------------------  */

	/* number of elements */
	len = fdbInputTabLenSel(ENTITY) ;

	len_all = fdbInputTabLenAll(ENTITY) ;

	size = 0 ; /* ! */

	/* description of entities: */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ENTITY], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		size += fdbInputGetInt(ENTITY, ENTITY_KPS, i) ;
    
		rtype_pos =	fdbInputGetInt(ENTITY, ENTITY_RS, i);
		mtype_pos =	fdbInputGetInt(ENTITY, ENTITY_MAT, i);

    if (rtype_pos < 0) {rtype_pos = 0 ;}
    if (mtype_pos < 0) {mtype_pos = 0 ;}

		fprintf(fw,"gep,%s,%li,%li,%li,%li,%li\n",
        ciGetVarNameFromGrp( fdbFemStrFromInt(
				fdbInputGetInt(ENTITY, ENTITY_TYPE, i))
					,"geom_ent"),
				fdbInputGetInt(ENTITY, ENTITY_ID, i),
				fdbInputGetInt(ENTITY, ENTITY_ETYPE, i),
				rtype_pos,
				mtype_pos,
				fdbInputGetInt(ENTITY, ENTITY_SET, i)
				);
	}

	/* list of keypoints */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ENTITY], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(ENTITY, ENTITY_KPS, i) ;

		pos = fdbInputGetInt(ENTITY, ENTITY_KFROM, i) ;
		
		if (isize > 0)
		{
			fprintf(fw,"ge,%s,%li",
        ciGetVarNameFromGrp( fdbFemStrFromInt(
				fdbInputGetInt(ENTITY, ENTITY_TYPE, i))
					,"geom_ent"),
					fdbInputGetInt(ENTKP,ENTKP_ENT, pos)
					);
			for (j=0; j<isize; j++)
			{
				fprintf(fw,",%li",fdbInputGetInt(ENTKP,ENTKP_KP,pos+j));
			}
		}
		fprintf(fw,"\n");
	}

	/* list of divisions */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ENTITY], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputCountInt(ENTDIV, ENTDIV_ENT,
				fdbInputGetInt(ENTITY,ENTITY_ID, i), &pos) ;
		
		if (isize > 0)
		{
			fprintf(fw,"gediv,%li",
					fdbInputGetInt(ENTDIV,ENTDIV_ENT, pos)
					);
			for (j=0; j<isize; j++)
			{
				fprintf(fw,",%li",fdbInputGetInt(ENTDIV,ENTDIV_DIV,pos+j));
			}
		}
		fprintf(fw,"\n");
	}

	fprintf(fw,"\n");
#endif

	/* ** Nodes  -------------------------------------------  */

	len_all = fdbInputTabLenAll(NODE) ;

	for (i=0; i<len_all; i++)
	{
    if (fdbInputTestSelect(NODE,i) == AF_YES)
    {
	  	fprintf(fw, "n,%li,%e,%e,%e\n", 
				fdbInputGetInt(NODE,NODE_ID, i),
				fdbInputGetDbl(NODE,NODE_X, i),
				fdbInputGetDbl(NODE,NODE_Y, i),
				fdbInputGetDbl(NODE,NODE_Z, i)
			);
    }
	}

	fprintf(fw,"\n");

	/* ** Elements  ----------------------------------------  */
#if 0

	/* number of elements */
	len = fdbInputTabLenSel(ELEM) ;

	len_all = fdbInputTabLenAll(ELEM) ;

	size = 0 ; /* ! */

	/* description of elements: */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		size += fdbInputGetInt(ELEM, ELEM_NODES, i) ;
		etype_id = fdbInputGetInt(ELEM, ELEM_TYPE, i) ;
		fdbInputCountInt(ETYPE, ETYPE_ID, etype_id, &pos) ;
		etype_type = fdbInputGetInt(ETYPE, ETYPE_TYPE, pos) ;
    
		rtype_pos =	fdbInputGetInt(ELEM, ELEM_RS, i);
		mtype_pos =	fdbInputGetInt(ELEM, ELEM_MAT, i);

    if (rtype_pos < 0) {rtype_pos = 0 ;}
    if (mtype_pos < 0) {mtype_pos = 0 ;}

		fprintf(fw,"ep,%li,%li,%li,%li,%li\n",
				fdbInputGetInt(ELEM, ELEM_ID, i),
				etype_id,
				rtype_pos,
				mtype_pos,
				fdbInputGetInt(ELEM, ELEM_SET, i)
				);
	}

	/* list of nodes */
	for (i=0; i<len_all; i++)
	{
    if (fdbTestSelect(&InputTab[ELEM], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(ELEM, ELEM_NODES, i) ;

		pos = fdbInputGetInt(ELEM, ELEM_FROM, i) ;
		
		if (isize > 0)
		{
			fprintf(fw,"e,%li", fdbInputGetInt(ENODE,ENODE_ELEM, pos));
			for (j=0; j<isize; j++)
			{
				fprintf(fw,",%li",fdbInputGetInt(ENODE,ENODE_ID,pos+j));
			}
		}
		fprintf(fw,"\n");
	}

	fprintf(fw,"\n");
#endif

	/*  ** Gravitation  ------------------------------------  */
#if 0
	if (fdbInputTabLenSel(GRAV) >= 1)
	{
		len_all = fdbInputTabLenAll(GRAV) ;

		for (i=0; i<len_all; i++)
		{
    if (fdbTestSelect(&InputTab[GRAV], &inputData.intfld, i) != AF_YES) 
			 { continue ; }
		
		fprintf(fw,"accel,%li,%e,%li,%li\n",
				fdbInputGetInt(GRAV,GRAV_DIR,i),
				fdbInputGetDbl(GRAV,GRAV_VAL,i),
				fdbInputGetInt(GRAV,GRAV_SET,i),
				fdbInputGetInt(GRAV,GRAV_ID,i)
				);
		}
	}

	fprintf(fw,"\n");
#endif

	/* nodal disp/load data length */
#if 0
	len_nd = fdbInputTabLenSel(NDISP) ;
	len_nl = fdbInputTabLenSel(NLOAD) ;
	
	/* nodal disps. + nodal load should be here..  */
	for (i=0; i<len_nd; i++)
	{
    if (fdbTestSelect(&InputTab[NDISP], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

    ndisp_pos = fdbInputGetInt(NDISP, NDISP_NODE, i);

		fprintf(fw,"d,%li,%s,%e,%li\n",
				ndisp_pos,
        ciGetVarNameFromGrp(
        fdbFemStrFromInt(
					fdbInputGetInt(NDISP, NDISP_TYPE, i)
					),"disp"),
				fdbInputGetDbl(NDISP, NDISP_VAL, i),
				fdbInputGetInt(NDISP, NDISP_SET, i)
				); 
	}

	fprintf(fw,"\n");

	for (i=0; i<len_nl; i++)
	{
		if (fdbTestSelect(&InputTab[NLOAD], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		fdb_to_fem_nl_type(fdbInputGetInt(NLOAD, NLOAD_TYPE, i), &type, &dir);

    nload_pos = fdbInputGetInt(NLOAD, NLOAD_NODE, i);
			
		fprintf(fw,"f,%li,%s,%e,%li\n",
				nload_pos,
        ciGetVarNameFromGrp(
        fdbFemStrFromInt(
					fdbInputGetInt(NLOAD, NLOAD_TYPE, i)
					),"load"),
				fdbInputGetDbl(NLOAD, NLOAD_VAL, i),
				fdbInputGetInt(NLOAD, NLOAD_SET, i)
				); 
	}

	fprintf(fw,"\n");
#endif

	/* TODO:  element load  should be here... maybe later  */
#if 0
	for (i=0; i<fdbInputTabLenAll(ELOAD); i++)
	{
    if (fdbTestSelect(&InputTab[ELOAD], &inputData.intfld, i) != AF_YES) 
		   { continue ; }

		isize = fdbInputGetInt(ELOAD, ELOAD_NVAL, i) ;

		pos = fdbInputGetInt(ELOAD, ELOAD_FROM, i) ;
		
		if (isize > 0)
		{
			fprintf(fw,"el,%li,%li", 
					fdbInputGetInt(ELOAD,ELOAD_ELEM, i),
					fdbInputGetInt(ELOAD,ELOAD_TYPE, i)
					);
			for (j=0; j<isize; j++)
			{
				fprintf(fw,",%e",fdbInputGetDbl(ELVAL,ELVAL_VAL,pos+j));
			}
		}
		fprintf(fw,"\n");
	}

	fprintf(fw,"\n");
#endif

  return(rv);
}

/* end of fdb_expt.c */
