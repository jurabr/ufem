/*
   File name: fdb_res.h
   Date:      2003/12/21 14:48
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

	 Database for FEM: results

   $Id: fdb_res.h,v 1.12 2005/02/13 23:23:40 jirka Exp $
*/

#ifndef __FDB_RES_H__
#define __FDB_RES_H__

#include "fdb_fem.h"
#include "fem_elem.h"

#define  PATH_NUM  10   /* max. number of paths                */
#define  PATH_LEN  256  /* max. lenght of path (num. of nodes) */

typedef struct
{
  long    elen ;
  long    vallen;
  long   *e_pos ;
  long   *e_from ;
  long   *e_rlen ;
  double *e_rval ;
}tResProc;


#define fdbResEPos(res_pos) ResElem[ResActStep].i_pos[res_pos]

/* results on elements */
typedef struct
{
	long    empty;     /* AF_YES|AF_NO - if data are loaded   */
  long    set_id  ;  /* result set identification           */
  double  set_time ; /* time in time units (if needed)      */
  long    i_len ;    /* lenght of i_id, i_pos, from, len    */
  long   *i_id  ;    /* item ID                             */
  long   *i_pos ;    /* item position in NODE or ELEM table */
  long   *from  ;    /* starting point in "data"            */
  long   *len   ;    /* number of values in "data"          */
  long    data_len ; /* lenght of "data"                    */
  double *data ;     /* list of values                      */
}tRes ;

/* average results on nodes */
typedef struct
{
  long    empty  ;   /* AF_YES|AF_NO - if data are loaded   */
  long    set_id ;   /* result set identification           */
  long    type   ;   /* type of data (RES_SX etc.)          */
  long    len    ;   /* lenght of "data" == lenght of NODE  */
  double *data   ;   /* list of values                      */
}tResAve;


/* path for data plot */
typedef struct
{
  long     len  ;      /* number of nodes                   */
  long    *node ;      /* list of nodes                     */
  char     desc[257] ; /* path description                  */
}tResPath;



extern long ResLen ;
extern long ResActStep ;

extern tRes *ResNode ;
extern tRes *ResElem ;
extern tRes *ResReac ;

extern long fdbResComputed ;

extern tResAve *aRes ;
extern long aResLen  ;
extern long aResAct  ;

extern tResPath femPath[] ;
extern long femActivePath ;

extern int fdbAllocResSpacePtrs(void);
extern void fdbFreeResSpacePtrs(void);
extern int fdbResReadStep(long step_pos, double set_time, char *fname);
extern int fdbResReadStepFromMPI(long step_pos, double set_time, char *fname);
extern void fdbResFreeStep(long step_pos);

extern int fdbResCopyStep(long orig_pos, long step_pos, double set_time);

extern int fdbResMaxMinAny(tRes *Res, double *max, double *min);
extern int fdbResMaxMinNode(long dir_index, double *max, double *min);
extern int fdbResMaxMinNodeUsum(double *max, double *min) ;
extern int fdbResMaxMinElem(long type, double *max, double *min);

extern long fdbResElemNumRepSets(long res_pos, long etype);
extern double fdbResElemGetVal(long res_pos, long etype, long type, long set);
extern double fdbResNodeGetVal(long node_pos, long dof);


extern long femCheckHaveResult(long etype, long res_type);
extern long femCheckHaveResultNoRep(long etype, long res_type);
extern long femCheckHaveResultRep(long etype, long res_type);

/* average results: */
extern void fdbAvResFreeSpace(void);
extern int fdbAvResPrepareData(long len, long *times, long *types);
extern int fdbAvResMaxMin(long pos, double *max, double *min);
extern double fdbAvResGetVal(long type, long n_pos);

/* printing of results */
extern int fdbResListReacts(FILE *fw);
extern int fdbResListDOFSolu(FILE *fw);
extern int fdbResListElemSolu(FILE *fw, long *res_type, long type_len);
extern int fdbResListAvSolu(FILE *fw, long *res_type, long type_len);
extern double fdbGetOneReact(long node, long dof);


/* monte carlo results */
extern int fdb_read_monte_res(char *fname);

/* result selections */
extern int fdbTestResElemSolu(long e_pos, long res_type, double min, double max);
extern int fdbTestResSelNodesDOF(long npos, long dof, double from, double to);

/* misc stuff:  */
extern double fdbGetAverDOF(long dof);
extern double fdbGetSumReact(long dof);
extern double fdbGetSetTime(void);

/* results computed in postprocessor: */ 
extern int fdbResComp_IsComp(long restype);
extern double fdbResComp_Val(long er_pos, long i_rep, long etype, long stress_type);

/* listing of (currently loaded) result sets: */
extern int fdbResListSets(int header, FILE *fw);
extern int fdbResListSetsPrn(FILE *fw);

/* listings of results to file: */
extern int fdbGetLoopRawRes_DOF(FILE *fw, long n_id, long dof_type);

/* path operations: */
extern void resPathNull(int num);
extern void resPathFree(int num);
extern void resPathNullAll(void);
extern void resPathFreeAll(void);
extern void femPathInit(void);
extern int resPathAlloc(int num);
extern int resPathSetName(char *name, int num);
extern int resPathAddNode(int num, long node);
extern int resPathChangeNode(int num, long node_old, long node_new);
extern int resPathDelLastNode(int num, int node);
extern int femPathList(FILE *fw, long from, long to);
extern int femPathResPrn(FILE *fw, long path_num, long *res_type, long type_len);

#endif

/* end of fdb_res.h */
