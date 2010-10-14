/*
   File name: fdb_nld.c
   Date:      2003/10/29 11:45
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

	 FEM - load (on nodes) handling
*/

#include "fdb_fem.h"
#include "cint.h"

/** Creates new node or changes existing
 * @param id number of node (0 == new node with defaukt number)
 * @param x x-coordinate
 * @param y y-coordinate
 * @param z z-coordinate
 * @return status
 */
int f_nl_new_change(long id, long node, long type, 
		                long set, double val)
{
	int    rv    = AF_OK ;
	int    nnew  = AF_NO ;
	long   pos   = -1 ;
	long   pos2  = -1 ;
	long   count = 0 ;
	long   newid = 0 ;
	long   myset = 0 ;
	double myval = 0.0 ;
  long   node_pos = 0 ;

	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(NLOAD, NLOAD_ID) + 1 ;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(NLOAD, NLOAD_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			newid = fdbInputGetInt(NLOAD, NLOAD_ID, pos) ;
			nnew = AF_NO ;
		}
		else /* new data */
		{
			newid = id ; 
			nnew = AF_YES ;
		}
	}

	myset = fdbSetInputLoadSet(set) ;
	myval = val ;

	if (nnew == AF_YES)
	{
		/* test if the same type and orientation
		   exists. if yes, accumulate (== do not
			 create new)
			 note: this is pretty strange
		 */
		count = fdbInputCountThreeInt(
				NLOAD, NLOAD_NODE, node, 
				NLOAD_TYPE, type, 
				NLOAD_SET, myset,  &pos2);

		if (count > 0) /* modification of existing */
		{
			nnew = AF_NO ;
			pos = pos2 ;
			if (fdbSummNodalLoads == AF_YES)
			{
				myval += fdbInputGetDbl(NLOAD, NLOAD_VAL, pos);
			}
		}
	}

	/*printf("count is %li\n", count);*/

	if (nnew == AF_YES) /* adding of new line */
	{
    if (fdbInputCountInt(NODE, NODE_ID, node, &node_pos) < 1)
    {
      fprintf(msgout,"[E] %s!\n", _("Invalid node"));
      return(AF_ERR_VAL);
    }

		if ((rv = fdbInputAppendTableRow(NLOAD, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(NLOAD, NLOAD_ID, pos, 1, &newid, AF_NO);

		fdbInputIntChangeVals(NLOAD, NLOAD_NPOS, pos, 1, &node_pos, AF_NO);
	}
	else /* changing of existing */
	{
		/* do nothing */
	}

	/* common for all: */
	fdbInputIntChangeVals(NLOAD, NLOAD_SET, pos, 1, &myset, AF_NO);
	fdbInputIntChangeVals(NLOAD, NLOAD_TYPE,pos, 1, &type,  AF_NO);
	fdbInputIntChangeVals(NLOAD, NLOAD_NODE,pos, 1, &node,  AF_NO);
	fdbInputDblChangeVals(NLOAD, NLOAD_VAL, pos, 1, &myval, AF_NO);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"nl(%li)[%li]: s=%li, t=%li, val=%e ##\n",
					fdbInputGetInt(NLOAD, NLOAD_ID, pos),
					fdbInputGetInt(NLOAD, NLOAD_NODE,pos),
					fdbInputGetInt(NLOAD, NLOAD_SET, pos),
					fdbInputGetInt(NLOAD, NLOAD_TYPE,pos),
					fdbInputGetDbl(NLOAD, NLOAD_VAL, pos)
					);
#endif

	return(rv) ;
}

/** Changes set of force:
 * @param id element identifier
 * @param set f.e. set number (0 means default)
 * @return status
 */
int f_nl_change_set(long id, long set)
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
		if ( fdbInputCountInt(NLOAD, NLOAD_ID, id, &pos) < 1)
    {
      fprintf(msgout,"[E] %s!\n", _("Invalid nodal load number"));
		  return(AF_ERR_VAL) ;
    }
    else
    {
	    fdbInputIntChangeVals(NLOAD, NLOAD_SET, pos, 1, &set, AF_NO);
    }
  }

	return(rv);
}

/** Deletes nodal load
 * @param id number of load
 * @return status
 */
int f_nl_delete(long id)
{
	long pos   = -1 ;
	long zero  =  0 ;

  /* tests: */
	if (fdbInputCountInt(NLOAD, NLOAD_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no node found */
	}

	/* removing */
	fdbInputIntChangeVals(NLOAD,NLOAD_ID,pos,1, &zero, AF_NO);
	return( fdbInputRemoveTableRow(NLOAD, pos, 1) );
}

/** Lists nodal load
 * @param id number of load (0 == new node with defaukt number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 */
int f_nl_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long print_mode = 0 ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(NLOAD, NLOAD_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if ( fdbTestSelect(&InputTab[NLOAD], &inputData.intfld, pos) != AF_YES) 
	{
		return(AF_OK); /* not selected */
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n         ID        SET          T          N      V\n");
	}

	rv = fdbInputPrintTabLine(fw, NLOAD, pos, print_mode);
	fprintf(fw, "\n");

	return(rv) ;
}

int f_nl_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id ;
  long i ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 5, _("Forces")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Node")) ;
  fdbPrnTableHeaderItem(fw, 2, _("Type")) ;
  fdbPrnTableHeaderItem(fw, 3, _("Size")) ;
  fdbPrnTableHeaderItem(fw, 4, _("Time")) ;

  for (i=0; i<fdbInputTabLenAll(NLOAD); i++)
  {
    if (fdbInputTestSelect(NLOAD, i) == AF_YES)
    {
      id = fdbInputGetInt(NLOAD,NLOAD_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemInt(fw,1,fdbInputGetInt(NLOAD,NLOAD_NODE,i));
      fdbPrnTableItemStr(fw,2,
            ciGetVarNameFromGrp(
              fdbFemStrFromInt(
          fdbInputGetInt(NLOAD,NLOAD_TYPE,i)), 
              "load")
          );
      fdbPrnTableItemDbl(fw,3,fdbInputGetDbl(NLOAD,NLOAD_VAL,i));
      fdbPrnTableItemInt(fw,4,fdbInputGetInt(NLOAD,NLOAD_SET,i));
    }
  }
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/* end of fdb_disp.c */
