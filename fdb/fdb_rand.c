/*
   File name: fdb_rand.c
   Date:      2006/10/18 21:14
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

   Random variable operations
*/

#include "fdb_fem.h"

/** Adds new random variable */
int fdb_rand_new(long id, long type, long itm, long sitm, long iter)
{
  int rv = AF_OK ;
	long pos   = -1 ;
	long newid = 0 ;

	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(RANDVAL, RANDVAL_ID) + 1 ;
    if ((rv = fdbInputAppendTableRow(RANDVAL, 1, &pos)) != AF_OK) 
		{return(rv);}

	  fdbInputIntChangeVals(RANDVAL, RANDVAL_ID, pos, 1, &newid, AF_NO);
	}
	else       /* ID specified */
	{
    newid = id ;

    if (fdbInputCountInt(RANDVAL,RANDVAL_ID,newid, &pos) >= 0)
		{
      if ((rv = fdbInputAppendTableRow(RANDVAL, 1, &pos)) != AF_OK) 
		  {return(rv);}

	    fdbInputIntChangeVals(RANDVAL, RANDVAL_ID, pos, 1, &newid, AF_NO);
    }
    else
    {
      return(AF_ERR);
    }
	}

	fdbInputIntChangeVals(RANDVAL, RANDVAL_TYPE, pos, 1, &type, AF_NO);
	fdbInputIntChangeVals(RANDVAL, RANDVAL_ITM,  pos, 1, &itm, AF_NO);
	fdbInputIntChangeVals(RANDVAL, RANDVAL_SITM, pos, 1, &sitm, AF_NO);
	fdbInputIntChangeVals(RANDVAL, RANDVAL_ITER, pos, 1, &iter, AF_NO);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"random var. %li:  t=%li, item-%li, subitem=%li, iter=%li\n",
					fdbInputGetInt(RANDVAL, RANDVAL_ID, pos ),
					fdbInputGetInt(RANDVAL, RANDVAL_TYPE, pos ),
					fdbInputGetInt(RANDVAL, RANDVAL_ITM, pos ),
					fdbInputGetInt(RANDVAL, RANDVAL_SITM, pos ),
					fdbInputGetInt(RANDVAL, RANDVAL_ITER, pos )
					);
#endif

  return(rv);
}

/** Removes random variable */
int fdb_rand_delete(long id)
{
  int rv = AF_OK ;
  long num, pos ;
  long i ;

  num = fdbInputCountInt(RANDVAL, RANDVAL_ID, id, &pos);
  if ((rv=fdbInputRemoveTableRow(RANDVAL, pos, 1)) != AF_OK) {return(AF_ERR);}

  if (num <= 1) {return(AF_OK);} /* nothing to do */

  for (i=1; i<num; i++)
  {
    if (pos > 0) {pos--;}
    if (fdbInputFindNextInt(RANDVAL, RANDVAL_ID, pos, id) > 0)
    {
      if ((rv=fdbInputRemoveTableRow(RANDVAL, pos, 1)) != AF_OK) {return(AF_ERR);}
    }
  }

  return(rv);
}

/** Checks availability of random variables */
int fdb_rand_check(void)
{
  long i ;
  long type ;
  long item ;
  long subitem ;
  long iter ;
  long num, pos ;
  long len ;

  /* TODO - makes only a chaos... */
  return(0);

  len = fdbInputTabLenAll(RANDVAL) ;

  for (i=0; i< len ; i++)
  {
    type    = fdbInputGetInt(RANDVAL, RANDVAL_TYPE, i) ;
    item    = fdbInputGetInt(RANDVAL, RANDVAL_ITM, i) ;
    subitem = fdbInputGetInt(RANDVAL, RANDVAL_SITM, i) ;
    iter    = fdbInputGetInt(RANDVAL, RANDVAL_ITER, i) ;

    switch (type)
    {
      case MONTE_VTYPE_RS:
        num=fdbInputCountTwoInt(
              RVAL,
              RVAL_RSET, item,
              RVAL_TYPE, subitem,
              &pos);

        if ((num < (iter-1))||(pos == -1))
        {
          fdbInputRemoveTableRow(RANDVAL, i, 1) ;
          len--;
        }
        break ;
      case MONTE_VTYPE_MAT: 
        num=fdbInputCountTwoInt(
              MVAL,
              MVAL_MAT, item,
              MVAL_TYPE, subitem,
              &pos);

        if ((num < (iter-1))||(pos == -1))
        {
          fdbInputRemoveTableRow(RANDVAL, i, 1) ;
          len--;
        }
        break ;
      case MONTE_VTYPE_N:   
        num=fdbInputCountInt( NODE, NODE_ID, item, &pos);
        if (num <= 0)
        {
          fdbInputRemoveTableRow(RANDVAL, i, 1) ;
          len--;
        }
        break ;
      case MONTE_VTYPE_NLD: 
        num=fdbInputCountTwoInt(
              NLOAD,
              NLOAD_NODE, item,
              NLOAD_TYPE, subitem,
              &pos);

        if (num <= 0)
        {
          num=fdbInputCountTwoInt(
              NDISP,
              NDISP_NODE, item,
              NDISP_TYPE, subitem,
              &pos);
        }

        if ((num <= 0))
        {
          fdbInputRemoveTableRow(RANDVAL, i, 1) ;
          len--;
        }
        break ;
      case MONTE_VTYPE_EL:  
        num=fdbInputCountTwoInt(
              ELOAD,
              ELOAD_ELEM, item,
              ELOAD_TYPE, subitem,
              &pos);

        if ((num <= 0))
        {
          fdbInputRemoveTableRow(RANDVAL, i, 1) ;
          len--;
        }
        break ;

      case MONTE_VTYPE_RES_D: 
        num=fdbInputCountInt( NODE, NODE_ID, item, &pos);
        if (num <= 0)
        {
          fdbInputRemoveTableRow(RANDVAL, i, 1) ;
          len--;
        }
        break ;
      case MONTE_VTYPE_RES_R: 
        num=fdbInputCountInt(
              NDISP,
              NDISP_NODE, item,
              &pos);

        if (num <= 0)
        {
          fdbInputRemoveTableRow(RANDVAL, i, 1) ;
          len--;
        }
        break ;
      case MONTE_VTYPE_RES_E: 
        /* TODO */
        num=fdbInputCountInt(
              ELEM,
              ELEM_ID, item,
              &pos);

        if (num <= 0)
        {
          fdbInputRemoveTableRow(RANDVAL, i, 1) ;
          len--;
        }
        break ;

      case MONTE_VTYPE_RES_SUM_E: 
        /* no tests are done here - hopefully the result is available
         * somewhere.... */
        break;
    }
  }
  
  return(AF_OK);
}


/** Prints one random variable */
int fdb_rand_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long print_mode = 0 ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(RANDVAL, RANDVAL_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n          V          T         Item      SubItem  Repeat num.\n");
	}

	rv = fdbInputPrintTabLine(fw, RANDVAL, pos, print_mode);
	fprintf(fw, "\n");

  return(rv);
}

/** Prints list of random variables */
int fdb_rand_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id ;
  long i ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 5, _("Random Variables")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Type")) ;
  fdbPrnTableHeaderItem(fw, 2, _("Item number")) ;
  fdbPrnTableHeaderItem(fw, 3, _("Item subnumber")) ;
  fdbPrnTableHeaderItem(fw, 4, _("Rep. index")) ;

  for (i=0; i<fdbInputTabLenAll(RANDVAL); i++)
  {
      id = fdbInputGetInt(RANDVAL,RANDVAL_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemInt(fw,1,fdbInputGetInt(RANDVAL,RANDVAL_TYPE,i));
      fdbPrnTableItemInt(fw,2,fdbInputGetInt(RANDVAL,RANDVAL_ITM,i));
      fdbPrnTableItemInt(fw,3,fdbInputGetInt(RANDVAL,RANDVAL_SITM,i));
      fdbPrnTableItemInt(fw,4,fdbInputGetInt(RANDVAL,RANDVAL_ITER,i));
  }
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/* end of fdb_rand.c */
