/*
   File name: fdb_mesh.c
   Date:      2004/04/25 18:55
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

	 FEM Database - finite element mesh creation
*/

#include "fdb_geom.h"

/* from fdb_nps.c */
extern double fdbDistTol ;
extern long found_suitable_node(double x, double y, double z, double tol, long max, int *test);

long fdbMeshCheckAllNodes = AF_NO ;

/** Meshes LINE entity (must be two-node) 
 * @param entPos entity index
 * @return status
 */
int fdbMeshEnt001(long entPos)
{
  int    rv = AF_OK ;
  long  *nodes = NULL ;
  long   entnum ;
  long   len, div_num, npos, posn, n_id, e_id ;
	int    test ;
  long   et, rs, mat, set ;
  long   enodelist[2] ;
  long   i ;
  double x,y,z ;
  double x1,y1,z1 ;
  double x2,y2,z2 ;

  entnum = fdbInputGetInt(ENTITY, ENTITY_ID, entPos) ;

  if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, entnum, &posn) >= 1)
  {
    div_num = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn) ;
  }
  else
  {
    return(AF_ERR_EMP);
  }

  if (div_num <= 1) { len = 2 ;           }
  else              { len = div_num + 1 ; }

  if ((nodes=femIntAlloc(len)) == NULL)
  {
    return(AF_ERR_MEM) ;
  }

  npos = fdbEntKpPos(entPos, 0) ;
	x1 = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	y1 = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	z1 = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;

  npos = fdbEntKpPos(entPos, 1) ;
	x2 = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	y2 = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	z2 = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;

  for (i=0; i<len; i++)
  {
    /* nodes: */
    x = x1 + (((double)i+0.0)/((double)len-1.0))*(x2 - x1) ;
    y = y1 + (((double)i+0.0)/((double)len-1.0))*(y2 - y1) ;
    z = z1 + (((double)i+0.0)/((double)len-1.0))*(z2 - z1) ;

		if ((i==0)||(i==(len-1))||(fdbMeshCheckAllNodes==AF_YES))
		{
			npos = found_suitable_node(x, y, z, fdbDistTol, -1, &test);
		}
		else
		{
			test = AF_NO ;
		}

    if (test == AF_YES)
		{
			/* existing node */
			n_id = fdbInputGetInt(NODE, NODE_ID, npos) ;
		}
		else
		{
			/* new node */
			if ((rv=f_n_new_change(0, x, y, z)) != AF_OK)
			{
				fprintf(msgout, "[E] %s!\n", _("Meshing failed"));
        femIntFree(nodes) ;
				return(rv);
			}
			n_id = fdbInputFindMaxInt(NODE, NODE_ID) ;
		}

		nodes[i] = n_id ;
  }    


  et  = fdbInputGetInt(ENTITY, ENTITY_ETYPE, entPos) ;
  rs  = fdbInputGetInt(ENTITY, ENTITY_RS,    entPos) ;
  mat = fdbInputGetInt(ENTITY, ENTITY_MAT,   entPos) ;
  set = fdbInputGetInt(ENTITY, ENTITY_SET,   entPos) ;

  /* elements: */
  for (i=0; i<(len-1); i++)
  {
    if ((rv=f_e_new_change(0, et, rs, mat, set)) != AF_OK)
		{
			fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			femIntFree(nodes) ;
			return(rv);
		}

		e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;

    enodelist[0] = nodes[i] ;
    enodelist[1] = nodes[i+1] ;
    
    if ((rv=f_en_change(e_id, enodelist, 2))  != AF_OK)
		{
			fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			femIntFree(nodes) ;
			return(rv);
		}
  }

  femIntFree(nodes) ;

  return(rv);
}


void fdbMeshGeomFuncCvRect(double *xr, double *yr, double *zr, double xi, double yi, double zi, double *x, double *y, double *z)
{
  int i ;
  double xv[21] = {0,-1, 0, 1, 1, 1,-1,-1,-1,-1, 1, 1,-1,-1, 0, 1, 1, 1, 0,-1,-1};
  double yv[21] = {0,-1,-1,-1, 0, 1, 1, 1, 0,-1,-1, 1, 1,-1,-1,-1, 0, 1, 1, 1, 0};
  double zv[21] = {0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	long p_a_nodes[8]={1,3,5,7,13,15,17,19};
	long p_b_nodes[4]={2,6,14,18};
	long p_c_nodes[4]={4,8,16,20};

  *x = 0.0 ;
  *y = 0.0 ;
  *z = 0.0 ;

  for (i=0; i<4; i++)
  {
    *x += 0.25*(1.0+xv[p_a_nodes[i]]*xi)*(1.0+yv[p_a_nodes[i]]*yi)*(1.0+zv[p_a_nodes[i]]*zi) *(xv[p_a_nodes[i]]*xi+yv[p_a_nodes[i]]*yi+zv[p_a_nodes[i]]*zi-1.0) * xr[p_a_nodes[i]-1]  ;
    *y += 0.25*(1.0+xv[p_a_nodes[i]]*xi)*(1.0+yv[p_a_nodes[i]]*yi)*(1.0+zv[p_a_nodes[i]]*zi) *(xv[p_a_nodes[i]]*xi+yv[p_a_nodes[i]]*yi+zv[p_a_nodes[i]]*zi-1.0) * yr[p_a_nodes[i]-1]  ;
    *z += 0.25*(1.0+xv[p_a_nodes[i]]*xi)*(1.0+yv[p_a_nodes[i]]*yi)*(1.0+zv[p_a_nodes[i]]*zi) *(xv[p_a_nodes[i]]*xi+yv[p_a_nodes[i]]*yi+zv[p_a_nodes[i]]*zi-1.0) * zr[p_a_nodes[i]-1]  ;
  }

	for (i=0; i<2; i++)
  {
    *x += 0.5*(1.0-xi*xi)*(1.0+yv[p_b_nodes[i]]*yi)*(1.0+zv[p_b_nodes[i]]*zi) * xr[p_b_nodes[i]-1]  ;
    *y += 0.5*(1.0-xi*xi)*(1.0+yv[p_b_nodes[i]]*yi)*(1.0+zv[p_b_nodes[i]]*zi) * yr[p_b_nodes[i]-1]  ;
    *z += 0.5*(1.0-xi*xi)*(1.0+yv[p_b_nodes[i]]*yi)*(1.0+zv[p_b_nodes[i]]*zi) * zr[p_b_nodes[i]-1]  ;
  }

	for (i=0; i<2; i++)
  {
    *x += 0.5*(1.0+xv[p_c_nodes[i]]*xi)*(1.0-yi*yi)*(1.0+zv[p_c_nodes[i]]*zi) * xr[p_c_nodes[i]-1]  ;
    *y += 0.5*(1.0+xv[p_c_nodes[i]]*xi)*(1.0-yi*yi)*(1.0+zv[p_c_nodes[i]]*zi) * yr[p_c_nodes[i]-1]  ;
    *z += 0.5*(1.0+xv[p_c_nodes[i]]*xi)*(1.0-yi*yi)*(1.0+zv[p_c_nodes[i]]*zi) * zr[p_c_nodes[i]-1]  ;
  }
}

void fdbMeshGeomFuncRect(double *xr, double *yr, double *zr, double xi, double yi, double zi, double *x, double *y, double *z)
{
  int i ;
  double xv[4] = {-1,1,1,-1};
  double yv[4] = {-1,-1,1,1};
  double zv[4] = {0,0,0,0};

  *x = 0.0 ;
  *y = 0.0 ;
  *z = 0.0 ;

  for (i=0; i<4; i++)
  {
    *x += 0.25*(1 + xv[i]*xi)*(1 + yv[i]*yi)*(1 + zv[i]*zi) * xr[i]  ;
    *y += 0.25*(1 + xv[i]*xi)*(1 + yv[i]*yi)*(1 + zv[i]*zi) * yr[i]  ;
    *z += 0.25*(1 + xv[i]*xi)*(1 + yv[i]*yi)*(1 + zv[i]*zi) * zr[i]  ;
  }
}

/** Meshes AREA entity (must be four-node or eight-node) 
 * @param entPos entity index
 * @return status
 */
int fdbMeshEnt002(long entPos)
{
  int    rv = AF_OK ;
  long  *nodes = NULL ;
  long   entnum ;
  long   ilen,jlen, nlen, npos, posn, n_id, e_id ;
	int    test ;
  long   et, rs, mat, set ;
  long   enodelist[4] ;
  long   i, j ;
  long   sum ;
  int    etype_pos ;
  int    etype = 2 ;
  double x,y,z ;
  double xi[8] ;
  double yi[8] ;
  double zi[8] ;

  entnum = fdbInputGetInt(ENTITY, ENTITY_ID, entPos) ;

  if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, entnum, &posn) >= 2)
  {
    ilen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn) ;
    jlen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+1) ;
  }
  else
  {
    return(AF_ERR_EMP);
  }

  if ((ilen < 1) || (jlen < 1))
  { 
    return(AF_ERR_VAL) ;
  }
  else
  { 
    nlen = (ilen+1)*(jlen+1) ;
  }

  if ((nodes=femIntAlloc(nlen+1)) == NULL)
  {
    return(AF_ERR_MEM) ;
  }


  for (i=0; i<8; i++)
  {
  	npos = fdbEntKpPos(entPos, i) ;
	  xi[i] = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	  yi[i] = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	  zi[i] = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;
  }

  sum = 0 ;

  for (j=0; j<=jlen; j++)
  {
    for (i=0; i<=ilen; i++)
    {
      /* nodes: */
      fdbMeshGeomFuncRect(
          xi, yi, zi, 
          (2.0*((((double)i+0.0)/(double)ilen))) - 1.0, 
          (2.0*((((double)j+0.0)/(double)jlen))) - 1.0, 
          0, 
          &x, &y, &z) ;

		  if ((i==0)||(i==ilen)||(j==0)||(j==jlen)||(fdbMeshCheckAllNodes==AF_YES))
		  {
			  npos = found_suitable_node(x, y, z, fdbDistTol, -1, &test);
		  }
		  else
		  {
			  test = AF_NO;
		  }

      if (test == AF_YES)
		  {
			  /* existing node */
			  n_id = fdbInputGetInt(NODE, NODE_ID, npos) ;
		  }
		  else
		  {
			  /* new node */
			  if ((rv=f_n_new_change(0, x, y, z)) != AF_OK)
			  {
				  fprintf(msgout, "[E] %s!\n", _("Meshing failed"));
          femIntFree(nodes) ;
				  return(rv);
			  }
			  n_id = fdbInputFindMaxInt(NODE, NODE_ID) ;
		  }

		  nodes[sum] = n_id ;
      sum++ ;
    }    
  }

  et  = fdbInputGetInt(ENTITY, ENTITY_ETYPE, entPos) ;
  rs  = fdbInputGetInt(ENTITY, ENTITY_RS,    entPos) ;
  mat = fdbInputGetInt(ENTITY, ENTITY_MAT,   entPos) ;
  set = fdbInputGetInt(ENTITY, ENTITY_SET,   entPos) ;

  etype_pos = fdbInputFindNextInt(ETYPE, ETYPE_ID, 0,  et) ;
  if (etype_pos < 0)
  { 
	  fprintf(msgout, "[E] %s!\n", _("Bad element type"));
	  femIntFree(nodes) ;
	  return(rv);
  }
  etype     = fdbInputGetInt(ETYPE, ETYPE_TYPE, etype_pos) ;

  /* elements: FOUR-NODES ELEMENTS ONLY */
  if ((etype != 11) && (etype != 18))
  {
  for (i=0; i<(ilen); i++)
  {
    for (j=0; j<(jlen); j++)
    {
      if ((rv=f_e_new_change(0, et, rs, mat, set)) != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
  
		  e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;
  
      enodelist[0] = nodes[i + j*(ilen+1) + 0] ;
      enodelist[1] = nodes[i + j*(ilen+1) + 1] ;
      enodelist[2] = nodes[i + (j+1)*(ilen+1) + 1] ;
      enodelist[3] = nodes[i + (j+1)*(ilen+1) + 0] ;
      
      if ((rv=f_en_change(e_id, enodelist, 4))  != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
    }
  }
  } else {
  /* elements: THREE-NODE ELEMENTS */
  for (i=0; i<(ilen); i++)
  {
    for (j=0; j<(jlen); j++)
    {
      if ((rv=f_e_new_change(0, et, rs, mat, set)) != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
  
		  e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;
  
      enodelist[0] = nodes[i + j*(ilen+1) + 0] ;
      enodelist[1] = nodes[i + j*(ilen+1) + 1] ;
      enodelist[2] = nodes[i + (j+1)*(ilen+1) + 1] ;
      
      if ((rv=f_en_change(e_id, enodelist, 3))  != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
      /* element 2 */
      if ((rv=f_e_new_change(0, et, rs, mat, set)) != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
  
		  e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;
  
      enodelist[0] = nodes[i + j*(ilen+1) + 0] ;
      enodelist[1] = nodes[i + (j+1)*(ilen+1) + 1] ;
      enodelist[2] = nodes[i + (j+1)*(ilen+1) + 0] ;
      
      if ((rv=f_en_change(e_id, enodelist, 3))  != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
    }
  }
  }

  femIntFree(nodes) ;
  return(rv);
}

void fdbMeshGeomFuncBrick(double *xr, double *yr, double *zr, double xi, double yi, double zi, double *x, double *y, double *z)
{
  int i ;
  double xv[8] = {-1,1,1,-1,-1,1,1,-1};
  double yv[8] = {-1,-1,1,1,-1,-1,1,1};
  double zv[8] = {-1,-1,-1,-1,1,1,1,1};

  *x = 0.0 ;
  *y = 0.0 ;
  *z = 0.0 ;

  for (i=0; i<8; i++)
  {
    *x += 0.125*(1 + xv[i]*xi)*(1 + yv[i]*yi)*(1 + zv[i]*zi) * xr[i]  ;
    *y += 0.125*(1 + xv[i]*xi)*(1 + yv[i]*yi)*(1 + zv[i]*zi) * yr[i]  ;
    *z += 0.125*(1 + xv[i]*xi)*(1 + yv[i]*yi)*(1 + zv[i]*zi) * zr[i]  ;
  }
}

/** New version of brick mesher: */
/** Meshes BRICK entity (must be eight-node) 
 * @param entPos entity index
 * @return status
 */
int fdbMeshEnt003(long entPos)
{
  int    rv = AF_OK ;
  long  *nodes = NULL ;
  long   entnum ;
  long   ilen,jlen,klen, nlen, npos, posn ;
	int    test ;
  long   et, rs, mat, set ;
  long   i, j, k, kk ;
  long   sum ;
  double x,y,z ;
  double xi[8] ;
  double yi[8] ;
  double zi[8] ;
  long  *enode_elem = NULL ;
  long  *enode_pos  = NULL ;
  long  *enode_npos = NULL ;
  long  *enode_id   = NULL ;
  long  *e_sel = NULL ;
  long  *e_type = NULL ;
  long  *e_rset = NULL ;
  long  *e_mat  = NULL ;
  long  *e_dset = NULL ;
  long  *e_nnum = NULL ;
  long  *e_nid  = NULL ;
  long  *e_from = NULL ;
  long   *n_nid = NULL ;
  long   *n_sel = NULL ;
  double *n_x   = NULL ;
  double *n_y   = NULL ;
  double *n_z   = NULL ;
  long    elen = 0 ;
  long    enlen = 0 ;
	long    n_tot = 0 ;
	long    e_tot = 0 ;
	long    en_tot = 0 ;
	long    startnpos = 0 ;
	long    actnpos = 0 ;
	long    maxnid ;
  long    startepos = 0 ;
  long    startenpos = 0 ;
	long    actepos = 0 ;
	long    actenpos = 0 ;
	long    maxeid ;


  entnum = fdbInputGetInt(ENTITY, ENTITY_ID, entPos) ;

  if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, entnum, &posn) >= 3)
  {
    ilen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn) ;
    jlen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+1) ; /* was: +8 */
    klen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+2) ; /* was: +1 */
  }
  else
  {
    return(AF_ERR_EMP);
  }

  if ((ilen < 1) || (jlen < 1) || (klen < 1))
  { 
    return(AF_ERR_VAL) ;
  }
  else
  { 
    nlen = (ilen+1)*(jlen+1)*(klen+1) ;
  }

  if ((nodes=femIntAlloc(nlen+1)) == NULL)
  {
    return(AF_ERR_MEM) ;
  }

  elen = ilen*jlen*klen ; /* number of new elements */
  enlen = nlen*8 ;        /* number of new element nodes */

  /* allocations: */
  if ((enode_elem = femIntAlloc(enlen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((enode_pos = femIntAlloc(enlen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((enode_npos = femIntAlloc(enlen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((enode_id = femIntAlloc(enlen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}

  if ((e_type = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_rset = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_mat = femIntAlloc(elen))  == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_dset = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_nnum = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_nid = femIntAlloc(elen))  == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_from = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_sel = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}

  if ((n_nid = femIntAlloc(nlen))  == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((n_sel = femIntAlloc(nlen))  == NULL) {rv=AF_ERR_MEM; goto memFree;}

  if ((n_x = femDblAlloc(nlen))    == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((n_y = femDblAlloc(nlen))    == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((n_z = femDblAlloc(nlen))    == NULL) {rv=AF_ERR_MEM; goto memFree;}


	/* parameters of geometric entity: */
  for (i=0; i<8; i++)
  {
  	npos = fdbEntKpPos(entPos, i) ;
	  xi[i] = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	  yi[i] = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	  zi[i] = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;
  }

  sum = 0 ;

	startnpos = fdbInputTabLenAll(NODE)  ;
	if (startnpos < 0)
	{
		startnpos = 0 ;
		maxnid    = 0 ;
	}
	else
	{
		maxnid = fdbInputFindMaxInt (NODE, NODE_ID) ;
	}
	/*actnpos = startnpos ;*/
	actnpos = 0 ;

  /* this makes things extremelly slow (too much searches and
   * verifications) */
  for (k=0; k<=klen; k++)
  {
    for (j=0; j<=jlen; j++)
    {
      for (i=0; i<=ilen; i++)
      {
        /* nodes: */
        fdbMeshGeomFuncBrick(
          xi, yi, zi, 
          (2.0*((((double)i+0.0)/(double)ilen))) - 1.0, 
          (2.0*((((double)j+0.0)/(double)jlen))) - 1.0, 
          (2.0*((((double)k+0.0)/(double)klen))) - 1.0, 
          &x, &y, &z) ;

				if ((i==0)||(j==0)||(k==0)||(i==ilen)||(j==jlen)||(k==klen)||(fdbMeshCheckAllNodes==AF_YES))
				{
					npos = found_suitable_node(x, y, z, fdbDistTol, startnpos+1, &test);
				}
				else
				{
					test = AF_NO ;
				}

        if (test != AF_YES)
		    {
					maxnid++ ;
					n_nid[actnpos] = maxnid ;
					n_sel[actnpos] = AF_YES ;
					n_x[actnpos] = x ;
					n_y[actnpos] = y ;
					n_z[actnpos] = z ;
          npos = actnpos+startnpos ;

					actnpos++ ;
					n_tot++ ;
		    }

		    nodes[sum] = npos ;
        sum++ ;
      }
    }    
  }

	if (n_tot > 0)
	{
    fdbInputAppendTableRow(NODE, n_tot, &startnpos) ;
		fdbInputIntChangeValsFast( NODE, NODE_ID, startnpos, n_tot, n_nid, AF_NO);
		fdbInputIntChangeValsFast( NODE, NODE_SEL, startnpos, n_tot, n_sel, AF_NO);
		fdbInputDblChangeValsFast( NODE, NODE_X , startnpos, n_tot, n_x, AF_NO);
		fdbInputDblChangeValsFast( NODE, NODE_Y , startnpos, n_tot, n_y, AF_NO);
		fdbInputDblChangeValsFast( NODE, NODE_Z , startnpos, n_tot, n_z, AF_NO);
	}

  et  = fdbInputGetInt(ENTITY, ENTITY_ETYPE, entPos) ;
  rs  = fdbInputGetInt(ENTITY, ENTITY_RS,    entPos) ;
  mat = fdbInputGetInt(ENTITY, ENTITY_MAT,   entPos) ;
  set = fdbInputGetInt(ENTITY, ENTITY_SET,   entPos) ;

  /* computing elements: */
  startepos = fdbInputTabLenAll(ELEM)  ;
	if (startepos < 0)
	{
		startepos = 0 ;
		maxeid    = 0 ;
	}
	else
	{
	  maxeid = fdbInputFindMaxInt (ELEM, ELEM_ID) ;
	}
	actepos = 0 ;

  startenpos = fdbInputTabLenAll(ENODE)  ;
	if (startenpos < 0)
	{
		startenpos = 0 ;
	}
	actenpos = 0 ;

  for (k=0; k<(klen); k++)
  {
    for (j=0; j<(jlen); j++)
    {
  		for (i=0; i<(ilen); i++)
      {
				maxeid++ ;
				e_nid[actepos] = maxeid ;
				e_type[actepos] = et ;
				e_rset[actepos] = rs ;
				e_mat[actepos] = mat ;
				e_dset[actepos] = set ;
				e_nnum[actepos] = 8 ;
				e_from[actepos] = actenpos+startenpos ;
        e_sel[actepos] = AF_YES ;

        enode_npos[actenpos+0] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;
        enode_npos[actenpos+1] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;
        enode_npos[actenpos+2] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;
        enode_npos[actenpos+3] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ;

        enode_npos[actenpos+4] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;
        enode_npos[actenpos+5] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;
        enode_npos[actenpos+6] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;
        enode_npos[actenpos+7] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ;

        for (kk=0; kk<8; kk++)
        {
          enode_pos[actenpos+kk] = e_tot+startepos ;
          enode_elem[actenpos+kk] = maxeid ;
          enode_id[actenpos+kk] = fdbInputGetInt(NODE, NODE_ID, enode_npos[actenpos+kk]) ; 
        }


        actepos++ ;
        actenpos += 8;
				e_tot++ ;
				en_tot+=8 ;
      }
    }
  }

  if (e_tot > 0)
	{
    fdbInputAppendTableRow(ELEM, e_tot, &startepos) ;
		fdbInputIntChangeValsFast( ELEM, ELEM_SEL, startepos, e_tot, e_sel, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_ID, startepos, e_tot, e_nid, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_SET, startepos, e_tot, e_dset, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_TYPE, startepos, e_tot, e_type, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_RS, startepos, e_tot, e_rset, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_MAT, startepos, e_tot, e_mat, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_NODES, startepos, e_tot, e_nnum, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_FROM, startepos, e_tot, e_from, AF_NO);

    fdbInputAppendTableRow(ENODE, en_tot, &startenpos) ;
		fdbInputIntChangeValsFast( ENODE, ENODE_ID, startenpos, en_tot, enode_id, AF_NO);
		fdbInputIntChangeValsFast( ENODE, ENODE_POS, startenpos, en_tot, enode_pos, AF_NO);
		fdbInputIntChangeValsFast( ENODE, ENODE_NPOS, startenpos, en_tot, enode_npos, AF_NO);
		fdbInputIntChangeValsFast( ENODE, ENODE_ELEM, startenpos, en_tot, enode_elem, AF_NO);
	}

memFree:
  femIntFree(nodes) ;

  femIntFree(enode_elem );
  femIntFree(enode_pos  );
  femIntFree(enode_npos );
  femIntFree(enode_id   );
  femIntFree(e_type );
  femIntFree(e_rset );
  femIntFree(e_mat  );
  femIntFree(e_dset );
  femIntFree(e_nnum );
  femIntFree(e_nid   );
  femIntFree(e_from );
  femIntFree(e_sel );
  femIntFree(n_nid  );
  femIntFree(n_sel  );
  femDblFree(n_x   );
  femDblFree(n_y   );
  femDblFree(n_z   );

  return(rv);
}

/* ------------------------------------------------ */

void fdbMeshGeomFuncBrick20(double *xr, double *yr, double *zr, double xi, double yi, double zi, double *x, double *y, double *z)
{
  int i ;
  double xv[21] = {0,-1, 0, 1, 1, 1,-1,-1,-1,-1, 1, 1,-1,-1, 0, 1, 1, 1, 0,-1,-1};
  double yv[21] = {0,-1,-1,-1, 0, 1, 1, 1, 0,-1,-1, 1, 1,-1,-1,-1, 0, 1, 1, 1, 0};
  double zv[21] = {0,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
	long p_a_nodes[8]={1,3,5,7,13,15,17,19};
	long p_b_nodes[4]={2,6,14,18};
	long p_c_nodes[4]={4,8,16,20};
	long p_d_nodes[4]={9,10,11,12};

  *x = 0.0 ;
  *y = 0.0 ;
  *z = 0.0 ;

  for (i=0; i<8; i++)
  {
    *x += 0.125*(1.0+xv[p_a_nodes[i]]*xi)*(1.0+yv[p_a_nodes[i]]*yi)*(1.0+zv[p_a_nodes[i]]*zi) *(xv[p_a_nodes[i]]*xi+yv[p_a_nodes[i]]*yi+zv[p_a_nodes[i]]*zi-2.0) * xr[p_a_nodes[i]-1]  ;
    *y += 0.125*(1.0+xv[p_a_nodes[i]]*xi)*(1.0+yv[p_a_nodes[i]]*yi)*(1.0+zv[p_a_nodes[i]]*zi) *(xv[p_a_nodes[i]]*xi+yv[p_a_nodes[i]]*yi+zv[p_a_nodes[i]]*zi-2.0) * yr[p_a_nodes[i]-1]  ;
    *z += 0.125*(1.0+xv[p_a_nodes[i]]*xi)*(1.0+yv[p_a_nodes[i]]*yi)*(1.0+zv[p_a_nodes[i]]*zi) *(xv[p_a_nodes[i]]*xi+yv[p_a_nodes[i]]*yi+zv[p_a_nodes[i]]*zi-2.0) * zr[p_a_nodes[i]-1]  ;
  }

	for (i=0; i<4; i++)
  {
    *x += 0.25*(1.0-xi*xi)*(1.0+yv[p_b_nodes[i]]*yi)*(1.0+zv[p_b_nodes[i]]*zi) * xr[p_b_nodes[i]-1]  ;
    *y += 0.25*(1.0-xi*xi)*(1.0+yv[p_b_nodes[i]]*yi)*(1.0+zv[p_b_nodes[i]]*zi) * yr[p_b_nodes[i]-1]  ;
    *z += 0.25*(1.0-xi*xi)*(1.0+yv[p_b_nodes[i]]*yi)*(1.0+zv[p_b_nodes[i]]*zi) * zr[p_b_nodes[i]-1]  ;
  }

	for (i=0; i<4; i++)
  {
    *x += 0.25*(1.0+xv[p_c_nodes[i]]*xi)*(1.0-yi*yi)*(1.0+zv[p_c_nodes[i]]*zi) * xr[p_c_nodes[i]-1]  ;
    *y += 0.25*(1.0+xv[p_c_nodes[i]]*xi)*(1.0-yi*yi)*(1.0+zv[p_c_nodes[i]]*zi) * yr[p_c_nodes[i]-1]  ;
    *z += 0.25*(1.0+xv[p_c_nodes[i]]*xi)*(1.0-yi*yi)*(1.0+zv[p_c_nodes[i]]*zi) * zr[p_c_nodes[i]-1]  ;
  }

	for (i=0; i<4; i++)
  {
    *x += 0.25*(1.0+xv[p_d_nodes[i]]*xi)*(1.0+yv[p_d_nodes[i]]*yi)*(1.0-zi*zi) * xr[p_d_nodes[i]-1]  ;
    *y += 0.25*(1.0+xv[p_d_nodes[i]]*xi)*(1.0+yv[p_d_nodes[i]]*yi)*(1.0-zi*zi) * yr[p_d_nodes[i]-1]  ;
    *z += 0.25*(1.0+xv[p_d_nodes[i]]*xi)*(1.0+yv[p_d_nodes[i]]*yi)*(1.0-zi*zi) * zr[p_d_nodes[i]-1]  ;
  }
}

/** Original meshing procedure - very slow but possibly more general *UNUSED* */
int fdbMeshEnt004old(long entPos)
{
  int    rv = AF_OK ;
  long  *nodes = NULL ;
  long   entnum, startnpos ;
  long   ilen,jlen,klen, nlen, npos, posn, n_id, e_id ;
	int    test ;
  long   et, rs, mat, set ;
  long   enodelist[8] ;
  long   i, j, k ;
  long   sum ;
  int    etype_pos ;
  int    etype = 9 ;
  double x,y,z ;
  double xi[20] ;
  double yi[20] ;
  double zi[20] ;

  entnum = fdbInputGetInt(ENTITY, ENTITY_ID, entPos) ;

  if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, entnum, &posn) >= 3)
  {
    ilen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn) ;
    jlen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+1) ;
    klen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+2) ;
  }
  else
  {
    return(AF_ERR_EMP);
  }

  if ((ilen < 1) || (jlen < 1) || (klen < 1))
  { 
    return(AF_ERR_VAL) ;
  }
  else
  { 
    nlen = (ilen+1)*(jlen+1)*(klen+1) ;
  }

  if ((nodes=femIntAlloc(nlen)) == NULL)
  {
    return(AF_ERR_MEM) ;
  }


  for (i=0; i<20; i++)
  {
  	npos = fdbEntKpPos(entPos, i) ;
	  xi[i] = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	  yi[i] = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	  zi[i] = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;
  }

	startnpos = fdbInputTabLenAll(NODE)  ;
	if (startnpos < 0) { startnpos = 0 ; }


  sum = 0 ;

  for (k=0; k<=klen; k++)
  {
    for (j=0; j<=jlen; j++)
    {
      for (i=0; i<=ilen; i++)
      {
        /* nodes: */
        fdbMeshGeomFuncBrick20(
          xi, yi, zi, 
          (2.0*((((double)i+0.0)/(double)ilen))) - 1.0, 
          (2.0*((((double)j+0.0)/(double)jlen))) - 1.0, 
          (2.0*((((double)k+0.0)/(double)klen))) - 1.0, 
          &x, &y, &z) ;

				if ((i==0)||(i==ilen)||(j==0)||(j==jlen)||(k==0)||(k==klen)||(fdbMeshCheckAllNodes==AF_YES))
				{
					npos = found_suitable_node(x, y, z, fdbDistTol, startnpos+1, &test);
				}
				else
				{
					test = AF_NO ;
				}

        if (test == AF_YES)
		    {
			    /* existing node */
			    n_id = fdbInputGetInt(NODE, NODE_ID, npos) ;
		    }
		    else
		    {
			    /* new node */
			    if ((rv=f_n_new_change(0, x, y, z)) != AF_OK)
			    {
				    fprintf(msgout, "[E] %s!\n", _("Meshing failed"));
            femIntFree(nodes) ;
				    return(rv);
			    }
			    n_id = fdbInputFindMaxInt(NODE, NODE_ID) ;
		    }

		    nodes[sum] = n_id ;
        sum++ ;
      }
    }    
  }

  et  = fdbInputGetInt(ENTITY, ENTITY_ETYPE, entPos) ;
  rs  = fdbInputGetInt(ENTITY, ENTITY_RS,    entPos) ;
  mat = fdbInputGetInt(ENTITY, ENTITY_MAT,   entPos) ;
  set = fdbInputGetInt(ENTITY, ENTITY_SET,   entPos) ;

  etype_pos = fdbInputFindNextInt(ETYPE, ETYPE_ID, 0,  et) ;
  if (etype_pos < 0)
  { 
	  fprintf(msgout, "[E] %s!\n", _("Bad element type"));
	  femIntFree(nodes) ;
	  return(rv);
  }
  etype     = fdbInputGetInt(ETYPE, ETYPE_TYPE, etype_pos) ;

  /* elements: EIGHT-NODES ELEMENTS ONLY */
  if ((etype == 4) || (etype == 19))
  {
  for (i=0; i<(ilen); i++)
  {
    for (j=0; j<(jlen); j++)
    {
      for (k=0; k<(klen); k++)
      {
		    e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;
        e_id++;

        /* element 1: */
        if ((rv=f_e_new_change(e_id, et, rs, mat, set)) != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
  
        enodelist[0] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;         /* 1 */
        enodelist[1] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;     /* 6 */
        enodelist[2] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;         /* 2 */
        enodelist[3] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;     /* 3 */
      
        if ((rv=f_en_change(e_id, enodelist, 4))  != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
        /* element 2: */
        e_id++;
        if ((rv=f_e_new_change(e_id, et, rs, mat, set)) != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
  
        enodelist[0] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;     /* 6 */
        enodelist[1] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;         /* 1 */
        enodelist[2] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;     /* 5 */
        enodelist[3] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ; /* 8 */
      
        if ((rv=f_en_change(e_id, enodelist, 4))  != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
        /* element 3: */
        e_id++;
        if ((rv=f_e_new_change(e_id, et, rs, mat, set)) != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
  
        enodelist[0] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ; /* 8 */
        enodelist[1] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;     /* 3 */
        enodelist[2] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;         /* 1 */
        enodelist[3] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ;     /* 4 */
      
        if ((rv=f_en_change(e_id, enodelist, 4))  != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
        /* element 4: */
        e_id++;
        if ((rv=f_e_new_change(e_id, et, rs, mat, set)) != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
  
        enodelist[0] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ; /* 8 */
        enodelist[1] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;     /* 6 */
        enodelist[2] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;     /* 3 */
        enodelist[3] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ; /* 7 */
      
        if ((rv=f_en_change(e_id, enodelist, 4))  != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
        /* internal: */
        e_id++;
        if ((rv=f_e_new_change(e_id, et, rs, mat, set)) != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
  
        enodelist[0] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;         /* 1 */
        enodelist[1] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ; /* 8 */
        enodelist[2] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;     /* 6 */
        enodelist[3] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;     /* 3 */
      
        if ((rv=f_en_change(e_id, enodelist, 4))  != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
      }
    }
  }
  } else {
  for (i=0; i<(ilen); i++)
  {
    for (j=0; j<(jlen); j++)
    {
      for (k=0; k<(klen); k++)
      {
        if ((rv=f_e_new_change(0, et, rs, mat, set)) != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
  
		    e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;
  
        enodelist[0] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;
        enodelist[1] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;
        enodelist[2] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;
        enodelist[3] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ;

        enodelist[4] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;
        enodelist[5] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;
        enodelist[6] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;
        enodelist[7] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ;
      
        if ((rv=f_en_change(e_id, enodelist, 8))  != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			    femIntFree(nodes) ;
			    return(rv);
		    }
      }
    }
  }
  }

  femIntFree(nodes) ;

  return(rv);
}

/** New version of brick mesher: */
/** Meshes BRICK entity (must be twelve-node) 
 * @param entPos entity index
 * @return status
 */
int fdbMeshEnt004(long entPos)
{
  int    rv = AF_OK ;
  long  *nodes = NULL ;
  long   entnum ;
  long   ilen,jlen,klen, nlen, npos, posn ;
	int    test ;
  long   et, rs, mat, set ;
  long   i, j, k, kk ;
  long   sum ;
  double x,y,z ;
  double xi[20] ;
  double yi[20] ;
  double zi[20] ;
  long  *enode_elem = NULL ;
  long  *enode_pos  = NULL ;
  long  *enode_npos = NULL ;
  long  *enode_id   = NULL ;
  long  *e_sel = NULL ;
  long  *e_type = NULL ;
  long  *e_rset = NULL ;
  long  *e_mat  = NULL ;
  long  *e_dset = NULL ;
  long  *e_nnum = NULL ;
  long  *e_nid  = NULL ;
  long  *e_from = NULL ;
  long   *n_nid = NULL ;
  long   *n_sel = NULL ;
  double *n_x   = NULL ;
  double *n_y   = NULL ;
  double *n_z   = NULL ;
  long    elen = 0 ;
  long    enlen = 0 ;
	long    n_tot = 0 ;
	long    e_tot = 0 ;
	long    en_tot = 0 ;
	long    startnpos = 0 ;
	long    actnpos = 0 ;
	long    maxnid ;
  long    startepos = 0 ;
  long    startenpos = 0 ;
	long    actepos = 0 ;
	long    actenpos = 0 ;
	long    maxeid ;


  entnum = fdbInputGetInt(ENTITY, ENTITY_ID, entPos) ;

  if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, entnum, &posn) >= 3)
  {
    ilen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn  ) ;
    jlen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+1) ; 
    klen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+2) ; 
  }
  else
  {
    return(AF_ERR_EMP);
  }

  if ((ilen < 1) || (jlen < 1) || (klen < 1))
  { 
    return(AF_ERR_VAL) ;
  }
  else
  { 
    nlen = (ilen+1)*(jlen+1)*(klen+1) ;
  }

  if ((nodes=femIntAlloc(nlen+1)) == NULL)
  {
    return(AF_ERR_MEM) ;
  }

  elen = ilen*jlen*klen ; /* number of new elements */
  enlen = nlen*20 ;        /* number of new element nodes */

  /* allocations: */
  if ((enode_elem = femIntAlloc(enlen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((enode_pos = femIntAlloc(enlen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((enode_npos = femIntAlloc(enlen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((enode_id = femIntAlloc(enlen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}

  if ((e_type = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_rset = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_mat = femIntAlloc(elen))  == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_dset = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_nnum = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_nid = femIntAlloc(elen))  == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_from = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((e_sel = femIntAlloc(elen)) == NULL) {rv=AF_ERR_MEM; goto memFree;}

  if ((n_nid = femIntAlloc(nlen))  == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((n_sel = femIntAlloc(nlen))  == NULL) {rv=AF_ERR_MEM; goto memFree;}

  if ((n_x = femDblAlloc(nlen))    == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((n_y = femDblAlloc(nlen))    == NULL) {rv=AF_ERR_MEM; goto memFree;}
  if ((n_z = femDblAlloc(nlen))    == NULL) {rv=AF_ERR_MEM; goto memFree;}


	/* parameters of geometric entity: */
  for (i=0; i<20; i++)
  {
  	npos = fdbEntKpPos(entPos, i) ;
	  xi[i] = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	  yi[i] = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	  zi[i] = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;
  }

  sum = 0 ;

	startnpos = fdbInputTabLenAll(NODE)  ;
	if (startnpos < 0)
	{
		startnpos = 0 ;
		maxnid    = 0 ;
	}
	else
	{
		maxnid = fdbInputFindMaxInt (NODE, NODE_ID) ;
	}
	/*actnpos = startnpos ;*/
	actnpos = 0 ;

  /* this makes things extremelly slow (too much searches and
   * verifications) */
  for (k=0; k<=klen; k++)
  {
    for (j=0; j<=jlen; j++)
    {
      for (i=0; i<=ilen; i++)
      {
        /* nodes: */
        fdbMeshGeomFuncBrick20(
          xi, yi, zi, 
          (2.0*((((double)i+0.0)/(double)ilen))) - 1.0, 
          (2.0*((((double)j+0.0)/(double)jlen))) - 1.0, 
          (2.0*((((double)k+0.0)/(double)klen))) - 1.0, 
          &x, &y, &z) ;

				if ((i==0)||(j==0)||(k==0)||(i==ilen)||(j==jlen)||(k==klen)||(fdbMeshCheckAllNodes==AF_YES))
				{
					npos = found_suitable_node(x, y, z, fdbDistTol, startnpos+1, &test);
				}
				else
				{
					test = AF_NO ;
				}

        if (test != AF_YES)
		    {
					maxnid++ ;
					n_nid[actnpos] = maxnid ;
					n_sel[actnpos] = AF_YES ;
					n_x[actnpos] = x ;
					n_y[actnpos] = y ;
					n_z[actnpos] = z ;
          npos = actnpos+startnpos ;

					actnpos++ ;
					n_tot++ ;
		    }

		    nodes[sum] = npos ;
        sum++ ;
      }
    }    
  }

	if (n_tot > 0)
	{
    fdbInputAppendTableRow(NODE, n_tot, &startnpos) ;
		fdbInputIntChangeValsFast( NODE, NODE_ID, startnpos, n_tot, n_nid, AF_NO);
		fdbInputIntChangeValsFast( NODE, NODE_SEL, startnpos, n_tot, n_sel, AF_NO);
		fdbInputDblChangeValsFast( NODE, NODE_X , startnpos, n_tot, n_x, AF_NO);
		fdbInputDblChangeValsFast( NODE, NODE_Y , startnpos, n_tot, n_y, AF_NO);
		fdbInputDblChangeValsFast( NODE, NODE_Z , startnpos, n_tot, n_z, AF_NO);
	}

  et  = fdbInputGetInt(ENTITY, ENTITY_ETYPE, entPos) ;
  rs  = fdbInputGetInt(ENTITY, ENTITY_RS,    entPos) ;
  mat = fdbInputGetInt(ENTITY, ENTITY_MAT,   entPos) ;
  set = fdbInputGetInt(ENTITY, ENTITY_SET,   entPos) ;

  /* computing elements: */
  startepos = fdbInputTabLenAll(ELEM)  ;
	if (startepos < 0)
	{
		startepos = 0 ;
		maxeid    = 0 ;
	}
	else
	{
	  maxeid = fdbInputFindMaxInt (ELEM, ELEM_ID) ;
	}
	actepos = 0 ;

  startenpos = fdbInputTabLenAll(ENODE)  ;
	if (startenpos < 0)
	{
		startenpos = 0 ;
	}
	actenpos = 0 ;

  for (k=0; k<(klen); k++)
  {
    for (j=0; j<(jlen); j++)
    {
  		for (i=0; i<(ilen); i++)
      {
				maxeid++ ;
				e_nid[actepos] = maxeid ;
				e_type[actepos] = et ;
				e_rset[actepos] = rs ;
				e_mat[actepos] = mat ;
				e_dset[actepos] = set ;
				e_nnum[actepos] = 8 ;
				e_from[actepos] = actenpos+startenpos ;
        e_sel[actepos] = AF_YES ;

        enode_npos[actenpos+0] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;
        enode_npos[actenpos+1] = nodes[i + k*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;
        enode_npos[actenpos+2] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;
        enode_npos[actenpos+3] = nodes[i + k*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ;

        enode_npos[actenpos+4] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 0] ;
        enode_npos[actenpos+5] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+j*(ilen+1) + 1] ;
        enode_npos[actenpos+6] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 1] ;
        enode_npos[actenpos+7] = nodes[i + (k+1)*((ilen+1)*(jlen+1))+(j+1)*(ilen+1) + 0] ;

        for (kk=0; kk<8; kk++)
        {
          enode_pos[actenpos+kk] = e_tot+startepos ;
          enode_elem[actenpos+kk] = maxeid ;
          enode_id[actenpos+kk] = fdbInputGetInt(NODE, NODE_ID, enode_npos[actenpos+kk]) ; 
        }


        actepos++ ;
        actenpos += 8;
				e_tot++ ;
				en_tot+=8 ;
      }
    }
  }

  if (e_tot > 0)
	{
    fdbInputAppendTableRow(ELEM, e_tot, &startepos) ;
		fdbInputIntChangeValsFast( ELEM, ELEM_SEL, startepos, e_tot, e_sel, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_ID, startepos, e_tot, e_nid, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_SET, startepos, e_tot, e_dset, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_TYPE, startepos, e_tot, e_type, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_RS, startepos, e_tot, e_rset, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_MAT, startepos, e_tot, e_mat, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_NODES, startepos, e_tot, e_nnum, AF_NO);
		fdbInputIntChangeValsFast( ELEM, ELEM_FROM, startepos, e_tot, e_from, AF_NO);

    fdbInputAppendTableRow(ENODE, en_tot, &startenpos) ;
		fdbInputIntChangeValsFast( ENODE, ENODE_ID, startenpos, en_tot, enode_id, AF_NO);
		fdbInputIntChangeValsFast( ENODE, ENODE_POS, startenpos, en_tot, enode_pos, AF_NO);
		fdbInputIntChangeValsFast( ENODE, ENODE_NPOS, startenpos, en_tot, enode_npos, AF_NO);
		fdbInputIntChangeValsFast( ENODE, ENODE_ELEM, startenpos, en_tot, enode_elem, AF_NO);
	}

memFree:
  femIntFree(nodes) ;

  femIntFree(enode_elem );
  femIntFree(enode_pos  );
  femIntFree(enode_npos );
  femIntFree(enode_id   );
  femIntFree(e_type );
  femIntFree(e_rset );
  femIntFree(e_mat  );
  femIntFree(e_dset );
  femIntFree(e_nnum );
  femIntFree(e_nid   );
  femIntFree(e_from );
  femIntFree(e_sel );
  femIntFree(n_nid  );
  femIntFree(n_sel  );
  femDblFree(n_x   );
  femDblFree(n_y   );
  femDblFree(n_z   );

  return(rv);
}



int fdbMeshEnt005(long entPos)
{
  int    rv = AF_OK ;
  long  *nodes = NULL ;
  long   entnum ;
  long   ilen,jlen, nlen, npos, posn, n_id, e_id ;
	int    test ;
  long   et, rs, mat, set ;
  long   enodelist[4] ;
  long   i, j ;
  long   sum ;
  int    etype_pos ;
  int    etype = 2 ;
  double x,y,z ;
  double xi[8] ;
  double yi[8] ;
  double zi[8] ;

  entnum = fdbInputGetInt(ENTITY, ENTITY_ID, entPos) ;

  if (fdbInputCountInt(ENTDIV, ENTDIV_ENT, entnum, &posn) >= 2)
  {
    ilen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn) ;
    jlen = fdbInputGetInt(ENTDIV, ENTDIV_DIV, posn+1) ;
  }
  else
  {
    return(AF_ERR_EMP);
  }

  if ((ilen < 1) || (jlen < 1))
  { 
    return(AF_ERR_VAL) ;
  }
  else
  { 
    nlen = (ilen+1)*(jlen+1) ;
  }

  if ((nodes=femIntAlloc(nlen+1)) == NULL)
  {
    return(AF_ERR_MEM) ;
  }


  for (i=0; i<8; i++)
  {
  	npos = fdbEntKpPos(entPos, i) ;
	  xi[i] = fdbInputGetDbl(KPOINT, KPOINT_X, npos) ;
	  yi[i] = fdbInputGetDbl(KPOINT, KPOINT_Y, npos) ;
	  zi[i] = fdbInputGetDbl(KPOINT, KPOINT_Z, npos) ;
  }

  sum = 0 ;

  for (j=0; j<=jlen; j++)
  {
    for (i=0; i<=ilen; i++)
    {
      /* nodes: */
      fdbMeshGeomFuncCvRect(
        xi, yi, zi, 
        (2.0*((((double)i+0.0)/(double)ilen))) - 1.0, 
        (2.0*((((double)j+0.0)/(double)jlen))) - 1.0, 
        0, 
        &x, &y, &z) ;

			if ((i==0)||(i==ilen)||(j==0)||(j==jlen)||(fdbMeshCheckAllNodes==AF_YES))
			{
				npos = found_suitable_node(x, y, z, fdbDistTol, -1, &test);
			}
			else
			{
				test = AF_NO ;
			}

      if (test == AF_YES)
	    {
		    /* existing node */
		    n_id = fdbInputGetInt(NODE, NODE_ID, npos) ;
	    }
	    else
	    {
		    /* new node */
		    if ((rv=f_n_new_change(0, x, y, z)) != AF_OK)
		    {
			    fprintf(msgout, "[E] %s!\n", _("Meshing failed"));
          femIntFree(nodes) ;
			    return(rv);
		    }
		    n_id = fdbInputFindMaxInt(NODE, NODE_ID) ;
	    }

	    nodes[sum] = n_id ;
      sum++ ;
    }
  }    

  et  = fdbInputGetInt(ENTITY, ENTITY_ETYPE, entPos) ;
  rs  = fdbInputGetInt(ENTITY, ENTITY_RS,    entPos) ;
  mat = fdbInputGetInt(ENTITY, ENTITY_MAT,   entPos) ;
  set = fdbInputGetInt(ENTITY, ENTITY_SET,   entPos) ;

  etype_pos = fdbInputFindNextInt(ETYPE, ETYPE_ID, 0,  et) ;
  if (etype_pos < 0)
  { 
	  fprintf(msgout, "[E] %s!\n", _("Bad element type"));
	  femIntFree(nodes) ;
	  return(rv);
  }
  etype     = fdbInputGetInt(ETYPE, ETYPE_TYPE, etype_pos) ;

  /* elements: FOUR-NODES ELEMENTS ONLY */
  if ((etype != 11) && (etype != 18))
  {
  for (i=0; i<(ilen); i++)
  {
    for (j=0; j<(jlen); j++)
    {
      if ((rv=f_e_new_change(0, et, rs, mat, set)) != AF_OK)
		  {
		    fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
		    femIntFree(nodes) ;
		    return(rv);
		  }
  
		  e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;

      enodelist[0] = nodes[i + j*(ilen+1) + 0] ;
      enodelist[1] = nodes[i + j*(ilen+1) + 1] ;
      enodelist[2] = nodes[i + (j+1)*(ilen+1) + 1] ;
      enodelist[3] = nodes[i + (j+1)*(ilen+1) + 0] ;
  
      if ((rv=f_en_change(e_id, enodelist, 4))  != AF_OK)
		  {
		    fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
		    femIntFree(nodes) ;
		    return(rv);
		  }
    }
  }
	} else {
  /* elements: THREE-NODE ELEMENTS */
  for (i=0; i<(ilen); i++)
  {
    for (j=0; j<(jlen); j++)
    {
      if ((rv=f_e_new_change(0, et, rs, mat, set)) != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
  
		  e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;
  
      enodelist[0] = nodes[i + j*(ilen+1) + 0] ;
      enodelist[1] = nodes[i + j*(ilen+1) + 1] ;
      enodelist[2] = nodes[i + (j+1)*(ilen+1) + 1] ;
      
      if ((rv=f_en_change(e_id, enodelist, 3))  != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
      /* element 2 */
      if ((rv=f_e_new_change(0, et, rs, mat, set)) != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
  
		  e_id = fdbInputFindMaxInt(ELEM, ELEM_ID) ;
  
      enodelist[0] = nodes[i + j*(ilen+1) + 0] ;
      enodelist[1] = nodes[i + (j+1)*(ilen+1) + 1] ;
      enodelist[2] = nodes[i + (j+1)*(ilen+1) + 0] ;
      
      if ((rv=f_en_change(e_id, enodelist, 3))  != AF_OK)
		  {
			  fprintf(msgout, "[E] %s!\n", _("Creating of element failed (bad nodes)"));
			  femIntFree(nodes) ;
			  return(rv);
		  }
    }
  }

	}

  femIntFree(nodes) ;

  return(rv);
}

/* ------------------------------------------------ */

/** Creates finite element mesh from geometry data
 * @return status
 * */
int fdbGeomCreateMesh(void)
{
  int  rv = AF_OK ;
  long i ;
	long num = 0 ;
	long len = 0 ;

	len = fdbInputTabLenSel(ENTITY);

	fprintf(msgout,"[i]  %s -  %s: %li.\n", _("Mesh creation started"),_("entities"), len);

  for (i=0; i<fdbInputTabLenAll(ENTITY); i++)
  {
		if (fdbInputTestSelect(ENTITY,i) == AF_YES)
		{
			num++ ;
			fprintf(msgout, "[ ]    %s %li/%li...", _("meshing entity"), num, len);

    	switch(fdbInputGetInt(ENTITY, ENTITY_TYPE, i))
    	{
      	case 5: if ((rv=fdbMeshEnt005(i)) != AF_OK) {goto onError;} break;
      	case 4: if ((rv=fdbMeshEnt004(i)) != AF_OK) {goto onError;} break;
      	case 3: if ((rv=fdbMeshEnt003(i)) != AF_OK) {goto onError;} break;
      	case 2: if ((rv=fdbMeshEnt002(i)) != AF_OK) {goto onError;} break;
      	case 1: if ((rv=fdbMeshEnt001(i)) != AF_OK) {goto onError;} break;
      	default: rv = AF_ERR_VAL ; goto onError; break;
    	}

			fprintf(msgout, " %s.\n", _("done"));
		}
  }

	if (rv == AF_OK) { fprintf(msgout,"[i]  %s.\n", _("mesh creation done") ); }
	else { fprintf(msgout,"[w]  %s.\n", _("Mesh creation failed") ); }

  return(rv) ;
onError:
  fprintf(msgout,"\n[E] %s!\n", _("Mesh operation failed"));
  return(rv) ;
}

/* end of fdb_mesh.c */
