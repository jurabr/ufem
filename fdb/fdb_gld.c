/*
   File name: fdb_gld.c
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

	 FEM - acceleration/gravitation load
*/

#include "fdb_fem.h"
#include "cint.h"

/** Creates new gravitaion or changes existing
 * @param id number of node (0 == new gr. with default number)
 * @param x x-coordinate
 * @param y y-coordinate
 * @param z z-coordinate
 * @return status
 */
int f_gl_new_change(long id, long set, long dir, double val)
{
	int    rv    = AF_OK ;
	int    nnew  = AF_NO ;
	long   pos   = -1 ;
	long   pos2  = -1 ;
	long   count = 0 ;
	long   newid = 0 ;
	long   myset = 0 ;
	double myval = 0.0 ;

	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(GRAV, GRAV_ID) + 1 ;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(GRAV, GRAV_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			newid = fdbInputGetInt(GRAV, GRAV_ID, pos) ;
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
		/* text if the same type and orientation
		   exists. if yes, accumulate (== do not
			 create new)
			 note: this is pretty strange
		 */
		count = fdbInputCountTwoInt(GRAV,
				GRAV_DIR, dir, 
				GRAV_SET, myset,  &pos2);

		if (count > 0) /* modification of existing */
		{
			nnew = AF_NO ;
			pos = pos2 ;
			myval += fdbInputGetDbl(GRAV, GRAV_VAL, pos);
		}
	}

	/*printf("count is %li\n", count);*/

	if (nnew == AF_YES) /* adding of new line */
	{
		if ((rv = fdbInputAppendTableRow(GRAV, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(GRAV, GRAV_ID, pos, 1, &newid, AF_NO);
	}
	else /* changing of existing */
	{
		/* do nothing */
	}

	/* common for all: */
	fdbInputIntChangeVals(GRAV, GRAV_SET, pos, 1, &myset, AF_NO);
	fdbInputIntChangeVals(GRAV, GRAV_DIR,pos, 1, &dir,  AF_NO);
	fdbInputDblChangeVals(GRAV, GRAV_VAL, pos, 1, &myval, AF_NO);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"g(%li): s=%li, d=%li, val=%e ##\n",
					fdbInputGetInt(GRAV, GRAV_ID, pos),
					fdbInputGetInt(GRAV, GRAV_SET, pos),
					fdbInputGetInt(GRAV, GRAV_DIR,pos),
					fdbInputGetDbl(GRAV, GRAV_VAL, pos)
					);
#endif
	return(rv) ;
}

/** Deletes gravitaion
 * @param id number of load
 * @return status
 */
int f_gl_delete(long id)
{
	long pos   = -1 ;

  /* tests: */
	if (fdbInputCountInt(GRAV, GRAV_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no node found */
	}

	/* removing */
	return( fdbInputRemoveTableRow(GRAV, pos, 1) );
}

/** Lists gravitaions
 * @param id number of load (0 == new node with default number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 */
int f_gl_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long print_mode = 0 ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(GRAV, GRAV_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if ( fdbTestSelect(&InputTab[GRAV], &inputData.intfld, pos) != AF_YES) 
	{
		return(AF_OK); /* not selected */
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n         ACC         SET       D       V\n");
	}

	rv = fdbInputPrintTabLine(fw, GRAV, pos, print_mode);
	fprintf(fw, "\n");

	return(rv) ;
}

int f_gl_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id ;
  long i ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 4, _("Accelerations")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Direction")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Size")) ;
  fdbPrnTableHeaderItem(fw, 2, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 3, _("Time")) ;

  for (i=0; i<fdbInputTabLenAll(GRAV); i++)
  {
    if (fdbInputTestSelect(GRAV, i) == AF_YES)
    {
      id = fdbInputGetInt(GRAV,GRAV_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemStr(fw,0,
          ciGetVarNameFromGrp(
            fdbFemStrFromInt(
            fdbInputGetInt(GRAV,GRAV_DIR,i)),"disp")
          );
      fdbPrnTableItemDbl(fw,1,fdbInputGetDbl(GRAV,GRAV_VAL,i));
      fdbPrnTableItemInt(fw,2,id);
      fdbPrnTableItemInt(fw,3,fdbInputGetInt(GRAV,GRAV_SET,i));
    }
  }
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/* end of fdb_gld.c */
