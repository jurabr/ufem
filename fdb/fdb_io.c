/*
   File name: fdb.c
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

	 Database for FEM - I/O routines

	 $Id: fdb_io.c,v 1.13 2004/11/11 21:41:06 jirka Exp $
*/

#include "fdb.h"
#include <locale.h>

/** Writes data to file (low-level function)
 * @param fw pointer to file
 * @param lablen number of tables
 * @param Tab tables
 * @param intData integer part of data
 * @param dblData floating point data
 */
int dbWriteTable(FILE *fw, long tablen, tTab *Tab, tInt intData, tDbl dblData)
{
	int i,j;

	errno = 0 ;

  /* locale (to avoid michmach in "." and ",") */
	setlocale(LC_NUMERIC,"C");

	fprintf(fw, "%li\n", tablen); /* number of tables */

	/* table headers */
	for (i=0; i<tablen; i++)
	{
		fprintf(fw, "%li %li %li\n", Tab[i].id, Tab[i].cols, Tab[i].selcolnum);
		for (j=0; j<Tab[i].cols; j++)
		{
			fprintf(fw, " %li %li ",Tab[i].col[j], Tab[i].type[j]);
		}
    fprintf(fw, "\n");
	}

	if (errno != 0) {return(errno);}

	/* data */

	/* "integers" */
	fprintf(fw, "%li\n", intData.rows);

	for (i=0; i<intData.rows; i++)
	{
		fprintf(fw, "%li\n", intData.len[i]);
		for (j=0; j<intData.len[i]; j++)
		{
			fprintf(fw, " %li ",intData.data[i][j]);
		}
		fprintf(fw,"\n");
	}

	/* doubles */
	fprintf(fw, "%li\n", dblData.rows);

	for (i=0; i<dblData.rows; i++)
	{
		fprintf(fw, "%li\n", dblData.len[i]);
		for (j=0; j<dblData.len[i]; j++)
		{
			fprintf(fw, " %e ",dblData.data[i][j]);
		}
		fprintf(fw,"\n");
	}

	return(errno);
}


/** Reads data to file (low-level function)
 * @param fw pointer to file
 * @param tablen number of tables
 * @param Tab tables
 * @param intData integer part of data
 * @param dblData floating point data
 */
int dbReadTable(FILE *fw, long tablen, tTab *Tab, tInt *intData, tDbl *dblData)
{
	int    rv = AF_OK ;
	long   i,j;
	long   tablen0 = 0 ;
	long   id, cols, selcolnum ;
	long   col, type ;
	long   rows = 0 ;
  long   len =  0;

	errno = 0 ;

	setlocale(LC_NUMERIC,"C");

  fdbIntSetEmpty(intData);
  fdbDblSetEmpty(dblData);

	fscanf(fw, "%li", &tablen0); /* number of tables */

	if (tablen0 != tablen) 
	{
		/* input table is not correct */
		fprintf(msgout,"[E] %s: %s!\n", _("Incorrect structure of data"), _("number of tables"));
		return(AF_ERR_IO);
	}

	/* table headers */
	for (i=0; i<tablen; i++)
	{
		fscanf(fw, "%li %li %li", &id, &cols, &selcolnum);

		if ( (Tab[i].id != id) || (Tab[i].cols != cols) || (Tab[i].selcolnum !=selcolnum) )
		{
			fprintf(msgout,"[E] %s: %s!\n", _("Incorrect structure of data"), _("table headers"));
			return(AF_ERR_IO);
		}

		for (j=0; j<Tab[i].cols; j++)
		{
			fscanf(fw, "%li %li ",&col, &type);
			if ( (Tab[i].col[j] != col) || (Tab[i].type[j] != type) )
			{
				fprintf(msgout,"[E] %s: %s!\n", _("Incorrect structure of data"), _("collumns in table headers"));
				return(AF_ERR_IO);
			}
		}
	}

	if (errno != 0) {return(errno);}

	/* data */

	/* "integers" */
	fscanf(fw, "%li", &rows);

  if (rows != intData->rows)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid size of integer data"));
    goto memFree ;
  }

	for (i=0; i<rows; i++)
	{
		fscanf(fw, "%li", &len);
		if ((rv =  fdbIntReAlloc(intData, i, len)) != AF_OK)
		{
			fprintf(msgout,"[E] %s!\n", _("Out of memory"));
			goto memFree;
		}

		for (j=0; j<intData->len[i]; j++)
		{
			fscanf(fw, "%li",&intData->data[i][j]);
		}
	}

	/* doubles */
	fscanf(fw, "%li", &rows);

  if (rows != dblData->rows)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid size of floating point data"));
    goto memFree ;
  }

	for (i=0; i<rows; i++)
	{
		fscanf(fw, "%li", &len);
		if ((rv =  fdbDblReAlloc(dblData, i, len)) != AF_OK)
		{
			fprintf(msgout,"[E] %s!\n", _("Out of memory"));
			goto memFree;
		}

		for (j=0; j<dblData->len[i]; j++)
		{
			fscanf(fw, "%lf",&dblData->data[i][j]);
		}
	}
  
	return(errno);

memFree:
  fdbIntSetEmpty(intData);
  fdbDblSetEmpty(dblData);
	return(AF_ERR_IO);
}

/* ----------------------------------------------------------------- */

/** Writes string to file  (extremely INEFFECTIVE!!!)
 * @param fname name of file
 * @param str string to be written
 * @return status
 * */
int fdbWriteStrAppend(char *fname, char *str)
{
	FILE *fw = NULL ;

	if ((fw = fopen(fname,"a")) == NULL){return(AF_ERR_IO);}
	fprintf(fw," %s ",str);
	fclose(fw);
	fw = NULL ;
	return(AF_OK);
}

/** Writes string to NEW file  (extremely INEFFECTIVE!!!)
 * @param fname name of file
 * @param str string to be written
 * @return status
 * */
int fdbWriteStrRewrite(char *fname, char *str)
{
	FILE *fw = NULL ;

	if ((fw = fopen(fname,"w")) == NULL){return(AF_ERR_IO);}
	fprintf(fw," %s ",str);
	fclose(fw);
	fw = NULL ;
	return(AF_OK);
}


/* end of fdb.c */
