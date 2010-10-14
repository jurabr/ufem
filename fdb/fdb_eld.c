/*
   File name: fdb_eld.c
   Date:      2003/11/01 16:47
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

	 FEM - load (on elements handling
*/

#include "fdb_fem.h"
#include "fdb_edef.h"

/** Creates new element load or changes existing
 * @param id number of node (0 == new node with default number)
 * @param x x-coordinate
 * @param y y-coordinate
 * @param z z-coordinate
 * @return status
 */
int f_el_new_change(long id, long element, long type, 
		                long set, long nval0, double val[])
{
	int    rv    = AF_OK ;
	int    nnew  = AF_NO ;
	long   pos   = -1 ;
	long   posn  = -1 ;
	long   count = 0 ;
	long   newid = 0 ;
	long   myset = 0 ;
  long   nval ;
	long   i ;
  long   element_pos ;

	/* test if load type exists:  */
	if (type >= fdbElementLoadTypeLen) { return(AF_ERR_VAL) ; }
	/* test if new and old number of values is the same */
	if (nval0 < (nval=fdbElementLoadType[type].vals)) { return(AF_ERR_VAL) ; }
  

	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(ELOAD, ELOAD_ID) + 1 ;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(ELOAD, ELOAD_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			newid = id ;
			nnew = AF_NO ;

      if (type != fdbInputGetInt(ELOAD, ELOAD_TYPE, pos))
      {
        fprintf(msgout, "[E] %s: %li!\n", _("Invalid attempt to change type of element load"), newid);
        return(AF_ERR_TYP);
      }
		}
		else /* new data */
		{
			newid = id ; 
			nnew = AF_YES ;
		}
	}

	myset = fdbSetInputLoadSet(set) ;

	if (nnew == AF_YES) /* adding of new line */
	{
    if (fdbInputCountInt(ELEM, ELEM_ID, element, &element_pos) < 1)
    {
      fprintf(msgout,"[E] %s!\n", _("Invalid element"));
      return(AF_ERR_VAL);
    }

		if ((rv = fdbInputAppendTableRow(ELOAD, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(ELOAD, ELOAD_ID, pos, 1, &newid, AF_NO);

		fdbInputIntChangeVals(ELOAD, ELOAD_EPOS, pos, 1, &element_pos, AF_NO);

		fdbInputIntChangeVals(ELOAD, ELOAD_NVAL, pos, 1, &nval, AF_NO);
	}
	else /* changing of existing */
	{
		/* do nothing */
	}

	/* common for all: */
	fdbInputIntChangeVals(ELOAD, ELOAD_SET, pos, 1, &myset, AF_NO);
	fdbInputIntChangeVals(ELOAD, ELOAD_TYPE,pos, 1, &type,  AF_NO);
	fdbInputIntChangeVals(ELOAD, ELOAD_ELEM,pos, 1, &element,  AF_NO);


	if (nnew == AF_YES)		/* allocation of space for nodes */
	{
  	if ((rv = fdbInputAppendTableRow (ELVAL, nval, &posn)) != AF_OK)
  	{
			/* cannot allocate space => delete whole element */
			fdbInputRemoveTableRow (ELOAD, pos, 1);
			return (rv);
		}

    fdbInputIntChangeVals(ELOAD, ELOAD_FROM, pos,1, &posn, AF_NO);
 	}

	if (fdbInputCountInt(ELVAL, ELVAL_ELID, id, &posn) < 0)
  {
    posn = 0 ;
  }

  for (i = 0; i < nval; i++)
  {
		fdbInputDblChangeVals (ELVAL, ELVAL_VAL, posn + i, 1, &val[i], AF_NO);
		fdbInputIntChangeVals (ELVAL, ELVAL_ELID, posn + i, 1, &newid, AF_NO);
  }


#ifdef DEVEL_VERBOSE
	fprintf(msgout,"nd(%li)[%li]: s=%li, t=%li, ##\n",
					fdbInputGetInt(ELOAD, ELOAD_ID, pos),
					fdbInputGetInt(ELOAD, ELOAD_ELEM,pos),
					fdbInputGetInt(ELOAD, ELOAD_SET, pos),
					fdbInputGetInt(ELOAD, ELOAD_TYPE,pos)
					);
#endif

	return (rv) ;
}

/** Changes set of element load:
 * @param id element identifier
 * @param set f.e. set number (0 means default)
 * @return status
 */
int f_el_change_set(long id, long set)
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
		if ( fdbInputCountInt(ELOAD, ELOAD_ID, id, &pos) < 1)
    {
      fprintf(msgout,"[E] %s!\n", _("Invalid nodal load number"));
		  return(AF_ERR_VAL) ;
    }
    else
    {
	    fdbInputIntChangeVals(ELOAD, ELOAD_SET, pos, 1, &set, AF_NO);
    }
  }

	return(rv);
}

/** Deletes element load
 * @param id number of load
 * @return status
 */
int f_el_delete(long id)
{
  int  rv = AF_OK ;
	long pos   = -1 ;
	long posn  = -1 ;
	long count = -1 ;
	long zero  =  0 ;

  /* tests: */
	if (fdbInputCountInt(ELOAD, ELOAD_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no node found */
	}

	if ((count = fdbInputCountInt(ELVAL, ELVAL_ELID, id, &posn)) < 1)
	{
		return(AF_ERR_EMP); /* no data !? */
	}

  /* deleting nodes */
  if ((fdbInputRemoveTableRow(ELVAL, posn, count)) != AF_OK) 
  {
		return(AF_ERR); /* cannot delete nodes */
  }
  else
  {
	  /* removing element */
		fdbInputIntChangeVals(ELOAD,ELOAD_ELEM,pos,1, &zero, AF_NO);
	  rv = fdbInputRemoveTableRow(ELOAD, pos, 1) ;

    /* updating _FROM info */
    rv = fdbInputRenumFromFlds(ELOAD,
                          ELOAD_FROM,
                          ELOAD_ID,
                          ELVAL, 
                          ELVAL_ELID);
  }

  return(rv);
}

/** Lists element load
 * @param id number of load (0 == new node with defaukt number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 */
int f_el_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long posn  = -1 ;
	long count = -1 ;
	long print_mode = 0 ;
	long i ;
	double ival ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(ELOAD, ELOAD_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if ( fdbTestSelect(&InputTab[ELOAD], &inputData.intfld, pos) != AF_YES) 
	{
		return(AF_OK); /* not selected */
	}

	/* finding of values: */
	count = fdbInputCountInt(ELVAL, ELVAL_ELID, id, &posn) ;

	if (header == AF_YES)
	{
		fprintf(fw, "\n         EL         SET       T       N      Z    V\n");
	}

	rv = fdbInputPrintTabLine(fw, ELOAD, pos, print_mode);

  for (i=0; i<count; i++) 
  { 
    ival = fdbInputGetDbl(ELVAL, ELVAL_VAL, posn+i) ;
    fdbPrintDbl(fw, ival, print_mode); 
  }
	fprintf(fw, "\n");

	return (rv) ;
}

/* end of fdb_eld.c */
