/*
   File name: fdb_nops.c
   Date:      2003/12/09 14:12
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

	 FEM - node copying, mirroring etc.
*/

#include "fdb_fem.h"
#include <math.h>

extern long   fdbCoordSysType ; /* coordinate system type - from "fdb_csys.c" */

double fdbDistTol = 1e-6 ;  /* lenght to test entities as identical */

double fem_signum(double val)
{
  if ( val >= 0 ) { return( 1.0) ; }
  else            { return(-1.0) ; }

}

int compute_n_coords(long node_pos,
                     long copy_num, long mode, long dir, 
                     double dx, double dy, double dz,
                     double dx1, double dy1, double dz1,
                     double *ox, double *oy, double *oz)
{
  int rv = AF_OK ;
	double ddx, ddy, ddz ;
  double copy_val = 1 ;
  double r, phi, z ;

  if (mode == FDB_COPY)
  {
    copy_val = (double) (copy_num+1) ;

    if (fdbCoordSysType == FDB_CSYS_CART)
    {
	    ddx = copy_val*dx + ( fem_signum(dx1) * pow (copy_val*dx1, 2 ) ) ;
	    ddy = copy_val*dy + ( fem_signum(dy1) * pow (copy_val*dy1, 2 ) ) ;
	    ddz = copy_val*dz + ( fem_signum(dz1) * pow (copy_val*dz1, 2 ) ) ;

	    *ox = fdbInputGetDbl(NODE, NODE_X, node_pos) + ddx ;
	    *oy = fdbInputGetDbl(NODE, NODE_Y, node_pos) + ddy ;
	    *oz = fdbInputGetDbl(NODE, NODE_Z, node_pos) + ddz ;
    }
    else
    {
      fdbCSysGetCylCoord(
	        fdbInputGetDbl(NODE, NODE_X, node_pos),
	        fdbInputGetDbl(NODE, NODE_Y, node_pos),
	        fdbInputGetDbl(NODE, NODE_Z, node_pos),
          &r, &phi, &z);

      *ox = r ;
      *oy = (180.0*phi/FEM_PI)+(copy_val*dx) ;
      *oz = z ;
    }
  }
  else
  {
    if (dir == U_X)
    {
	    ddx = fdbInputGetDbl(NODE, NODE_X, node_pos) ;
      *ox = dx + (dx - ddx) ;
    }
    else
    {
	    *ox = fdbInputGetDbl(NODE, NODE_X, node_pos) ;
    }

    if (dir == U_Y)
    {
	    ddy = fdbInputGetDbl(NODE, NODE_Y, node_pos) ;
      *oy = dy + (dy - ddy) ;
    }
    else
    {
	    *oy = fdbInputGetDbl(NODE, NODE_Y, node_pos) ;
    }

    if (dir == U_Z)
    {
	    ddz = fdbInputGetDbl(NODE, NODE_Z, node_pos) ;
      *oz = dz + (dz - ddz) ;
    }
    else
    {
	    *oz = fdbInputGetDbl(NODE, NODE_Z, node_pos) ;
    }
  }

  return(rv);
}

/** Creates new nodes by making copies of selected ones
 * @param num number of copies
 * @param dx distance in x direction
 * @param dy distance in y direction
 * @param dz distance in z direction
 * @return (status)
 */
int f_n_gen_1d(long num, long mode, long dir, double dx, double dy, double dz, 
    double dx1, double dy1, double dz1)
{
	int  rv = AF_OK ;
	long nnodes ;
	long i,j ;
	double ox, oy, oz ;
	long count ;

	if ((nnodes = fdbInputTabLenAll(NODE)) <= 0)
	{
		fprintf(msgout, "[E] %s!\n", _("No selected nodes to be copied"));
		return(AF_ERR) ;
	}

	count = 0 ;

	for (i=0; i<num; i++)
	{

		for (j=0; j<nnodes; j++)
		{
			if (fdbInputTestSelect(NODE, j) != AF_YES) {continue;}

      compute_n_coords(j,
                     i, mode, dir, 
                     dx, dy, dz,
                     dx1, dy1, dz1,
                     &ox, &oy, &oz) ;



			if ((rv=f_n_new_change(0, ox, oy, oz)) == AF_OK)
			{
				count++ ;
			}
			else
			{
				fprintf(msgout, "[E] %s!\n", _("Copying of node failed"));
				return(rv);
			}
		}
	}


#ifdef RUN_VERBOSE
	fprintf(msgout, "[ ] %s: %li\n", _("Number of new nodes"), count) ;
#endif
	return(rv) ;
}

/** Returns position of first node at [x, y, z]
 * @param x wanted x coordinate
 * @param y wanted y coordinate
 * @param z wanted z coordinate
 * @param tol allowed tolerance of coordiantes
 * @param max position of maximum tested node (-1 to ignore)
 * @param test returns AF_YES if found, AF_NO if not
 * @return node position
 */
long found_suitable_node(double x, double y, double z, double tol, long max, int *test)
{
	long node            = -1 ;
	long node_not_so_far = -1 ;
	long i, stop ;
	double nx, ny,nz, ndist ;

	*test = AF_YES ;
  stop = fdbInputTabLenAll(NODE);
  if ((max >= 0) && (max < stop)) {stop = max;}

	for (i=0; i< stop; i++)
	{
		/* unselected nodes are ignored */
		if (fdbInputTestSelect(NODE,i) != AF_YES) {continue;}

		nx = fdbInputGetDbl(NODE, NODE_X, i) ;
		ny = fdbInputGetDbl(NODE, NODE_Y, i) ;
		nz = fdbInputGetDbl(NODE, NODE_Z, i) ;

		ndist = sqrt ( (x-nx)*(x-nx) + (y-ny)*(y-ny) + (z-nz)*(z-nz) ) ;

		if (ndist < fabs(tol))
		{
			node_not_so_far = i ;
		}

		if (ndist == 0.0)
		{
			node = i ;
			return(node) ;
		}
	}
	
	node = node_not_so_far ;

	if (node == -1)
	{
		*test = AF_NO ;
	}

	return(node) ;
}

/** Creates new elements (and nodes) by making copies of selected ones
 * @param num number of copies
 * @param dx distance in x direction
 * @param dy distance in y direction
 * @param dz distance in z direction
 * @return (status)
 */
int f_e_n_gen_1d(long num, long mode, long dir, double dx, double dy, double dz, 
    double dx1, double dy1, double dz1)
{
	int    rv = AF_OK ;
	long   nelem ;
	long   i,j,k ;
	double ox, oy, oz ;
	long   count, ncount, nncount ;
	long   enodes ;
	long  *enodelist = NULL ;
	long   npos ;
	long   e_id ;
	long   n_id ;
	int    test;
	double tol ;
	long   etype, rset,mat,egrp;
	long   ival ;

	tol = fdbDistTol ;

	if ((nelem = fdbInputTabLenAll(ELEM)) <= 0)
	{
		fprintf(msgout, "[E] %s!\n", _("No selected nodes to be copied"));
		return(AF_ERR) ;
	}

	count = 0 ;
	ncount = 0 ;
	nncount = 0 ;

	for (i=0; i<num; i++)
	{
		for (j=0; j<nelem; j++)
		{
			if (fdbInputTestSelect(ELEM, j) != AF_YES) {continue;}

			e_id = fdbInputGetInt(ELEM, ELEM_ID, j ) ;

			if ((enodes = fdbInputGetInt(ELEM, ELEM_NODES, j)) < 1)
			{
				fprintf(msgout, "[E] %s: %li!\n", _("Invalid node found"), e_id);
				return(AF_ERR_VAL);
			}

			if ((enodelist = femIntAlloc(enodes)) == NULL)
			{
				fprintf(msgout, "[E] %s!\n", _("Out of memory"));
				return(AF_ERR_MEM);
			}

      npos = fdbInputGetInt(ELEM, ELEM_FROM, j);

			for (k=0; k<enodes; k++)
			{
				enodelist[k] = fdbInputGetInt(ENODE, ENODE_ID, npos+k) ;
			}

			for (k=0; k<enodes; k++)
			{
        npos = fdbEnodePos(j, k) ;
        
        compute_n_coords(npos,
                     i, mode, dir, 
                     dx, dy, dz,
                     dx1, dy1, dz1,
                     &ox, &oy, &oz) ;

				npos =  found_suitable_node(ox, oy, oz, tol, -1, &test) ;

				if (test == AF_YES)
				{
					/* existing node */
				  n_id = fdbInputGetInt(NODE, NODE_ID, npos) ;
				}
				else
				{
					/* new node */
					if ((rv=f_n_new_change(0, ox, oy, oz)) == AF_OK)
					{
						nncount++ ;
					}
					else
					{
						fprintf(msgout, "[E] %s!\n", _("Copying of node failed"));
            femIntFree(enodelist) ;
						return(rv);
					}
					n_id = fdbInputFindMaxInt(NODE, NODE_ID) ;
				}

				enodelist[k] = n_id ;
			}


			etype = fdbInputGetInt(ELEM, ELEM_TYPE, j) ;
			rset  = fdbInputGetInt(ELEM, ELEM_RS, j) ;
			mat   = fdbInputGetInt(ELEM, ELEM_MAT, j) ;
			egrp  = fdbInputGetInt(ELEM, ELEM_SET, j) ;

			/** TODO: Postprocess positions of nodes in elements (FDB_MIRROR)  */
#if 1
			if (mode == FDB_MIRROR)
			{
				for (k=0; k<enodes; k++)
				{
					if (k >= ((long)(enodes/2))) { break ; }

					ival = enodelist[k] ;
					enodelist[k] = enodelist[enodes-k-1] ;
					enodelist[enodes-k-1] = ival ;
				}
			}
#endif
			/* End of node positions postprocessing */

			
			if ((rv=f_e_new_change(0, etype, rset, mat,  egrp)) != AF_OK)
			{
				fprintf(msgout, "[E] %s!\n", _("Copying of element failed (on whole element)"));
				femIntFree(enodelist) ;
				return(rv);
			}
			
			e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;

			if ((rv=f_en_change(e_id, enodelist, enodes))  != AF_OK)
			{
				fprintf(msgout, "[E] %s!\n", _("Copying of element failed (on nodes)"));
				femIntFree(enodelist) ;
				return(rv);
			}

			count++ ;
			femIntFree(enodelist) ;
		}
	}

#ifdef RUN_VERBOSE
	fprintf(msgout, "[ ] %s: %li\n", _("Number of new nodes"), nncount) ;
	fprintf(msgout, "[ ] %s: %li\n", _("Number of new elements"), count) ;
#endif

	return(rv) ;
}

/** Creates new displacements (on nodes) by making copies of selected ones
 * @param num number of copies
 * @param dx distance in x direction
 * @param dy distance in y direction
 * @param dz distance in z direction
 * @return (status)
 */
int f_nd_gen_1d(long num, long mode, long dir, double dx, double dy, double dz, 
    double dx1, double dy1, double dz1)
{
	int  rv = AF_OK ;
	long ndisps ;
	long n_id ;
	long i,j ;
	double ox, oy, oz ;
	long count ;
	long nodepos ;
	double tol ;
	int test ;
	long type, set;
	double val ;

	tol = fdbDistTol ;

	if ((ndisps = fdbInputTabLenAll(NDISP)) <= 0)
	{
		fprintf(msgout, "[E] %s!\n", _("No selected nodes to be copied"));
		return(AF_ERR) ;
	}

	count = 0 ;

	for (i=0; i<num; i++)
	{

		for (j=0; j<ndisps; j++)
		{
			if (fdbInputTestSelect(NDISP, j) != AF_YES) {continue;}

			n_id = fdbInputGetInt(NDISP, NDISP_NODE, j) ;

			type = fdbInputGetInt(NDISP, NDISP_TYPE, j) ;
			set = fdbInputGetInt(NDISP, NDISP_SET, j) ;
			val = fdbInputGetDbl(NDISP, NDISP_VAL, j) ;

			/* node: */
      nodepos = fdbNdispNodePos(j) ;

			n_id = fdbInputGetInt(NODE, NODE_ID, nodepos) ;
			
      compute_n_coords(nodepos,
                     i, mode, dir, 
                     dx, dy, dz,
                     dx1, dy1, dz1,
                     &ox, &oy, &oz) ;

			/* have new node? */
			nodepos = found_suitable_node(ox, oy, oz, tol, -1, &test) ;

			if (test != AF_YES)
			{
				fprintf (msgout, "[W] %s \"%li\"!\n", _("No node to copy load from"), n_id );
				continue ;
			}

			n_id = fdbInputGetInt(NODE, NODE_ID, nodepos) ;
			
			if ((rv=f_nd_new_change(0, n_id, type, set, val)) == AF_OK)
			{
				count++ ;
			}
			else
			{
				fprintf(msgout, "[E] %s!\n", _("Copying of displacement failed"));
				return(rv);
			}
		}
	}

#ifdef RUN_VERBOSE
	fprintf(msgout, "[ ] %s: %li\n", _("Number of new displacements"), count) ;
#endif

	return(rv) ;
}

/** Creates new loads (on nodes) by making copies of selected ones
 * @param num number of copies
 * @param dx distance in x direction
 * @param dy distance in y direction
 * @param dz distance in z direction
 * @return (status)
 */
int f_nl_gen_1d(long num, long mode, long dir, double dx, double dy, double dz, 
    double dx1, double dy1, double dz1)
{
	int  rv = AF_OK ;
	long nldisps ;
	long n_id ;
	long i,j ;
	double ox, oy, oz ;
	long count ;
	long nodepos ;
	double tol ;
	int test ;
	long type, set;
	double val ;

	tol = fdbDistTol ;

	if ((nldisps = fdbInputTabLenAll(NLOAD)) <= 0)
	{
		fprintf(msgout, "[E] %s!\n", _("No selected nodes to be copied"));
		return(AF_ERR) ;
	}

	count = 0 ;

	for (i=0; i<num; i++)
	{

		for (j=0; j<nldisps; j++)
		{
			if (fdbInputTestSelect(NLOAD, j) != AF_YES) {continue;}

			n_id = fdbInputGetInt(NLOAD, NLOAD_NODE, j) ;

			type = fdbInputGetInt(NLOAD, NLOAD_TYPE, j) ;
			set = fdbInputGetInt(NLOAD, NLOAD_SET, j) ;
			val = fdbInputGetDbl(NLOAD, NLOAD_VAL, j) ;

			/* node: */
      nodepos = fdbNloadNodePos(j) ;

			n_id = fdbInputGetInt(NODE, NODE_ID, nodepos) ;
			
      compute_n_coords(nodepos,
                     i, mode, dir, 
                     dx, dy, dz,
                     dx1, dy1, dz1,
                     &ox, &oy, &oz) ;

			/* have new node? */
			nodepos = found_suitable_node(ox, oy, oz, tol, -1, &test) ;

			if (test != AF_YES)
			{
				fprintf (msgout, "[W] %s \"%li\"!\n", _("No node to copy load from"), n_id );
				continue ;
			}

			n_id = fdbInputGetInt(NODE, NODE_ID, nodepos) ;
			
			if ((rv=f_nl_new_change(0, n_id, type, set, val)) == AF_OK)
			{
				count++ ;
			}
			else
			{
				fprintf(msgout, "[E] %s!\n", _("Copying of load failed"));
				return(rv);
			}
		}
	}

#ifdef RUN_VERBOSE
	fprintf(msgout, "[ ] %s: %li\n", _("Number of new loads"), count) ;
#endif

	return(rv) ;
}

/** Moves nodes 
 * @param nPos node position
 * @param dx x-move
 * @param dy y-move
 * @param dz z-move
 * @return status
 * */
int n_move_coord(long nPos, double dx, double dy, double dz)
{
  int rv = AF_OK ;

  if (fdbInputTestSelect(NODE,nPos) != AF_YES) {return(AF_OK);}

	fdbInputDblChangeVals(NODE, NODE_X, nPos, 1, &dx, AF_YES);
	fdbInputDblChangeVals(NODE, NODE_Y, nPos, 1, &dy, AF_YES);
	fdbInputDblChangeVals(NODE, NODE_Z, nPos, 1, &dz, AF_YES);

  return(rv);
}

/* end of fdb_nops.c */
