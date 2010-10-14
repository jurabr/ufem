/*
   File name: fdb_mp.c
   Date:      2003/08/20 19:04
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

	 FEM Database - materials and material properties
*/

#include "fdb_fem.h"
#include "fdb_mdef.h"
#include "cint.h"

/** Tests if data type exist also in non-repeating data
 * @param mat_id identifier of material (MA_ID - NOT the program type)
 * @param type property type to be found
 * @return AF_YES (found) or AF_NO (all other cases)
 */
long f_mat_check_stat_type(long mat_id, long dtype)
{
	long i ;
	long type ;
	long pos   = -1 ;

	if (fdbInputCountInt(MAT, MAT_ID, mat_id, &pos) > 0)
	{
		type = fdbInputGetInt(MAT, MAT_TYPE, pos) ; 

		if ((type > 0) && (type < fdbMatTypeLen))
		{
			for (i=0; i<fdbMatType[type].vals; i++)
			{
				if (fdbMatType[type].val[i] == dtype)
				{
					return(AF_YES);
				}
			}
		}
	}

	return ( AF_NO ) ;
}

/** Creates new material type
 * @param id number of node (0 == new node with default number)
 * @param etype material type
 * @param rep_num number of repeating data sets
 * @return status
 */
int f_mat_new_change(long id, long etyp, long rep_num)
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


	for (i=1; i<fdbMatTypeLen; i++)
	{
		if (fdbMatType[i].id == etyp)
		{
			type_pos = i ;
			found = AF_YES ;
			break ;
		}
	}

	if (found != AF_YES) 
	{
		return(AF_ERR_VAL); /* invalid material type */
	}

	/* repeating values */
	if (rep_num < 0) { rep_num = 0 ; }
	
	/* number of data */
	nval = fdbMatType[type_pos].vals + (rep_num * fdbMatType[type_pos].vals_rp) ;

	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(MAT, MAT_ID) + 1;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(MAT, MAT_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			return(AF_ERR_VAL) ; /* can NOT be chandged - must be DELETED first */
			/*newid = fdbInputGetInt(MAT, MAT_ID, pos) ; 
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
		if ((rv = fdbInputAppendTableRow(MAT, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(MAT, MAT_ID, pos, 1, &newid, AF_NO);

		/* space for values:  */
		if ((rv = fdbInputAppendTableRow(MVAL, nval, &posn)) != AF_OK) 
		{
			 /* cannot allocate space => delete whole material */
			 fdbInputRemoveTableRow(MAT, pos, 1);
			 return(rv);
		}
		else
		{
			for (i=0; i<fdbMatType[type_pos].vals; i++)
			{
				fdbInputIntChangeVals(MVAL, MVAL_MAT,  posn+i, 1, &newid, AF_NO);
				fdbInputIntChangeVals(MVAL, MVAL_TYPE,  posn+i, 1, &fdbMatType[type_pos].val[i], AF_NO);
			}
			if (rep_num > 0)
			{
				jj = fdbMatType[type_pos].vals ;

				for (i=0; i<rep_num; i++)
				{
					for (j=0; j<fdbMatType[type_pos].vals_rp; j++ )
					{
						fdbInputIntChangeVals(MVAL, MVAL_MAT, posn+jj, 1, &newid, AF_NO);
						fdbInputIntChangeVals(MVAL, MVAL_TYPE, posn+jj, 1, &fdbMatType[type_pos].val_rp[j], AF_NO);
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
	fdbInputIntChangeVals(MAT, MAT_TYPE, pos, 1, &etyp, AF_NO);
	fdbInputIntChangeVals(MAT, MAT_NVAL, pos, 1, &nval, AF_NO); /* number of values */

	return(rv) ;
}


/** Defines value of material property
 * @param id material id (from MAT_ID)
 * @param type type of property (e.g. MAT_EX etc.)
 * @param val value of property
 * @param rep_index repeating data indicator/counter (0 for non-repeating)
 * @return status
 */
int f_mp_change(long id, long type, double val, long rep_index)
{
	int rv = AF_OK ;
	int  count     = 0 ;
	int  elemcount = 0 ;
	long pos       = -1 ;
	int  i ;


	if ((count=fdbInputCountInt(MVAL, MVAL_MAT, id, &pos)) < 1)
	{
		/* material not found */
    fprintf(msgout,"[E] %s: %li!\n",_("Material not found"), id);
		return(AF_ERR_EMP);
	}
	else
	{
		elemcount = 0 ;

		if ((f_mat_check_stat_type(id, type) == AF_NO)&&(rep_index > 0))
		{
			elemcount = 1 ;
		}

		for (i=0; i<count; i++)
		{
			if ((fdbInputGetInt(MVAL, MVAL_TYPE, pos+i)) == type)
			{
				if (rep_index == elemcount)
				{
					/* found */
					rv = fdbInputDblChangeVals(MVAL, MVAL_VAL, pos+i, 1, &val, AF_NO);
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

	return(rv);
}

/** Deletes material
 * @param id number of node
 * @return status
 */
int f_mat_delete(long id)
{
	int  rv     = AF_OK ;
	long pos    = -1 ;
	long valpos = -1 ;
	long count  =  0 ;
	int  i ;

  /* tests: */
	if (fdbInputCountInt(MAT, MAT_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no material found */
	}

	if (fdbInputTestDep(id, ELEM, ELEM_MAT) == AF_YES){return(AF_ERR);}
	if (fdbInputTestDep(id, ENTITY, ENTITY_MAT) == AF_YES){return(AF_ERR);}

	/* removing */
	if ((rv = fdbInputRemoveTableRow(MAT, pos, 1)) == AF_OK)
	{
		/* values */
		if ((count=fdbInputCountInt(MVAL, MVAL_MAT, id, &valpos)) > 0)
		{
			for (i=valpos; i<(valpos+count); i++)
			{
				fdbInputRemoveTableRow(MVAL, i, 1) ;
			}
		}
	}
	return(rv) ;
}

/** Lists one material (without properties!)
 * @param id number of node (0 == new node with defaukt number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 *
 *** ###############  UNFINISHED  ############## ***
 *
 */
int f_mat_list(long id, int header, FILE *fw)
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

	if (fdbInputCountInt(MAT, MAT_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n    MP     T\n");
	}

	rv = fdbInputPrintTabLine(fw, MAT, pos, print_mode);
	fprintf(fw, "\n");

	/* values  */
	count = fdbInputCountInt(MVAL, MVAL_MAT, id, &posn) ;

	fprintf(fw, " ");
	for (i=0; i<count; i++) 
  { 
    ival = fdbInputGetInt(MVAL,  MVAL_TYPE, posn+i) ;
    dval = fdbInputGetDbl(MVAL,  MVAL_VAL, posn+i) ;
		fprintf(fw, " \"%li\"=%e ",ival, dval);
  }
	fprintf(fw, "\n");

	return(rv) ;
}

int f_mat_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id, vals, pos ;
  long i,j ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 2, _("Material Types Summary")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Type")) ;

  for (i=0; i<fdbInputTabLenAll(MAT); i++)
  {
    if (fdbInputTestSelect(MAT, i) == AF_YES)
    {
      id = fdbInputGetInt(MAT,MAT_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemStr(fw,1,
            ciGetVarNameFromGrp(
              fdbFemStrFromInt(
          fdbInputGetInt(MAT,MAT_TYPE,i)),
          "mattype")
          );
    }
  }
  fdbPrnEndTable(fw) ;


  for (i=0; i<fdbInputTabLenAll(MAT); i++)
  {
    if (fdbInputTestSelect(MAT, i) == AF_YES)
    {
      id = fdbInputGetInt(MAT,MAT_ID,i) ;
      fdbPrnBeginTable(fw, 2, 
          ciStrCat2ParsNoCommas(
          _("Material Type"), 
          fdbFemStrFromInt(id) ) );

      vals = fdbInputCountInt(MVAL, MVAL_MAT, id, &pos) ;

      for (j=0; j<vals; j++)
      {
        fdbPrnTableItemStr(fw,0,
            ciGetVarNameFromGrp(
              fdbFemStrFromInt(
              fdbInputGetInt(MVAL,MVAL_TYPE,pos+j)),
            "material")
            );
        fdbPrnTableItemDbl(fw,1,
            fdbInputGetDbl(MVAL,MVAL_VAL,pos+j));
      }
      fdbPrnEndTable(fw) ;
    }
  }

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/* end of fdb_mp.c */
