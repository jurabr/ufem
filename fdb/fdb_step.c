/*
   File name: fdb_step.c
   Date:      2004/12/29 13:55
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

	 FEM Database - load steps
*/

#include "fdb.h"
#include "fdb_fem.h"
#include "cint.h"


/** Creates new load set
 * @param id identifier (0 == new with default number)
 * @param time time information (in [sec] or so)
 * @param val_num number of values (set numbers)
 * @param values list of values
 * @return status
 */
int f_step_new_change(long id, double time, long val_num, long *values)
{
	int  rv    = AF_OK ;
	int  nnew  = AF_NO ;
	long pos   = -1 ;
	long count = 0 ;
	long newid = 0 ;
	long nval = 0 ;
	long posn = 0 ;
	int  found = AF_NO ;
  double mult = 1.0 ;
  double zero = 0.0 ;
	int  i ;

  if ((val_num <= 0) || (values == NULL)) 
  {
    if (id <= 0) { return(AF_ERR_EMP) ; }

    if ((fdbInputCountInt(STEP, STEP_ID, id, &pos) > 0)&&(time >= 0.0))
    {

	    fdbInputDblChangeVals(STEP, STEP_TIME,  pos, 1, &time, AF_NO);
      return(AF_OK) ;
    }
    else
    {
      return(AF_ERR_EMP) ;
    }
  }
  
  /* test if set number exists inside data (only warning will be * produced) */
  for (i=0; i<val_num; i++)
	{
    found = AF_NO ;

    if (values[i] <= 0)
    {
      fprintf(msgout,"[E] %s: %li\n", _("Illegal set number"), values[i]);
      return(AF_ERR_VAL);
    }

		if (fdbInputCountInt(ELEM, ELEM_SET, values[i], &pos) > 0) 
    { 
      found = AF_YES ; 
    }
    else
    {
      if (fdbInputCountInt(NDISP, NDISP_SET, values[i], &pos) > 0) 
      { 
        found = AF_YES ; 
      }
      else
      {
        if (fdbInputCountInt(NLOAD, NLOAD_SET, values[i], &pos) > 0)
        { 
          found = AF_YES ; 
        }
        else
        {
          if (fdbInputCountInt(ELOAD, NLOAD_SET, values[i], &pos) > 0) 
          { 
            found = AF_YES ; 
          }
        }
      }
    }

    if (found != AF_YES)
    {
      fprintf(msgout,"[W] %s: %li\n", _("Unused set number given"), values[i]);
    }
	}

  nval = val_num ;


	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(STEP, STEP_ID) + 1 ;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(STEP, STEP_ID, id, &pos);

		if (count != nval) /* existing data */
		{
			return(AF_ERR_VAL) ; /* can NOT be chandged - must be DELETED first */
		}
		else /* new data */
		{
			newid = id ;
			nnew  = AF_NO ;
		}
	}

	if (nnew == AF_YES) /* adding of new line */
	{
		if ((rv = fdbInputAppendTableRow(STEP, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(STEP, STEP_ID, pos, 1, &newid, AF_NO);

		/* space for values:  */
		if ((rv = fdbInputAppendTableRow(STEPVAL, nval, &posn)) != AF_OK) 
		{
			 /* cannot allocate space => delete whole real set */
			 fdbInputRemoveTableRow(STEP, pos, 1);
       fprintf(msgout,"[E] %s!\n",_("Out of memory - cannot add step"));
			 return(rv);
		}
		else
		{
	    fdbInputIntChangeVals(STEP, STEP_NVAL,  pos, 1, &nval, AF_NO);

			for (i=0; i<nval; i++)
			{
				fdbInputIntChangeVals(STEPVAL, STEPVAL_STEP,  posn+i, 1, &newid, AF_NO);
				fdbInputIntChangeVals(STEPVAL, STEPVAL_SET,  posn+i, 1, &values[i], AF_NO);
				fdbInputDblChangeVals(STEPVAL, STEPVAL_MULT,  posn+i, 1, &mult, AF_NO);
			}
		}

    if (time >= 0.0)
    {
	    fdbInputDblChangeVals(STEP, STEP_TIME,  pos, 1, &time, AF_NO);
    }
    else
    {
	    fdbInputDblChangeVals(STEP, STEP_TIME,  pos, 1, &zero, AF_NO);
    }
	}
	else /* changing of existing */
	{
		/* do nothing */
    for (i=0; i<nval; i++)
		{
			fdbInputIntChangeVals(STEPVAL, STEPVAL_STEP,  posn+i, 1, &newid, AF_NO);
			fdbInputIntChangeVals(STEPVAL, STEPVAL_SET,  posn+i, 1, &values[i], AF_NO);
		}
    
    if (time >= 0.0)
    {
	    fdbInputDblChangeVals(STEP, STEP_TIME,  pos, 1, &time, AF_NO);
    }
	}

	return(rv) ;
}

/** Defines multiplier of set inside load step
 * @param id step number
 * @param set set number
 * @param val multiplier value
 * @return status
 */
int f_stepval_change(long id, long set, double val)
{
	int rv = AF_OK ;
	long pos       = -1 ;

	if ((fdbInputCountTwoInt(STEPVAL, STEPVAL_STEP, id, STEPVAL_SET, set, &pos)) < 1)
	{
		/* real set not found */
    fprintf(msgout,"[E] %s: %li/%li!\n",_("Requested combination of step and set/time not found"), id, set);
		return(AF_ERR_EMP);
	}
	else
	{
		rv = fdbInputDblChangeVals(STEPVAL, STEPVAL_MULT, pos, 1, &val, AF_NO);
	}

	return(rv);
}


/** Deletes step
 * @param id number of node
 * @return status
 */
int f_step_delete(long id)
{
	int  rv     = AF_OK ;
	long pos    = -1 ;
	long valpos = -1 ;
	long count  =  0 ;
	int  i ;

  /* tests: */
	if (fdbInputCountInt(STEP, STEP_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no real set found */
	}

	/* removing */
	if ((rv = fdbInputRemoveTableRow(STEP, pos, 1)) == AF_OK)
	{
		/* values */
		if ((count=fdbInputCountInt(STEPVAL, STEPVAL_STEP, id, &valpos)) > 0)
		{
			for (i=valpos; i<(valpos+count); i++)
			{
				fdbInputRemoveTableRow(STEPVAL, i, 1) ;
			}
		}
	}

	return(rv) ;
}


/** Lists one step
 * @param id number of node (0 == new node with default number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 *
 */
int f_step_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long posn  = -1 ;
	long print_mode = 0 ;
	long count = 0 ;
	long ival ;
	double dval ;
	int  i ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(STEP, STEP_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n    STEP    T\n");
	}

	rv = fdbInputPrintTabLine(fw, STEP, pos, print_mode);
	fprintf(fw, "\n");

	/* values  */
	count = fdbInputCountInt(STEPVAL, STEPVAL_STEP, id, &posn) ;

	fprintf(fw, " ");
	for (i=0; i<count; i++) 
  { 
    ival = fdbInputGetInt(STEPVAL,  STEPVAL_SET, posn+i) ;
    dval = fdbInputGetDbl(STEPVAL,  STEPVAL_MULT, posn+i) ;
		fprintf(fw, " \"%li\"=%e ",ival, dval);
  }
	fprintf(fw, "\n");

	return(rv) ;
}

int f_step_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id, vals, pos ;
  long i,j ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 3, _("Step Data Summary")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Real Time")) ;
  fdbPrnTableHeaderItem(fw, 2, _("Sets")) ;

  for (i=0; i<fdbInputTabLenAll(STEP); i++)
  {
    if (fdbInputTestSelect(STEP, i) == AF_YES)
    {
      id = fdbInputGetInt(STEP,STEP_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemDbl(fw,1,fdbInputGetInt(STEP,STEP_TIME,i));
      fdbPrnTableItemInt(fw,2,fdbInputGetInt(STEP,STEP_NVAL,i));
    }
  }
  fdbPrnEndTable(fw) ;


  /* Individual steps: */
  for (i=0; i<fdbInputTabLenAll(STEP); i++)
  {
    if (fdbInputTestSelect(STEP, i) == AF_YES)
    {
      id = fdbInputGetInt(STEP,STEP_ID,i) ;

      fdbPrnBeginTable(fw, 3, 
          ciStrCat2ParsNoCommas(
          _("Real Set"), fdbFemStrFromInt(id)
          ) );

      fdbPrnTableHeaderItem(fw, 0, _("Step")) ;
      fdbPrnTableHeaderItem(fw, 1, _("Set/Time")) ;
      fdbPrnTableHeaderItem(fw, 2, _("Multipier")) ;

      vals = fdbInputCountInt(STEPVAL, STEPVAL_STEP, id, &pos) ;

      for (j=0; j<vals; j++)
      {
        fdbPrnTableItemInt(fw,0,fdbInputGetInt(STEPVAL,STEPVAL_STEP,pos+j));
        fdbPrnTableItemInt(fw,1,fdbInputGetInt(STEPVAL,STEPVAL_SET,pos+j));
        fdbPrnTableItemDbl(fw,2,fdbInputGetDbl(STEPVAL,STEPVAL_MULT,pos+j));
      }
      fdbPrnEndTable(fw) ;
    }
  }

  rv = fdbPrnCloseFile(fw);

  return(rv);
}

/* end of fdb_step.c */
