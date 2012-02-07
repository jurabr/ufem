/*
   File name: fdb_ent.c
   Date:      2003/08/22 12:11
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

	 FEM Database - geometric entities
*/

extern double fdbDistTol ;
extern long found_suitable_kpoint(double x, double y, double z, double tol, int *test);

#include "fdb_fem.h"
#include "fdb_edef.h"
#include "fdb_mdef.h"
#include "fdb_geom.h"

/** Creates new entity or changes properties of existing one
 * @param id element identifier
 * @param ent_type type of entity (number: line, area,...)
 * @param et type of element (user-defined etype id)
 * @param rs used real set (user-defined id)
 * @param mat material type - user defined material
 * @param set f.e. set number (0 means default)
 * @return status
 */
int f_ent_new_change(long id, long ent_type, long et, long rs, long mat, long set)
{
	int  rv    = AF_OK ;
	int  nnew  = AF_NO ;
	long pos   = -1 ;
	long posn  = -1 ;
	long posn0 = -1 ;
	long dset  = 1 ;
	long count = 0 ;
	long newid = 0 ;
  long intdiv = 1;
  double dblrat = 1.0;
	int  i ;
	long e_type = 0 ; /* program's element type identifier */

	if (fdbInputCountInt(ETYPE, ETYPE_ID, et, &pos) < 1)
	{
		fprintf(msgout,"[E] %s (%li)!\n", _("Invalid element type"), et);
		return(AF_ERR_VAL) ; /* suggested element type not found */
	}
	else
	{
		e_type = fdbInputGetInt(ETYPE, ETYPE_TYPE, pos);
	}

  if (fdbTestCompEtype(ent_type, e_type) != AF_YES)
  {
    fprintf(msgout,"[E] %s (%li)!\n",_("Suggested element type cannot be used here"),e_type);
    return(AF_ERR_VAL);
  }

  intdiv = fdbSetInputDefDiv(0) ;

	if ((fdbElementType[e_type].reals != 0) || (fdbElementType[e_type].reals_rep != 0))
	{
		if (fdbInputCountInt(RSET, RSET_ID, rs, &pos) < 1)
		{
			fprintf(msgout,"[W] %s!\n",_("Incorrect of no real set"));
#if 1
			return(AF_ERR_VAL) ; /* suggested real set not found */
#endif
		}
		else
		{
			if (fdbInputGetInt(RSET, RSET_TYPE, pos) != e_type)
			{
				fprintf(msgout,"[E] %s!\n",_("Element type michmach"));
				return(AF_ERR_VAL) ; /* element type michmach */
			}
		}
	}


	if (id <= 0) /* no ID specified => new */
	{
		newid = ( fdbInputFindMaxInt(ENTITY, ENTITY_ID)+1 );
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(ENTITY, ENTITY_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			newid = id ;
			nnew = AF_NO ;
		}
		else /* new data */
		{
			newid = id ;
			nnew = AF_YES ;
		}
	}

	
	if (nnew == AF_YES) /* adding of new line */
	{
		if ((rv = fdbInputAppendTableRow(ENTITY, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(ENTITY, ENTITY_ID, pos, 1, &newid, AF_NO);
		fdbInputIntChangeVals(ENTITY, ENTITY_KPS, pos, 1, 
				&fdbGeomEnt[ent_type].kps, AF_NO);
		fdbInputIntChangeVals(ENTITY, ENTITY_DIVS, pos, 1, 
				&fdbGeomEnt[ent_type].divs, AF_NO);
	}
	else /* changing of existing */
	{
		/* do nothing */
		if (fdbGeomEnt[ent_type].kps != fdbInputGetInt(ENTITY, ENTITY_KPS, pos)) 
		{
			return(AF_ERR_VAL) ; /* number of kps cannot be changed */
		}
		if (fdbGeomEnt[ent_type].divs != fdbInputGetInt(ENTITY, ENTITY_DIVS, pos)) 
		{
			return(AF_ERR_VAL) ; /* number of divs cannot be changed */
		}
	}

	/* common for all: */
	if (set > 0) 
	{ 
		dset = set ; 
	}
	else
	{
		dset = fdbSetInputDefEset(0) ; /* default set */
	}

	fdbInputIntChangeVals(ENTITY, ENTITY_TYPE, pos, 1, &ent_type, AF_NO);
	fdbInputIntChangeVals(ENTITY, ENTITY_SET, pos, 1, &dset, AF_NO);
	fdbInputIntChangeVals(ENTITY, ENTITY_ETYPE, pos, 1, &et, AF_NO);
	fdbInputIntChangeVals(ENTITY, ENTITY_RS, pos, 1, &rs, AF_NO);
	fdbInputIntChangeVals(ENTITY, ENTITY_MAT, pos, 1, &mat, AF_NO);


	if (nnew == AF_YES) /* allocation of space for nodes */
	{
    /* entity keypoints */
		if ((rv = fdbInputAppendTableRow(ENTKP, 
																		 fdbGeomEnt[ent_type].kps, 
																		 &posn)) != AF_OK) 
		{
			/* cannot allocate space => delete whole element */
			fdbInputRemoveTableRow(ENTITY, pos, 1);
      fprintf(msgout,"[E] %s!\n",_("Out of memory"));
			return(rv);
		}

	  fdbInputIntChangeVals(ENTITY, ENTITY_KFROM, pos, 1, &posn, AF_NO);

		for (i=0; i<fdbGeomEnt[ent_type].kps; i++)
		{
			fdbInputIntChangeVals(ENTKP, ENTKP_ENT,  posn+i, 1, &newid, AF_NO);
			fdbInputIntChangeVals(ENTKP, ENTKP_POS, posn+i, 1, (long *)&i,  AF_NO);
		}

    posn0 = posn ;
    posn = -1 ;

    /* entity divisions */
		if ((rv = fdbInputAppendTableRow(ENTDIV, 
																		 fdbGeomEnt[ent_type].divs, 
																		 &posn)) != AF_OK) 
		{
			/* cannot allocate space => delete whole element */
			fdbInputRemoveTableRow(ENTITY, pos, 1);
			fdbInputRemoveTableRow(ENTKP, posn0, fdbGeomEnt[ent_type].kps);
      fprintf(msgout,"[E] %s!\n",_("Out of memory"));
			return(rv);
		}

	  fdbInputIntChangeVals(ENTITY, ENTITY_DFROM, pos, 1, &posn, AF_NO);

		for (i=0; i<fdbGeomEnt[ent_type].divs; i++)
		{
			fdbInputIntChangeVals(ENTDIV, ENTDIV_ENT,  posn+i, 1, &newid, AF_NO);
			fdbInputIntChangeVals(ENTDIV, ENTDIV_POS, posn+i, 1, (long *)&i,  AF_NO);

			fdbInputIntChangeVals(ENTDIV, ENTDIV_DIV, posn+i, 1, &intdiv,  AF_NO);
			fdbInputDblChangeVals(ENTDIV, ENTDIV_DRAT, posn+i, 1, &dblrat,  AF_NO);
		}
	}

	return(rv);
}

/** Changes set of entity:
 * @param id element identifier
 * @param set f.e. set number (0 means default)
 * @return status
 */
int f_ent_change_set(long id, long set)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;

  if (set < 1)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid set number"));
		return(AF_ERR_VAL) ;
  }

	if (id <= 0) /* no ID specified */
  {
    fprintf(msgout,"[E] %s!\n", _("Element number is required"));
		return(AF_ERR_VAL) ;
  }
  else
  {
		if ( fdbInputCountInt(ENTITY, ENTITY_ID, id, &pos) < 1)
    {
      fprintf(msgout,"[E] %s!\n", _("Invalid element number"));
		  return(AF_ERR_VAL) ;
    }
    else
    {
	    fdbInputIntChangeVals(ENTITY, ENTITY_SET, pos, 1, &set, AF_NO);
    }
  }

	return(rv);
}

/** Tests if keypoints in "nodes" field really exists
 * @param nodes list of nodes (field)
 * @param nodes_len lenght of "nodes"
 * @return AF_YES if O.K. (valid), AF_ERR_VAL if K.O. (e.g. invalid)
 */
int test_kpoints_validity(long *nodes, long *node_pos, long nodes_len)
{
  int rv = AF_OK ;
  long pos = -1 ;
  int i ;

  for (i=0; i<nodes_len; i++)
  {
	  if (fdbInputCountInt(KPOINT, KPOINT_ID, nodes[i], &pos) < 1)
    {
      /* node not found => invalid! */
      return(AF_ERR_VAL);
    }
    else
    {
      if (node_pos != NULL) { node_pos[i] = pos ; }
    }
  }
  return(rv);
}

/** Sets keypoints for entity
 * @param id element id
 * @param nodes list of nodes (field)
 * @param nodes_len lenght of "nodes"
 * @return status
 */
int f_entkp_change(long id, long *nodes, long nodes_len, long ent_type)
{
  int rv = AF_OK ;
  long newid = 0 ;
	long pos   = -1 ;
	long posn  = -1 ;
  long nnodes = 0 ;
  long *node_pos = NULL ;
	long etpos;
	long etyp;
  int  i ;

  if ((node_pos=(long *)malloc(sizeof(long)*nodes_len))!=NULL)
  {
    for (i=0; i<nodes_len; i++) { node_pos[i] = 0;}
  }

  /* test of validity of nodes (if they exists) */
  if ((rv = test_kpoints_validity(nodes, node_pos, nodes_len)) != AF_OK)
  {
    free(node_pos); node_pos = NULL ;
    fprintf(msgout,"[E] %s!\n",_("Invalid nodes"));
    return(rv); /* incorrect nodes */
  }

  if (id <= 0)
  {
		newid =  fdbInputFindMaxInt(ENTITY, ENTITY_ID) + 1 ;
  }
  else
  {
    newid = id ;
  }

	if (fdbInputCountInt(ENTITY, ENTITY_ID, newid, &pos) < 1)
	{
    /* new entity will be created */
    if (fdbSetInputDefET(0) <= 0) { return(AF_ERR); }

		if (fdbInputCountInt(ETYPE, ETYPE_ID, fdbSetInputDefET(0), &etpos) < 1)
		{
      fprintf(msgout,"[E] %s!\n",_("Cannot set type of element"));
      free(node_pos); node_pos = NULL ;
			return(AF_ERR_VAL);
		}

		etyp = fdbInputGetInt(ETYPE, ETYPE_TYPE, etpos);

#if 0
    if (fdbElementType[etyp].nodes > nodes_len)
    {
      fprintf(msgout,"[E] %s!\n",_("Bad number of nodes"));
      free(node_pos); node_pos = NULL ;
      return(AF_ERR_VAL); /* bad number of nodes added! */
    }
#endif

    if ((rv = f_ent_new_change(newid, ent_type,
                                fdbSetInputDefET(0),
                                fdbSetInputDefRS(0),
                                fdbSetInputDefMAT(0),
                                fdbSetInputDefEset(0))
       ) != AF_OK)
    {
      fprintf(msgout,"[E] %s!\n",_("Cannot create new entity"));
      free(node_pos); node_pos = NULL ;
      return(rv); /* something is wrong */
    }

	  fdbInputCountInt(ENTITY, ENTITY_ID, newid, &pos) ; /* pos is needed */
  }

  nnodes = fdbInputGetInt(ENTITY, ENTITY_KPS, pos) ;

  /* is number of nodes still correct? */
  if (nnodes > nodes_len)
  {
    fprintf(msgout,"[E] %s (%li)!\n",_("Bad number of keypoints"), nnodes);
    free(node_pos); node_pos = NULL ;
    return(AF_ERR_VAL); /* bad number of nodes! */
  }

  /* now we can create the nodes (new "pos" needed): */
	if ((fdbInputCountInt(ENTKP, ENTKP_ENT, newid, &posn)) != nnodes)
  {
    fprintf(msgout,"[E] %s!\n",_("Michmach in number of keypoints"));
    free(node_pos); node_pos = NULL ;
    return(AF_ERR_VAL); /* bad number of nodes saved */
  }
  else
  {
    for (i=0; i<nnodes; i++)
    {
			fdbInputIntChangeVals(ENTKP, ENTKP_KP,  posn+i, 1, &nodes[i], AF_NO);
      if (node_pos != NULL)
         { fdbInputIntChangeVals(ENTKP, ENTKP_KPFROM,posn+i, 1, &node_pos[i], AF_NO); }
    }
  }

  free(node_pos); node_pos = NULL ;
  return(rv);
}

/** Set divisions for entity 
 * @param id entity id
 * @param div field of the divisions
 * @param div_len lenght of "div"
 * @return status
 */
int f_entkp_div_change(long id, long *div, long div_len)
{
  int rv = AF_OK ;
  long pos, posn, ival ;
  long len;
  int i ;

  if ( (div_len <=0) || (div == NULL) || (id <= 0))
  {
    fprintf(msgout,"[E] %s!\n", _("Nothing to do"));
    return(AF_ERR_EMP) ;
  }
  
  if (fdbInputCountInt(ENTITY, ENTITY_ID, id, &pos) < 1)
	{
    fprintf(msgout,"[E] %s!\n", _("Entity must be created first"));
    return(AF_ERR_EMP);
  }

  len = fdbInputGetInt(ENTITY, ENTITY_DIVS, pos) ;

  if (len > div_len) { len = div_len ; }
  
  if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, id, &posn) < 1)
	{
    fprintf(msgout,"[E] %s!\n", _("Invalid entity - please delete it"));
    return(AF_ERR_EMP);
	}

  for (i=0; i<len; i++)
  {
		if (i < div_len)
		{
    	ival = fabs(div[i]) ;
		}
		else
		{
    	ival = 0 ;
		}


    if (ival <= 0) 
    { 
      ival = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+i);
    }

    fdbInputIntChangeVals(ENTDIV, ENTDIV_DIV, posn+i, 1, &ival, AF_NO);
/*printf("ent=%li [pos=%li] val[%li->%li]=%li\n",id,)*/
  }

  return(rv);
}

/** Deletes entities and keypoints (ENTKPs)
 * @param id number of node
 * @return status
 */
int f_ent_delete(long id, long ent_type)
{
  int rv = AF_OK ;
	long pos   = -1 ;
	long posn  = -1 ;
	long posnd = -1 ;
  long nnodes = 0 ;
  long ndivs  = 0 ;
	long count = 0 ;
	long zero  = 0 ;
	long i ;

  /* element exists? */
	if (fdbInputCountInt(ENTITY, ENTITY_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no element found */
	}
  else
  {
    if (ent_type != fdbInputGetInt(ENTITY,ENTITY_TYPE, pos))
    {
		  return(AF_ERR_VAL); /* no element found */
    }
    /* number of nodes on element (hope that's okay) */
    nnodes = fdbInputGetInt(ENTITY, ENTITY_KPS,  pos) ;
    ndivs  = fdbInputGetInt(ENTITY, ENTITY_DIVS, pos) ;
  }

  /* where are keypoints? */
	if ((count=fdbInputCountInt(ENTKP, ENTKP_ENT, id, &posn)) < 1)
	{
		return(AF_ERR_EMP); /* no node found */
  }
  /* where are divs? */
	if ((count=fdbInputCountInt(ENTDIV, ENTDIV_ENT, id, &posnd)) < 1)
	{
		return(AF_ERR_EMP); /* no node found */
  }

	/* workaround for non-working deleting */
	for (i=0; i<nnodes; i++) 
	{ 
		fdbInputIntChangeVals(ENTKP,ENTKP_KP,i+posn,1, &zero, AF_NO); 
		fdbInputIntChangeVals(ENTKP,ENTKP_ENT,i+posn,1, &zero, AF_NO); 
	}

  fdbInputUpdateEntStats();

  /* deleting nodes */
  if ((rv = fdbInputRemoveTableRow(ENTKP, posn, nnodes) ) != AF_OK) 
  {
		return(AF_ERR); /* cannot delete nodes */
  }
  else
  {
    rv = fdbInputRemoveTableRow(ENTDIV, posnd, ndivs) ;

	  /* removing entity */
		fdbInputIntChangeVals(ENTITY,ENTITY_ETYPE,pos,1, &zero, AF_NO);
		fdbInputIntChangeVals(ENTITY,ENTITY_RS,pos,1, &zero, AF_NO);
		fdbInputIntChangeVals(ENTITY,ENTITY_MAT,pos,1, &zero, AF_NO);

	  rv =  fdbInputRemoveTableRow(ENTITY, pos, 1) ;

    /* updating _FROM info */
    rv = fdbInputRenumFromFlds(ENTITY,
                            ENTITY_KFROM,
                            ENTITY_ID,
                            ENTKP, 
                            ENTKP_ENT);
    rv = fdbInputRenumFromFlds(ENTITY,
                            ENTITY_DFROM,
                            ENTITY_ID,
                            ENTDIV, 
                            ENTDIV_ENT);
  }
  return(rv);
}

int f_ent_list_prn(FILE *fw, long from, long to, long ent_type)
{
  int  rv = AF_OK ;
  long id ;
	long tab_len ;
  long type ;
	long n_max = 0 ;
	long d_max = 0 ;
	long nodes ;
	long posn ;
  long i, j ;

  if (fdbInputTabLenAll(ENTITY) <= 0) { return(rv); }

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK) { return(rv); }

	for (i=0; i<fdbInputTabLenAll(ENTITY); i++)
	{
    type = fdbInputGetInt(ENTITY,ENTITY_TYPE,i) ;
    if ((ent_type != type)&&(ent_type > 0)) {continue;}

		if (n_max < fdbGeomEnt[type].kps)
		{
			n_max = fdbGeomEnt[type].kps ;
		}

		if (d_max < fdbGeomEnt[type].divs)
		{
			d_max = fdbGeomEnt[type].divs ;
		}
	}

	tab_len = 6 + n_max + d_max ;

  fdbPrnBeginTable(fw, tab_len, _("Entity")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fw, 2, _("E. Type")) ;
  fdbPrnTableHeaderItem(fw, 3, _("Real Set")) ;
  fdbPrnTableHeaderItem(fw, 4, _("Material")) ;
  fdbPrnTableHeaderItem(fw, 5, _("Group")) ;

  for (i=0; i<n_max; i++)
  {
  	fdbPrnTableHeaderItem(fw, 6+i, _("Keypoint")) ;
	}

  for (i=0; i<d_max; i++)
  {
  	fdbPrnTableHeaderItem(fw, 6+n_max+i, _("Division")) ;
	}

  for (i=0; i<fdbInputTabLenAll(ENTITY); i++)
  {
    if (fdbInputTestSelect(ENTITY, i) != AF_YES)
    {
      continue;
    }
      id    = fdbInputGetInt(ENTITY,ENTITY_ID,i) ;
			nodes = fdbInputGetInt(ENTITY,ENTITY_KPS,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemInt(fw,1,fdbInputGetInt(ENTITY,ENTITY_TYPE,i));
      fdbPrnTableItemInt(fw,2,fdbInputGetInt(ENTITY,ENTITY_ETYPE,i));
      fdbPrnTableItemInt(fw,3,fdbInputGetInt(ENTITY,ENTITY_RS,i));
      fdbPrnTableItemInt(fw,4,fdbInputGetInt(ENTITY,ENTITY_MAT,i));
      fdbPrnTableItemInt(fw,5,fdbInputGetInt(ENTITY,ENTITY_SET,i));

		fdbInputCountInt(ENTKP, ENTKP_ENT, id, &posn) ;

		for (j=0; j<nodes; j++)
		{
      fdbPrnTableItemInt(fw, 6+j, fdbInputGetInt(ENTKP,ENTKP_KP,posn+j));
		}

		if (nodes < n_max)
		{
			for (j=nodes; j<n_max; j++)
			{
      	fdbPrnTableItemStr(fw, 6+j, " ");
			}
		}


    /* */
		nodes = fdbInputGetInt(ENTITY,ENTITY_DIVS,i) ;
		fdbInputCountInt(ENTDIV, ENTDIV_ENT, id, &posn) ;

		for (j=0; j<nodes; j++)
		{
      fdbPrnTableItemInt(fw, 6+n_max+j, fdbInputGetInt(ENTDIV,ENTDIV_DIV,posn+j));
		}

		if (nodes < d_max)
		{
			for (j=nodes; j<d_max; j++)
			{
      	fdbPrnTableItemStr(fw, 6+n_max+j, " ");
			}
		}


  }
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/** Creates rectangular area from given coordinates (x,y,dx,dy)
 *
 */
int f_ent_create_dim( long type, long id, double x, double y, double z, double dx, double dy, double dz)
{
  int rv= AF_OK ;
  long   posi ;
  long   nid[20] ;
  double xi[20] ;
  double yi[20] ;
  double zi[20] ;
  long   i ;
  long   klen;
  int    test ;

  for (i=0; i<20; i++) { xi[i] = 0 ; yi[i] = 0 ; zi[i] = 0 ; }

  switch (type)
  {
    case 2: klen =  4 ;
            xi[0] = x    ; yi[0] = y    ;
            xi[1] = x+dx ; yi[1] = y    ;
            xi[2] = x+dx ; yi[2] = y+dy ;
            xi[3] = x    ; yi[3] = y+dy ;

            break ;
    case 3: klen =  8 ;
            xi[0] = x    ; yi[0] = y    ; zi[0] = z ;
            xi[1] = x+dx ; yi[1] = y    ; zi[1] = z ;
            xi[2] = x+dx ; yi[2] = y+dy ; zi[2] = z ;
            xi[3] = x    ; yi[3] = y+dy ; zi[3] = z ;

            for (i=4; i<8; i++)
            {
              xi[i] = xi[i-4] ;
              yi[i] = yi[i-4] ;
              zi[i] = z + dz ;
            }

            break ;
    case 4: klen = 20 ;
            xi[0] = x    ; yi[0] = y    ; zi[0] = z ;
            xi[1] = x+(0.5)*dx ; yi[1] = y    ; zi[1] = z ;
            xi[2] = x+dx ; yi[2] = y    ; zi[2] = z ;
            xi[3] = x+dx ; yi[3] = y+(0.5*dy)    ; zi[3] = z ;
            xi[4] = x+dx ; yi[4] = y+dy    ; zi[4] = z ;
            xi[5] = x+(0.5*dx) ; yi[5] = y+dy    ; zi[5] = z ;
            xi[6] = x ; yi[6] = y+dy    ; zi[6] = z ;
            xi[7] = x ; yi[7] = y+(0.5*dy)    ; zi[7] = z ;

            xi[8] = x ; yi[8] = y  ; zi[8] = z+(0.5*dz) ;
            xi[9] = x+dx ; yi[9] = y  ; zi[9] = z+(0.5*dz) ;
            xi[10] = x+dx; yi[10] = y+dy ; zi[10] = z+(0.5*dz) ;
            xi[11] = x; yi[11] = y+dy ; zi[11] = z+(0.5*dz) ;

            for (i=12; i<20; i++)
            {
              xi[i] = xi[i-12] ;
              yi[i] = yi[i-12] ;
              zi[i] = z + dz ;
            }

            break ;
    case 5: klen = 8 ;
            xi[0] = x    ; yi[0] = y    ; zi[0] = z ;
            xi[1] = x+(0.5)*dx ; yi[1] = y    ; zi[1] = z ;
            xi[2] = x+dx ; yi[2] = y    ; zi[2] = z ;
            xi[3] = x+dx ; yi[3] = y+(0.5*dy)    ; zi[3] = z ;
            xi[4] = x+dx ; yi[4] = y+dy    ; zi[4] = z ;
            xi[5] = x+(0.5*dx) ; yi[5] = y+dy    ; zi[5] = z ;
            xi[6] = x ; yi[6] = y+dy    ; zi[6] = z ;
            xi[7] = x ; yi[7] = y+(0.5*dy)    ; zi[7] = z ;
            break ;

    default: 
            fprintf(msgout,"[E] %s!\n",_("Invalid type of geometric entity"));
            return(AF_ERR_VAL); 
            break;
  }


  for (i=0; i<klen; i++)
  {
    posi = -1 ;
	  posi = found_suitable_kpoint(xi[i], yi[i], zi[i], fdbDistTol, &test);

    if (posi < 0)
    {
      if (f_k_new_change(0, xi[i], yi[i], zi[i], &nid[i]) != AF_OK){return(AF_ERR);}
      if (fdbInputCountInt(KPOINT, KPOINT_ID, nid[i], &posi) <= 0){return(AF_ERR);}
    }
    else
    {
      nid[i] = fdbInputGetInt(KPOINT, KPOINT_ID, posi) ;
    }
  }

  if ((rv=f_entkp_change(id, nid, klen, type)) != AF_OK ){return(rv);}

  return(rv);
}


/** Creates brick from area by extrusion/dragging along path defined by keypoints
 * @param area_id id number of area to be extruded
 * @param k_len number of keypoints (drag path definition)
 * @param klist array of keypoint id numbers
 * @param et element type vor created volume(s)
 * @param rs reals set type vor created volume(s)
 * @param mat material type type vor created volume(s)
 * @param zdiv division (number of elements) along drag path for each volume
 * @return status
 */
int f_ent_extrude_area(
    long  area_id, 
    long  k_len,
    long *klist,
    long  et, 
    long  rs,
    long  mat,
    long  zdiv)
{
  int rv= AF_OK ;
  long   nid[20] ;
  double xi[20] ;
  double yi[20] ;
  double zi[20] ;
  double *dx0 = NULL ;
  double *dy0 = NULL ;
  double *dz0 = NULL ;
  long   d_len; 
  long   i, i_pos ;
  long   pos, npos, type, posi, dpos ;
  long   div[3];
  int    test ;
  long   id = 0;
  long   klen ;
  double dx[2] ;
  double dy[2] ;
  double dz[2] ;
  double x0[8];
  double y0[8];
  double z0[8];
  long   j, jmult ;
  long   vtype = 3 ;

  ldiv_t divval ;
  int last = 0 ;


  /* Get area type */
  if (fdbInputCountInt(ENTITY,ENTITY_ID, area_id, &pos) < 1)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout, "[E] %s: %li!\n", _("Invalid area"), area_id);
#endif
    return(AF_ERR_IO);
  }

  type = fdbInputGetInt(ENTITY,ENTITY_TYPE,pos) ;

  if ((type != 2) && (type != 5))
  {
    fprintf(stdout,"[E] %s!\n", _("This entity type can not be extruded"));
    return(AF_ERR_VAL);
  }

  /* Allocate space (dx0, dy0, dz0) */
  d_len = 0 ;
  if (type == 5)
  {
    vtype = 4 ;
    if (k_len < 3 ) 
    {
      d_len = 2;
      last  = 1 ;
    }
    else
    {
      divval = ldiv(k_len-1, 2) ;
      d_len = k_len+divval.rem ; 
      if (divval.rem != 0) { last  = 1 ; }
    }
  }
  else /* type == 2 */
  {
    vtype = 3 ;
    d_len = k_len - 1 ;
  }

  if (k_len < 2 ) {d_len = 0;}

  if (d_len < 1)
  {
    fprintf(stdout,"[E] %s!\n", _("This entity type can not be extruded"));
    return(AF_ERR_VAL);
  }
  if ((dx0 = femDblAlloc(d_len)) == NULL) {rv =AF_ERR_MEM; goto memFree;}
  if ((dy0 = femDblAlloc(d_len)) == NULL) {rv =AF_ERR_MEM; goto memFree;} 
  if ((dz0 = femDblAlloc(d_len)) == NULL) {rv =AF_ERR_MEM; goto memFree;} 

  /* Compute dx0, dy0, dz0 */
  for (i=0; i<d_len; i++)
  {
    if ( (i == (d_len - 1)) && (last == 1) )
    {
      f_k_dist(klist[i], klist[i+1], &dx0[i], &dy0[i], &dz[i]);
      dx0[i-1] = 0.5*dx0[i] ;
      dy0[i-1] = 0.5*dy0[i] ;
      dz0[i-1] = 0.5*dz0[i] ;
    }
    else
    {
      f_k_dist(klist[i], klist[i+1], &dx0[i], &dy0[i], &dz0[i]);
    }
  }

  /* get divisions: */
  if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, area_id, &dpos) < 1)
	{
    fprintf(msgout,"[E] %s!\n", _("Invalid entity - please delete it"));
    return(AF_ERR_EMP);
	}
  for (i=0; i<2; i++) { div[i] = fdbInputGetInt(ENTDIV, ENTDIV_DIV, dpos+i); }
  div[2] = zdiv ;

  if (type == 2)
  {
    jmult = 1 ;
  }
  else
  {
    jmult = 2 ;
  }
  

  for (j=0; j< d_len; j+=jmult)
  {

  for (i=0; i<20; i++) { xi[i] = 0 ; yi[i] = 0 ; zi[i] = 0 ; nid[i] = -1 ; }

  switch (type)
  {
    case 2: /* rectangle -> brick */
      klen = 8 ;

      dx[0] = dx0[j];
      dy[0] = dy0[j];
      dz[0] = dz0[j];

      if (j == 0)
      {
        for (i=0; i<4; i++) /* get nodes from area */
        {
  	      npos = fdbEntKpPos(pos, i) ;
          nid[i] = fdbInputGetInt(KPOINT, KPOINT_ID, pos);
	        xi[i] = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	        yi[i] = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	        zi[i] = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;
        }
      }
      else
      {
        for (i=0; i<4; i++)
        {
         xi[i] = x0[i] ;
         yi[i] = y0[i] ;
         zi[i] = z0[i] ;
        }
      }

      for (i=4; i<8; i++) /* expand nodes */
      {
        xi[i] = xi[i-4] + dx[0] ;
        yi[i] = yi[i-4] + dy[0] ;
        zi[i] = zi[i-4] + dz[0] ;
      }

      for (i=0; i<8; i++) /* expand nodes */
      {
        posi = -1 ;
	      posi = found_suitable_kpoint(xi[i], yi[i], zi[i], fdbDistTol, &test);

        if (posi < 0)
        {
          if (f_k_new_change(0, xi[i], yi[i], zi[i], &nid[i]) != AF_OK){return(AF_ERR);}
          if (fdbInputCountInt(KPOINT, KPOINT_ID, nid[i], &posi) <= 0){return(AF_ERR);}
        }
        else
        {
          nid[i] = fdbInputGetInt(KPOINT, KPOINT_ID, posi) ;
        }
      }

      for (i=0; i<4; i++)
      {
        x0[i] = xi[i+4] ;
        y0[i] = yi[i+4] ;
        z0[i] = zi[i+4] ;
      }
      break ;

    case 5 /* cv rectangle -> cv brick */:
      klen = 20 ;

      if ((j+2) >= d_len) {break;}

      dx[0] = dx0[j] ;
      dy[0] = dy0[j] ;
      dz[0] = dz0[j] ;

      dx[1] = dx0[j+2] ;
      dy[1] = dy0[j+2] ;
      dz[1] = dz0[j+2] ;

      if (j == 0)
      {
        for (i=0; i<8; i++) /* get nodes from area */
        {
  	      npos = fdbEntKpPos(pos, i) ;
          nid[i] = fdbInputGetInt(KPOINT, KPOINT_ID, pos);
	        xi[i] = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	        yi[i] = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	        zi[i] = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;
        }
      }
      else
      {
        for (i=0; i<8; i++)
        {
         xi[i] = x0[i] ;
         yi[i] = y0[i] ;
         zi[i] = z0[i] ;
        }
      }

      for (i=8; i<12; i++) /* expand nodes */
      {
        i_pos = (i - 8) * 2 ; /* 0 2 4 6 */

        xi[i] = xi[i_pos] + dx[0] ;
        yi[i] = yi[i_pos] + dy[0] ;
        zi[i] = zi[i_pos] + dz[0] ;
      }

      for (i=12; i<20; i++) /* expand nodes */
      {
        i_pos = (i - 12)  ; /* 0 2 4 6 */

        xi[i] = xi[i_pos] + dx[1] ;
        yi[i] = yi[i_pos] + dy[1] ;
        zi[i] = zi[i_pos] + dz[1] ;
      }
      
      for (i=0; i<20; i++)
      {
        posi = -1 ;
	      posi = found_suitable_kpoint(xi[i], yi[i], zi[i], fdbDistTol, &test);

        if (posi < 0)
        {
          if (f_k_new_change(0, xi[i], yi[i], zi[i], &nid[i]) != AF_OK){return(AF_ERR);}
          if (fdbInputCountInt(KPOINT, KPOINT_ID, nid[i], &posi) <= 0){return(AF_ERR);}
        }
        else
        {
          nid[i] = fdbInputGetInt(KPOINT, KPOINT_ID, posi) ;
        }
      }

      for (i=0; i<8; i++)
      {
        x0[i] = xi[i] ;
        y0[i] = yi[i] ;
        z0[i] = zi[i] ;
      }
      break ;

    default: 
            fprintf(msgout,"[E] %s!\n",_("Invalid type of geometric entity"));
            return(AF_ERR_VAL); 
            break;
  }

  /* Common for both types: */
	id=0;
  if ((rv=f_entkp_change(id, nid, klen, vtype)) != AF_OK )
  {
    return(rv);
  }
  else
  {
	  id = fdbInputFindMaxInt(ENTITY, ENTITY_ID) ;
    f_entkp_div_change(id, div, 3);
  }
  } /**/

memFree:
  femDblFree(dx0);
  femDblFree(dy0);
  femDblFree(dz0);
  return(rv);
}

/* end of fdb_ent.c */
