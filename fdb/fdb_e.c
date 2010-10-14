/*
   File name: fdb_e.c
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

	 FEM Database - finite elements

	 $Id: fdb_e.c,v 1.13 2005/07/11 17:57:02 jirka Exp $
*/

#include "fdb_fem.h"
#include "fdb_edef.h"
#include "fdb_mdef.h"

/** Creates new finite element or changes properties of existing one
 * @param id element identifier
 * @param et type of element (user-defined etype id)
 * @param rs used real set (user-defined id)
 * @param mat material type - user defined material
 * @param set f.e. set number (0 means default)
 * @return status
 */
int f_e_new_change(long id, long et, long rs, long mat, long set)
{
	int  rv    = AF_OK ;
	int  nnew  = AF_NO ;
	long pos   = -1 ;
	long posn  = -1 ;
	long dset  = 1 ;
	long count = 0 ;
	long newid = 0 ;
	long i ;
	long e_type = 0 ; /* program's element type identifier */

	if (fdbInputCountInt(ETYPE, ETYPE_ID, et, &pos) < 1)
	{
		fprintf(msgout,"[E] %s: %li!\n",_("Undefined element type"),et);
		return(AF_ERR_VAL) ; /* suggested element type not found */
	}
	else
	{
		e_type = fdbInputGetInt(ETYPE, ETYPE_TYPE, pos);
	}

	if ((fdbElementType[e_type].reals != 0) || (fdbElementType[e_type].reals_rep != 0))
	{
		if (fdbInputCountInt(RSET, RSET_ID, rs, &pos) < 1)
		{
			return(AF_ERR_VAL) ; /* suggested real set not found */
		}
		else
		{
			if (fdbInputGetInt(RSET, RSET_TYPE, pos) != e_type)
			{
				fprintf(msgout,"[E] %s: %li!\n",_("Incompatible type of real set"),et);
				return(AF_ERR_VAL) ; /* element type michmach */
			}
		}
	}


	if (id <= 0) /* no ID specified => new */
	{
		newid = ( fdbInputFindMaxInt(ELEM, ELEM_ID)+1 );
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(ELEM, ELEM_ID, id, &pos);

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
    /*fdbInputUpdateElemStats();*/

		if ((rv = fdbInputAppendTableRow(ELEM, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(ELEM, ELEM_ID, pos, 1, &newid, AF_NO);
		fdbInputIntChangeVals(ELEM, ELEM_NODES, pos, 1, 
				&fdbElementType[e_type].nodes, AF_NO);
	}
	else /* changing of existing */
	{
		/* do nothing */
		if (fdbElementType[e_type].nodes != fdbInputGetInt(ELEM, ELEM_NODES, pos)) 
		{
			return(AF_ERR_VAL) ; /* number of nodes cannot be changed */
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

	fdbInputIntChangeVals(ELEM, ELEM_SET, pos, 1, &dset, AF_NO);
	fdbInputIntChangeVals(ELEM, ELEM_TYPE, pos, 1, &et, AF_NO);
	fdbInputIntChangeVals(ELEM, ELEM_RS, pos, 1, &rs, AF_NO);
	fdbInputIntChangeVals(ELEM, ELEM_MAT, pos, 1, &mat, AF_NO);


	if (nnew == AF_YES) /* allocation of space for nodes */
	{
		if ((rv = fdbInputAppendTableRow(ENODE, 
																		 fdbElementType[e_type].nodes, 
																		 &posn)) != AF_OK) 
		 {
			 /* cannot allocate space => delete whole element */
			 fdbInputRemoveTableRow(ELEM, pos, 1);
       fprintf(msgout,"[E] %s!\n",_("Out of memory"));
			 return(rv);
		 }

		  for (i=0; i<fdbElementType[e_type].nodes; i++)
		  {
			  fdbInputIntChangeVals(ENODE, ENODE_ELEM,  posn+i, 1, &newid, AF_NO);
			  fdbInputIntChangeVals(ENODE, ENODE_POS, posn+i, 1, &i,  AF_NO);
		  }
	}
	else
	{
		fdbInputCountInt(ENODE, ENODE_ELEM, newid, &posn) ;
    /* is it usefull? : */
		for (i=0; i<fdbElementType[e_type].nodes; i++)
		{
			fdbInputIntChangeVals(ENODE, ENODE_ELEM,  posn+i, 1, &newid, AF_NO);
			fdbInputIntChangeVals(ENODE, ENODE_POS, posn+i, 1, &i,  AF_NO);
		}
	}

  /* "FROM" position in ENODE: */
  fdbInputIntChangeVals(ELEM, ELEM_FROM, pos,1, &posn, AF_NO);

	return(rv);
}

/** Changes set of element:
 * @param id element identifier
 * @param set f.e. set number (0 means default)
 * @return status
 */
int f_e_change_set(long id, long set)
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
		if ( fdbInputCountInt(ELEM, ELEM_ID, id, &pos) < 1)
    {
      fprintf(msgout,"[E] %s!\n", _("Invalid element number"));
		  return(AF_ERR_VAL) ;
    }
    else
    {
	    fdbInputIntChangeVals(ELEM, ELEM_SET, pos, 1, &set, AF_NO);
    }
  }

	return(rv);
}

/** Tests if nodes in "nodes" field really exists
 * @param nodes list of nodes (field)
 * @param nodes_len lenght of "nodes"
 * @return AF_YES if O.K. (valid), AF_ERR_VAL if K.O. (e.g. invalid)
 */
int test_nodes_validity(long *nodes, long *node_pos, long nodes_len)
{
  int rv = AF_OK ;
  long pos = -1 ;
  int i ;

  for (i=0; i<nodes_len; i++)
  {
	  if (fdbInputCountInt(NODE, NODE_ID, nodes[i], &pos) < 1)
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

/** Sets nodes for element 
 * @param id element id
 * @param nodes list of nodes (field)
 * @param nodes_len lenght of "nodes"
 * @return status
 */
int f_en_change(long id, long *nodes, long nodes_len)
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
  if ((rv = test_nodes_validity(nodes, node_pos, nodes_len)) != AF_OK)
  {
    free(node_pos); node_pos = NULL ;
    fprintf(msgout,"[E] %s!\n",_("Invalid nodes"));
    return(rv); /* incorrect nodes */
  }

  if (id <= 0)
  {
		newid =  fdbInputFindMaxInt(ELEM, ELEM_ID) + 1 ;
  }
  else
  {
    newid = id ;
  }

	if (fdbInputCountInt(ELEM, ELEM_ID, newid, &pos) < 1)
	{
    /* new element will be created */
    if (fdbSetInputDefET(0) <= 0) { return(AF_ERR); }

		if (fdbInputCountInt(ETYPE, ETYPE_ID, fdbSetInputDefET(0), &etpos) < 1)
		{
      fprintf(msgout,"[E] %s!\n",_("Cannot set type of element"));
      free(node_pos); node_pos = NULL ;
			return(AF_ERR_VAL);
		}

		etyp = fdbInputGetInt(ETYPE, ETYPE_TYPE, etpos);

    if (fdbElementType[etyp].nodes > nodes_len)
    {
      fprintf(msgout,"[E] %s!\n",_("Bad number of nodes"));
      free(node_pos); node_pos = NULL ;
      return(AF_ERR_VAL); /* bad number of nodes added! */
    }

    if ((rv = f_e_new_change(newid,fdbSetInputDefET(0),
                                fdbSetInputDefRS(0),
                                fdbSetInputDefMAT(0),
                                fdbSetInputDefEset(0))
       ) != AF_OK)
    {
      fprintf(msgout,"[E] %s!\n",_("Cannot create new element"));
      free(node_pos); node_pos = NULL ;
      return(rv); /* something is wrong */
    }

	  fdbInputCountInt(ELEM, ELEM_ID, newid, &pos) ; /* pos is needed */
  }

  nnodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;

  /* is number of nodes still correct? */
  if (nnodes > nodes_len)
  {
    fprintf(msgout,"[E] %s!\n",_("Bad number of nodes"));
    free(node_pos); node_pos = NULL ;
    return(AF_ERR_VAL); /* bad number of nodes! */
  }

  /* now we can create the nodes (new "pos" needed): */
	if ((fdbInputCountInt(ENODE, ENODE_ELEM, newid, &posn)) != nnodes)
  {
    fprintf(msgout,"[E] %s (%li<>%li)!\n",_("Michmach in number of nodes"),
					fdbInputCountInt(ENODE, ENODE_ELEM, newid, &posn) , nnodes
				);
    free(node_pos); node_pos = NULL ;
    return(AF_ERR_VAL); /* bad number of nodes saved */
  }
  else
  {
    for (i=0; i<nnodes; i++)
    {
			fdbInputIntChangeVals(ENODE, ENODE_ID,  posn+i, 1, &nodes[i], AF_NO);
      if (node_pos != NULL)
         { fdbInputIntChangeVals(ENODE, ENODE_NPOS,posn+i, 1, &node_pos[i], AF_NO); }
    }
  }

  free(node_pos); node_pos = NULL ;
  return(rv);
}

/** Deletes element and nodes (ENODEs)
 * @param id number of node
 * @return status
 */
int f_e_delete(long id)
{
  int rv = AF_OK ;
	long pos   = -1 ;
	long posn  = -1 ;
  long nnodes = 0 ;
	long count = 0 ;
	long zero  = 0 ;
	long i ;

  /* element exists? */
	if (fdbInputCountInt(ELEM, ELEM_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no element found */
	}
  else
  {
    /* number of nodes on element (hope that's okay) */
    nnodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;
  }

	if (fdbInputTestSelect(ELEM, pos) != AF_YES) { return(AF_ERR); }

	if (fdbInputTestDep(id, ELOAD, ELOAD_ELEM) == AF_YES){return(AF_ERR);}

  fdbInputUpdateElemStats();

  /* where are nodes? */
	if ((count=fdbInputCountInt(ENODE, ENODE_ELEM, id, &posn)) < 1)
	{
		return(AF_ERR_EMP); /* no node found */
  }

	/* workaround for non-working deleting*/
	for (i=0; i<nnodes; i++) 
	{ 
		fdbInputIntChangeVals(ENODE,ENODE_ID,i+posn,1, &zero, AF_NO); 
		fdbInputIntChangeVals(ENODE,ENODE_ELEM,i+posn,1, &zero, AF_NO); 
	}

  /* deleting nodes */
  if ((rv = fdbInputRemoveTableRow(ENODE, posn, nnodes) ) != AF_OK) 
  {
    fprintf(msgout, "[E]: %s %li!\n",_("Unable to delete element"),id);
		return(AF_ERR); /* cannot delete nodes */
  }
  else
  {
	  /* removing element */
		fdbInputIntChangeVals(ELEM,ELEM_TYPE,pos,1, &zero, AF_NO);
		fdbInputIntChangeVals(ELEM,ELEM_RS,pos,1, &zero, AF_NO);
		fdbInputIntChangeVals(ELEM,ELEM_MAT,pos,1, &zero, AF_NO);

	  rv =  fdbInputRemoveTableRow(ELEM, pos, 1) ;

    /* updating _FROM info */
    rv = fdbInputRenumFromFlds(ELEM,
                          ELEM_FROM,
                          ELEM_ID,
                          ENODE, 
                          ENODE_ELEM);
  }

  fdbInputUpdateElemStats();

  return(rv);
}


/** Lists one element (and nodes)
 * @param id number of node (0 == new node with default number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 */
int f_e_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long posn  = -1 ;
  long nnodes = 0 ;
	long print_mode = 0 ;
  long ival ;
  int  i ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(ELEM, ELEM_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

  /* finding of nodes: */
  nnodes = fdbInputGetInt(ELEM, ELEM_NODES, pos) ;
	fdbInputCountInt(ENODE, ENODE_ELEM, id, &posn) ;

	if (header == AF_YES)
	{
		fprintf(fw, "\n    E    TYPE   RS    MAT     N...\n");
	}

	rv = fdbInputPrintTabLine(fw, ELEM, pos, print_mode);
  for (i=0; i<nnodes; i++) 
  { 
    ival = fdbInputGetInt(ENODE,  ENODE_ID, posn+i) ;
    fdbPrintInt(fw, ival, print_mode); 
  }
	fprintf(fw, "\n");

	return(rv) ;
}

int f_e_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id ;
	long tab_len ;
	long n_max = 0 ;
	long nodes ;
	long posn ;
  long i, j ;

  if (fdbInputTabLenAll(ETYPE) <= 0) { return(rv); }

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK) { return(rv); }

	for (i=0; i<fdbInputTabLenAll(ETYPE); i++)
	{
		if (n_max < fdbElementType[fdbInputGetInt(ETYPE,ETYPE_TYPE,i)].nodes)
		{
			n_max = fdbElementType[fdbInputGetInt(ETYPE,ETYPE_TYPE,i)].nodes ;
		}
	}

	tab_len = 5 + n_max ;

  fdbPrnBeginTable(fw, tab_len, _("Elements")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fw, 2, _("Real Set")) ;
  fdbPrnTableHeaderItem(fw, 3, _("Material")) ;
  fdbPrnTableHeaderItem(fw, 4, _("Group")) ;

  for (i=0; i<n_max; i++)
  {
  	fdbPrnTableHeaderItem(fw, 5+i, _("Node")) ;
	}

  for (i=0; i<fdbInputTabLenAll(ELEM); i++)
  {
    if (fdbInputTestSelect(ELEM, i) == AF_YES)
    {
      id    = fdbInputGetInt(ELEM,ELEM_ID,i) ;
			nodes = fdbInputGetInt(ELEM,ELEM_NODES,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemInt(fw,1,fdbInputGetInt(ELEM,ELEM_TYPE,i));
      fdbPrnTableItemInt(fw,2,fdbInputGetInt(ELEM,ELEM_RS,i));
      fdbPrnTableItemInt(fw,3,fdbInputGetInt(ELEM,ELEM_MAT,i));
      fdbPrnTableItemInt(fw,4,fdbInputGetInt(ELEM,ELEM_SET,i));

		  fdbInputCountInt(ENODE, ENODE_ELEM, id, &posn) ;

		  for (j=0; j<nodes; j++)
		  {
        fdbPrnTableItemInt(fw, 5+j, fdbInputGetInt(ENODE,ENODE_ID,posn+j));
		  }

		  if (nodes < n_max)
		  {
			  for (j=nodes; j<n_max; j++)
			  {
      	  fdbPrnTableItemStr(fw, 5+j, " ");
			  }
		  }
    }
  }
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/* end of fdb_e.c */
