/*
   File name: fdb_mem.c
   Date:      2003/08/02 16:35
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

	 Database for FEM - memory handling

	 $Id: fdb_mem.c,v 1.8 2004/11/11 21:41:06 jirka Exp $
*/

#include "fdb.h"


/* TABLE HEADERS: */

/** Sets all table header elements to NULL or 0 */
void fdbTabNull(tTab *tab)
{
	tab->id = 0 ;
	tab->cols = 0 ;
	tab->col = NULL ;
	tab->type = NULL ;
	tab->selcolnum = 0 ;
}

/** Removes table header from memory */
void fdbTabFree(tTab *tab)
{
	free (tab->col  ) ;
	free (tab->type ) ;
	fdbTabNull(tab  ) ;
}

/** Sets collumn numbers and computes lenghts of related tInt and tDbl fields
 * @param tab table
 * @param tabLen lenght of "tab"
 * @param number of integer collumns (result)
 * @param number of floating point collumns (result)
 */
void fdbInitTableColNumbers(tTab *tab, long tabLen, long *iLen, long *dLen)
{
	long i,j;
	long posI = 0 ;
	long posD = 0 ;

	for (i=0; i<tabLen; i++)
	{
		for (j=0; j<tab[i].cols; j++)
		{
			switch (tab[i].type[j])
			{
				case FDB_INT: tab[i].col[j] = posI; posI++; break;
				case FDB_DBL: tab[i].col[j] = posD; posD++; break;
			}
		}
	}

	*iLen = posI ;
	*dLen = posD ;
}

/* INTEGER DATA: ###################################################### */

/** Sets integer field to NULL */
void fdbIntNull(tInt *fld)
{
	fld->rows = 0    ;
	fld->len  = NULL ;
	fld->data = NULL ;
}

/** Removes integer field from memory */
void fdbIntFree(tInt *fld)
{
	int i ;

	for (i=0; i<fld->rows; i++)
	{
		free(fld->data[i]) ;
	}

	free(fld->data) ;
	free(fld->len ) ;

	fdbIntNull(fld);
}

/** Removes data from integer field */
void fdbIntSetEmpty(tInt *fld)
{
	int i ;

	if (fld != NULL) 
	{
		for (i=0; i<fld->rows; i++)
		{
			if (fld->data[i] != NULL) 
			{
				free(fld->data[i]) ;
    		fld->data[i] = NULL ;
    		fld->len[i]  = 0 ;
			}
		}
	}
}

/** Initialization of integer field
 *  e.g. creates lenght and data pointer fields but no space for data
 * @param fld field
 * @param len number of "rows"
 * @return status
 */
int fdbIntInit(tInt *fld, long len)
{
	int rv = AF_OK ;
	long i ;

	fdbIntNull(fld) ;

	if ((fld->len = femIntAlloc(len)) == NULL) {rv = AF_ERR_MEM ; goto memFree;}
	if ((fld->data = (long **) malloc(len*sizeof(long *))) == NULL) 
	   {rv = AF_ERR_MEM ; goto memFree;}
	fld->rows = len ;
	for (i=0; i<len; i++) {fld->data[i] = NULL ;}

	return (rv) ;
memFree: /* on error: */
	fdbIntNull(fld) ;
	return (rv) ;
}

/** Reallocates  integer field (makes it larger or smaller)
 * @param fld field
 * @param pos index of modified row
 * @param len new lenght
 * @return status
 */
int fdbIntReAlloc(tInt *fld, long pos, long len)
{
	int     rv = AF_OK ;
	long    oldlen = 0 ;
	long    cpylen = 0 ;
	long   *tmp = NULL ;
	int     i ;

	if (len == 0) /* set field to zero lenght */
	{
		if (fld->len[pos] > 0)
		{
		  femIntFree(fld->data[pos]);
		}
		fld->data[pos] = NULL ;
		fld->len[pos] = 0 ;
		return (AF_OK) ;
	}

	/* Nonzero lenght: */
	oldlen = fld->len[pos] ;

	if (len > oldlen) { cpylen = oldlen ; } /* lenght for memcpy */
	else              { cpylen = len ; }

	if ((tmp = femIntAlloc(len)) == NULL) {return(AF_ERR_MEM);}

#if 0
	if (oldlen > 0)
	{
		tmp = (long *) memcpy((long *) tmp, (long *) fld->data[pos], cpylen*sizeof(long));
	}
#else
	for (i=0; i<cpylen; i++) { tmp[i] = fld->data[pos][i] ; }
#endif

	if (oldlen < len)
	{
		for (i= oldlen ; i< len ; i++)
		{
			tmp[i] = 0 ;
		}
	}

	free(fld->data[pos]) ;
	fld->data[pos] = tmp ;
	tmp = NULL ;

	fld->len[pos] = len ;
	return(rv);
}

/** Inserts zeros to field
 * @param fld field
 * @param pos index of modified row
 * @param from starting index
 * @param count number of inserted zeros
 * @return status
 */
int fdbIntInsertZero(tInt *fld, long pos, long from, long count)
{
	int rv = AF_OK ;
	long newlen = 0;
	long oldlen = 0;
	int i ;

	oldlen = fld->len[pos] ;
	newlen = oldlen+count ;

  if ((rv = fdbIntReAlloc(fld, pos, newlen)) != AF_OK) {return(rv);}

	for (i = (oldlen-1) ; i >= from ; i--)
	{
		fld->data[pos][i+count] = fld->data[pos][i] ;
	}

	for (i=from; i< (from+count); i++)
	{
		fld->data[pos][i] = 0 ;
	}

	return (rv) ;
}

/** Removes data from field
 * @param fld field
 * @param pos index of modified row
 * @param from starting index
 * @param count number of inserted zeros
 * @return status
 */
int fdbIntRemove(tInt *fld, long pos, long from, long count)
{
	int rv = AF_OK ;
	long newlen = 0;
	long oldlen = 0;
	long i ;

	oldlen = fld->len[pos] ;
	newlen = oldlen-count ;


	for (i = from ; i < (oldlen-count) ; i++)
	{
		fld->data[pos][i] = fld->data[pos][i+count] ;
	}

	/*for (i = (newlen-1) ; i < oldlen ; i++)*/
	for (i = (newlen) ; i < oldlen ; i++)
	{
		fld->data[pos][i] = 0 ;
	}

  if ((rv = fdbIntReAlloc(fld, pos, newlen)) != AF_OK) { fld->len[pos]=newlen; }

	return (rv) ;
}

/* FLOATING POINT DATA: ###################################################### */

/** Sets floating point field to NULL */
void fdbDblNull(tDbl *fld)
{
	fld->rows = 0    ;
	fld->len  = NULL ;
	fld->data = NULL ;
}

/** Removes floating point field from memory */
void fdbDblFree(tDbl *fld)
{
	int i ;

	for (i=0; i<fld->rows; i++)
	{
		free(fld->data[i]) ;
	}

	free(fld->data) ;
	free(fld->len ) ;

	fdbDblNull(fld);
}

/** Removes data from floating point field */
void fdbDblSetEmpty(tDbl *fld)
{
	int i ;

	for (i=0; i<fld->rows; i++)
	{
		free(fld->data[i]) ;
    fld->data[i] = NULL ;
    fld->len[i]  = 0 ;
	}
}

/** Initialization of floating point
 *  e.g. creates lenght and data pointer fields but no space for data
 * @param fld field
 * @param len number of "rows"
 * @return status
 */
int fdbDblInit(tDbl *fld, long len)
{
	int rv = AF_OK ;
	long i ;

	fdbDblNull(fld) ;

	if ((fld->len = femIntAlloc(len)) == NULL) {rv = AF_ERR_MEM ; goto memFree;}
	if ((fld->data = (double **) malloc(len*sizeof(double *))) == NULL) 
	   {rv = AF_ERR_MEM ; goto memFree;}
	fld->rows = len ;
	for (i=0; i<len; i++) {fld->data[i] = NULL ;}

	return (rv) ;
memFree: /* on error: */
	fdbDblNull(fld) ;
	return (rv) ;
}

/** Reallocates floating point field (makes it larger or smaller)
 * @param fld field
 * @param pos index of modified row
 * @param len new lenght
 * @return status
 */
int fdbDblReAlloc(tDbl *fld, long pos, long len)
{
	int     rv = AF_OK ;
	long    oldlen = 0 ;
	long    cpylen = 0 ;
	double *tmp = NULL ;
	int     i ;

	if (len == 0) /* set field to zero lenght */
	{
			if (fld->len[pos] > 0)
			{
				femDblFree(fld->data[pos]) ;
			}
			fld->data[pos] = NULL ;
			fld->len[pos] = 0 ;
		return (AF_OK) ;
	}

	/* Nonzero lenght: */
	oldlen = fld->len[pos] ;

	if (len > oldlen) { cpylen = oldlen ; } /* lenght for memcpy */
	else              { cpylen = len ; }

	if ((tmp = femDblAlloc(len)) == NULL) {return(AF_ERR_MEM);}

#if 0
	if (oldlen > 0)
	{
		tmp = (double *) memcpy((double *) tmp, (double *) fld->data[pos], cpylen*sizeof(double));
	}
#else
	for (i=0; i<cpylen; i++) { tmp[i] = fld->data[pos][i] ; }
#endif

	if (oldlen < len)
	{
		for (i= oldlen ; i< len ; i++)
		{
			tmp[i] = 0.0 ;
		}
	}

	free(fld->data[pos]) ;
	fld->data[pos] = tmp ;
	tmp = NULL ;

	fld->len[pos] = len ;
	return(rv);
}

/** Inserts zeros to field
 * @param fld field
 * @param pos index of modified row
 * @param from starting index
 * @param count number of inserted zeros
 * @return status
 */
int fdbDblInsertZero(tDbl *fld, long pos, long from, long count)
{
	int rv = AF_OK ;
	long newlen = 0;
	long oldlen = 0;
	int i ;

	oldlen = fld->len[pos] ;
	newlen = oldlen+count ;

  if ((rv = fdbDblReAlloc(fld, pos, newlen)) != AF_OK) {return(rv);}

	for (i = (oldlen-1) ; i >= from ; i--)
	{
		fld->data[pos][i+count] = fld->data[pos][i] ;
	}

	for (i=from; i< (from+count); i++)
	{
		fld->data[pos][i] = 0.0 ;
	}

	return (rv) ;
}

/** Removes data from field
 * @param fld field
 * @param pos index of modified row
 * @param from starting index
 * @param count number of inserted zeros
 * @return status
 */
int fdbDblRemove(tDbl *fld, long pos, long from, long count)
{
	int rv = AF_OK ;
	long newlen = 0;
	long oldlen = 0;
	int i ;

	oldlen = fld->len[pos] ;
	newlen = oldlen-count ;


	for (i = from ; i < (oldlen-count) ; i++)
	{
		fld->data[pos][i] = fld->data[pos][i+count] ;
	}

	/*for (i = (newlen-1) ; i < oldlen ; i++)*/
	for (i = (newlen) ; i < oldlen ; i++)
	{
		fld->data[pos][i] = 0.0 ;
	}

  if ((rv = fdbDblReAlloc(fld, pos, newlen)) != AF_OK) { fld->len[pos]=newlen; }

	return (rv) ;
}

/* end of fdb_mem.c */
