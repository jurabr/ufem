/*
   File name: fem_accs.c
   Date:      2003/04/07 17:33
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.

	 FEM Solve - data access

  $Id: fem_accs.c,v 1.16 2005/01/16 22:59:31 jirka Exp $
*/

#include "fem_dama.h"
#include "fem_data.h"

/** returnes position of value "val" in field (or -1 if not found)
 * @param val value
 * @param fld field to be searched
 * @param fldLen lenght of "fld"
 * @return first position of value of -1
 */
long femFindIntVal(long val, long *fld, long fldLen)
{
	long retVal = -1;
	long i;

	for (i=0; i<fldLen; i++)
	{
		if (fld[i] == val) 
		{
			retVal = fld[i];
			break;
		}
	}

	return(retVal);
}

/** gets LONG node value from position
 * @param pos node position in field
 * @param fld field
 * @param fldLen lenght of field
 * @return value
 */
long femGetIntPos(long pos, long *fld, long fldLen)
{
	long coordVal = 0 ;
	
	if (pos < fldLen)
	{ 
		coordVal = fld[pos] ;
	}
#ifdef DEVEL_VERBOSE
	else
	{
		fprintf(msgout,"[E] %s: %li\n",_("Out of range"), pos);
	}
#endif
	return(coordVal);
}

/** gets DOUBLE node value from position
 * @param pos node position in field
 * @param fld field
 * @param fldLen lenght of field
 * @return value
 */
double femGetDblPos(long pos, double *fld, long fldLen)
{
	double coordVal = 0 ;
	
	if (pos < fldLen)
	{ 
		coordVal = fld[pos] ;
	}
#ifdef DEVEL_VERBOSE
	else
	{
		fprintf(msgout,"[E] %s: %li\n",_("Out of range"), pos);
	}
#endif
	return(coordVal);
}


/** returns "long" value from fldL field
 * @param pos position if fld_from (e.g. "row" in table)
 * @param val_pos position of value (e.g. "collumn" - from 0)
 * @param  from_fld field with starts in fldL
 * @param  fldL acessed field 
 * @param  fldLen lenght of access field
 * @return value (-1 on error)
 */
long femGetIntLPos(long pos, long val_pos, long *from_fld, long *fldL, long fldLLen)
{
	static long pos_val =  -1;
	long ind = 0 ;
		
	ind = from_fld[pos]+val_pos ;

#if DEVEL
	if (ind >= fldLLen)
	{
#if DEVEL_VERBOSE
		fprintf(msgout,"[E] %s\n!", _("index out of field"));
#endif
	  return(pos_val);
	}
#endif

	pos_val = fldL[ind] ;
	return(pos_val);
}

/** returns "double" value from fldL field
 * @param pos position if fld_from (e.g. "row" in table)
 * @param val_pos position of value (e.g. "collumn" - from 0)
 * @param  from_fld field with starts in fldL
 * @param  fldL acessed field 
 * @param  fldLen lenght of access field
 * @return value (-1 on error)
 */
double femGetDblLPos(long pos, long val_pos, long *from_fld, double *fldL, long fldLLen)
{
	static double pos_val =  -1;
	long ind = 0 ;
		
	ind = from_fld[pos]+val_pos ;

#if DEVEL
	if (ind >= fldLLen)
	{
#if DEVEL_VERBOSE
		fprintf(msgout,"[E] %s\n!", _("index out of field"));
#endif
	  return(pos_val);
	}
#endif

	pos_val = fldL[ind] ;
	return(pos_val);
}

/** computes number of items pointed by starting point if fldFrom[fldLen]
 * @param Pos position of line to compute
 * @param fldFrom wield with starting positions
 * @param fldLen length of fldFrom field
 * @return computed lenth (number >= 0)
 */
long femFldItemLen(long Pos, long *fldFrom, long fldLen, long fldLenL)
{
	static long len = 0;
	if (Pos < 0) {return(0);}

	if ((Pos+1) >= fldLen)
	{
		len = (fldLenL - fldFrom[Pos] );
	}
	else
	{
		len = (fldFrom[Pos+1] - fldFrom[Pos] );
	}
	return(len);
}

/** Gets value from  non/repeating data set
 * @param pos pos of line in SetFrom
 * @param valType type of data
 * @param repeat if value is in "repeating" part (AF_YES of AF_NO)
 * @param repNum set number for repeatable data (1,2,3,n !!)
 * @param SetL field with values
 * @param SetLenL lenth of SetL
 * @param SetFrom starts in SetL
 * @param SetLen lenth of SetFrom
 * @param valList list of possible values
 * @param valLen lenth of valList
 * @param valListRP list of possible repeating values
 * @param valLenRP lenth of valListRP
 * @return value
 */
double femGetRepVal(long pos, long valType, 
										long repNum,
										double *SetL, long SetLenL,
										long *SetFrom, long SetLen,
										long *valList, long valLen,
										long *valListRP, long valLenRP
										)
{
	static double val = 0 ;
	int i;
	int repeat = AF_YES ;
	long i_pos = -1 ;
	long len = 0;

#ifdef DEVEL
	if (pos >= SetLen) 
	{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Value outside field requested"));
#endif
		return(0);
	}
#endif

	if (repNum <= 0) {repeat = AF_NO;}

	len = femFldItemLen(pos, SetFrom, SetLen, SetLenL) ;

	if (repeat == AF_NO)
	{
		/* non-repeating */
		for (i=0; i<valLen; i++)
		{
			if (valType == valList[i])
			{
				i_pos = i ;
				break;
			}
		}
		if (i_pos == -1)
		{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s (%li) %s!\n", _("Requested asked non-repeating value"),valType, _("is invalid"));
#endif
			return(0);
		}
	}
	else
	{
		/* repeating */
		for (i=0; i<valLenRP; i++)
		{
			if (valType == valListRP[i])
			{
				i_pos = i + valLenRP*(repNum-1) + valLen;
				break;
			}
		}
		if (i_pos == -1)
		{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s (%li) %s!\n", _("Requested asked repeating value"),valType, _("is invalid"));
#endif
			return(0);
		}
	}

	if (i_pos >= len)
	{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Value outside field requested - program error"));
	fprintf(msgout,"[E] %s (%li>%li)(%li|%li|%li)!\n", _("Value outside field requested - program error"),i_pos,len, pos, valType,repNum);
#endif
		return(0);
	}
	else
	{
		val = SetL[SetFrom[pos]+i_pos] ;
#if 0
		fprintf(msgout,"VALUE is %e\n",val);
#endif
		return(val);
	}
}

/* ----------------------------------- */
/** Gets index of the non/repeating data
 * @param pos pos of line in SetFrom
 * @param valType type of data
 * @param repeat if value is in "repeating" part (AF_YES of AF_NO)
 * @param repNum set number for repeatable data (1,2,3,n !!)
 * @param SetL field with values
 * @param SetLenL lenth of SetL
 * @param SetFrom starts in SetL
 * @param SetLen lenth of SetFrom
 * @param valList list of possible values
 * @param valLen lenth of valList
 * @param valListRP list of possible repeating values
 * @param valLenRP lenth of valListRP
 * @return value
 */
long femGetRepValIndex(long pos, long valType, 
										long repNum,
										double *SetL, long SetLenL,
										long *SetFrom, long SetLen,
										long *valList, long valLen,
										long *valListRP, long valLenRP
										)
{
	static long index = 0 ;
	int i;
	int repeat = AF_YES ;
	long i_pos = -1 ;
	long len = 0;

#ifdef DEVEL
	if (pos >= SetLen) 
	{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Value outside field requested"));
#endif
		return(-1);
	}
#endif

	if (repNum <= 0) {repeat = AF_NO;}

	len = femFldItemLen(pos, SetFrom, SetLen, SetLenL) ;

	if (repeat == AF_NO)
	{
		/* non-repeating */
		for (i=0; i<valLen; i++)
		{
			if (valType == valList[i])
			{
				i_pos = i ;
				break;
			}
		}
		if (i_pos == -1)
		{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s (%li) %s!\n", _("Requested non-repeating index"),valType, _("is invalid"));
#endif
			return(-1);
		}
	}
	else
	{
		/* repeating */
		for (i=0; i<valLenRP; i++)
		{
			if (valType == valListRP[i])
			{
				i_pos = i + valLenRP*(repNum-1) + valLen;
				break;
			}
		}
		if (i_pos == -1)
		{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s (%li) %s!\n", _("Requested repeating index"),valType, _("is invalid"));
#endif
			return(0);
		}
	}

	if (i_pos >= len)
	{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Value outside field requested - program error"));
	fprintf(msgout,"[E] %s (%li>%li)(%li|%li|%li)!\n", _("Value outside field requested - program error"),i_pos,len, pos, valType,repNum);
#endif
		return(-1);
	}
	else
	{
		index = SetFrom[pos]+i_pos ;
		return(index);
	}
}
/* ----------------------------------- */

/** Puts/Adds value to  non/repeating data set
 * @param pos pos of line in SetFrom
 * @param valType type of data
 * @param repeat if value is in "repeating" part (AF_YES of AF_NO)
 * @param repNum set number for repeatable data (1,2,3,n !!)
 * @param SetL field with values
 * @param SetFrom starts in SetL
 * @param SetLen lenth of SetFrom
 * @param valList list of possible values
 * @param valLen lenth of valList
 * @param valListRP list of possible repeating values
 * @param valLenRP lenth of valListRP
 * @param AddMode (AF_YES of AF_NO)
 * @param Val value ;-)
 * @return state value
 */
double femAddPutRepVal(long pos, long valType, 
										long repNum,
										double *SetL, long SetLenL,
										long *SetFrom, long SetLen,
										long *valList, long valLen,
										long *valListRP, long valLenRP,
										long AddMode,
										double val
										)
{
	int i;
	int repeat = AF_YES ;
	long i_pos = -1 ;
	long len = 0;

#ifdef DEVEL
	if (pos >= SetLen) 
	{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Value outside field requested"));
#endif
		return(AF_ERR_VAL);
	}
#endif

	if (repNum <= 0) {repeat = AF_NO;}

	len = femFldItemLen(pos, SetFrom, SetLen, SetLenL) ;

	if (repeat == AF_NO)
	{
		/* non-repeating */
		for (i=0; i<valLen; i++)
		{
			/*fprintf(msgout,"iii(%i)iii\n",i);*/
			if (valType == valList[i])
			{
				i_pos = i ;
				break;
			}
		}
		if (i_pos == -1)
		{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s (%li) %s!\n", _("Requested added non-repeating value"),valType, _("is invalid"));
#endif
			return(AF_ERR_VAL);
		}
	}
	else
	{
		/* repeating */
		for (i=0; i<valLenRP; i++)
		{
			if (valType == valListRP[i])
			{
				i_pos = i + valLenRP*(repNum-1) + valLen;
				break;
			}
		}
		if (i_pos == -1)
		{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s (%li) %s!\n", _("Requested added repeating value"),valType, _("is invalid"));
#endif
			return(AF_ERR_VAL);
		}
	}

	if (i_pos >= len)
	{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s (%li>%li)!\n", _("Value outside field requested - program error"),i_pos,len);
#endif
		return(AF_ERR_VAL);
	}
	else
	{
		if (AddMode == AF_YES)
		{
			SetL[SetFrom[pos]+i_pos] += val;
		}
		else
		{
			SetL[SetFrom[pos]+i_pos]  = val;
		}

		return(AF_OK);
	}
}

/**  returns index of fld1 and fld2 for (fld1 == val1) AND (fld2 == val2)
 * @param val1 1st value
 * @param fld1 1st field
 * @param val2 2nd value
 * @param fld2 2nd field
 * @param fldLen length of fld1 and fld2 (fields must be the same size)
 * @return index (or -1 if fails)
 */
long fem2IntPos(long val1, long *fld1, long val2, long *fld2, long fldLen)
{
	long i;
	static long pos;

	pos = -1 ;

	for (i=0; i<fldLen; i++)
	{
		if ( (val1 == fld1[i]) && (val2 == fld2[i]) )
		{
			pos = i;
			return(pos);
		}
	}
	return(-1);
}
/** Returnes number of REPEATING data sets
 * @param pos pos of line in SetFrom
 * @param valType type of data
 * @param repeat if value is in "repeating" part (AF_YES of AF_NO)
 * @param repNum set number for repeatable data (1,2,3,n !!)
 * @param SetL field with values
 * @param SetLenL lenth of SetL
 * @param SetFrom starts in SetL
 * @param SetLen lenth of SetFrom
 * @param valList list of possible values
 * @param valLen lenth of valList
 * @param valListRP list of possible repeating values
 * @param valLenRP lenth of valListRP
 * @return value
 */

long femGetRepNum(long pos,
										double *SetL, long SetLenL,
										long *SetFrom, long SetLen,
										long *valList, long valLen,
										long *valListRP, long valLenRP
										)
{
	long len = 0;
  double repNum0 = 0.0 ;
	static long repNum = 0;

#ifdef DEVEL
	if (pos >= SetLen) 
	{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Value outside field requested"));
#endif
		return(0);
	}
#endif

  if (valLenRP <= 0) {return(0);}

	len = femFldItemLen(pos, SetFrom, SetLen, SetLenL) ;

  repNum0 = ((double)(len-valLen)/(double)valLenRP) ;
  if ((long)repNum0 <= 0) {return(0);}

  if (repNum0 > (long)repNum0)
  {
    repNum = (long)repNum0 + 1 ;
  }
  else
  {
    repNum = (long)repNum0 ;
  }

  return(repNum);
}

/* end of fem_accs.c */
