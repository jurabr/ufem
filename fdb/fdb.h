/*
   File name: fdb.h
   Date:      2003/06/16 22:24
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

	 Database for FEM

	 $Id: fdb.h,v 1.16 2004/12/30 22:26:00 jirka Exp $
*/


#ifndef __FDB_H__
#define __FDB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#ifdef _USE_THREADS_ /* POSIX/UNIX THREADS */
#include <pthread.h>
#endif

#include "fem.h"
#include "fem_mem.h"
#include "fem_comm.h"

#define FDB_OK   0
#define FDB_ERR -1

#define FDB_INT 0
#define FDB_DBL 1

#define FDB_NONE -1

#define FDB_SELE_SELECT 1
#define FDB_SELE_RESELE 2
#define FDB_SELE_ADD    3
#define FDB_SELE_UNSELE 4
#define FDB_SELE_INVERT 5
#define FDB_SELE_ALL    6
#define FDB_SELE_NONE   7

#define FDB_COPY        1
#define FDB_MIRROR      2

#define FDB_FORMAT_TEXT 1
#define FDB_FORMAT_LTX  2
#define FDB_FORMAT_HTML 3

#define FDB_CSYS_CART   1
#define FDB_CSYS_CYL_XY 2
#define FDB_CSYS_CYL_YZ 3
#define FDB_CSYS_CYL_ZX 4

#ifdef _USE_THREADS_ /* POSIX/UNIX THREADS */
typedef struct 
{
  long     from  ;
  long     to    ;
  long    *idata ;
  double  *ddata ;
  long     ilen  ;
  long     dlen  ;
  long     ival  ;
  long     ipos  ;
  long     dval  ;
  long     count ;
} tFdbThr ;
#endif

/* abstract table header */
typedef struct
{
	long  id;					/* table identifier   */
	long  cols;				/* number of collumns */
	long *cid;				/* collumn numbers    */
	long *col;				/* indexes in tInt and tDbl    */
	long *type;				/* types of collumns  */
	long  selcolnum;	/* collumn number of selection information */
}tTab;

/* "integer" (actually long) data field */
typedef struct
{
	long   rows;
	long  *len;
	long **data;
} tInt;

/* "double" data field */
typedef struct
{
	long     rows;
	long    *len;
	double **data;
} tDbl;

/* input data */
typedef struct
{
	tInt  intfld;
	tDbl  dblfld;
}tInData;

/* dependencies */
typedef struct
{
	long  col ;   /* collumn ID                               */
	long  deplen; /* lenght of "deps"                          */
	long *deps;   /* list of collumns that depends on this one */
} tDep;

extern int femCmdNumProcUI ; /* number of processes for operations */

extern FILE *fdbPrnFile ;  /* file pointer */

/* Functions: */
extern void fdbTabNull(tTab *tab) ;
extern void fdbTabFree(tTab *tab) ;

extern void fdbInitTableColNumbers(tTab *tab, long tabLen, long *iLen, long *dLen) ;

extern void fdbIntNull(tInt *fld) ;
extern void fdbIntFree(tInt *fld) ;
extern void fdbIntSetEmpty(tInt *fld) ;
extern int fdbIntInit(tInt *fld, long len) ;
extern int fdbIntReAlloc(tInt *fld, long pos, long len) ;
extern int fdbIntInsertZero(tInt *fld, long pos, long from, long count) ;
extern int fdbIntRemove(tInt *fld, long pos, long from, long count) ;

extern void fdbDblNull(tDbl *fld) ;
extern void fdbDblFree(tDbl *fld) ;
extern void fdbDblSetEmpty(tDbl *fld) ;
extern int fdbDblInit(tDbl *fld, long len) ;
extern int fdbDblReAlloc(tDbl *fld, long pos, long len) ;
extern int fdbDblInsertZero(tDbl *fld, long pos, long from, long count) ;
extern int fdbDblRemove(tDbl *fld, long pos, long from, long count) ;


/* Functions to be used: */
extern int fdbWhatCol(tTab *tab, long col_id, long *col, long *type);

extern int fdbIntChangeVals(tInt *fld, long pos, long from, long count, long *val, int add);
extern int fdbDblChangeVals(tDbl *fld, long pos, long from, long count, double *val, int add);
extern int fdbAppendTableRow(tTab *tab, tInt *dataI, tDbl *dataD, long count, long *index);
extern int fdbInsertTableRow(tTab *tab, tInt *dataI, tDbl *dataD, long from, long count);
extern int fdbRemoveTableRow(tTab *tab, tInt *dataI, tDbl *dataD, long from, long count);

/* finding of values */
extern long fdbCountInt(long *data, long dataLen, long val, long *first);
extern long fdbFindNextInt(long *data, long dataLen, long previous, long val);
extern long fdbCountDbl(double *data, long dataLen, double val, long *first);
extern long fdbFindNextDbl(double *data, long dataLen, long previous, double val);

extern long fdbFindMaxInt(long *fld, long len);


/* checking of depenedencies */
extern int fdbCheckDep(tInt *dataI, tTab *tab, long checkedRow, tDep *depTab, long depLen);


/* output (listing) */
extern void fdbPrintInt(FILE *fw, long val, long print_mode);
extern void fdbPrintDbl(FILE *fw, double val, long print_mode);
extern int fdbPrintTabLine(FILE *fw, tTab *tab, tInt *intfld, tDbl *dblfld, long tab_id, long row, long print_mode);

/* selections */
extern int fdbTestSelect(tTab *tab, tInt *intfld, long row);
extern void fdbSetSelect(tTab *tab, tInt *intfld, long row);

extern int fdbSelectIntInterval(tTab *tab, tInt *intfld, long collumn, long mode, long from, long to);
extern int fdbSelectDblInterval(tTab *tab, tInt *intfld, tDbl *dblfld, long collumn, long mode, double from, double to);

extern int fdbSelectIntList(tTab *tab, tInt *intfld, long collumn, long mode, long val_len, long *values);

/* input/output */
extern int dbWriteTable(FILE *fw, long tablen, tTab *Tab, tInt intData, tDbl dblData);
extern int dbReadTable(FILE *fw, long tablen, tTab *Tab, tInt *intData, tDbl *dblData);

/* dependencies */
extern int fdbInputInitInputDeps(void) ;


/* formated output (to file) */
extern char *fdbPrnSafeStr(char *str, char *defstr) ;
extern void fdbPrnBeginTable(FILE *fw, long rows, char *title) ;
extern void fdbPrnTableHeaderItem(FILE *fw, long pos, char *text) ;
extern void fdbPrnTableItemStr(FILE *fw, long pos, char *text) ;
extern void fdbPrnTableItemInt(FILE *fw, long pos, long text) ;
extern void fdbPrnTableItemDbl(FILE *fw, long pos, double text) ;
extern void fdbPrnEndTable(FILE *fw) ;

extern int fdbPrnOpenFile(FILE *fw);
extern int fdbPrnCloseFile(FILE *fw);

/* ineffective I/O (for interactive usage etc.) */
extern int fdbWriteStrAppend(char *fname, char *str);
extern int fdbWriteStrRewrite(char *fname, char *str);

/* testing of data consistency */
extern int fdb_test_data(void);
#endif

/* end of fdb.h */
