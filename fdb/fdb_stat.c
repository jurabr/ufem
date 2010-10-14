/*
   File name: fdb_stat.c
   Date:      2003/12/30 13:37
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

	 FEM Database - data status tracking

	 $Id: fdb_stat.c,v 1.4 2004/11/11 21:41:07 jirka Exp $
*/

#include "fdb_fem.h"

#define fdbInputRenumEnodeNode() fdbInputRenumIndexes(NODE, NODE_ID, ENODE, ENODE_ID, ENODE_NPOS, &nodeStat, &elemNodeStat)
#define fdbInputRenumNloadNode() fdbInputRenumIndexes(NODE, NODE_ID, NLOAD, NLOAD_NODE,NLOAD_NPOS, &nodeStat, &nloadNodeStat)
#define fdbInputRenumNdispNode() fdbInputRenumIndexes(NODE, NODE_ID, NDISP, NDISP_NODE,NDISP_NPOS, &nodeStat, &ndispNodeStat)
#define fdbInputRenumEloadElem() fdbInputRenumIndexes(ELEM, ELEM_ID, ELOAD, ELOAD_ELEM,ELOAD_EPOS, &elemStat, &eloadElemStat)

#define fdbInputRenumEntKpKpoint() fdbInputRenumIndexes(KPOINT, KPOINT_ID, ENTKP, ENTKP_ENT, ENTKP_KPFROM, &kpointStat, &entityKPointStat)

long nodeStat  = 0 ;      /* node change counter     */
long elemNodeStat = 0 ;   /* sync status for element */
long nloadNodeStat = 0 ;  /* sync status for nload   */
long ndispNodeStat = 0;   /* sync status for ndisp   */

long elemStat = 0 ;       /* element change counter  */
long eloadElemStat = 0 ;  /* sync status for eload   */

long kpointStat  = 0 ;      /* keypoint change counter     */
long entityKPointStat = 0 ; /* sync status for geometry entity */

long entityStat  = 0 ;      /* entity change counter     */



/** Sets indexes up to date
 * @param i_tab table to be searched
 * @param i_col_name collumn with names
 * @param j_tab table to be indexed
 * @param j_col_name collumn with names
 * @param j_col_pos collumn with indexes (will be changed)
 * @param i_status pointer to status of i_tab
 * @param j_status pointer to status of j_tab (wil be changed)
 * @return status
 */
int fdbInputRenumIndexes(  long  i_tab, 
                           long  i_col_name, 
                           long  j_tab,
                           long  j_col_name,
                           long  j_col_pos,
                           long *i_status,
                           long *j_status
                          )
{
  int  rv = AF_OK ;
  long i,j ;
  long nlen, enlen;
  long nnum;
  long val ;
	long col,type ;

  if ((nlen = fdbInputTabLenAll(i_tab)) <= 0) {return(AF_OK);}
  if ((enlen = fdbInputTabLenAll(j_tab)) <= 0) {return(AF_OK);}

  /* setting the whole field to "-1" */
  val = -1 ;
#if 0
  fdbInputIntChangeVals(j_tab, j_col_pos,0, enlen, &val, AF_NO ) ;
#else
 	fdbWhatCol(&InputTab[j_tab], j_col_pos, &col, &type);

  for (j=0; j<enlen; j++)
  {
		fdbIntChangeVals(&inputData.intfld, col, j, 1, &val, AF_NO);
	}
#endif

  for (i=0; i<nlen; i++)
  {
    nnum = fdbInputGetInt(i_tab, i_col_name, i) ;
    
    for (j=0; j<enlen; j++)
    {
      if (fdbInputGetInt(j_tab, j_col_name, j) == nnum)
      {
        val = i ;

#if 0
        fdbInputIntChangeVals(j_tab, j_col_pos, j, 1, &val, AF_NO);
#else
				fdbIntChangeVals(&inputData.intfld, col, j, 1, &val, AF_NO);
#endif
      }
    }
  }

  for (j=0; j<enlen; j++)
  {
    if ((nnum = fdbInputGetInt(j_tab, j_col_pos, j)) == -1)
    {
      rv = AF_ERR_VAL ;
      break ;
    }
  }

  if (rv == AF_OK)
  {
    *j_status = *i_status ;
  }
  
  return(rv);
}


/** Sets all status indicators to zero */
void fdbInputResetStats(void)
{
  nodeStat      = 0 ;
  elemNodeStat  = 0 ;
  nloadNodeStat = 0 ;
  ndispNodeStat = 0 ;

  elemStat      = 0 ;
  eloadElemStat = 0 ;

  kpointStat       = 0 ;     
  entityKPointStat = 0 ;

  entityStat       = 0 ;
}

/** Sets all status indicators to zero and sync them */
void fdbInputSyncStats(void)
{
  if (nodeStat != elemNodeStat ) fdbInputRenumEnodeNode() ;
  if (nodeStat != nloadNodeStat) fdbInputRenumNloadNode() ;
  if (nodeStat != ndispNodeStat) fdbInputRenumNdispNode() ;

  if (elemStat != eloadElemStat) fdbInputRenumEloadElem() ;

  if (kpointStat != entityKPointStat) fdbInputRenumEntKpKpoint() ;

  nodeStat      = 0 ;
  elemNodeStat  = 0 ;
  nloadNodeStat = 0 ;
  ndispNodeStat = 0 ;

  elemStat      = 0 ;
  eloadElemStat = 0 ;

  kpointStat       = 0 ;     
  entityKPointStat = 0 ;

  entityStat       = 0 ;     
}

/** Founds position of element's node  
 * @param  e_pos position of element
 * @param  n_num position of node _on element_
 * @return position of node in "NODE" (-1 on fatal error)
 * */
long fdbEnodePos(long e_pos, long n_num)
{
  long n_pos = 0 ;
  long tmp = 0 ;

  if (nodeStat != elemNodeStat)
  {
    /* outdated! */
    if (fdbInputRenumEnodeNode() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("FATAL - Invalid data"));
      return(-1);
    }
  }

  tmp = fdbInputGetInt(ELEM, ELEM_FROM, e_pos) + n_num ;
  n_pos = fdbInputGetInt(ENODE, ENODE_NPOS, tmp) ;
  return(n_pos) ;
}

/** Founds position nload's node
 * @param  nl_pos position of load
 * @return position of node in "NODE" (-1 on fatal error)
 * */
long fdbNloadNodePos(long nl_pos)
{
  long n_pos = 0 ;

  if (nodeStat != nloadNodeStat)
  {
    /* outdated! */
    if (fdbInputRenumNloadNode() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("FATAL - Invalid data"));
      return(-1);
    }
  }

  n_pos = fdbInputGetInt(NLOAD, NLOAD_NPOS,nl_pos) ;
  return(n_pos) ;
}

/** Founds position ndisp's node
 * @param  nd_pos position of disp
 * @return position of node in "NODE" (-1 on fatal error)
 * */
long fdbNdispNodePos(long nd_pos)
{
  long n_pos = 0 ;

  if (nodeStat != ndispNodeStat)
  {
    /* outdated! */
    if (fdbInputRenumNdispNode() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("FATAL - Invalid data"));
      return(-1);
    }
  }

  n_pos = fdbInputGetInt(NDISP, NDISP_NPOS,nd_pos) ;
  return(n_pos) ;
}

/** Founds position eload's elem
 * @param  nd_pos position of disp
 * @return position of node in "NODE" (-1 on fatal error)
 * */
long fdbEloadElemPos(long el_pos)
{
  long e_pos = 0 ;

  if (elemStat != eloadElemStat)
  {
    /* outdated! */
    if (fdbInputRenumEloadElem() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("FATAL - Invalid data"));
      return(-1);
    }
  }

  e_pos = fdbInputGetInt(ELOAD, ELOAD_EPOS, el_pos) ;
  return(e_pos) ;
}

/** Founds position of entity's kpoint
 * @param  e_pos position of element
 * @param  n_num position of node _on element_
 * @return position of node in "NODE" (-1 on fatal error)
 * */
long fdbEntKpPos(long e_pos, long n_num)
{
  long n_pos = 0 ;
  long tmp = 0 ;

  if (kpointStat != entityKPointStat)
  {
    /* outdated! */
    if (fdbInputRenumEntKpKpoint() != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("FATAL - Invalid data"));
      return(-1);
    }
  }

  tmp = fdbInputGetInt(ENTITY, ENTITY_KFROM, e_pos) + n_num ;
  n_pos = fdbInputGetInt(ENTKP, ENTKP_KPFROM, tmp) ;
  return(n_pos) ;
}

/** Updates nodeStat number in secure way */
void fdbInputUpdateNodeStats(void)
{
  if (nodeStat >= 10e9)
  {
    nodeStat = 1 ;

    elemNodeStat  = 0 ;
    nloadNodeStat = 0 ;
    ndispNodeStat = 0 ;
  }
  else
  {
    nodeStat++;
  }
}

/** Updates kpointStat number in secure way */
void fdbInputUpdateKPointStats(void)
{
  if (kpointStat >= 10e9)
  {
    kpointStat = 1 ;

    nodeStat      = 0 ;
    elemNodeStat  = 0 ;
    nloadNodeStat = 0 ;
    ndispNodeStat = 0 ;
  }
  else
  {
    kpointStat++;
  }
}


/** Updates elemStat number in secure way */
void fdbInputUpdateElemStats(void)
{
  if (elemStat >= 10e9)
  {
    elemStat = 1 ;

    eloadElemStat  = 0 ;
  }
  else
  {
    elemStat++;
  }
}

/** Updates elemStat number in secure way */
void fdbInputUpdateEntStats(void)
{
  if (entityStat >= 10e9)
  {
    entityStat = 1 ;

    entityKPointStat  = 0 ;
  }
  else
  {
    entityStat++;
  }
}


/** Updates _FROM information
 * @param tab_target table to be changed
 * @param col_target _FROM to be changed
 * @param col_target_name names to be searched in col_src
 * @param tab_src source table
 * @param col_src source collumn
 * @return status
 * */
int fdbInputRenumFromFlds(long tab_target,
                          long col_target,
                          long col_target_name,
                          long tab_src, 
                          long col_src)
{
  long i;
  long pos;
  long val;
	long col,type ;
  long tab_target_len = 0 ;

  if ((tab_target_len = fdbInputTabLenAll(tab_target)) == 0)
     { return(AF_OK); }

 	fdbWhatCol(&InputTab[tab_target], col_target, &col, &type);

  for (i=0; i<tab_target_len; i++)
  {
    val = fdbInputGetInt(tab_target, col_target_name, i) ;

		pos = -1 ;
    if ((fdbInputCountIntAll(tab_src, col_src, val, &pos)) < 1) 
    {
      return(AF_ERR);
    }
    else
    {
			fdbIntChangeVals(&inputData.intfld, col, i, 1, &pos, AF_NO);
    }
  }

  return(AF_OK);
}

/* end of fdb_stat.c */
