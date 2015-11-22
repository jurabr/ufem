/*
   File name: fdb_sele.c
   Date:      2003/11/28 18:59
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

	 Database for FEM - selections

	 $Id: fdb_sele.c,v 1.8 2005/02/12 18:34:50 jirka Exp $
*/

#include "fdb.h"
#include "fdb_data.h"
#include "fdb_fem.h"
#include "fdb_res.h"


/** Returns AF_YES if row is selected, AF_NO if  not
 * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param rows table row to be tested
 */
int fdbTestSelect(tTab *tab, tInt *intfld, long row)
{
	long col  = -1 ;
	long type = -1 ;

	if (tab->selcolnum == FDB_NONE) 
	{
		return(AF_YES);
	}
	else
	{
    fdbWhatCol(tab, tab->selcolnum, &col, &type);
		if (intfld->data[col][row] == AF_YES) 
		{
			return(AF_YES) ;
		}
		else
		{
			return(AF_NO);
		}
	}
}

/** Select row of table
 * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param rows table row to be set
 */
void fdbSetSelect(tTab *tab, tInt *intfld, long row)
{
	long col  = -1 ;
	long type = -1 ;

	if (tab->selcolnum != FDB_NONE) 
	{
    fdbWhatCol(tab, tab->selcolnum, &col, &type);
		intfld->data[col][row] = AF_YES ;
	}
}

/** Select rows by Int interval
 * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param collumn collumnt to test values
 * @param mode selection mode
 * @param from lower limit
 * @param to upper limit
 * @return status
 */ 
int fdbSelectIntInterval(tTab *tab, tInt *intfld, long collumn, long mode, long from, long to)
{
  int  rv = AF_OK ;
  long i ;
  long type;
  long col ;
  long sel_col ;
  long sel_status ;
  long val ;

  if (tab->selcolnum != FDB_NONE) 
	{
    fdbWhatCol(tab, tab->selcolnum, &sel_col, &type);
	}
  else
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("Table IS NOT selectable"));
#endif
    return(AF_ERR_VAL);
  }

  fdbWhatCol(tab, collumn, &col, &type);

  if (type != FDB_INT)
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("INVALID TYPE"));
#endif
    return(AF_ERR_VAL);
  }

  for (i=0; i<intfld->len[sel_col]; i++)
  {
	  sel_status = intfld->data[sel_col][i] ;
    val = intfld->data[col][i] ;

    if (mode == FDB_SELE_INVERT)
    {
      if (sel_status == AF_YES) { intfld->data[sel_col][i] = AF_NO ; }
      else                      { intfld->data[sel_col][i] = AF_YES ; }

      continue ;
    }

    if (mode == FDB_SELE_ALL)
    {
      intfld->data[sel_col][i] = AF_YES ;
      continue ;
    }

    if (mode == FDB_SELE_NONE)
    {
      intfld->data[sel_col][i] = AF_NO ;
      continue ;
    }

    if ( (from <= val) && (val <= to) )
    {
      switch (mode)
      {
        case FDB_SELE_SELECT: intfld->data[sel_col][i] = AF_YES ;
                              break;
        case FDB_SELE_RESELE: /* do nothing */ break;
        case FDB_SELE_ADD   : if (sel_status == AF_NO)
                                { intfld->data[sel_col][i] = AF_YES ; }
                              break;
        case FDB_SELE_UNSELE: if (sel_status == AF_YES)
                                { intfld->data[sel_col][i] = AF_NO ; }
                              break;
        case FDB_SELE_INVERT: /* do nothing */ break;
        case FDB_SELE_ALL   : /* do nothing */ break;
        case FDB_SELE_NONE  : /* do nothing */ break;
        default: return (AF_ERR_VAL); break;
      }
    }
    else
    {
      if(mode == FDB_SELE_SELECT)
			{
				intfld->data[sel_col][i] = AF_NO ;
			}

      if (mode == FDB_SELE_RESELE)
      {
        intfld->data[sel_col][i] = AF_NO ;
      }
    }
  }

  return(rv);
}

/** Select rows by Dbl interval
 * * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param dblfld floating point field
 * @param collumn collumnt to test values
 * @param mode selection mode
 * @param from lower limit
 * @param to upper limit
 * @return status
 */ 
int fdbSelectDblInterval(tTab *tab, tInt *intfld, tDbl *dblfld, long collumn, long mode, double from, double to)
{
  int  rv = AF_OK ;
  long i ;
  long type;
  long col ;
  long sel_col ;
  long sel_status ;
  double val ;

  if (tab->selcolnum != FDB_NONE) 
	{
    fdbWhatCol(tab, tab->selcolnum, &sel_col, &type);
	}
  else
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("Table IS NOT selectable"));
#endif
    return(AF_ERR_VAL);
  }

  fdbWhatCol(tab, collumn, &col, &type);

  if (type != FDB_DBL)
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("INVALID TYPE"));
#endif
    return(AF_ERR_VAL);
  }

  for (i=0; i<intfld->len[sel_col]; i++)
  {
	  sel_status = intfld->data[sel_col][i] ;
    val = dblfld->data[col][i] ;

    if (mode == FDB_SELE_INVERT)
    {
      if (sel_status == AF_YES) { intfld->data[sel_col][i] = AF_NO ; }
      else                      { intfld->data[sel_col][i] = AF_YES ; }

      continue ;
    }

    if (mode == FDB_SELE_ALL)
    {
      intfld->data[sel_col][i] = AF_YES ;
      continue ;
    }

    if (mode == FDB_SELE_NONE)
    {
      intfld->data[sel_col][i] = AF_NO ;
      continue ;
    }

    if ( ((from-100.0*FEM_ZERO) <= val) && ((val-100.0*FEM_ZERO) <= to) )
    {
      switch (mode)
      {
        case FDB_SELE_SELECT: intfld->data[sel_col][i] = AF_YES ;
                              break;
        case FDB_SELE_RESELE: /* do nothing */ break;
        case FDB_SELE_ADD   : if (sel_status == AF_NO)
                                { intfld->data[sel_col][i] = AF_YES ; }
                              break;
        case FDB_SELE_UNSELE: if (sel_status == AF_YES)
                                { intfld->data[sel_col][i] = AF_NO ; }
                              break;
        case FDB_SELE_INVERT: /* do nothing */ break;
        case FDB_SELE_ALL   : /* do nothing */ break;
        case FDB_SELE_NONE  : /* do nothing */ break;
        default: return (AF_ERR_VAL); break;
      }
    }
    else
    {
      if (mode == FDB_SELE_SELECT)
			{
				intfld->data[sel_col][i] = AF_NO ;
			}
      if (mode == FDB_SELE_RESELE)
      {
        intfld->data[sel_col][i] = AF_NO ;
      }
    }
  }

  return(rv);
}

/** Select rows by list of Int values
 * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param collumn collumnt to test values
 * @param mode selection mode
 * @param val_len number of values
 * @param values list of values to be tested
 * @return status
 */ 
int fdbSelectIntList(tTab *tab, tInt *intfld, long collumn, long mode, long val_len, long *values)
{
  int  rv = AF_OK ;
  long i, j ;
  long type;
  long col ;
  long sel_col ;
  long sel_status ;
  long is_in = AF_NO ;
  long val ;

  if (tab->selcolnum != FDB_NONE) 
	{
    fdbWhatCol(tab, tab->selcolnum, &sel_col, &type);
	}
  else
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("Table IS NOT selectable"));
#endif
    return(AF_ERR_VAL);
  }

  fdbWhatCol(tab, collumn, &col, &type);

  if (type != FDB_INT)
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("INVALID TYPE"));
#endif
    return(AF_ERR_VAL);
  }

  for (i=0; i<intfld->len[sel_col]; i++)
  {
	  sel_status = intfld->data[sel_col][i] ;
    val = intfld->data[col][i] ;

    if (mode == FDB_SELE_INVERT)
    {
      if (sel_status == AF_YES) { intfld->data[sel_col][i] = AF_NO ; }
      else                      { intfld->data[sel_col][i] = AF_YES ; }

      continue ;
    }

    if (mode == FDB_SELE_ALL)
    {
      intfld->data[sel_col][i] = AF_YES ;
      continue ;
    }

    if (mode == FDB_SELE_NONE)
    {
      intfld->data[sel_col][i] = AF_NO ;
      continue ;
    }

    is_in = AF_NO ;
    for (j=0; j<val_len; j++){if(val==values[j]){is_in=AF_YES;break;}}

    /*if ( (from <= val) && (val <= to) )*/
    if (is_in == AF_YES)
    {
      switch (mode)
      {
        case FDB_SELE_SELECT: intfld->data[sel_col][i] = AF_YES ;
                              break;
        case FDB_SELE_RESELE: /* do nothing */ break;
        case FDB_SELE_ADD   : if (sel_status == AF_NO)
                                { intfld->data[sel_col][i] = AF_YES ; }
                              break;
        case FDB_SELE_UNSELE: if (sel_status == AF_YES)
                                { intfld->data[sel_col][i] = AF_NO ; }
                              break;
        case FDB_SELE_INVERT: /* do nothing */ break;
        case FDB_SELE_ALL   : /* do nothing */ break;
        case FDB_SELE_NONE  : /* do nothing */ break;
        default: return (AF_ERR_VAL); break;
      }
    }
    else
    {
      if(mode == FDB_SELE_SELECT)
			{
				intfld->data[sel_col][i] = AF_NO ;
			}

      if (mode == FDB_SELE_RESELE)
      {
        intfld->data[sel_col][i] = AF_NO ;
      }
    }
  }

  return(rv);
}

/* Table - specific functions follows: ------------------ */

/** Select elements by element centroid
 * * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param dblfld floating point field
 * @param collumn collumnt to test values
 * @param mode selection mode
 * @param from lower limit
 * @param to upper limit
 * @return status
 */ 
int fdbSelectElemPos(long collumn, long mode, double from, double to)
{
  int  rv = AF_OK ;
  long i,j ;
  long nodes;
  long type;
  long sel_col ;
  long sel_status ;
  double val ;
	tTab tab = InputTab[ELEM];
	tInt intfld = inputData.intfld; 

  if (tab.selcolnum != FDB_NONE) 
	{
    fdbWhatCol(&tab, tab.selcolnum, &sel_col, &type);
	}
  else
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("Table IS NOT selectable"));
#endif
    return(AF_ERR_VAL);
  }

  for (i=0; i<intfld.len[sel_col]; i++)
  {
	  sel_status = intfld.data[sel_col][i] ;

    if (mode == FDB_SELE_INVERT)
    {
      if (sel_status == AF_YES) { intfld.data[sel_col][i] = AF_NO ; }
      else                      { intfld.data[sel_col][i] = AF_YES ; }

      continue ;
    }

    if (mode == FDB_SELE_ALL)
    {
      intfld.data[sel_col][i] = AF_YES ;
      continue ;
    }

    if (mode == FDB_SELE_NONE)
    {
      intfld.data[sel_col][i] = AF_NO ;
      continue ;
    }

		/* compute "val" here */
		val = 0.0 ;
    nodes = fdbInputGetInt(ELEM, ELEM_NODES, i) ;
		if (nodes == 0) 
		{
			rv = AF_ERR_VAL ;
			continue;
		}
		for (j=0; j<nodes; j++)
		{
			val += fdbInputGetDbl(NODE, collumn, fdbEnodePos(i, j)) ;
		}

		val = val / (double)nodes ;

    if ( (from <= val) && (val <= to) )
    {
      switch (mode)
      {
        case FDB_SELE_SELECT: intfld.data[sel_col][i] = AF_YES ;
                              break;
        case FDB_SELE_RESELE: /* do nothing */ break;
        case FDB_SELE_ADD   : if (sel_status == AF_NO)
                                { intfld.data[sel_col][i] = AF_YES ; }
                              break;
        case FDB_SELE_UNSELE: if (sel_status == AF_YES)
                                { intfld.data[sel_col][i] = AF_NO ; }
                              break;
        case FDB_SELE_INVERT: /* do nothing */ break;
        case FDB_SELE_ALL   : /* do nothing */ break;
        case FDB_SELE_NONE  : /* do nothing */ break;
        default: return (AF_ERR_VAL); break;
      }
    }
    else
    {
      if (mode == FDB_SELE_SELECT)
			{
				intfld.data[sel_col][i] = AF_NO ;
			}
      if (mode == FDB_SELE_RESELE)
      {
        intfld.data[sel_col][i] = AF_NO ;
      }
    }
  }

  return(rv);
}

/** Select elements by element results
 * * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param dblfld floating point field
 * @param collumn DATA TYPE
 * @param mode selection mode
 * @param from lower limit
 * @param to upper limit
 * @return status
 */ 
int fdbSelectElemRes(long collumn, long mode, double from, double to)
{
  int  rv = AF_OK ;
  long i ;
  long type;
  long sel_col ;
  long sel_status ;
	tTab tab = InputTab[ELEM];
	tInt intfld = inputData.intfld; 

  if (tab.selcolnum != FDB_NONE) 
	{
    fdbWhatCol(&tab, tab.selcolnum, &sel_col, &type);
	}
  else
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("Table IS NOT selectable"));
#endif
    return(AF_ERR_VAL);
  }

  for (i=0; i<intfld.len[sel_col]; i++)
  {
	  sel_status = intfld.data[sel_col][i] ;

    if (mode == FDB_SELE_INVERT)
    {
      if (sel_status == AF_YES) { intfld.data[sel_col][i] = AF_NO ; }
      else                      { intfld.data[sel_col][i] = AF_YES ; }

      continue ;
    }

    if (mode == FDB_SELE_ALL)
    {
      intfld.data[sel_col][i] = AF_YES ;
      continue ;
    }

    if (mode == FDB_SELE_NONE)
    {
      intfld.data[sel_col][i] = AF_NO ;
      continue ;
    }

    if (fdbTestResElemSolu(i, collumn, from, to) == AF_YES)
    {
      switch (mode)
      {
        case FDB_SELE_SELECT: intfld.data[sel_col][i] = AF_YES ;
                              break;
        case FDB_SELE_RESELE: /* do nothing */ break;
        case FDB_SELE_ADD   : if (sel_status == AF_NO)
                                { intfld.data[sel_col][i] = AF_YES ; }
                              break;
        case FDB_SELE_UNSELE: if (sel_status == AF_YES)
                                { intfld.data[sel_col][i] = AF_NO ; }
                              break;
        case FDB_SELE_INVERT: /* do nothing */ break;
        case FDB_SELE_ALL   : /* do nothing */ break;
        case FDB_SELE_NONE  : /* do nothing */ break;
        default: return (AF_ERR_VAL); break;
      }
    }
    else
    {
      if (mode == FDB_SELE_SELECT)
			{
				intfld.data[sel_col][i] = AF_NO ;
			}
      if (mode == FDB_SELE_RESELE)
      {
        intfld.data[sel_col][i] = AF_NO ;
      }
    }
  }

  return(rv);
}

/** Select nodes by DOF results
 * * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param dblfld floating point field
 * @param collumn DOF TYPE
 * @param mode selection mode
 * @param from lower limit
 * @param to upper limit
 * @return status
 */ 
int fdbSelectNodeRes(long collumn, long mode, double from, double to)
{
  int  rv = AF_OK ;
  long i ;
  long type;
  long sel_col ;
  long sel_status ;
	tTab tab = InputTab[NODE];
	tInt intfld = inputData.intfld; 

  if (tab.selcolnum != FDB_NONE) 
	{
    fdbWhatCol(&tab, tab.selcolnum, &sel_col, &type);
	}
  else
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("Table IS NOT selectable"));
#endif
    return(AF_ERR_VAL);
  }

  for (i=0; i<intfld.len[sel_col]; i++)
  {
	  sel_status = intfld.data[sel_col][i] ;

    if (mode == FDB_SELE_INVERT)
    {
      if (sel_status == AF_YES) { intfld.data[sel_col][i] = AF_NO ; }
      else                      { intfld.data[sel_col][i] = AF_YES ; }

      continue ;
    }

    if (mode == FDB_SELE_ALL)
    {
      intfld.data[sel_col][i] = AF_YES ;
      continue ;
    }

    if (mode == FDB_SELE_NONE)
    {
      intfld.data[sel_col][i] = AF_NO ;
      continue ;
    }

    if (fdbTestResSelNodesDOF(i, collumn, from, to) == AF_YES)
    {
      switch (mode)
      {
        case FDB_SELE_SELECT: intfld.data[sel_col][i] = AF_YES ;
                              break;
        case FDB_SELE_RESELE: /* do nothing */ break;
        case FDB_SELE_ADD   : if (sel_status == AF_NO)
                                { intfld.data[sel_col][i] = AF_YES ; }
                              break;
        case FDB_SELE_UNSELE: if (sel_status == AF_YES)
                                { intfld.data[sel_col][i] = AF_NO ; }
                              break;
        case FDB_SELE_INVERT: /* do nothing */ break;
        case FDB_SELE_ALL   : /* do nothing */ break;
        case FDB_SELE_NONE  : /* do nothing */ break;
        default: return (AF_ERR_VAL); break;
      }
    }
    else
    {
      if (mode == FDB_SELE_SELECT)
			{
				intfld.data[sel_col][i] = AF_NO ;
			}
      if (mode == FDB_SELE_RESELE)
      {
        intfld.data[sel_col][i] = AF_NO ;
      }
    }
  }

  return(rv);
}


/** Select entities by their centroid
 * * @param tab pointer to table header (e.g. &tab[number])
 * @param intfld integer field
 * @param dblfld floating point field
 * @param collumn collumnt to test values
 * @param mode selection mode
 * @param from lower limit
 * @param to upper limit
 * @return status
 */ 
int fdbSelectEntPos(long collumn, long mode, double from, double to)
{
  int  rv = AF_OK ;
  long i,j ;
  long nodes;
  long type;
  long sel_col ;
  long sel_status ;
  double val ;
	tTab tab = InputTab[ENTITY];
	tInt intfld = inputData.intfld; 

  if (tab.selcolnum != FDB_NONE) 
	{
    fdbWhatCol(&tab, tab.selcolnum, &sel_col, &type);
	}
  else
  {
#if DEVEL_VERBOSE
    fprintf(msgout,"[E] %s!\n",_("Table IS NOT selectable"));
#endif
    return(AF_ERR_VAL);
  }

  for (i=0; i<intfld.len[sel_col]; i++)
  {
	  sel_status = intfld.data[sel_col][i] ;

    if (mode == FDB_SELE_INVERT)
    {
      if (sel_status == AF_YES) { intfld.data[sel_col][i] = AF_NO ; }
      else                      { intfld.data[sel_col][i] = AF_YES ; }

      continue ;
    }

    if (mode == FDB_SELE_ALL)
    {
      intfld.data[sel_col][i] = AF_YES ;
      continue ;
    }

    if (mode == FDB_SELE_NONE)
    {
      intfld.data[sel_col][i] = AF_NO ;
      continue ;
    }

		/* compute "val" here */
		val = 0.0 ;
    nodes = fdbInputGetInt(ENTITY, ENTITY_KPS, i) ;
		if (nodes == 0) 
		{
			rv = AF_ERR_VAL ;
			continue;
		}
		for (j=0; j<nodes; j++)
		{
			val += fdbInputGetDbl(KPOINT, collumn, fdbEntKpPos(i, j)) ;
		}

		val = val / (double)nodes ;

    if ( (from <= val) && (val <= to) )
    {
      switch (mode)
      {
        case FDB_SELE_SELECT: intfld.data[sel_col][i] = AF_YES ;
                              break;
        case FDB_SELE_RESELE: /* do nothing */ break;
        case FDB_SELE_ADD   : if (sel_status == AF_NO)
                                { intfld.data[sel_col][i] = AF_YES ; }
                              break;
        case FDB_SELE_UNSELE: if (sel_status == AF_YES)
                                { intfld.data[sel_col][i] = AF_NO ; }
                              break;
        case FDB_SELE_INVERT: /* do nothing */ break;
        case FDB_SELE_ALL   : /* do nothing */ break;
        case FDB_SELE_NONE  : /* do nothing */ break;
        default: return (AF_ERR_VAL); break;
      }
    }
    else
    {
      if (mode == FDB_SELE_SELECT)
			{
				intfld.data[sel_col][i] = AF_NO ;
			}
      if (mode == FDB_SELE_RESELE)
      {
        intfld.data[sel_col][i] = AF_NO ;
      }
    }
  }

  return(rv);
}

/* end of fdb_sele.c */
