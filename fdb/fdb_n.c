/*
   File name: fdb_h.c
   Date:      2003/08/09 22:47
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

	 FEM - node handling
*/

#include "fdb_fem.h"

extern double fdbDistTol ; /* from fdb_nops.c*/

/** Creates new node or changes existing
 * @param id number of node (0 == new node with defaukt number)
 * @param x x-coordinate
 * @param y y-coordinate
 * @param z z-coordinate
 * @return status
 */
int f_n_new_change(long id, double x, double y, double z)
{
	int  rv    = AF_OK ;
	int  nnew  = AF_NO ;
	long pos   = -1 ;
	long count = 0 ;
	long newid = 0 ;
  double n_x, n_y, n_z ;

	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(NODE, NODE_ID) + 1 ;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(NODE, NODE_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			newid = fdbInputGetInt(NODE, NODE_ID, pos) ;
			nnew = AF_NO ;
		}
		else /* new data */
		{
			newid = id ; 
			nnew = AF_YES ;
		}
	}

	/*printf("count is %li\n", count);*/

	if (nnew == AF_YES) /* adding of new line */
	{
    /*fdbInputUpdateNodeStats();*/

		if ((rv = fdbInputAppendTableRow(NODE, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(NODE, NODE_ID, pos, 1, &newid, AF_NO);
	}
	else /* changing of existing */
	{
		/* do nothing */
	}

	/* common for all: */
  fdbCSysGetXYZ(x, y, z, &n_x, &n_y, &n_z);

	fdbInputDblChangeVals(NODE, NODE_X, pos, 1, &n_x, AF_NO);
	fdbInputDblChangeVals(NODE, NODE_Y, pos, 1, &n_y, AF_NO);
	fdbInputDblChangeVals(NODE, NODE_Z, pos, 1, &n_z, AF_NO);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"n(%li) = [%e, %e, %e] ##\n",
					fdbInputGetInt(NODE, NODE_ID, pos ),
					fdbInputGetDbl(NODE, NODE_X, pos ),
					fdbInputGetDbl(NODE, NODE_Y, pos ),
					fdbInputGetDbl(NODE, NODE_Z, pos )
					);
#endif

	return(rv) ;
}

/** Deletes node
 * @param id number of node
 * @return status
 */
int f_n_delete(long id)
{
	int  rv = AF_OK ;
	long pos   = -1 ;

  /* tests: */
	if (fdbInputCountInt(NODE, NODE_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no node found */
	}

	if (fdbInputTestSelect(NODE,pos) != AF_YES) 
	{
		return(AF_OK); /* ignore inselected node */
	}
	
	if (fdbInputTestDep(id, ENODE, ENODE_ID) == AF_YES) { return(AF_ERR); }
	if (fdbInputTestDep(id, NLOAD, NLOAD_NODE) == AF_YES) { return(AF_ERR); }
	if (fdbInputTestDep(id, NDISP, NDISP_NODE) == AF_YES) { return(AF_ERR); }

	
  fdbInputUpdateNodeStats();

	/* removing */
	rv =  fdbInputRemoveTableRow(NODE, pos, 1) ;

	fdbInputSyncStats();
	return(rv);
}

/** Lists one node
 * @param id number of node (0 == new node with defaukt number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 */
int f_n_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long print_mode = 0 ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(NODE, NODE_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if ( fdbTestSelect(&InputTab[NODE], &inputData.intfld, pos) != AF_YES) 
	{
		return(AF_OK); /* not selected */
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n          N          X            Y            Z\n");
	}

	rv = fdbInputPrintTabLine(fw, NODE, pos, print_mode);
	fprintf(fw, "\n");

	return(rv) ;
}

int f_n_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id ;
  long i ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 4, _("Nodes")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("X")) ;
  fdbPrnTableHeaderItem(fw, 2, _("Y")) ;
  fdbPrnTableHeaderItem(fw, 3, _("Z")) ;

  for (i=0; i<fdbInputTabLenAll(NODE); i++)
  {
    if (fdbInputTestSelect(NODE, i) == AF_YES)
    {
      id = fdbInputGetInt(NODE,NODE_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemDbl(fw,1,fdbInputGetDbl(NODE,NODE_X,i));
      fdbPrnTableItemDbl(fw,2,fdbInputGetDbl(NODE,NODE_Y,i));
      fdbPrnTableItemDbl(fw,3,fdbInputGetDbl(NODE,NODE_Z,i));
    }
  }
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}


/** Make 2 nodes from one and link them with separate nodes
 * Note: it assimes one unique KP id per entity!
 * @param kPos position of node
 * @return status
 */
int f_n_split_through_e_pos(long kPos)
{
  int rv = AF_OK ;
  long i ;
  long id = -1 ;
  long pos = -1 ;
  long num = 0 ;
  long newid, pos1 ;
  double x, y, z ;

  id = fdbInputGetInt(NODE, NODE_ID, kPos);

	if ( fdbTestSelect(&InputTab[NODE], &inputData.intfld, kPos) != AF_YES) 
	{
		return(AF_OK); /* not selected - nothing to do */
	}

  /* number of uses of the KP: */
	if ((num=fdbInputCountInt(ENODE, ENODE_ID, id, &pos)) < 2)
  {
    return(AF_OK); /* nothing to do here */
  }
  else
  {
    /* have to make some elements: */
    x = fdbInputGetDbl(NODE, NODE_X, kPos);
    y = fdbInputGetDbl(NODE, NODE_Y, kPos);
    z = fdbInputGetDbl(NODE, NODE_Z, kPos);

    for (i=1; i<num; i++)
    {
      newid = 0 ;
      if ((rv=f_n_new_change(0, x, y, z)) != AF_OK)
      {
        return(rv);
      }

      newid = fdbInputFindMaxInt(NODE, NODE_ID) ;

      if (newid <= 0) { return(AF_ERR); }

      pos1 = fdbInputFindNextInt(ENODE, ENODE_ID, pos, id); ;
      if (pos1 > pos)
      {
        fdbInputPutInt(ENODE,ENODE_ID,pos1, newid);
      }
      pos = pos1 ;
    }
  }

  fdbInputUpdateNodeStats();
  
  return(rv);
}

/** Make 2 nodes from one and link them with separate elements
 * Note: it assumes one unique node id per elements!
 * @param is number (ID) of the node
 * @return status
 */
int f_n_split_through_e_id(long id)
{
  long kPos ;
  if (fdbInputCountInt(NODE, NODE_ID, id, &kPos) > 0)
  {
    return( f_n_split_through_e_pos(kPos) );
  }
  else
  {
    return(AF_ERR_VAL);
  }
  return(AF_ERR);
}

/** Join (merge) nodes
 * @return status
 */
int f_n_join_all(void)
{
  int rv = AF_OK ;
  long i, j, k ;
  long pos = -1 ;
  long id ,id_r, num ;
  double x, y, z ;
  double nx, ny, nz ;
  double ndist ;
  long count = 0 ;

  for (i=0; i<fdbInputTabLenAll(NODE); i++)
  {
	  if (fdbInputTestSelect(NODE,i) != AF_YES) {continue;} 

    id  = fdbInputGetInt(NODE, NODE_ID, i);

    x = fdbInputGetDbl(NODE, NODE_X, i);
    y = fdbInputGetDbl(NODE, NODE_Y, i);
    z = fdbInputGetDbl(NODE, NODE_Z, i);

    for (j=0; j<fdbInputTabLenAll(NODE); j++)
    {
/*printf("nmerge: %li,%li\n",i,j);*/
      if  (i == j) {continue;}
	    if (fdbInputTestSelect(NODE,j) != AF_YES) {continue;} 

      id_r  = fdbInputGetInt(NODE, NODE_ID, j);
    
		  nx = fdbInputGetDbl(NODE, NODE_X, j) ;
		  ny = fdbInputGetDbl(NODE, NODE_Y, j) ;
		  nz = fdbInputGetDbl(NODE, NODE_Z, j) ;

		  ndist = sqrt ( (x-nx)*(x-nx) + (y-ny)*(y-ny) + (z-nz)*(z-nz) ) ;
      
      if (ndist >= fabs(fdbDistTol)) { continue ; }

      pos = 0 ;

	    if ((num=fdbInputCountInt(NDISP, NDISP_NODE, id_r, &pos)) > 0)
      {
        for (k=0; k<num; k++)
        {
          fdbInputPutInt(NDISP, NDISP_NODE, pos, id);
          fdbInputPutInt(NDISP, NDISP_NPOS, pos, i);

	        if ((fdbInputCountInt(NDISP, NDISP_NODE, id_r, &pos)) < 1)
          {
            break ;
          }
        }
      }

      pos = 0 ;

	    if ((num=fdbInputCountInt(NLOAD, NLOAD_NODE, id_r, &pos)) > 0)
      {
        for (k=0; k<num; k++)
        {
          fdbInputPutInt(NLOAD, NLOAD_NODE, pos, id);
          fdbInputPutInt(NLOAD, NLOAD_NPOS, pos, i);

	        if ((fdbInputCountInt(NLOAD, NLOAD_NODE, id_r, &pos)) < 1)
          {
            break ;
          }
        }
      }

      pos = 0 ;

	    if ((num=fdbInputCountInt(ENODE, ENODE_ID, id_r, &pos)) > 0)
      {
        for (k=0; k<num; k++)
        {
          fdbInputPutInt(ENODE, ENODE_ID, pos, id);
          fdbInputPutInt(ENODE, ENODE_NPOS, pos, i);

	        if ((fdbInputCountInt(ENODE, ENODE_ID, id_r, &pos)) < 1)
          {
            break ;
          }
        }
      }

      if (f_n_delete(id_r) == AF_OK)  /* delete node "j" */
      {
        count++;
        j-- ;
      }
      else
      {
#ifdef RUN_VERBOSE
        fprintf(msgout,"[W] %s: %li!\n",_("Node can NOT be deleted"),id_r);
#endif
      }
      fdbInputUpdateNodeStats(); /* fix dependencies */
    }
  }

  fprintf(msgout,"[ ]    %s: %li.\n",_("Number of removed nodes"), count);
  return(rv);
}

/** Computes distance between two nodes
 * @param k1 first node
 * @param k2 second node
 * @param dx,dy,dz distances in x,y,z directions (results)
 * @return status
 */
int f_n_dist(long k1, long k2, double *dx, double *dy, double *dz)
{
  long pos1, pos2;

  *dx = 0 ;
  *dy = 0 ;
  *dz = 0 ;

  if (fdbInputCountInt(NODE, NODE_ID, k1, &pos1) < 1) {return(AF_ERR_EMP);}
  if (fdbInputCountInt(NODE, NODE_ID, k2, &pos2) < 1) {return(AF_ERR_EMP);}

  *dx = fdbInputGetDbl(NODE, NODE_X, pos2) - fdbInputGetDbl(NODE, NODE_X, pos1) ;
  *dy = fdbInputGetDbl(NODE, NODE_Y, pos2) - fdbInputGetDbl(NODE, NODE_Y, pos1) ;
  *dz = fdbInputGetDbl(NODE, NODE_Z, pos2) - fdbInputGetDbl(NODE, NODE_Z, pos1) ;
  
  return(AF_OK);
}



/* end of fdb_n.c */
