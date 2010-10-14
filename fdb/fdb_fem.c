/*
   File name: fdb_fem.c
   Date:      2003/08/17 11:08
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

	 FEM database - FEM data usage

	 $Id: fdb_fem.c,v 1.25 2005/02/15 19:55:50 jirka Exp $
*/

#include "fdb_fem.h"
#include "fdb_edef.h"
#include "cint.h"

tInData inputData ;

int   femOneDirSupp = 0 ;

/* if nodal loads/disps are added or rewritten: */
long fdbSummNodalLoads = AF_NO ;
long fdbSummNodalDisps = AF_NO ;


/** Changes data in integer table (input data only)
 * @param tab_id table identifier (NODE,...)
 * @param col_ide collumnt identifier (NODE_ID,...)
 * @param from starting point
 * @param count number of changed lines
 * @param val field with values
 * @param add if AF_YES data are added (AF_NO means data are replaced)
 * @return status 
 */
int fdbInputIntChangeVals(long tab_id,
													long col_id, 
													long from, 
													long count, 
													long *val, 
													long add)
{
	int rv = AF_OK ; 
	int  i ;
	long col  = 0 ;
	long type = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	if (type != FDB_INT) {return(AF_ERR_TYP);}

	for (i=from; i<(from+count); i++)
	{
		/* selection test */
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			/* do nothing - not selected */
			continue ;
		}

		rv = fdbIntChangeVals(&inputData.intfld, col, i, 1, val, add);
	}

	return(rv);
}

/** Changes data in integer table WITHOUT care about selections
 * @param tab_id table identifier (NODE,...)
 * @param col_ide collumnt identifier (NODE_ID,...)
 * @param from starting point
 * @param count number of changed lines
 * @param val field with values
 * @param add if AF_YES data are added (AF_NO means data are replaced)
 * @return status 
 */
int fdbInputIntChangeValsFast(long tab_id,
													long col_id, 
													long from, 
													long count, 
													long *val, 
													long add)
{
	long col  = 0 ;
	long type = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	if (type != FDB_INT) {return(AF_ERR_TYP);}

	return( fdbIntChangeVals(&inputData.intfld, col, from, count, val, add) );
}

/** Changes data in floating point table (input data only)
 * @param tab_id table identifier (NODE,...)
 * @param col_ide collumnt identifier (NODE_ID,...)
 * @param from starting point
 * @param count number of changed lines
 * @param val field with values
 * @param add if AF_YES data are added (AF_NO means data are replaced)
 * @return status 
 */
int fdbInputDblChangeVals(long    tab_id,
													long    col_id, 
													long    from, 
													long    count, 
													double *val, 
													long    add)
{
	int rv = AF_OK ; 
	int i ;
	long col  = 0 ;
	long type = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	if (type != FDB_DBL) {return(AF_ERR_TYP);}

	for (i=from; i<(from+count); i++)
	{
		/* selection test */
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			/* do nothing - not selected */
			continue ;
		}

		rv = fdbDblChangeVals(&inputData.dblfld, col, i, 1, val, add);
	}

	return(rv);
}

/** Changes data in floating point table  WITHOUT care about selections
 * @param tab_id table identifier (NODE,...)
 * @param col_ide collumnt identifier (NODE_ID,...)
 * @param from starting point
 * @param count number of changed lines
 * @param val field with values
 * @param add if AF_YES data are added (AF_NO means data are replaced)
 * @return status 
 */
int fdbInputDblChangeValsFast(long    tab_id,
													long    col_id, 
													long    from, 
													long    count, 
													double *val, 
													long    add)
{
	long col  = 0 ;
	long type = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	if (type != FDB_DBL) {return(AF_ERR_TYP);}

	return( fdbDblChangeVals(&inputData.dblfld, col, from, count, val, add) );
}


/** Adds row (line) to input data
 * @param tab_id table identifier (NODE,...)
 * @param count number of added lines
 * @param index pointer to position of first added line (result)
 * @return status
 */
int fdbInputAppendTableRow(long tab_id, long count, long *index)
{
	int rv = AF_OK ;
	int i;

	rv = fdbAppendTableRow( &InputTab[tab_id],
				 								  &inputData.intfld, 
													&inputData.dblfld, 
													 count, 
													 index
												) ;

	if (rv != AF_OK) {return(rv);}

	for (i= *index; i< ( *index + count); i++)
	{
		fdbSetSelect(&InputTab[tab_id], &inputData.intfld, i) ;
	}

	/* table row must be selected here! */

	return(rv);
}


/** Inserts row (line) to input data
 * @param tab_id table identifier (NODE,...)
 * @param from starting position
 * @param count number of added lines
 * @return status
 */
int fdbInputInsertTableRow(long tab_id, long from, long count)
{
	return ( fdbInsertTableRow(&InputTab[tab_id],
				 										 &inputData.intfld, 
														 &inputData.dblfld, 
														 from,
														 count) 
			   );
}

/** Deletes rows (lines) from input data
 * @param tab_id table identifier (NODE,...)
 * @param from starting position
 * @param count number of added lines
 * @return status
 */
int fdbInputRemoveTableRow(long tab_id, long from, long count)
{
	int rv = AF_OK ;
	int i ;
  long skip = 0 ;

	for (i=from; i<from+count; i++)
	{
		/* selection test */
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, from+skip) != AF_YES) 
		{
			/* do nothing - not selected */
      skip++ ;
			continue ;
		}

		fdbRemoveTableRow(&InputTab[tab_id],
										  &inputData.intfld, 
										  &inputData.dblfld, 
										  from+skip,
										  1) ;
	}

	return(rv);
}



/** Computes number of "val" values in input data collumn
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param val value to be found
 * @param first pointer to position of first found value (result)
 * @return number of values
 */
long fdbInputCountInt(long tab_id, long col_id, long val, long *first)
{
	long col  = 0 ;
	long type = 0 ;
	int  i;
	static long count = 0 ;
	long my_first = -1 ;

  count = 0 ;
	my_first = -1 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	for (i=0; i<inputData.intfld.len[col]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if (inputData.intfld.data[col][i] == val)
		{
			count++;
			if (my_first == -1) {my_first = i ;}
		}
	}

	if (first != NULL)
	{
		*first = my_first ;
	}

	return(count);
}

/** Computes number of "val" values in input data collumn
 * from ALL items (not only selected)
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param val value to be found
 * @param first pointer to position of first found value (result)
 * @return number of values
 */
long fdbInputCountIntAll(long tab_id, long col_id, long val, long *first)
{
	long col  = 0 ;
	long type = 0 ;
#ifndef _USE_THREADS_
	int  i;
#endif
	static long count = 0 ;
	long my_first = -1 ;

  count = 0 ;
	my_first = -1 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

#ifndef _USE_THREADS_
	for (i=0; i<inputData.intfld.len[col]; i++)
	{
		if (inputData.intfld.data[col][i] == val)
		{
			count++;
			if (my_first == -1) {my_first = i ;}
		}
	}
#else
  /* threaded code: */
  count = fdbCountInt( inputData.intfld.data[col], inputData.intfld.len[col], val, &my_first) ;
#endif

	if (first != NULL)
	{
		*first = my_first ;
	}

	return(count);
}

/** Computes number of "val" values in TWO input data collumns
 * @param tab_id table identifier (NODE,..)
 * @param col_id1 collumn identifier (NODE_ID,..)
 * @param val1 value to be found
 * @param col_id2 collumn identifier (NODE_ID,..)
 * @param val2 value to be found
 * @param first pointer to position of first found value (result)
 * @return number of values
 */
long fdbInputCountTwoInt(long tab_id, long col_id1, long val1, long col_id2, long val2, long *first)
{
	long col1  = 0 ;
	long type1 = 0 ;
	long col2  = 0 ;
	long type2 = 0 ;
	int  i;
	static long count = 0 ;
	long my_first = -1 ;

  count = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id1, &col1, &type1);
 	fdbWhatCol(&InputTab[tab_id], col_id2, &col2, &type2);

#ifdef DEVEL
  if (type1 != type2) {return(0);} /* something is bad - different data types */
#endif

	for (i=0; i<inputData.intfld.len[col1]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if ((inputData.intfld.data[col1][i] == val1)&&(inputData.intfld.data[col2][i] == val2))
		{
			count++;
			if (my_first == -1) {my_first = i ;}
		}
	}

	if (first != NULL)
	{
		*first = my_first ;
	}

	return(count);
}

/** Computes number of "val" values in TWO input data collumns
 * @param tab_id table identifier (NODE,..)
 * @param col_id1 collumn identifier (NODE_ID,..)
 * @param val1 value to be found
 * @param col_id2 collumn identifier (NODE_ID,..)
 * @param val2 value to be found
 * @param col_id3 collumn identifier (NODE_ID,..)
 * @param val3 value to be found
 * @param first pointer to position of first found value (result)
 * @return number of values
 */
long fdbInputCountThreeInt(long tab_id, 
                           long col_id1, long val1, 
                           long col_id2, long val2, 
                           long col_id3, long val3, 
                           long *first)
{
	long col1  = 0 ;
	long type1 = 0 ;
	long col2  = 0 ;
	long type2 = 0 ;
	long col3  = 0 ;
	long type3 = 0 ;
	int  i;
	static long count = 0 ;
	long my_first = -1 ;

  count = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id1, &col1, &type1);
 	fdbWhatCol(&InputTab[tab_id], col_id2, &col2, &type2);
 	fdbWhatCol(&InputTab[tab_id], col_id3, &col3, &type3);

#ifdef DEVEL
  if ((type1 != type2)||(type2 != type3)) {return(0);} /* something is bad - different data types */
#endif

	for (i=0; i<inputData.intfld.len[col1]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if ((inputData.intfld.data[col1][i] == val1)&&(inputData.intfld.data[col2][i] == val2)&&(inputData.intfld.data[col3][i] == val3))
		{
			count++;
			if (my_first == -1) {my_first = i ;}
		}
	}

	if (first != NULL)
	{
		*first = my_first ;
	}

	return(count);
}

/** Computes number of "val" values in input data collumn
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param val value to be found
 * @param first pointer to position of first found value (result)
 * @return number of values
 */
long fdbInputCountDbl(long tab_id, long col_id, double val, long *first)
{
	long col  = 0 ;
	long type = 0 ;
	int  i;
	static long count = 0 ;
	long my_first = -1 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	for (i=0; i<inputData.dblfld.len[col]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if (inputData.dblfld.data[col][i] == val)
		{
			count++;
			if (my_first == -1) {my_first = i ;}
		}
	}

	if (first != NULL)
	{
		*first = my_first ;
	}

	return(count);
}


/** Return next location of  of "val"
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param previous previous "val" location (searching strats from "previous+1")
 * @param val value to be found
 * @return
 */
long fdbInputFindNextInt(long tab_id, long col_id, long previous, long val)
{
	long col  = 0 ;
	long type = 0 ;
	int  i;
	static long my_pos = 0 ;

	my_pos = previous ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	for (i=(previous+1); i<inputData.intfld.len[col]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if (inputData.intfld.data[col][i] == val)
		{
			my_pos = i ;
			return(my_pos);
		}
	}

	return(my_pos);
}

/** Return next location of  of "val"
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param previous previous "val" location (searching strats from "previous+1")
 * @param val value to be found
 * @return
 */
long fdbInputFindNextDbl(long tab_id, long col_id, long previous, double val)
{
	long col  = 0 ;
	long type = 0 ;
	int  i;
	static long my_pos = 0 ;

	my_pos = previous ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	for (i=(previous+1); i<inputData.dblfld.len[col]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if (inputData.dblfld.data[col][i] == val)
		{
			my_pos = i ;
			return(my_pos);
		}
	}

	return(my_pos);
}

/** Finds max. integer value in collumn (really searches data)
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param val value to be found
 * @param first pointer to position of first found value (result)
 * @return value (or zero)
 */
long fdbInputFindMaxIntSlow(long tab_id, long col_id)
{
	long col  = 0 ;
	long type = 0 ;
	int  i;
	long my_max = 0 ;

  my_max = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

  if (inputData.intfld.len[col] < 1) { return(0); }

	for (i=0; i<inputData.intfld.len[col]; i++)
	{
		if (inputData.intfld.data[col][i] > my_max)
		{
			my_max = inputData.intfld.data[col][i] ;
		}
	}

	return(my_max);
}

/** Finds max. integer value in collumn (assumes that latest value is highest)
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param val value to be found
 * @param first pointer to position of first found value (result)
 * @return value (or zero)
 */
long fdbInputFindMaxInt(long tab_id, long col_id)
{
	long col  = 0 ;
	long type = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

  if (inputData.intfld.len[col] < 1) 
	{ 
		return(0); 
	}
	else
	{
		return(inputData.intfld.data[col][inputData.intfld.len[col]-1]); 
	}
}

/** Finds max. integer value in collumn
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param val value to be found
 * @param first pointer to position of first found value (result)
 * @return value (or zero)
 */
long fdbInputFindMaxIntSel(long tab_id, long col_id)
{
	long col  = 0 ;
	long type = 0 ;
	int  i;
	long my_max = 0 ;

  my_max = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

  if (inputData.intfld.len[col] < 1) { return(0); }

	for (i=0; i<inputData.intfld.len[col]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if (inputData.intfld.data[col][i] > my_max)
		{
			my_max = inputData.intfld.data[col][i] ;
		}
	}

	return(my_max);
}




/** Returnes specified value
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param pos position (row/line) of value
 * @return value (or 0)
 */
long fdbInputGetInt(long tab_id, long col_id, long pos)
{
	long col, type ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	if (pos <inputData.intfld.len[col])
	{
		return(inputData.intfld.data[col][pos]);
	}
	else
	{
		return(0);
	}
}

/** Returnes specified value
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param pos position (row/line) of value
 * @return value (or 0)
 */
double fdbInputGetDbl(long tab_id, long col_id, long pos)
{
	long col, type ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	/*printf("tab=%li col=%li (%li)  type=%li | pos=%li\n",tab_id, col, col_id, type, pos);*/

	if (pos <inputData.dblfld.len[col])
	{
		return(inputData.dblfld.data[col][pos]);
	}
	else
	{
		return(0);
	}
}


/** Sets specified value
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param pos position (row/line) of value
 * @param val value to be set
 * @return status
 */
int fdbInputPutInt(long tab_id, long col_id, long pos, long val)
{
	long col, type ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	if (pos <inputData.intfld.len[col])
	{
		inputData.intfld.data[col][pos] = val ;
		return(AF_OK);
	}
	else
	{
		return(AF_ERR_VAL);;
	}
}

/** Sets specified value
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param pos position (row/line) of value
 * @param val value to be set
 * @return status
 */
int fdbInputPutDbl(long tab_id, long col_id, long pos, double val)
{
	long col, type ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	if (pos <inputData.dblfld.len[col])
	{
		inputData.dblfld.data[col][pos] = val ;
		return(AF_OK);
	}
	else
	{
		return(AF_ERR_VAL);
	}
}

/** Finds max. double value in collumn
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param val value to be found
 * @param first pointer to position of first found value (result)
 * @return value (or zero)
 */
double fdbInputFindMaxDbl(long tab_id, long col_id)
{
	long col  = 0 ;
	long type = 0 ;
	int  i;
	double my_max = 0.0 ;

  my_max = 0.0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

  if (inputData.dblfld.len[col] < 1) {return(0.0);}

	for (i=0; i<inputData.dblfld.len[col]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if (inputData.dblfld.data[col][i] > my_max)
		{
			my_max = inputData.dblfld.data[col][i] ;
		}
	}

	return(my_max);
}

/** Finds min. double value in collumn
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param val value to be found
 * @param first pointer to position of first found value (result)
 * @return value (or zero)
 */
double fdbInputFindMinDbl(long tab_id, long col_id)
{
	long col  = 0 ;
	long type = 0 ;
	int  i;
	double my_max = 0 ;

 	fdbWhatCol(&InputTab[tab_id], col_id, &col, &type);

	for (i=0; i<inputData.dblfld.len[col]; i++)
	{
		if ( fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i) != AF_YES) 
		{
			continue ;
		}
		if (inputData.dblfld.data[col][i] < my_max)
		{
			my_max = inputData.dblfld.data[col][i] ;
		}
	}

	return(my_max);
}

/** Prints line of table
 * @param fw pointer to file
 * @param tab_id table identifier (NODE,..)
 * @param col_id collumn identifier (NODE_ID,..)
 * @param print_mode mode of print
 * @return status
 */
int fdbInputPrintTabLine(FILE *fw, long tab_id, long row, long print_mode)
{
	return(fdbPrintTabLine( fw, 
			   									&InputTab[tab_id],
			   									&inputData.intfld,
			   									&inputData.dblfld,
			   									tab_id, row, print_mode));
}

/* Returnes number of selected items in table tab_id
 * @param tab_id id of table
 * @return number of selected items
 */
long fdbInputTabLenSel(long tab_id)
{
	long        len ;
	long        i;
	long        len0 ;

	len = 0 ;
	len0 = 0 ;

	if (InputTab[tab_id].type[0] == FDB_INT)
	{
		len0 = inputData.intfld.len[InputTab[tab_id].col[0]] ; 
	}
	if (InputTab[tab_id].type[0] == FDB_DBL)
	{
		len0 = inputData.dblfld.len[InputTab[tab_id].col[0]] ; 
	}

	len = 0 ;

	for (i=0; i<len0; i++)
	{
		if (fdbInputTestSelect(tab_id, i) == AF_YES) { len++ ; }
	}

	return(len);
}

/* Returnes number of all items in table tab_id
 * @param tab_id id of table
 * @return number of items
 */
long fdbInputTabLenAll(long tab_id)
{
	static long len ;

	len = 0 ;

	if (InputTab[tab_id].type[0] == FDB_INT)
	{
		len = inputData.intfld.len[InputTab[tab_id].col[0]] ; 
	}
	if (InputTab[tab_id].type[0] == FDB_DBL)
	{
		len = inputData.dblfld.len[InputTab[tab_id].col[0]] ; 
	}

	return(len);
}


/** Saves FEM data to file (no backup is provided)
 * @param fname name of file
 * @return status
 */
int fdbInputSaveData(char *fname)
{
	int   rv = AF_OK ;
	FILE *fw = NULL ;

  /** setting up indexes */
  fdbInputSyncStats();

  fdbInputRenumFromFlds(ELEM, ELEM_FROM, ELEM_ID, ENODE, ENODE_ELEM);  
  fdbInputRenumFromFlds(ELOAD, ELOAD_FROM, ELOAD_ID, ELVAL, ELVAL_ELID);


	if ((fw=fopen(fname, "w")) == NULL)
	{
		fprintf(msgout, "[E] %s \"%s\"!\n", _("Cannot open file for writing"), fname) ;
		return(AF_ERR_IO) ;
	}

	if ((rv = dbWriteTable(fw, InputTabLen, InputTab, inputData.intfld, inputData.dblfld)) != AF_OK)
	{
		fprintf(msgout, "[E] %s!\n", _("Writing of data failed")) ;
	}

	if ((fclose(fw)) != 0)
	{
		fprintf(msgout, "[E] %s \"%s\"!\n", _("Cannot close file"), fname) ;
		return(AF_ERR_IO);
	}

	return(rv);
}


/** Deletes/cleares data in memory
 * @return status
 */
int fdbInputCleanData(void)
{
  fdbInputResetStats();
  fdbIntSetEmpty(&inputData.intfld);
  fdbDblSetEmpty(&inputData.dblfld);
	return(AF_OK);
}

/** Reads FEM data from file (no backup is provided)
 * @param fname name of file
 * @return status
 */
int fdbInputReadData(char *fname)
{
	int   rv = AF_OK ;
	FILE *fr = NULL ;

  /* reseting of index status */
  fdbInputResetStats();

	if ((fr=fopen(fname, "r")) == NULL)
	{
		fprintf(msgout, "[E] %s \"%s\"!\n", _("Cannot open file for reading"), fname) ;
		return(AF_ERR_IO) ;
	}

	if ((rv = dbReadTable(fr, InputTabLen, InputTab, &inputData.intfld, &inputData.dblfld)) != AF_OK)
	{
		fprintf(msgout, "[E] %s!\n", _("Reading of data failed")) ;
	}

	if ((fclose(fr)) != 0)
	{
		fprintf(msgout, "[E] %s \"%s\"!\n", _("Cannot close file"), fname) ;
		return(AF_ERR_IO);
	}

	return(rv);
}

void fdbInputListSumm(int header, FILE *fw)
{	
	if (header == AF_YES)
	{
		fprintf(fw, "\n    %s:\n\n",_("Model Summary"));
	}

	fprintf(fw," %20s: %10li  (%li)\n",_("Element Types"), fdbInputTabLenSel(ETYPE), fdbInputTabLenAll(ETYPE)) ;
	fprintf(fw," %20s: %10li  (%li)\n",_("Real Sets"), fdbInputTabLenSel(RSET), fdbInputTabLenAll(RSET)) ;
	fprintf(fw," %20s: %10li  (%li)\n",_("Material Types"), fdbInputTabLenSel(MAT), fdbInputTabLenAll(MAT)) ;

	fprintf(fw," %20s: %10li  (%li)\n",_("Keypoints"), fdbInputTabLenSel(KPOINT), fdbInputTabLenAll(KPOINT)) ;
	fprintf(fw," %20s: %10li  (%li)\n",_("Geom. Entities"), fdbInputTabLenSel(ENTITY), fdbInputTabLenAll(ENTITY)) ;

	fprintf(fw," %20s: %10li  (%li)\n",_("Nodes"), fdbInputTabLenSel(NODE), fdbInputTabLenAll(NODE)) ;
	fprintf(fw," %20s: %10li  (%li)\n",_("Elements"), fdbInputTabLenSel(ELEM), fdbInputTabLenAll(ELEM)) ;
	fprintf(fw," %20s: %10li  (%li)\n",_("Displacements"), fdbInputTabLenSel(NDISP), fdbInputTabLenAll(NDISP)) ;
	fprintf(fw," %20s: %10li  (%li)\n",_("Nodal Loads"), fdbInputTabLenSel(NLOAD), fdbInputTabLenAll(NLOAD)) ;
	fprintf(fw," %20s: %10li  (%li)\n",_("Element Loads"), fdbInputTabLenSel(ELOAD), fdbInputTabLenAll(ELOAD)) ;
}

/** Prints fem data summary: */
int fdbInputListSummPrn(FILE *fw)
{	
  int  rv = AF_OK ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 3, _("Model Summary")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Entity")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Selected")) ;
  fdbPrnTableHeaderItem(fw, 2, _("All")) ;

	fdbPrnTableItemStr(fw,0,_("Elem. Types"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(ETYPE));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(ETYPE));

	fdbPrnTableItemStr(fw,0,_("Real Sets"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(RSET));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(RSET));
  
	fdbPrnTableItemStr(fw,0,_("Materials"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(MAT));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(MAT));

  
	fdbPrnTableItemStr(fw,0,_("Keypoints"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(KPOINT));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(KPOINT));
  
	fdbPrnTableItemStr(fw,0,_("Geom. Ents"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(ENTITY));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(ENTITY));


	fdbPrnTableItemStr(fw,0,_("Nodes"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(NODE));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(NODE));
  
	fdbPrnTableItemStr(fw,0,_("Elements"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(ELEM));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(ELEM));
	
  fdbPrnTableItemStr(fw,0,_("Nodal Disps"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(NDISP));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(NDISP));
	
  fdbPrnTableItemStr(fw,0,_("Nodal Loads"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(NLOAD));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(NLOAD));
	
  fdbPrnTableItemStr(fw,0,_("Elem. Loads"));
  fdbPrnTableItemInt(fw,1,fdbInputTabLenSel(ELOAD));
  fdbPrnTableItemInt(fw,2,fdbInputTabLenAll(ELOAD));

  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);

  return(rv);
}


/* SELECTIONS -------------------------------------  **/

/** Selects nodes by elements */
int fem_sele_nsle(void)
{
  int rv = AF_OK ;
  long elen, nlen;
  long i,j ;
  long selcol, type;

  elen = fdbInputTabLenAll(ELEM) ;
  nlen = fdbInputTabLenAll(NODE) ;

  fdbWhatCol(&InputTab[NODE], InputTab[NODE].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<nlen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    if (fdbInputTestSelect(ELEM, i) == AF_YES)
    {
      for (j=0; j<fdbInputGetInt(ELEM, ELEM_NODES,i); j++)
      {
        inputData.intfld.data[selcol][fdbEnodePos(i,j)] = AF_YES ;
      }
    }
  }

  return(rv);
}

/** Selects nodes by displacements */
int fem_sele_nsld(void)
{
  int rv = AF_OK ;
  long elen, nlen;
  long i ;
  long selcol, type;

  elen = fdbInputTabLenAll(NDISP) ;
  nlen = fdbInputTabLenAll(NODE) ;

  fdbWhatCol(&InputTab[NODE], InputTab[NODE].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<nlen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    if (fdbInputTestSelect(NDISP, i) == AF_YES)
    {
      inputData.intfld.data[selcol][fdbNdispNodePos(i)] = AF_YES ;
    }
  }

  return(rv);
}

/** Selects nodes by forces */
int fem_sele_nslf(void)
{
  int rv = AF_OK ;
  long elen, nlen;
  long i ;
  long selcol, type;

  elen = fdbInputTabLenAll(NLOAD) ;
  nlen = fdbInputTabLenAll(NODE) ;

  fdbWhatCol(&InputTab[NODE], InputTab[NODE].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<nlen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    if (fdbInputTestSelect(NLOAD, i) == AF_YES)
    {
      inputData.intfld.data[selcol][fdbNloadNodePos(i)] = AF_YES ;
    }
  }

  return(rv);
}

/** Selects elements by element loads */
int fem_sele_eslel(void)
{
  int rv = AF_OK ;
  long elen, nlen;
  long i ;
  long selcol, type;

  elen = fdbInputTabLenAll(ELOAD) ;
  nlen = fdbInputTabLenAll(ELEM) ;

  fdbWhatCol(&InputTab[ELEM], InputTab[ELEM].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<nlen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    if (fdbInputTestSelect(ELOAD, i) == AF_YES)
    {
      inputData.intfld.data[selcol][fdbEloadElemPos(i)] = AF_YES ;
    }
  }

  return(rv);
}

/** Selects elements load by elements */
int fem_sele_elsle(void)
{
  int rv = AF_OK ;
  long elen;
  long i ;
  long pos;
  long selcol, type;

  elen = fdbInputTabLenAll(ELOAD) ;

  fdbWhatCol(&InputTab[ELOAD], InputTab[ELOAD].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<elen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    pos = fdbEloadElemPos(i) ;
    if (fdbInputTestSelect(ELEM, pos) == AF_YES)
    {
      inputData.intfld.data[selcol][i] = AF_YES ;
    }
  }

  return(rv);
}

/** Selects forces by nodes */
int fem_sele_fsln(void)
{
  int rv = AF_OK ;
  long elen;
  long i ;
  long pos;
  long selcol, type;

  elen = fdbInputTabLenAll(NLOAD) ;

  fdbWhatCol(&InputTab[NLOAD], InputTab[NLOAD].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<elen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    pos = fdbNloadNodePos(i) ;
    if (fdbInputTestSelect(NODE, pos) == AF_YES)
    {
      inputData.intfld.data[selcol][i] = AF_YES ;
    }
  }

  return(rv);
}

/** Selects displacements by nodes */
int fem_sele_dsln(void)
{
  int rv = AF_OK ;
  long elen;
  long i ;
  long pos;
  long selcol, type;

  elen = fdbInputTabLenAll(NDISP) ;

  fdbWhatCol(&InputTab[NDISP], InputTab[NDISP].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<elen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    pos = fdbNdispNodePos(i) ;
    if (fdbInputTestSelect(NODE, pos) == AF_YES)
    {
      inputData.intfld.data[selcol][i] = AF_YES ;
    }
  }

  return(rv);
}

/** Selects elements by nodes (some nodes can remain unused) */ 
int fem_sele_esln(void)
{
  int rv = AF_OK ;
  long elen;
  long i,j ;
  long nodes, count ;
  long pos;
  long selcol, type;

  elen = fdbInputTabLenAll(ELEM) ;

  fdbWhatCol(&InputTab[ELEM], InputTab[ELEM].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<elen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    nodes = fdbInputGetInt(ELEM, ELEM_NODES, i);
    count = 0 ;

    for (j=0; j<nodes; j++)
    {
      pos = fdbEnodePos(i,j) ;
      if (fdbInputTestSelect(NODE, pos) == AF_YES)
      {
        count++;
      }
    }

    /* all elemnt's nodes must be selected */
    if (count == nodes)
    {
      inputData.intfld.data[selcol][i] = AF_YES ;
    }
  }

  return(rv);
}

/** Selects keypoints by entities */
int fem_sele_kslge(void)
{
  int rv = AF_OK ;
  long elen, nlen;
  long i,j ;
  long selcol, type;

  elen = fdbInputTabLenAll(ENTITY) ;
  nlen = fdbInputTabLenAll(KPOINT) ;

  fdbWhatCol(&InputTab[KPOINT], InputTab[KPOINT].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<nlen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    if (fdbInputTestSelect(ENTITY, i) == AF_YES)
    {
      for (j=0; j<fdbInputGetInt(ENTITY, ENTITY_KPS,i); j++)
      {
        inputData.intfld.data[selcol][fdbEntKpPos(i,j)] = AF_YES ;
      }
    }
  }

  return(rv);
}

/** Selects entities by keypoints (some keypoints can remain unused) */ 
int fem_sele_geslk(void)
{
  int rv = AF_OK ;
  long elen;
  long i,j ;
  long nodes, count ;
  long pos;
  long selcol, type;

  elen = fdbInputTabLenAll(ENTITY) ;

  fdbWhatCol(&InputTab[ENTITY], InputTab[ENTITY].selcolnum, &selcol, &type);

  /* unselect all nodes  */
  for (i=0; i<elen; i++)
  {
    inputData.intfld.data[selcol][i] = AF_NO ;
  }

  for (i=0; i<elen; i++)
  {
    nodes = fdbInputGetInt(ENTITY, ENTITY_KPS, i);
    count = 0 ;

    for (j=0; j<nodes; j++)
    {
      pos = fdbEnodePos(i,j) ;
      if (fdbInputTestSelect(KPOINT, pos) == AF_YES)
      {
        count++;
      }
    }

    /* all elemnt's nodes must be selected */
    if (count == nodes)
    {
      inputData.intfld.data[selcol][i] = AF_YES ;
    }
  }

  return(rv);
}


/** Creates string (comma separated values) from table collumn data */
char *femInputStringFromFldInt(long tab, long col, char *varGrp)
{
	char  num[1024];
	char *tmp = NULL ;
	long  len = 0 ;
	long  i,j ;

	len = fdbInputTabLenSel(tab) + 10 ;

	for (i=0; i<fdbInputTabLenAll(tab); i++)
	{
		if (fdbInputTestSelect(tab, i) != AF_YES) {continue;}
		for (j=0; j<1024; j++) { num[j] = '\0' ; }

		sprintf(num, "%li", fdbInputGetInt(tab,col,i));

		if (varGrp == NULL)
		{ len += strlen(num); }
		else
		{
			if (ciGetVarValFromGrp(num,varGrp) != NULL)
			{
				len += strlen(ciGetVarNameFromGrp(num,varGrp)); 
			}
			else
			{
				len += strlen(num);
			}
		}
	}
	
	if ((tmp = (char *)malloc((len+1)*sizeof(char))) == NULL)
	{
		return(" ");
	}

	for (i=0; i<=len; i++) {tmp[i] = '\0';}

	for (i=0; i<fdbInputTabLenAll(tab); i++)
	{
		if (fdbInputTestSelect(tab, i) != AF_YES) {continue;}
		for (j=0; j<1024; j++) { num[j] = '\0' ; }

		if (i > 0) { strcat(tmp,",");}

		sprintf(num, "%li", fdbInputGetInt(tab,col,i));

		if (varGrp == NULL)
		{ 
			strcat(tmp,(num)); 
		}
		else
		{ 
			if (ciGetVarNameFromGrp(num,varGrp) != NULL)
			{
				strcat(tmp,ciGetVarValFromGrp(num,varGrp)); 
			}
			else
			{
				strcat(tmp,(num)); 
			}
		}
	}
	return(tmp);
}

/** Creates string (comma separated values) from DBL table collumn data */
char *femInputStringFromFldDbl(long tab, long col)
{
	char  num[1024];
	char *tmp = NULL ;
	long  len = 0 ;
	long  i,j ;

	len = fdbInputTabLenSel(tab) + 10 ;

	for (i=0; i<fdbInputTabLenAll(tab); i++)
	{
		if (fdbInputTestSelect(tab, i) != AF_YES) {continue;}
		for (j=0; j<1024; j++) { num[j] = '\0' ; }

		sprintf(num, "%e", fdbInputGetDbl(tab,col,i));
		len += strlen(num);
	}
	
	if ((tmp = (char *)malloc((len+1)*sizeof(char))) == NULL)
	{
		return(" ");
	}

	for (i=0; i<=len; i++) {tmp[i] = '\0';}

	for (i=0; i<fdbInputTabLenAll(tab); i++)
	{
		if (fdbInputTestSelect(tab, i) != AF_YES) {continue;}
		for (j=0; j<1024; j++) { num[j] = '\0' ; }

		if (i > 0) { strcat(tmp,",");}

		sprintf(num, "%e", fdbInputGetDbl(tab,col,i));
		strcat(tmp,(num)); 
	}
	return(tmp);
}

/** Makes STR from INT */
char *fdbFemStrFromInt(long number)
{
	char  snum[1024];
	char *tmp = NULL ;
	long i ;
	long len = 0 ;

	for (i=0; i<1024; i++) { snum[i] = '\0' ; }
	sprintf(snum,"%li", number);

	if((len = strlen(snum)) <= 0) { return(" "); }

	if ((tmp = (char *)malloc((len+1)*sizeof(char))) == NULL)
	{ return(" "); }

	for (i=0; i<=len; i++) { tmp[i] = '\0' ; }

	strcpy(tmp, snum);

	return(tmp);
}

/** Makes STR from DBL */
char *fdbFemStrFromDbl(double number)
{
	char  snum[1024];
	char *tmp = NULL ;
	long i ;
	long len = 0 ;

	for (i=0; i<1024; i++) { snum[i] = '\0' ; }
	sprintf(snum,"%e", number);

	if((len = strlen(snum)) <= 0) { return(" "); }

	if ((tmp = (char *)malloc((len+1)*sizeof(char))) == NULL)
	{ return(" "); }

	for (i=0; i<=len; i++) { tmp[i] = '\0' ; }

	strcpy(tmp, snum);

	return(tmp);
}


/* Makes command separated string for possible results */
char *fdbFemStrEResListInt(void)
{
  long restypes[270];
  long len = 270 ;
  long restypes_ret[270];
  long len_ret = 0 ;
  long type ;
  long i, j, k ;

  for (i=0; i<len; i++) 
  { 
    restypes[i] = 0 ; 
    restypes_ret[i] = 0 ; 
  }
  len_ret = 0 ;

  for (i=0; i<len; i++)
  {
    restypes[i] = 0 ;

    for (j=0; j<fdbInputTabLenAll(ETYPE); j++ ) /* type=0 is not used */
    {
      type = fdbInputGetInt(ETYPE,ETYPE_TYPE, j) ;
      if ((type<=0) || (type >=fdbElementTypeLen)) { break ; }
      
      for (k=0; k<fdbElementType[type].res; k++)
      {
        if ((i+1) == fdbElementType[type].nres[k])
        {
          restypes[i] = 1 ;
          break ;
        }
      }
      if (restypes[i] == 1) {break;}
      
      for (k=0; k<fdbElementType[type].res_rp; k++)
      {
        if ((i+1) == fdbElementType[type].nres_rp[k])
        {
          restypes[i] = 1 ;
          break ;
        }
      }
      if (restypes[i] == 1) {break;}
    }
  }
  for (i=0; i<len; i++)
  {
    if (restypes[i] == 1)
    {
      restypes_ret[len_ret] = (i+1) ;
      len_ret++ ;
    }
  }

  if (len_ret < 1) { return(" ") ; } 

  return(ciListVarsListedCSV("result", restypes_ret, len_ret)) ;
}


/* Makes command separated string for possible dispalcement/force * types */
char *fdbFemStrSuppForcTypListInt(char *dtype)
{
  long restypes[270];
  long len = 270 ;
  long restypes_ret[270];
  long len_ret = 0 ;
  ldiv_t divval ;
  ldiv_t divval2 ;
  long type ;
  long i, j, k ;

  for (i=0; i<len; i++) 
  { 
    restypes[i] = 0 ; 
    restypes_ret[i] = 0 ; 
  }
  len_ret = 0 ;


  for (i=0; i<len; i++)
  {
    restypes[i] = 0 ;

    for (j=0; j<fdbInputTabLenAll(ETYPE); j++ ) /* type=0 is not used */
    {
      type = fdbInputGetInt(ETYPE,ETYPE_TYPE, j) ;
      if ((type<=0) || (type >=fdbElementTypeLen)) { break ; }
      
      for (k=0; k<fdbElementType[type].ndofs; k++)
      {
        divval  = ldiv(i+1,KNOWN_DOFS);
        divval2 = ldiv(fdbElementType[type].ndof[k],KNOWN_DOFS);

        if (
            (divval.rem == fdbElementType[type].ndof[k])
          ||
          ((divval2.quot >= 1) && (divval2.rem == 0) && (divval.rem == 0))
           )
        {
          restypes[i] = 1 ;
          break ;
        }
      }
      if (restypes[i] == 1) {break;}
    }
  }

  for (i=0; i<len; i++)
  {
    if (restypes[i] == 1)
    {
      restypes_ret[len_ret] = (i+1) ;
      len_ret++ ;
    }
  }

  if (len_ret < 1) { return(" ") ; } 

  return(ciListVarsListedCSV(dtype, restypes_ret, len_ret)) ;
}


/* Makes command separated string for possible displacement types */
char *fdbFemStrSuppTypListInt(void)
{
  return(fdbFemStrSuppForcTypListInt("disp")) ;
}


/* Makes command separated string for possible force types */
char *fdbFemStrNLoadTypListInt(void)
{
  return(fdbFemStrSuppForcTypListInt("load")) ;
}

/* end of fdb_fem.c */
