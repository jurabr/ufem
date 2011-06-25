/*
   File name: fem_dama.h
   Date:      2003/04/07 10:04
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

	 FEM Solver - data manipulations (public routines)

  $Id: fem_dama.h,v 1.14 2005/01/01 21:01:25 jirka Exp $
*/


#ifndef __FEM_DAMA_H__
#define __FEM_DAMA_H__

#include "fem.h"
#include "fem_mem.h"
#include "fem_elem.h"

/* Memory operations: */
extern int femDataNull(void);
extern int femDataFree(void);
extern int femResNull(void);
extern int femResFree(void);
extern int femBackResNull(void);
extern int femBackResFree(void);
extern int femBackResGet(void);
extern int femBackResPut(void);

/* I/O Operations: */
extern int femReadInput(char *str);
extern int femReadRes(char *fname);
extern int femWriteRes(char *fname);

extern int femReadThermRes(char *fname, tVector *u);
extern int femWriteThermDOFS(char *fname, tVector *u);

extern void femOpenSolNormFile(char *fname);
extern void femWriteSolNorm(long iter, double load_mult, double val, double crit_min, double crit_max);
extern void femCloseSolNormFile(void);


extern int femWriteNodeResVTK(char *fname);
extern int femWriteNodeResTxt(char *fname);

/* Data access: */
extern long femFindIntVal(long val, long *fld, long fldLen);
extern long femGetIntPos(long pos, long *fld, long fldLen);
extern double femGetDblPos(long pos, double *fld, long fldLen);
extern long femGetIntLPos(long pos, long val_pos, long *from_fld, long *fldL, long fldLLen);
extern double femGetDblLPos(long pos, long val_pos, long *from_fld, double *fldL, long fldLLen);
extern long femFldItemLen(long Pos, long *fldFrom, long fldLen, long fldLenL);
extern double femGetRepVal(long pos, long valType,
										long repNum, 
										double *SetL,long SetLenL,
										long *SetFrom, long SetLen,
										long *valList, long valLen,
										long *valListRP, long valLenRP
										);
extern double femAddPutRepVal(long pos, long valType, 
										long repNum,
										double *SetL,long SetLenL,
										long *SetFrom, long SetLen,
										long *valList, long valLen,
										long *valListRP, long valLenRP,
										long AddMode,
										double val
										);
extern long fem2IntPos(long val1, long *fld1, long val2, long *fld2, long fldLen);

extern long femGetRepValIndex(long pos, long valType, 
										long repNum,
										double *SetL, long SetLenL,
										long *SetFrom, long SetLen,
										long *valList, long valLen,
										long *valListRP, long valLenRP
										);

extern long femGetRepNum(long pos,
										double *SetL, long SetLenL,
										long *SetFrom, long SetLen,
										long *valList, long valLen,
										long *valListRP, long valLenRP
										);


	
/* Nodes: */
#define femGetNIDPos(pos) femGetIntPos(pos, nID, nLen)
#define femFindNbyID(n_id) femFindIntVal(n_id, nID, nLen)

#define femGetNCoordPosX(pos) femGetDblPos(pos, n_x, nLen)
#define femGetNCoordPosY(pos) femGetDblPos(pos, n_y, nLen)
#define femGetNCoordPosZ(pos) femGetDblPos(pos, n_z, nLen)

/* Elements: */
#define femGetEIDPos(pos) femGetIntPos(pos, eID, eLen)
#define femFindEbyID(e_id) femFindIntVal(e_id, eID, eLen)

#define femGetETypePos(pos) femGetIntPos(pos, eType, eLen)
#define femGetERSPos(pos) femGetIntPos(pos, eRS, eLen)
#define femGetEMPPos(pos) femGetIntPos(pos, eMP, eLen)

#define femGetENodePos(e_pos,n_pos) femGetIntLPos(e_pos, n_pos, eFrom, eNodesL, eLenL)


/* Real sets: */
#define femGetRSIDPos(pos) femGetIntPos(pos, rsID, rsLen)
#define femGetRSTypePos(pos) femGetIntPos(pos, rsType, rsLen)
#define femFindRSbyID(rs_id) femFindIntVal(rs_id, rsID, rsLen)
#define femRSNumPos(rs_pos) femFldItemLen(rs_pos, rsFrom, rsLen, rsLenL)

/*#define femGetRSValPos(eType, pos, valType, repNum) femGetRepVal(pos, valType, repNum, rsValL, rsLenL, rsFrom, rsLen, Elem[eType].real, Elem[eType].rs, Elem[eType].real_rp, Elem[eType].rs_rp ) obsolete */
extern double femGetRSValPos(long ePos, long valType, long repNum);
extern long femGetRSRepNumPos(long ePos);

#define femSetRSValPos(eType, pos, valType, repNum, val) femAddPutRepVal(pos, valType, repNum, rsValL, rsLenL, rsFrom, rsLen, Elem[eType].real, Elem[eType].rs, Elem[eType].real_rp, Elem[eType].rs_rp, AF_NO, val) 


/* Material properties: */
#define femGetMPIDPos(pos) femGetIntPos(pos, mpID, mpLen)
#define femGetMPTypePos(pos) femGetIntPos(pos, mpType, mpLen)
#define femFindMPbyID(mp_id) femFindIntVal(mp_id, mpID, mpLen)
#define femMPNumPos(mp_pos) femFldItemLen(mp_pos, mpFrom, mpLen, mpLenL)

/*#define femGetMPValPos(mType, pos, valType, repNum) femGetRepVal (pos, valType, repNum, mpValL, mpLenL, mpFrom, mpLen, Mat[mType].val, Mat[mType].num, Mat[mType].val_rp, Mat[mType].num_rp) obsolete */
extern double femGetMPValPos(long ePos, long valType, long repNum);
extern long femGetMPRepNumPos(long ePos);

extern long femGetSystemMatType(long ePos); /* system id of material */

/* #define femSetMPValPos(mType, pos, valType, repNum, val) femAddPutRepVal (pos, valType, repNum, mpValL, mpLenL, mpFrom, mpLen, Mat[mType].val, Mat[mType].num, Mat[mType].val_rp, Mat[mType].num_rp, AF_NO, val) faulty for some reason.... */


/* Element results */
#define femGetEResVal(ePos, Type, set_rp) femGetRepVal(ePos, Type, set_rp, resEVal, resELen, resEFrom, eLen, Elem[eType[ePos]].nres, Elem[eType[ePos]].res, Elem[eType[ePos]].nres_rp, Elem[eType[ePos]].res_rp)
#define femAddEResVal(ePos, Type, set_rp, val) femAddPutRepVal(ePos, Type, set_rp, resEVal, resELen, resEFrom, eLen, Elem[eType[ePos]].nres, Elem[eType[ePos]].res, Elem[eType[ePos]].nres_rp, Elem[eType[ePos]].res_rp, AF_YES, val)
#define femPutEResVal(ePos, Type, set_rp, val) femAddPutRepVal(ePos, Type, set_rp, resEVal, resELen, resEFrom, eLen, Elem[eType[ePos]].nres, Elem[eType[ePos]].res, Elem[eType[ePos]].nres_rp, Elem[eType[ePos]].res_rp, AF_NO, val)

#endif

/* end of fem_dama.h */
