/*
   File name: fdb_get.c
   Date:      2006/02/12 20:03
   Author:    

   Copyright (C) 2006 

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

	 Database for FEM - returning of data

	 $Id$
*/

#include "fdb_fem.h"
#include "fdb_res.h"
#include "fdb_edef.h"

/** Tests if row with ID in table is selected 
 * @param tab table identified (NODE..)
 * @param tab_id table ID identified (NODE_ID..)
 * @param id ID number of thing
 * @return 0 if NOT selected, 1 if selected
 */
long fdb_user_get_sel(long tab, long tab_id, long id)
{
  long pos ;

  if (fdbInputCountInt(tab, tab_id, id, &pos) < 0)
  {
    return(0);
  }
  else
  {
    if (pos < 1)
    {
      return(0);
    }
    else
    {
      if (fdbInputTestSelect(tab, pos) == AF_YES)
      {
        return(1);
      }
      else
      {
        return(0);
      }
    }
  }
  
  return(0);
}

/** Gets node coordinate
 * @param id node number
 * @param dir direction (NODE_X, NODE_Y, NODE_Z)
 * @return coordinate of node in given direction
 */
double fdb_user_get_node_xyz(long id, long dir)
{
  long pos ;

  if (fdbInputCountInt(NODE, NODE_ID, id, &pos) < 0)
  {
    return(0);
  }
  else
  {
    if (fdbInputTestSelect(NODE, pos) == AF_YES)
    {
      return(fdbInputGetDbl(NODE,dir,pos));
    }
    else
    {
      return(0);
    }
  }
  
  return(0);
}


/** Gets elements property (set number, et, rs, mat, number of nodes)
 * @param id node number
 * @param dir property (ELEM_SET, ELEM_TYPE, ELEM_RS, ELEM_MAT, ELEM_NODES)
 * @return value of property
 */
long fdb_user_get_elem_prop(long id, long dir)
{
  long pos ;

  if (fdbInputCountInt(ELEM, ELEM_ID, id, &pos) < 0)
  {
    return(0);
  }
  else
  {
    if (fdbInputTestSelect(ELEM, pos) == AF_YES)
    {
      return(fdbInputGetInt(ELEM,dir,pos));
    }
    else
    {
      return(0);
    }
  }
  
  return(0);
}

/** Gets ID on nnum-th element's node
 * @param id node number
 * @param nnum number of wanted node (numbered from 1 !!! )
 * @return ID of nnum-th node
 */
long fdb_user_get_elem_node_id(long id, long nnum)
{
  long pos, npos, max ;

  if (fdbInputCountInt(ELEM, ELEM_ID, id, &pos) < 0)
  {
    return(0);
  }
  else
  {
    if (fdbInputTestSelect(ELEM, pos) == AF_YES)
    {
      max = fdbInputGetInt(ELEM,ELEM_NODES,pos);
      if (max > nnum) { return(0); }
      
      npos = fdbInputGetInt(ELEM,ELEM_FROM,pos);

      return(
          fdbInputGetInt(
            NODE,NODE_ID,fdbInputGetInt(ENODE,ENODE_NPOS, npos+nnum-1)
            ) 
          );
    }
    else
    {
      return(0);
    }
  }
  
  return(0);
}


/** Gets nload value
 * @param id node number
 * @param type load type
 * @return ID of nnum-th node
 */
double fdb_user_get_elem_nload_val(long node_id, long type)
{
  long pos ;

  if (fdbInputCountTwoInt(NLOAD, NLOAD_NODE, node_id, NLOAD_TYPE, type, &pos) < 0)
  {
    return(0.0);
  }
  else
  {
    if (fdbInputTestSelect(NLOAD, pos) == AF_YES)
    {
			return(fdbInputGetDbl(NLOAD, NLOAD_VAL, pos));
    }
    else
    {
      return(0.0);
    }
  }
  
  return(0.0);
}


/** Gets nval value (input data - not result !!!)
 * @param id node number
 * @param type displacement type
 * @return ID of nnum-th node
 */
double fdb_user_get_ndisp_val(long node_id, long type)
{
  long pos ;

  if (fdbInputCountTwoInt(NDISP, NDISP_NODE, node_id, NDISP_TYPE, type, &pos) <= 0)
  {
    return(0.0);
  }
  else
  {
    if (fdbInputTestSelect(NDISP, pos) == AF_YES)
    {
			return(fdbInputGetDbl(NDISP, NDISP_VAL, pos));
    }
    else
    {
      return(0.0);
    }
  }
  
  return(0.0);
}

/** Gets number of element loads on given element (no info about selection!)
 * @param id node number
 * @param dir property (ELEM_SET, ELEM_TYPE, ELEM_RS, ELEM_MAT, ELEM_NODES)
 * @return value of property
 */
long fdb_user_get_num_of_eloads(long elem_id)
{
	long pos ;

	return ( fdbInputCountInt(ELOAD, ELOAD_ID, elem_id, &pos) );
}


/** Gets number of element loads on given element (no info about selection!)
 * @param id node number
 * @param dir property (ELEM_SET, ELEM_TYPE, ELEM_RS, ELEM_MAT, ELEM_NODES)
 * @return value of property
 */
long fdb_user_get_num_of_eloads_sel(long elem_id)
{
	long pos, num ;
	long i ;
	long ipos ;

	num = 0 ;

	if ((num = fdbInputCountInt(ELOAD, ELOAD_ID, elem_id, &pos) ) > 0)
	{
		for (i=0; i<num; i++)
		{
			ipos = fdbInputFindNextInt(ELOAD, ELOAD_ID, pos, elem_id);
			pos = ipos ;
    	if (fdbInputTestSelect(ELOAD, ipos) == AF_YES) {num++ ;}
		}
	}

	return(num);
}

/** Tests if non-repeating results are there
 * @param elem_id element identifier
 * @param res_type type of result
 * @return number of non-repeating results
 */

long fdb_user_get_eres_no_rep_num(long elem_id, long res_type)
{
	long etype, e_pos;

	/* check if we are in postprocessor: */
  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(0); }

	/* find element position: */
	if (fdbInputCountInt(ELEM, ELEM_ID, elem_id, &e_pos) < 0) { return(0.0); }

	/* identify element type: */
	etype = fdbInputGetInt(ELEM, ELEM_TYPE, e_pos) ;

  if (femCheckHaveResultNoRep(etype, res_type) == AF_YES) 
	{
		return(1);
	}
	else
	{
		return(0);
	}

	return(0);
}

/** Tests how much repeating results are there
 * @param elem_id element identifier
 * @param res_type type of result
 * @return number of repeating results
 */
long fdb_user_get_eres_rep_num(long elem_id, long res_type)
{
	long etype, e_pos;

	/* check if we are in postprocessor: */
  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(0); }

	/* find element position: */
	if (fdbInputCountInt(ELEM, ELEM_ID, elem_id, &e_pos) < 0) { return(0.0); }

	/* identify element type: */
	etype = fdbInputGetInt(ELEM, ELEM_TYPE, e_pos) ;

  if (femCheckHaveResultRep(etype, res_type) == AF_YES) 
	{
		return(fdbResElemNumRepSets(e_pos, etype));
	}
	else
	{
		return(0);
	}

	return(0);
}

/** Gets element result from iven ipoint or maximum/minimum/average on element
 * @param elem_id element identifier
 * @param res_type type of result
 * @param ipoint_1 integration point number (1...n)
 * @param maxminav 0=read ipoint, 1=max, 2=min, 3=average
 * @return result value
 */
double fdb_user_get_eres(long elem_id, long res_type, long ipoint, long maxminav)
{
	double val  = 0.0 ;
	long   i ;
	long   num  = 0 ;
	long   num0 = 0 ;
	long   etype, e_pos, et_pos ;
	double val0 ;

	/* check if we are in postprocessor: */
  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(0); }

	/* find element position: */
	if ((fdbInputCountInt(ELEM, ELEM_ID, elem_id, &e_pos)) < 0) { return(0.0); }

	/* identify element type: */
	etype = fdbInputGetInt(ELEM, ELEM_TYPE, e_pos) ;

	if (fdbInputCountInt(ETYPE, ETYPE_ID, etype, &et_pos) < 0)
  {
    fprintf(msgout,"[W] %s!\n", _("Element data michmach"));
    return(0.0);
  }
	etype = fdbInputGetInt(ETYPE, ETYPE_TYPE, et_pos) ;

	/* find if the result is possible: */
	if (maxminav == 0)
	{
  	if (femCheckHaveResult(etype, res_type) != AF_YES) {return(0.0);}
	}

	if (maxminav > 0)
	{
  	if (femCheckHaveResultNoRep(etype, res_type) == AF_YES) 
		{
			num0 = 1 ;
		}
		
		num = fdbResElemNumRepSets(e_pos, etype);
	}

	switch (maxminav)
	{
		case 0 : /* see ipoint number */
							val = fdbResElemGetVal(e_pos, etype, res_type, ipoint) ;
							break ;
		case 1 : /* maximum */
							if (num0 > 0)
							{
							  val = fdbResElemGetVal(e_pos, etype, res_type, 0) ;
							}
							else
							{
								val = 0 ;
							}

							for (i=1; i<=num; i++)
							{
							  val0 = fdbResElemGetVal(e_pos, etype, res_type, i) ;
								if (val0 > val) {val = val0;}
							}
							break ;

		case 2 : /* minimum */
							if (num0 > 0)
							{
							  val = fdbResElemGetVal(e_pos, etype, res_type, 0) ;
							}
							else
							{
								val = 0 ;
							}

							for (i=1; i<=num; i++)
							{
							  val0 = fdbResElemGetVal(e_pos, etype, res_type, i) ;
								if (val0 < val) {val = val0;}
							}
							break ;

		case 3 : /* average */
							if (num0 > 0)
							{
							  val = fdbResElemGetVal(e_pos, etype, res_type, 0) ;
							}
							else
							{
								val = 0 ;
							}

							for (i=1; i<=num; i++)
							{
							  val += fdbResElemGetVal(e_pos, etype, res_type, i) ;
							}

							if ((num0+num) > 0)
							{
								val = val / ((double)(num+num0)) ;
							}
							else
							{
								val = 0 ;
							}
							break ;
	}

	return(val);
}

/** returns ID of first selected item in the given table 
 * @param type type of table (NODE, ELEM,...)
 * @param type_id type of ID column (NODE_ID, ELEM_ID,...)
 * @return 0 if note is found or ID number of item 
 */
long fdb_user_get_first_selected(long type, long type_id)
{
	long i, n ;

	n = fdbInputTabLenAll(type);
	if (n <= 0){return(0);}

	for (i=0; i<n; i++)
	{
		if (fdbInputTestSelect(type, i) == AF_OK)
		{
			return(fdbInputGetInt(type, type_id, i));
		}
	}

	return(0);
}

/** Gets node coordinate
 * @param id node number
 * @param dir direction (NODE_X, NODE_Y, NODE_Z)
 * @return coordinate of node in given direction
 */
double fdb_user_get_kp_xyz(long id, long dir)
{
  long pos ;

  if (fdbInputCountInt(KPOINT, KPOINT_ID, id, &pos) < 0)
  {
    return(0);
  }
  else
  {
    if (fdbInputTestSelect(KPOINT, pos) == AF_YES)
    {
      return(fdbInputGetDbl(KPOINT,dir,pos));
    }
    else
    {
      return(0);
    }
  }
  
  return(0);
}

/* end of fdb_get.c */
