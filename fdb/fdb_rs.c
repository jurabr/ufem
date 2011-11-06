/*
   File name: fdb_rs.c
   Date:      2003/10/25 20:33
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

	 FEM Database - real constants
*/

#include "fdb_fem.h"
#include "fdb_edef.h"
#include "cint.h"

/** Tests if data type exist also in non-repeating data
 * @param mat_id identifier of material (MA_ID - NOT the program type)
 * @param type property type to be found
 * @return AF_YES (found) or AF_NO (all other cases)
 */
long f_rs_check_stat_type(long mat_id, long dtype)
{
	long i ;
	long type ;
	long pos   = -1 ;

	if (fdbInputCountInt(MAT, MAT_ID, mat_id, &pos) > 0)
	{
		type = fdbInputGetInt(MAT, MAT_TYPE, pos) ; 

		if ((type > 0) && (type < fdbElementTypeLen))
		{
			for (i=0; i<fdbElementType[type].reals; i++)
			{
				if (fdbElementType[type].r[i] == dtype)
				{
					return(AF_YES);
				}
			}
		}
	}

	return ( AF_NO ) ;
}



/** Creates new real set
 * @param id number of node (0 == new node with default number)
 * @param etype element type
 * @param rep_num number of repeating data sets
 * @return status
 */
int f_rset_new_change(long id, long etyp, long rep_num)
{
	int  rv    = AF_OK ;
	int  nnew  = AF_NO ;
	long pos   = -1 ;
	long type_pos = 0 ;
	long count = 0 ;
	long newid = 0 ;
	long nval = 0 ;
	long posn = 0 ;
	int  found = AF_NO ;
	int  i,j,jj ;

  if (etyp <=0)
  {
    fprintf(msgout,"[E] %s: %li\n", _("Invalid element type"), etyp);
		return(AF_ERR_VAL); /* invalid element type */
  }

	for (i=1; i<fdbElementTypeLen; i++)
	{
		if (fdbElementType[i].id == etyp)
		{
			type_pos = i ;
			found = AF_YES ;
			break ;
		}
	}

	if (found != AF_YES) 
	{
    fprintf(msgout,"[E] %s: %li\n", _("Invalid element type"), etyp);
		return(AF_ERR_VAL); /* invalid element type */
	}

	/* repeating values */
	if (rep_num < 0) { rep_num = 0 ; }
	
	/* number of data */
	nval = fdbElementType[type_pos].reals + (rep_num * fdbElementType[type_pos].reals_rep) ;

	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(RSET, RSET_ID) + 1 ;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(RSET, RSET_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			return(AF_ERR_VAL) ; /* can NOT be chandged - must be DELETED first */
			/*newid = fdbInputGetInt(RSET, RSET_ID, pos) ; 
			nnew = AF_NO ; */
		}
		else /* new data */
		{
			newid = id ;
			nnew  = AF_YES ;
		}
	}

	if (nnew == AF_YES) /* adding of new line */
	{
		if ((rv = fdbInputAppendTableRow(RSET, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(RSET, RSET_ID, pos, 1, &newid, AF_NO);

		/* space for values:  */
		if ((rv = fdbInputAppendTableRow(RVAL, nval, &posn)) != AF_OK) 
		{
			 /* cannot allocate space => delete whole real set */
			 fdbInputRemoveTableRow(RSET, pos, 1);
       fprintf(msgout,"[E] %s!\n",_("Out of memory - cannot add real set"));
			 return(rv);
		}
		else
		{
			for (i=0; i<fdbElementType[type_pos].reals; i++)
			{
				fdbInputIntChangeVals(RVAL, RVAL_RSET,  posn+i, 1, &newid, AF_NO);
				fdbInputIntChangeVals(RVAL, RVAL_TYPE,  posn+i, 1, &fdbElementType[type_pos].r[i], AF_NO);
			}
			if (rep_num > 0)
			{
				jj = fdbElementType[type_pos].reals ;

				for (i=0; i<rep_num; i++)
				{
					for (j=0; j<fdbElementType[type_pos].reals_rep; j++ )
					{
						fdbInputIntChangeVals(RVAL, RVAL_RSET, posn+jj, 1, &newid, AF_NO);
						fdbInputIntChangeVals(RVAL, RVAL_TYPE, posn+jj, 1, &fdbElementType[type_pos].r_rep[j], AF_NO);
						jj++;
					}
				}
			}
		}
	}
	else /* changing of existing */
	{
		/* do nothing */
	}

	/* common for all: */
	fdbInputIntChangeVals(RSET, RSET_TYPE, pos, 1, &etyp, AF_NO);
	fdbInputIntChangeVals(RSET, RSET_NVAL, pos, 1, &nval, AF_NO); /* number of values */

	return(rv) ;
}


/** Defines value of real constant
 * @param id real set id (from RSET_ID)
 * @param type type of property (e.g. RSET_XXX)
 * @param val value of property
 * @param rep_index repeating data indicator/counter (0 for non-repeating)
 * @return status
 */
int f_rs_change(long id, long type, double val, long rep_index)
{
	int rv = AF_OK ;
#if 1
	long count     = 0 ;
	long elemcount = 0 ;
	long mytype    = -1 ;
	long pos       = -1 ;
	int  i ;

	if ((count=fdbInputCountInt(RVAL, RVAL_RSET, id, &pos)) < 1)
	{
		/* real set not found */
    fprintf(msgout,"[E] %s: %li!\n",_("Set not found"), id);
		return(AF_ERR_EMP);
	}
	else
	{
		if ((f_rs_check_stat_type(id, type) == AF_NO)&&(rep_index > 0)){elemcount = 1 ;}

		for (i=0; i<count; i++)
		{
			if ((mytype=fdbInputGetInt(RVAL, RVAL_TYPE, pos+i)) ==  type)
			{
				if (rep_index == elemcount)
				{
					/* found */
					rv = fdbInputDblChangeVals(RVAL, RVAL_VAL, pos+i, 1, &val, AF_NO);
					return(rv);
				}
				else
				{
					elemcount++;
				}
			}
		}
		rv = AF_ERR_VAL; /* value NOT FOUND  */
	}
#endif
	return(rv);
}

/** Deletes real set
 * @param id number of node
 * @return status
 */
int f_rset_delete(long id)
{
	int  rv     = AF_OK ;
	long pos    = -1 ;
	long valpos = -1 ;
	long count  =  0 ;
	int  i ;

  /* tests: */
	if (fdbInputCountInt(RSET, RSET_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no real set found */
	}

	if (fdbInputTestDep(id, ELEM, ELEM_RS) == AF_YES){return(AF_ERR);}
	if (fdbInputTestDep(id, ENTITY, ENTITY_RS) == AF_YES){return(AF_ERR);}

	/* removing */
	if ((rv = fdbInputRemoveTableRow(RSET, pos, 1)) == AF_OK)
	{
		/* values */
		if ((count=fdbInputCountInt(RVAL, RVAL_RSET, id, &valpos)) > 0)
		{
			for (i=valpos; i<(valpos+count); i++)
			{
				fdbInputRemoveTableRow(RVAL, i, 1) ;
			}
		}
	}
	return(rv) ;
}

/** Lists one real set (without properties!)
 * @param id number of node (0 == new node with defaukt number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 *
 *** ###############  UNFINISHED  ############## ***
 *
 */
int f_rset_list(long id, int header, FILE *fw)
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

	if (fdbInputCountInt(RSET, RSET_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n    RSET    T\n");
	}

	rv = fdbInputPrintTabLine(fw, RSET, pos, print_mode);
	fprintf(fw, "\n");

	/* values  */
	count = fdbInputCountInt(RVAL, RVAL_RSET, id, &posn) ;

	fprintf(fw, " ");
	for (i=0; i<count; i++) 
  { 
    ival = fdbInputGetInt(RVAL,  RVAL_TYPE, posn+i) ;
    dval = fdbInputGetDbl(RVAL,  RVAL_VAL, posn+i) ;
		fprintf(fw, " \"%li\"=%e ",ival, dval);
  }
	fprintf(fw, "\n");

	return(rv) ;
}

int f_rs_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id, vals, pos ;
  long i,j ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 2, _("Real Sets Summary")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Type")) ;

  for (i=0; i<fdbInputTabLenAll(RSET); i++)
  {
    if (fdbInputTestSelect(RSET, i) == AF_YES)
    {
      id = fdbInputGetInt(RSET,RSET_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemInt(fw,1,fdbInputGetInt(RSET,RSET_TYPE,i));
    }
  }
  fdbPrnEndTable(fw) ;


  for (i=0; i<fdbInputTabLenAll(RSET); i++)
  {
    if (fdbInputTestSelect(RSET, i) == AF_YES)
    {
      id = fdbInputGetInt(RSET,RSET_ID,i) ;
      fdbPrnBeginTable(fw, 2, 
          ciStrCat2ParsNoCommas(
          _("Real Set"), fdbFemStrFromInt(id)
          ) );

      vals = fdbInputCountInt(RVAL, RVAL_RSET, id, &pos) ;

      for (j=0; j<vals; j++)
      {
        fdbPrnTableItemStr(fw,0,
            ciGetVarNameFromGrp(
              fdbFemStrFromInt(
              fdbInputGetInt(RVAL,RVAL_TYPE,pos+j)),
            "real")
            );
        fdbPrnTableItemDbl(fw,1,
            fdbInputGetDbl(RVAL,RVAL_VAL,pos+j));
      }
      fdbPrnEndTable(fw) ;
    }
  }

  rv = fdbPrnCloseFile(fw);
  return(rv);
}


/** Creates field with numbers of existing (and selected) rows */
long *f_list_existing_rs(long *num)
{
  long *tmp = NULL ;
  long  i, len, len0  ;
  long  unum = 0 ;
 *num = 0 ;

  len =  fdbInputTabLenSel(ETYPE) ;
  len0 =  fdbInputTabLenAll(ETYPE) ;

  if ((tmp = femIntAlloc(len)) == NULL) {return(NULL);}

  for (i=0; i<len0; i++)
  {
    if (unum >= len) { break; }
    if (fdbInputTestSelect(ETYPE, i) == AF_YES)
    {
      tmp[unum] = fdbInputGetInt(ETYPE,ETYPE_TYPE,i) ;
      unum++;
    }
  }
  
  *num = len ;
  return(tmp);
}

/* end of fdb_rs.c */
