/*
   File name: fdb_impt.c
   Date:      2008/06/21 14:49
   Author:    Jiri Brozovsky

   Copyright (C) 2008  Jiri Brozovsky

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

	 Database for FEM - import from model "fem" solver
*/

#include "fdb_fem.h"
#include "cint.h"
#include "fdb_edef.h"
#include "fdb_mdef.h"


/** Imports data from "fem" file format 
 *  PREPARSED FEM FILE is expected
 *  Note that not all data are imported (ignored: random, dynamics,...)
 *  Note that element/disp/load groups are still ignored
 * @param fr pointer to input file
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdb_import_from_fem(FILE *fr, long *opts, long optlen)
{
  int  rv     = AF_OK ;
	long i, j, k ;
	long step   =  0 ;
	long dep_on = -1 ;
	long solver =  0 ;
	long nlsolv =  0 ;
	long steps  =  0 ;
	long len, vlen, sum ;
	long set_id, set_type, set_nvals, set_et, set_rt, set_mt, rep_num, set_dir, dt, type  ;
	long set_eti, set_rti, set_mti ;
	long nodes[510]; /* field for nodes */
	double val, x,y,z ;

	fdbInputCleanData();

	/* ** General information */

	/* step/time number, depends on number (-1 means on nothing) */
	fscanf(fr,"%li %li", &step, &dep_on) ;

	/* linear solver, non-linear solver, number of steps for non-lin */
	fscanf(fr,"%li %li %li\n", &solver, &nlsolv, &steps);

	/* ** Real sets and element types ----------------------  */

	fscanf(fr,"%li\n", &len);

	if (len == 0) 
	{
		fprintf(msgout,"[w] %s\n",
				_("No real sets are defined. It may be correct for some 3D problems."));
	}

	sum = 0 ;

	for (i=0; i<len; i++)
	{
		fscanf(fr,"%li %li %li", &set_id, &set_type, &set_nvals );
		if ((set_type > 0) && (set_type <fdbElementTypeLen)) 
		{
			if (fdbElementType[set_type].reals_rep > 0)
			{
				rep_num = (long)((set_nvals - fdbElementType[set_type].reals)/fdbElementType[set_type].reals_rep);
				if (rep_num < 0) 
				{
					fprintf(msgout,"[E] %s: %li\n",_("Invalid number of real set data for id"), set_id);
					return(AF_ERR_VAL);
				}
			}
			else
			{
				rep_num = 0 ;
			}

			if ((rv = f_rset_new_change(set_id, set_type, rep_num)) != AF_OK)
			{
				fprintf(msgout,"[E] %s: %li\n",_("Unable to create real set"), set_id);
				return(AF_ERR_VAL);
			}

			if ((rv = f_et_new_change(set_id, set_type)) != AF_OK)
			{
				fprintf(msgout,"[E] %s: %li\n",_("Unable to create element type"), set_id);
				return(AF_ERR_VAL);
			}
		}
		else
		{
			fprintf(msgout,"[E] %s: %li\n",_("Invalid real set (element) type"), set_type);
			return(AF_ERR_VAL);
		}

		sum += fdbElementType[set_type].reals + rep_num*fdbElementType[set_type].reals_rep ;
	}

	/* ** Real set data  ------------------------------------  */
	fscanf(fr,"%li\n", &vlen);

	if (vlen != sum)
	{
		fprintf(msgout,"[E] %s: %li\n",_("Not all real set data are included in input file."), set_id);
		fprintf(msgout,"[E] %s: %li\n",_("Please modify the input file and include all real set data."), set_id);
		return(AF_ERR_VAL);
	}

	for (i=0; i<len; i++)
	{
		set_type = fdbInputGetInt(RSET, RSET_TYPE, i) ;
		set_id = fdbInputGetInt(RSET, RSET_ID, i) ;
		if (fdbElementType[set_type].reals_rep > 0)
		{
			rep_num = (long)((fdbInputGetInt(RSET, RSET_NVAL, i)  - fdbElementType[set_type].reals)/fdbElementType[set_type].reals_rep);
		}
		else
		{
			rep_num = 0 ;
		}

		for (j=0; j<fdbElementType[set_type].reals; j++)
		{
			fscanf(fr,"%lf", &val);
			f_rs_change(set_id, fdbElementType[set_type].r[j], val, 0) ;
		}
		
		for (j=1; j<=rep_num; j++)
		{
			for (k=0; k<fdbElementType[set_type].reals_rep; k++)
			fscanf(fr,"%lf", &val);
			f_rs_change(set_id, fdbElementType[set_type].r_rep[k], val, j) ;
		}

	}


	/* ** Material types  ----------------------------------  */

	fscanf(fr,"%li\n", &len);

	if (len <= 0) 
	{
		fprintf(msgout,"[E] %s!\n", _("No materials are defined") );
		return(AF_ERR_VAL);
	}

	sum = 0 ;

	for (i=0; i<len; i++)
	{
		fscanf(fr,"%li %li %li", &set_id, &set_type, &set_nvals );
		if ((set_type > 0) && (set_type <fdbMatTypeLen)) 
		{
			if (fdbMatType[set_type].vals_rp > 0)
			{
				rep_num = (long)((set_nvals - fdbMatType[set_type].vals)/fdbMatType[set_type].vals_rp);
				if (rep_num < 0) 
				{
					fprintf(msgout,"[E] %s: %li\n",_("Invalid number of material data for id"), set_id);
					return(AF_ERR_VAL);
				}
			}
			else
			{
				rep_num = 0 ;
			}
			if ((rv = f_mat_new_change(set_id, set_type, rep_num)) != AF_OK)
			{
				fprintf(msgout,"[E] %s: %li\n",_("Unable to create material"), set_id);
				return(AF_ERR_VAL);
			}
		}
		else
		{
			fprintf(msgout,"[E] %s: %li\n",_("Invalid material type"), set_type);
			return(AF_ERR_VAL);
		}

		sum += fdbMatType[set_type].vals + rep_num*fdbMatType[set_type].vals_rp ;
	}


	/* ** Material data  ------------------------------------  */
	fscanf(fr,"%li\n", &vlen);

	if (vlen != sum)
	{
		fprintf(msgout,"[E] %s: %li\n",_("Not all val set data are included in input file."), set_id);
		fprintf(msgout,"[E] %s: %li\n",_("Please modify the input file and include all val set data."), set_id);
		return(AF_ERR_VAL);
	}

	for (i=0; i<len; i++)
	{
		set_type = fdbInputGetInt(MAT, MAT_TYPE, i) ;
		set_id = fdbInputGetInt(MAT, MAT_ID, i) ;

		if (fdbMatType[set_type].vals_rp > 0)
		{
			rep_num = (long)((fdbInputGetInt(MAT, MAT_NVAL, i)  - fdbMatType[set_type].vals)/fdbMatType[set_type].vals_rp);
		}
		else
		{
			rep_num = 0 ;
		}

		for (j=0; j<fdbMatType[set_type].vals; j++)
		{
			fscanf(fr,"%lf", &val);
			f_mp_change(set_id, fdbMatType[set_type].val[j], val, 0) ;
		}
		
		for (j=1; j<=rep_num; j++)
		{
			for (k=0; k<fdbMatType[set_type].vals_rp; k++)
			fscanf(fr,"%lf", &val);
			f_mp_change(set_id, fdbMatType[set_type].val_rp[k], val, j) ;
		}
	}

	/* ** Nodes  -------------------------------------------  */

	fscanf(fr,"%li\n", &len);

	if (len < 2)
	{
		fprintf(msgout,"[E] %s!\n",_("Number of nodes is too low"));
		return(AF_ERR_VAL);
	}

	for (i=0; i<len; i++)
	{
		fscanf(fr, "%li %lf %lf %lf", &set_id, &x, &y, &z);
		f_n_new_change(set_id, x, y, z) ;
	}

	/* ** Element definitions  -----------------------------  */

	fscanf(fr,"%li\n", &len);

	if (len < 1)
	{
		fprintf(msgout,"[E] %s!\n",_("Number of elements is too low"));
		return(AF_ERR_VAL);
	}

	sum = 0 ;
	for (i=0; i<len; i++)
	{
		fscanf(fr,"%li %li %li %li %li", &set_id, &set_eti,  &set_rti, &set_mti, &set_nvals );
		sum += set_nvals ;

		set_et = fdbInputGetInt(RSET, RSET_ID, set_rti) ;
		set_rt = set_et ;
		set_mt = fdbInputGetInt(MAT, MAT_ID, set_mti) ;

		f_e_new_change(set_id, set_et, set_rt, set_mt, set_nvals) ;
	}

	/* ** Element nodes  -----------------------------------  */

	fscanf(fr,"%li\n", &vlen);

	if (vlen != sum)
	{
		fprintf(msgout,"[E] %s: %li!\n",_("Invalid number of element nodes"), set_id);
		return(AF_ERR_VAL);
	}

	for (i=0; i<len; i++)
	{
		vlen = fdbInputGetInt(ELEM, ELEM_NODES, i) ;
		set_id = fdbInputGetInt(ELEM, ELEM_ID, i) ;

		for (j=0; j<vlen; j++)
		{
			fscanf(fr,"%li", &nodes[j]) ;
			nodes[j] = fdbInputGetInt(NODE,NODE_ID, nodes[j]) ;
		}

		f_en_change(set_id, nodes, vlen) ;
	}

	/*  ** Gravitation  ------------------------------------  */

	/* TODO */
	fscanf(fr,"%li %lf", &set_type, &val);
	if (val != 0.0)
	{
		f_gl_new_change(0, 0, set_type, val);
	}

	/* ** Loads and supports in nodes  ---------------------  */

	fscanf(fr,"%li\n", &len);

	if (len <= 0)
	{
		fprintf(msgout,"[E] %s: %li!\n",_("No loads and supports"), set_id);
		return(AF_ERR_VAL);
	}

	for (i=0; i<len; i++)
	{
		fscanf(fr,"%li %li %li %lf",&set_id, &set_type, &set_dir, &val);

		set_id = fdbInputGetInt(NODE,NODE_ID, set_id) ; /* node to be used */

		dt = 0 ;
		
		switch (set_type)
		{
			case 1: /* displacements */
				type = 0 + set_dir ;
				dt = 1 ;
				break ;
			case 2: /* stiffnesses */
				type = 6 + set_dir ;
				dt = 1 ;
				break ;
			case 3: /* forces */
				type = 0 + set_dir ;
				dt = 2 ;
				break ;
			case 4: /* displacements one-dir */
				type = 24 + set_dir ;
				dt = 1 ;
				break ;
			case 5: /* displacements one-dir */
				type = 12 + set_dir ;
				dt = 1 ;
				break ;
			case 6: /* displacements one-dir */
				type = 30 + set_dir ;
				dt = 1 ;
				break ;
			case 7: /* displacements one-dir */
				type = 18 + set_dir ;
				dt = 1 ;
				break ;

			default:
				type = 0 ;
				dt   = 0 ;
				fprintf(msgout,"[W] %s: %s %li, %s, %li!\n",_("Unrecognized load found"),
						_("type"),set_type,_("direction"),set_dir);
				break ;
		}

		switch(dt)
		{
			case 1: f_nd_new_change(0,set_id,type,0, val ); break;
			case 2: f_nl_new_change(0,set_id,type,0, val ); break;
		}
	}
		
	/* TODO: eloads,... */

	return(rv);
}

/** Converts gmsh element type to uFEM type
 * Note: line/plane entities are converted to 2D plane problem in uFEM!
 * @param gmsh_type element type according to GMSH
 * @return uFEM type number (0 if unsupported)
 */ 
long fdb_etype_gmsh_to_fem(long gmsh_type)
{
  switch(gmsh_type)
  {
    case 1: return(3); /* line/beam */ break ;
    case 2: return(11); /* triangle "T6" */ break ;
    case 3: return(2); /* four-node isoparametric */ break ;
    case 4: return(4); /* tetrahedron */ break ;
    case 5: return(10); /* brick */ break ;
    case 17: return(8); /* brick wit 20 nodes */ break ;
    default: return(0); /* all other are unsupported */ break ;
  }
  return(0);
}

/** Imports data from GMSH mesh format 
 *  Note that not all data are imported (only nodes/elements)
 * @param fr pointer to input file
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdb_import_from_gmsh1(FILE *fr, long *opts, long optlen)
{
  int  rv     = AF_OK ;
	long i, j ;
  long nlen, elen, id ;
  long etyp0, etyp, et_id, set, rset, mat, nnum, pos;
  long nodes[256]; /* max. 256 nodes is possible! */
  double x, y, z ;
  char str[CI_STR_LEN];

  for (i=0; i<CI_STR_LEN; i++) { str[i] = '\0'; }
  fgets(str,CI_STR_LEN, fr);
  ciStrCompr(str);
  if (strcmp("$nod", str) != 0)
  {
    fprintf(msgout,"[E] %s!\n", _("Not a GMSH mesh file"));
    return(AF_ERR_VAL);
  }


  /* Nodes  ------------------------------------ */

  fscanf(fr,"%li", &nlen);

  if (nlen < 1)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid (too low) number of nodes")) ;
    return(AF_ERR_SIZ);
  }

  for (i=0; i<nlen; i++)
  {
    fscanf(fr,"%li %lf %lf %lf", &id, &x, &y, &z);
		rv =  f_n_new_change(id, x, y, z) ;
    if (rv != AF_OK){return(rv);}
  }

  for (i=0; i<CI_STR_LEN; i++) { str[i] = '\0'; }
  fgets(str,CI_STR_LEN, fr); /*$ENDNOD */
  for (i=0; i<CI_STR_LEN; i++) { str[i] = '\0'; }
  fgets(str,CI_STR_LEN, fr); /*$ENDNOD */

  /* Elements  --------------------------------- */

  for (i=0; i<CI_STR_LEN; i++) { str[i] = '\0'; }
  fgets(str,CI_STR_LEN, fr);
  ciStrCompr(str);
  if (strcmp("$elm", str) != 0)
  {
    fprintf(msgout,"[E] %s!\n", _("Not a correct GMSH mesh file"));
    return(AF_ERR_VAL);
  }

  fscanf(fr,"%li", &elen);


  if (elen < 1)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid (too low) number of elements")) ;
    return(AF_ERR_SIZ);
  }

  for (i=0; i<elen; i++)
  {
    fscanf(fr,"%li %li %li %li %li", &id, &etyp0, &mat, &set, &nnum );

    if (nnum < 1) 
    {
      fprintf(msgout,"[E] %s: %li\n", _("Invalid element found"), id);
      return(AF_ERR_VAL);
    }

    if ((etyp = fdb_etype_gmsh_to_fem(etyp0)) == 0)
    {
      fprintf(msgout,"[w] %s: %li\n", _("Element is ignored"), id);
      continue ;
    }

    if (fdbInputCountInt(ETYPE, ETYPE_TYPE, etyp, &pos) > 0)
    {
      /* exists */
      et_id = fdbInputGetInt(ETYPE, ETYPE_ID, pos) ;
    }
    else
    {
      /* new */
			if ((rv = f_et_new_change(0, etyp )) != AF_OK)
      {
        fprintf(msgout,"[E] %s!\n", _("Element type creation failed"));
        return(rv);
      }
      et_id = fdbInputFindMaxInt(ETYPE, ETYPE_ID) ;
    }

    if (fdbInputCountInt(RSET, RSET_ID, mat, &pos) > 0)
    {
      /* real set exists */
      if (fdbInputGetInt(RSET, RSET_TYPE, pos) == etyp)
      {
        /* OK */
        rset = mat ;
      }
      else
      {
       if ((rv = f_rset_new_change(0, etyp, 0)) != AF_OK)
       {
         fprintf(msgout,"[E] %s!\n", _("Real set creation failed"));
         return(rv);
       }
       rset = fdbInputFindMaxInt(RSET, RSET_ID) ;
      }
    }
    else
    {
      if ((rv = f_rset_new_change(mat, etyp, 0)) != AF_OK)
      {
        fprintf(msgout,"[E] %s!\n", _("Real set creation failed"));
        return(rv);
      }
      rset = mat ;
    }

    if (fdbInputCountInt(MAT, MAT_ID, mat, &pos) > 0)
    {
      /* no action required */
     }
    else
    {
      if ((rv = f_mat_new_change(mat, 1, 0)) != AF_OK)
      {
        fprintf(msgout,"[E] %s!\n", _("Material type creation failed"));
        return(rv);
      }
    }

	  f_e_new_change(id, et_id, rset, mat, nnum) ;

    for (j=0; j<nnum; j++)
    {
      fscanf(fr,"%li", &nodes[j]);
      if (nodes[j] < 1)
      {
        fprintf(msgout,"[E] %s: %li\n", _("Invalid nodes for element"), id);
        return(rv);
      }
    }

		f_en_change(id, nodes, nnum) ;
  } /* end for(elen) */

	return(rv);
}

/** Imports data from NETGEN "neutral" mesh format  (UNTESTED!)
 *  Note that only nodes and trinagual elems. are imported (no boundary elements)
 * @param fr pointer to input file
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdb_import_from_netgen(FILE *fr, long *opts, long optlen)
{
  int  rv     = AF_OK ;
	long i ;
  long nlen, elen, id, eid ;
  /*long etyp0, etyp, et_id, set, rset, mat, nnum, pos;*/
	long rset0, mat, etyp0, pos ;
  long nodes[5];
  double x, y, z ;

  /* Nodes  ------------------------------------ */
  fscanf(fr,"%li", &nlen);

  if (nlen < 1)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid (too low) number of nodes")) ;
    return(AF_ERR_SIZ);
  }

	id = fdbInputFindMaxInt(NODE, NODE_ID) ;

  for (i=0; i<nlen; i++)
  {
    fscanf(fr,"%lf %lf %lf", &x, &y, &z);
		id++;
		rv =  f_n_new_change(id, x, y, z) ;
    if (rv != AF_OK){return(rv);}
  }

  /* Elements  --------------------------------- */
	etyp0 = fdbInputFindMaxInt(ETYPE, ETYPE_ID) ;
	rset0 = fdbInputFindMaxInt(RSET, RSET_ID) ;
	eid   = fdbInputFindMaxInt(ELEM, ELEM_ID) ;

  fscanf(fr,"%li", &elen);

  if (elen < 1)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid (too low) number of elements")) ;
    return(AF_ERR_SIZ);
  }

	etyp0++ ;
	rset0++ ;

	if ((rv = f_et_new_change(etyp0, 4 )) != AF_OK)
  {
    fprintf(msgout,"[E] %s!\n", _("Element type creation failed"));
    return(rv);
  }

	if ((rv = f_rset_new_change(rset0, 4, 0)) != AF_OK)
  {
     fprintf(msgout,"[E] %s!\n", _("Real set creation failed"));
     return(rv);
  }

  for (i=0; i<elen; i++)
  {
		eid++ ;
    fscanf(fr,"%li %li %li %li %li", &mat, &nodes[0], &nodes[1], &nodes[2], &nodes[3] );

		if (fdbInputCountInt(MAT, MAT_ID, mat, &pos) > 0)
    {
      /* no action required */
     }
    else
    {
      if ((rv = f_mat_new_change(mat, 1, 0)) != AF_OK)
      {
        fprintf(msgout,"[E] %s!\n", _("Material type creation failed"));
        return(rv);
      }
    }

	  f_e_new_change(eid, etyp0, rset0, mat, 4) ;

		f_en_change(eid, nodes, 4) ;

  } /* end for(elen) */

	return(rv);
}


/** Calls import function
 * @param fname filename (including path and extension, if required)
 * @param format file format (1=fem solver input,...)
 * @param opts field of option (or NULL)
 * @param optlen lenght of "opts"
 * @return status
 */
int fdbImport(char *fname, long format, long *opts, long optlen)
{
  int     rv = AF_OK ;
  FILE   *fr = NULL ;

  if ((fr = fopen(fname, "r")) == NULL)
  {
    fprintf(msgout, "[E] %s!\n", _("Cannot open import file"));
    return(AF_ERR_IO);
  }
  else
  {
    /* import */
    switch (format)
    {
      case 1 : /* "fem" solver format */
               if ((rv = fdb_import_from_fem(fr, opts, optlen)) != AF_OK)
                  { fprintf(msgout, "[E] %s!\n", _("Import failed")); }
               break ;

      case 2 : /* "GMSH" modeller format */
               if ((rv = fdb_import_from_gmsh1(fr, opts, optlen)) != AF_OK)
                  { fprintf(msgout, "[E] %s!\n", _("Import failed")); }
               break ;

      case 3 : /* "NETGEN" mesh generator format */
               if ((rv = fdb_import_from_netgen(fr, opts, optlen)) != AF_OK)
                  { fprintf(msgout, "[E] %s!\n", _("Import failed")); }
               break ;

      default: /* unknown format */
               fprintf(msgout, "[E] %s!\n", _("Unknown format"));
               rv = AF_ERR_VAL ;
               break ;
    }

    if (fclose(fr) != 0)
    {
      fprintf(msgout, "[E] %s!\n", _("Cannot close import file"));
      return(AF_ERR_IO);
    }
    else
    {
      return(rv) ;
    }
  }
}
/* end of fdb_impt.c */
