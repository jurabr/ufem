/*
   File name: fdb_et.c
   Date:      2003/08/19 16:45
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

	 FEM Database - Element Type

	 $Id: fdb_et.c,v 1.6 2004/11/11 21:41:05 jirka Exp $
*/

#include "fdb_fem.h"
#include "fdb_edef.h"
#include "cint.h"


/** Creates new element type
 * @param id number of node (0 == new element type with default number)
 * @param etype element type (program name)
 * @return status
 */
int f_et_new_change(long id, long etyp)
{
	int  rv    = AF_OK ;
	int  nnew  = AF_NO ;
	long pos   = -1 ;
	long count = 0 ;
	long newid = 0 ;
	int  found = AF_NO ;
	int  i ;

	for (i=1; i<fdbElementTypeLen; i++)
	{
		if (fdbElementType[i].id == etyp)
		{
			found = AF_YES ;
			break ;
		}
	}

	if (found != AF_YES) 
	{
		return(AF_ERR_VAL); /* invalid element type */
	}


	if (id <= 0) /* no ID specified => new */
	{
		newid =  ( fdbInputFindMaxInt(ETYPE, ETYPE_ID) + 1 ) ;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(ETYPE, ETYPE_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			newid = fdbInputGetInt(ETYPE, ETYPE_ID, pos) ;
			nnew = AF_NO ;
		}
		else /* new data */
		{
			newid = ( fdbInputFindMaxInt(ETYPE, ETYPE_ID) + 1 ) ;
			nnew = AF_YES ;
		}
	}

	if (nnew == AF_YES) /* adding of new line */
	{
		if ((rv = fdbInputAppendTableRow(ETYPE, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(ETYPE, ETYPE_ID, pos, 1, &newid, AF_NO);
	}
	else /* changing of existing */
	{
		/* do nothing */
	}

	/* common for all: */
	fdbInputIntChangeVals(ETYPE, ETYPE_TYPE, pos, 1, &etyp, AF_NO);

	return(rv) ;
}

/** Deletes node
 * @param id number of node
 * @return status
 */
int f_et_delete(long id)
{
	long pos   = -1 ;

  /* tests: */
	if (fdbInputCountInt(ETYPE, ETYPE_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no node found */
	}
	if (fdbInputTestDep(id, ELEM, ELEM_TYPE) == AF_YES){return(AF_ERR);}
	if (fdbInputTestDep(id, ENTITY, ENTITY_ETYPE) == AF_YES){return(AF_ERR);}

	/* removing */
	return( fdbInputRemoveTableRow(ETYPE, pos, 1) );
}

/** Lists one node
 * @param id number of node (0 == new node with defaukt number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 */
int f_et_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long print_mode = 0 ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(ETYPE, ETYPE_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n    ET     T\n");
	}

	rv = fdbInputPrintTabLine(fw, ETYPE, pos, print_mode);
	fprintf(fw, "\n");

	return(rv) ;
}

int f_et_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id ;
  long i ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 2, _("Element Types")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Type")) ;

  for (i=0; i<fdbInputTabLenAll(ETYPE); i++)
  {
    if (fdbInputTestSelect(ETYPE, i) == AF_YES)
    {
      id = fdbInputGetInt(ETYPE,ETYPE_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemStr(fw,1,
            ciGetVarNameFromGrp(
              fdbFemStrFromInt(
          fdbInputGetInt(ETYPE,ETYPE_TYPE,i)),
          "etype")
          );
    }
  }
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/* end of fdb_et.c */
