/*
   File name: fdb_accs.c
   Date:      2003/08/03 15:22
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

	 Database for FEM - data access

	 $Id: fdb_accs.c,v 1.3 2003/11/28 19:15:32 jirka Exp $
*/

#include "fdb.h"


/* INTEGER DATA: */



/** Changes values (in field) on any possition
 * @param fld field
 * @param pos index of modified row
 * @param from starting point
 * @param count number of values
 * @param val added values (FIELD!)
 * @param add if AF_YES then values are added else replaced
 * @return status
 */
int fdbIntChangeVals(tInt *fld, long pos, long from, long count, long *val, int add)
{
	int   rv = AF_OK ;
	long  len = 0 ;
	int   i ;

	len = fld->len[pos] ;

	if ((from+count) > len) { return (AF_ERR_SIZ) ; }

	for (i=from; i<(from+count); i++) 
	{
		if (add == AF_YES) { fld->data[pos][i] += val[i-from] ; }
		else               { fld->data[pos][i] = val[i-from] ; }
	}

	return (rv) ;
}

/* FLOATING POINT DATA: */


/** Changes values (in field) on any possition
 * @param fld field
 * @param pos index of modified row
 * @param from starting point
 * @param count number of values
 * @param val added values
 * @param add if AF_YES then values are added else replaced
 * @return status
 */
int fdbDblChangeVals(tDbl *fld, long pos, long from, long count, double *val, int add)
{
	int   rv = AF_OK ;
	long  len = 0 ;
	int   i ;

	len = fld->len[pos] ;

	if ((from+count) > len) { return (AF_ERR_SIZ) ; }

	for (i=from; i<(from+count); i++) 
	{
		if (add == AF_YES) { fld->data[pos][i] += val[i-from] ; }
		else               { fld->data[pos][i] = val[i-from] ; }
	}

	return (rv) ;
}



/* ------- TABLE-LEVEL OPERATIONS -------------------------------- */


/* In fact, I only need few functions:       ###################################
 *
 * 1) append (whole) table     => 1 function
 * 2) insert rows to table     => 1 function
 * 3) delete rows              => 1 function
 * 4) change items (add & put) => 4 funcions
 *
 */

/** Appends table. New space is filled with zeros
 * @param tab table
 * @param dataI tInt data storage
 * @param dataD tDbl data storage
 * @param count number of added data
 * @param index index of the first added row
 * @return status
 */
int fdbAppendTableRow(tTab *tab, tInt *dataI, tDbl *dataD, long count, long *index)
{
	int rv = AF_OK ;
	int i ;
	long len ;

	len = count + dataI->len[tab->col[0]] ;

	if (index != NULL) { *index = len - count ; }

	for (i=0; i<tab->cols; i++)
	{
	  switch (tab->type[i])
		{
			case FDB_INT:	
						if ((rv = fdbIntReAlloc(dataI, tab->col[i], len)) != AF_OK)
							 { goto memFree; }
						break;
			case FDB_DBL:	
						if ((rv = fdbDblReAlloc(dataD, tab->col[i], len)) != AF_OK)
							 { goto memFree; }
						break;
		}
	}

memFree:
	return(rv);
}
 
/** Inserts lines to table. New space is filled with zeros
 * @param tab table
 * @param dataI tInt data storage
 * @param dataD tDbl data storage
 * @param from starting point (row index) to insert
 * @param count number of added data
 * @return status
 */
int fdbInsertTableRow(tTab *tab, tInt *dataI, tDbl *dataD, long from, long count)
{
	int rv = AF_OK ;
	int    i ;

	for (i=0; i<tab->cols; i++)
	{
	  switch (tab->type[i])
		{
			case FDB_INT:	
						if ((rv = fdbIntInsertZero(dataI, tab->col[i], from, count)) != AF_OK)
							 { goto memFree; }
						break;
			case FDB_DBL:	
						if ((rv = fdbDblInsertZero(dataD, tab->col[i], from, count)) != AF_OK)
							 { goto memFree; }
						break;
		}
	}

memFree:
	return(rv);
}

/** Removes lines from table. New space is filled with zeros
 * @param tab table
 * @param dataI tInt data storage
 * @param dataD tDbl data storage
 * @param from starting point (row index) to insert
 * @param count number of added data
 * @return status
 */
int fdbRemoveTableRow(tTab *tab, tInt *dataI, tDbl *dataD, long from, long count)
{
	int rv = AF_OK ;
	int    i ;

	for (i=0; i<tab->cols; i++)
	{
	  switch (tab->type[i])
		{
			case FDB_INT:	
						if ((rv = fdbIntRemove(dataI, tab->col[i], from, count)) != AF_OK)
							 { goto memFree; }
						break;
			case FDB_DBL:	
						if ((rv = fdbDblRemove(dataD, tab->col[i], from, count)) != AF_OK)
							 { goto memFree; }
						break;
		}
	}

memFree:
	return(rv);
}

/* Finding of value(s)  ---------- */

#ifdef _USE_THREADS_
void *thFdbCountInt (void *param)
{
  tFdbThr *p = (tFdbThr*) param ;
	long     count = 0 ;
	long     myfirst = -1 ;
	int      i ;

	for (i=p->from; i<p->to; i++)
	{
		if (p->idata[i] == p->ival)
		{
			count++ ;
			if (myfirst == -1) {myfirst = i ;}
		}
	}
  p->ipos  = myfirst ;
  p->count = count ;

  return(NULL);
}
#endif

/** Computes number of "val" values in INTEGER field
 * @param searched field
 * @param length of field
 * @param val value to search
 * @param position of first value (pointer)
 * @return number of found values (0 if none) 
 */
long fdbCountInt(long *data, long dataLen, long val, long *first)
{
	static long count = 0 ;
	long        myfirst = -1 ;
	int         i ;
#ifdef _USE_THREADS_
  tFdbThr data1 ;
  tFdbThr datat[AF_MAX_THREADS];
  ldiv_t  dnum ;
  pthread_t Thread_ID[AF_MAX_THREADS];
  pthread_t Thread_1  ;

  
  if (dataLen < (femCmdNumProcUI*100))
  {
    data1.from = 0; 
    data1.to   = dataLen ;
    data1.idata = data ;
    data1.ival  = val ;
    data1.ipos  =  -1 ;
    data1.count =  0 ;

    pthread_create(&Thread_1,NULL,thFdbCountInt,&data1);
    pthread_join(Thread_1, NULL) ;
    myfirst = data1.ipos ;
    count   = data1.count ;
  }
  else
  {
    dnum = ldiv(dataLen, femCmdNumProcUI) ;
    if (dnum.quot < 1)
    {
      dnum.quot = 0 ;
      dnum.rem  = dataLen ;
    }

    for (i=0; i<femCmdNumProcUI; i++)
    {
      datat[i].from = i*dnum.quot ;
      datat[i].to   =  (i+1)*dnum.quot ;
      datat[i].idata = data ;
      datat[i].ival  = val ;
      datat[i].ipos  =  -1 ;
      datat[i].count =  0 ;

      if (i == (femCmdNumProcUI-1)) { datat[i].to += dnum.rem ; }

      pthread_create(&Thread_ID[i],NULL,thFdbCountInt,&datat[i]);
    }

    for (i=0; i<femCmdNumProcUI; i++)
    {
      pthread_join(Thread_ID[i], NULL) ;
    }

    count = 0 ;

    for (i=0; i<femCmdNumProcUI; i++)
    {
      if (datat[i].ipos != -1)
      {
        count += datat[i].count ;

        if (myfirst == -1) 
        {
          myfirst = datat[i].ipos ;
        }
        else
        {
          if (myfirst > datat[i].ipos) {myfirst = datat[i].ipos;}
        }
      }
    }
  }
#else /* original (working) sequential code: */
	for (i=0; i<dataLen; i++)
	{
		if (data[i] == val)
		{
			count++ ;
			if (myfirst == -1) {myfirst = i ;}
		}
	}
#endif

	if (first != NULL) { *first = myfirst ; }

	return(count);
}


/** Finds next position of "val" value in field
 * @param data searched field
 * @param dataLen length of field
 * @param previous position of previous "val" (e.g. -1 if first is wanted)
 * @param val searched falue
 * @return position of value (-1 if not found)
 */
long fdbFindNextInt(long *data, long dataLen, long previous, long val)
{
	static long pos = -1 ;
	int i;

	for (i=(previous+1); i< dataLen; i++)
	{
		if (data[i] == val)
		{
			pos = i ;
			return(pos);
		}
	}

	return(pos);
}

/** Find maximum in non-negative INT field 
 * @param fld field
 * @param len length of field
 * @return maximum (or 0)
 */
long fdbFindMaxInt(long *fld, long len)
{
	static long max = 0 ;
	int         i ;

	for (i=0; i<len; i++)
	{
		if (max < fld[i])
		{
			max = fld[i] ;
		}
	}

	return(max);
}


/** Computes number of "val" values in DOUBLE field
 * @param searched field
 * @param length of field
 * @param val value to search
 * @param position of first value (pointer)
 * @return mumber of found values (0 if none) 
 */
long fdbCountDbl(double *data, long dataLen, double val, long *first)
{
	static long count = 0 ;
	long        myfirst = -1 ;
	int         i ;

	for (i=0; i<dataLen; i++)
	{
		if (data[i] == val)
		{
			count++ ;
			if (myfirst == -1) {myfirst = i ;}
		}
	}

	if (first != NULL) { *first = myfirst ; }

	return(count);
}

/** Finds next position of "val" value in DOUBLE field
 * @param data searched field
 * @param dataLen length of field
 * @param previous position of previous "val" (e.g. -1 if first is wanted)
 * @param val searched falue
 * @return position of value (-1 if not found)
 */
long fdbFindNextDbl(double *data, long dataLen, long previous, double val)
{
	static long pos = -1 ;
	int i;

	for (i=(previous+1); i< dataLen; i++)
	{
		if (data[i] == val)
		{
			pos = i ;
			return(pos);
		}
	}

	return(pos);
}

/** Founds collumn in tInt/tDbl related to table collumn named "col_id"
 * @param tab pointer to table (single table row)
 * @param col_id name of collumn (N_ID etc.)
 * @param col pointer to collumn in tInt/tDbl (result)
 * @param type type of data (int/dbl) (result) SHOULD NOT BE NULL !!!
 * @return status
 */
int fdbWhatCol(tTab *tab, long col_id, long *col, long *type)
{
	int rv = AF_ERR_VAL ;
	int i ;

	if (type != NULL) {*type = -1 ;}
	*col  =  0 ;

	for (i=0; i<tab->cols; i++)
	{
		if (tab->cid[i] == col_id)
		{
			if (type != NULL) {*type = tab->type[i] ;}
			*col  = tab->col[i]  ;
			 rv   = AF_OK ;
			break ;
		}
	}

	return(rv) ;
}

/** Prints long value */
void fdbPrintInt(FILE *fw, long val, long print_mode)
{
  fprintf(fw, " %10li", val ) ;
}

/** Prints double value */
void fdbPrintDbl(FILE *fw, double val, long print_mode)
{
  fprintf(fw, " %e", val ) ;
}

/** Prints 
 *
 */
int fdbPrintTabLine(FILE *fw, tTab *tab, tInt *intfld, tDbl *dblfld, long tab_id, long row, long print_mode)
{
	int rv = AF_OK ;
	int    i ;

	for (i=0; i<tab->cols; i++)
	{
	  switch (tab->type[i])
		{
			case FDB_INT:	
						if (tab->cid[i] != tab->selcolnum) 
						{
							fdbPrintInt(fw, intfld->data[tab->col[i]][row], print_mode ) ;
						}
						break;
			case FDB_DBL:	
						fdbPrintDbl(fw, dblfld->data[tab->col[i]][row], print_mode ) ;
						break;
		}
	}

	return(rv);
}

/* end of fdb_accs.c */
