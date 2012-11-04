/*
   File name: fdb_fem.h
   Date:      2003/08/09 22:47
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

	 FEM - database integration

	 $Id: fdb_fem.h,v 1.33 2005/02/12 18:34:50 jirka Exp $
*/

#ifndef __FDB_FEM_H__
#define __FDB_FEM_H__

#include "fem_math.h"
#include "fem_mont.h"

#include "fdb.h"
#include "fdb_data.h"

#define  U_X 1
#define  U_Y 2
#define  U_Z 3

#define PICK_GRID_XY 0
#define PICK_GRID_YZ 1
#define PICK_GRID_ZX 2

extern tInData inputData ;
extern tTab    InputTab[] ;

extern long    InputTabLen ;

extern long fdbSummNodalLoads ;
extern long fdbSummNodalDisps ;

extern long nodeStat  ;      /* node change counter     */
extern long elemNodeStat ;   /* sync status for element */
extern long nloadNodeStat ;  /* sync status for nload   */
extern long ndispNodeStat;   /* sync status for ndisp   */

extern long elemStat ;       /* element change counter  */
extern long eloadElemStat ;  /* sync status for eload   */

extern long kpointStat ;      /* keypoint change counter     */
extern long entityKPointStat; /* sync status for geometry entity */

extern long pick_grid_x0 ; 
extern long pick_grid_y0 ;
extern long pick_grid_z0 ;
extern long pick_grid_siz_x ;
extern long pick_grid_siz_y ;
extern double pick_grid_dx ;
extern double pick_grid_dy ;
extern double pick_grid_angle_in  ;
extern double pick_grid_angle_out ;
extern long   pick_grid_plane ;

extern int   femOneDirSupp ;

extern long      fdb_matrix_len ;
extern tMatrix  *fdb_matrix ;

/* custom dialog box data are here: */
extern int  custom_dlg_len   ;
extern char *custom_dlg_title[30] ;
extern char *custom_dlg_value[30] ;


#define fdbInputTestSelect(tab_id, i)  fdbTestSelect(&InputTab[tab_id], &inputData.intfld, i)

#define fdbInputSelectIntInterval(tab_id, collumn, mode, from, to) fdbSelectIntInterval(&InputTab[tab_id], &inputData.intfld, collumn, mode, from, to)
#define fdbInputSelectDblInterval(tab_id, collumn, mode, from, to) fdbSelectDblInterval(&InputTab[tab_id], &inputData.intfld, &inputData.dblfld, collumn, mode, from, to)

#define fdbInputSelectIntList(tab_id, collumn, mode, val_num, values) fdbSelectIntList(&InputTab[tab_id], &inputData.intfld, collumn, mode, val_num, values)

/* Functions for INPUT DATA __ONLY__ : */
extern int fdbInputIntChangeVals(
													long tab_id,
													long col_id, 
													long from, 
													long count, 
													long *val, 
													long add);
extern int fdbInputDblChangeVals(
													long    tab_id,
													long    col_id, 
													long    from, 
													long    count, 
													double *val, 
													long    add);
extern int fdbInputIntChangeValsFast(
													long tab_id,
													long col_id, 
													long from, 
													long count, 
													long *val, 
													long add);
extern int fdbInputDblChangeValsFast(
													long    tab_id,
													long    col_id, 
													long    from, 
													long    count, 
													double *val, 
													long    add);

extern int fdbInputAppendTableRow(long tab_id, long count, long *index);
extern int fdbInputInsertTableRow(long tab_id, long from, long count);
extern int fdbInputRemoveTableRow(long tab_id, long from, long count);
extern long fdbInputCountInt(long tab_id, long col_id, long val, long *first);
extern long fdbInputCountIntAll(long tab_id, long col_id, long val, long *first);
extern long fdbInputCountTwoInt(long tab_id, long col_id1, long val1, long col_id2, long val2, long *first);
extern long fdbInputCountThreeInt(long tab_id, 
                           long col_id1, long val1, 
                           long col_id2, long val2, 
                           long col_id3, long val3, 
                           long *first);
extern long fdbInputFindNextInt(long tab_id, long col_id, long previous, long val);
extern long fdbInputFindMaxInt(long tab_id, long col_id);
extern long fdbInputFindMaxIntSlow(long tab_id, long col_id);
extern long fdbInputCountDbl(long tab_id, long col_id, double val, long *first);
extern long fdbInputFindNextDbl(long tab_id, long col_id, long previous, double val);
extern double fdbInputFindMaxDbl(long tab_id, long col_id);
extern double fdbInputFindMinDbl(long tab_id, long col_id);
	

extern long fdbInputGetInt(long tab_id, long col_id, long pos);
extern double fdbInputGetDbl(long tab_id, long col_id, long pos);

extern int fdbInputPutInt(long tab_id, long col_id, long pos, long val);
extern int fdbInputPutDbl(long tab_id, long col_id, long pos, double val);

extern int fdbInputPrintTabLine(FILE *fw, long tab_id, long row, long print_mode);


extern long fdbInputTabLenSel(long tab_id);
extern long fdbInputTabLenAll(long tab_id);
extern int fdbSelectElemPos(long collumn, long mode, double from, double to);
extern int fdbSelectElemRes(long collumn, long mode, double from, double to);
extern int fdbSelectNodeRes(long collumn, long mode, double from, double to);

extern int fdbSelectEntPos(long collumn, long mode, double from, double to);


extern void fdbInputListSumm(int header, FILE *fw);
extern int fdbInputListSummPrn(FILE *fw);

/* default values: */

extern long fdbSetInputDefET(long set_to) ;
extern long fdbSetInputDefRS(long set_to) ;
extern long fdbSetInputDefMAT(long set_to) ;
extern long fdbSetInputDefEset(long set_to) ;
extern long fdbSetInputLoadSet(long set_to) ;
extern long fdbSetInputDefDiv(long set_to);
extern long fdbSetInputDefGEnt(long set_to);

/* data input/output: */

extern int fdbInputSaveData(char *fname) ;
extern int fdbInputReadData(char *fname) ;
extern int fdbInputCleanData(void) ;

extern int fdbExport(char *fname, long format, long *opts, long optlen);
extern int fdbImport(char *fname, long format, long *opts, long optlen);
extern int fdbImport2(char *fname, char *fname1, long format, long *opts, long optlen);


/* ------------------------------------------------------------------ */

/* handling of FEM data: */
extern int f_n_new_change(long id, double x, double y, double z);
extern int f_n_delete(long id);
extern int f_n_list(long id, int header, FILE *fw);
extern int f_n_list_prn(FILE *fw, long from, long to);
extern int f_n_join_all(void);

extern int f_e_new_change(long id, long et, long rs, long mat, long set);
extern int f_en_change(long id, long *nodes, long nodes_len);
extern int f_e_delete(long id);
extern int f_e_list(long id, int header, FILE *fw);
extern int f_e_list_prn(FILE *fw, long from, long to);
extern int f_e_change_set(long id, long set);

extern int f_n_split_through_e_pos(long kPos);
extern int f_n_split_through_e_id(long id);
extern int f_n_dist(long k1, long k2, double *dx, double *dy, double *dz);

extern int f_et_new_change(long id, long etyp);
extern int f_et_delete(long id);
extern int f_et_list_prn(FILE *fw, long from, long to);

extern int f_rset_new_change(long id, long etyp, long rep_num);
extern int f_rs_change(long id, long type, double val, long rep_index);
extern int f_rset_delete(long id);
extern int f_rs_list_prn(FILE *fw, long from, long to);

extern int f_mat_new_change(long id, long etyp, long rep_num);
extern int f_mp_change(long id, long type, double val, long rep_index);
extern int f_mat_delete(long id);
extern int f_mat_list_prn(FILE *fw, long from, long to);

extern int f_nd_new_change(long id, long node, long type, long set, double val);
extern int f_nd_delete(long id);
extern int f_nd_list(long id, int header, FILE *fw);
extern int f_nd_list_prn(FILE *fw, long from, long to);
extern int f_nd_change_set(long id, long set);

extern int f_nl_new_change(long id, long node, long type, long set, double val);
extern int f_nl_delete(long id);
extern int f_nl_list(long id, int header, FILE *fw);
extern int f_nl_list_prn(FILE *fw, long from, long to);
extern int f_nl_change_set(long id, long set);

extern int f_el_new_change(long id, long element, long type, long set, long nval, double val[]);
extern int f_el_delete(long id);
extern int f_el_list(long id, int header, FILE *fw);
extern int f_el_change_set(long id, long set);

extern int f_gl_new_change(long id, long set, long dir, double val);
extern int f_gl_delete(long id);
extern int f_gl_list(long id, int header, FILE *fw);
extern int f_gl_list_prn(FILE *fw, long from, long to);

extern int f_n_gen_1d(long num, long mode, long dir,  double dx, double dy, double dz, double dx1, double dy1, double dz1);
extern int f_e_n_gen_1d(long num, long mode, long dir,  double dx, double dy, double dz, double dx1, double dy1, double dz1);
extern int f_nd_gen_1d(long num, long mode, long dir,  double dx, double dy, double dz, double dx1, double dy1, double dz1);
extern int f_nl_gen_1d(long num, long mode, long dir,  double dx, double dy, double dz, double dx1, double dy1, double dz1);

extern int n_move_coord(long nPos, double dx, double dy, double dz);


/* geometry */
extern int f_k_new_change(long id, double x, double y, double z, long *new_id);
extern int f_k_delete(long id);
extern int f_k_list(long id, int header, FILE *fw);
extern int f_k_list_prn(FILE *fw, long from, long to);

extern int f_k_gen_1d(long num, long mode, long dir, double dx, double dy, double dz, 
    double dx1, double dy1, double dz1);

extern int f_k_split_through_ge_pos(long kPos);
extern int f_k_split_through_ge_id(long id);
extern int f_k_join_all(void);
extern int f_k_dist(long k1, long k2, double *dx, double *dy, double *dz);

extern int f_ent_extrude_area(
    long  area_id, 
    long  k_len,
    long *klist,
    long  et, 
    long  rs,
    long  mat,
    long  zdiv);
extern int f_ent_extrude_line(
    long  area_id, 
    long  k_len,
    long *klist,
    long  et, 
    long  rs,
    long  mat,
    long  zdiv);



extern int f_ent_new_change(long id, long ent_type, long et, long rs, long mat, long set);
extern int f_entkp_change(long id, long *nodes, long nodes_len, long ent_type);
extern int f_entkp_div_change(long id, long *div, long div_len);
extern int f_ent_delete(long id, long ent_type);
extern int f_ent_list_prn(FILE *fw, long from, long to, long ent_type);
extern int f_ent_change_set(long id, long set);

extern int f_ent_k_gen_1d(long num, long mode, long dir, double dx, double dy, double dz, 
    double dx1, double dy1, double dz1);

extern int f_ent_create_dim( long type, long id, 
		double x, double y, double z, double dx, double dy, double dz);

extern int k_move_coord(long nPos, double dx, double dy, double dz);
extern int k_rotate_coord(long nPos, long plane, double angle, double x0, double y0, double z0);

/* counters and stats */
extern void fdbInputResetStats(void);
extern void fdbInputSyncStats(void);

extern long fdbEnodePos(long e_pos, long n_num);
extern long fdbNloadNodePos(long nl_pos);
extern long fdbNdispNodePos(long nd_pos);
extern long fdbEloadElemPos(long el_pos);

extern long fdbEntKpPos(long e_pos, long n_num);

extern void fdbInputUpdateNodeStats(void);
extern void fdbInputUpdateElemStats(void);
extern void fdbInputUpdateKPointStats(void);
extern void fdbInputUpdateEntStats(void);

extern int fdbInputRenumFromFlds(long tab_target,
                          long col_target,
                          long col_target_name,
                          long tab_src, 
                          long col_src);
/* selections */
extern int fem_sele_nsle(void);
extern int fem_sele_nsld(void);
extern int fem_sele_nslf(void);
extern int fem_sele_eslel(void);
extern int fem_sele_elsle(void);
extern int fem_sele_fsln(void);
extern int fem_sele_dsln(void);
extern int fem_sele_esln(void);
extern int fem_sele_kslge(void);
extern int fem_sele_geslk(void);

extern char *femInputStringFromFldInt(long tab, long col, char *varGrp);
extern char *fdbFemStrFromDbl(double number);
extern char *fdbFemStrFromInt(long number);
extern char *fdbFemStrEResListInt(void);
extern char *fdbFemStrSuppTypListInt(void);
extern char *fdbFemStrNLoadTypListInt(void);

extern void fdbCoodSysReset(void);
extern int fdbCoodSysSet(long type, double x, double y, double z);
extern int fdbCSysGetXYZ(double    x, double    y, double    z, 
		              double *n_x, double *n_y, double *n_z);
extern int fdbCSysGetCylCoord(double    x, double    y, double    z, 
		              		 double *n_x, double *n_y, double *n_z);
extern int fdbCSysGetCylCoordDEG(double    x, double    y, double    z, 
		              		 double *n_x, double *n_y, double *n_z);
extern char *fdbCSysTypeStr(void);
extern void fdbCSysTypeList(FILE *fw);
extern int fdbCSysTypePrn(FILE *fw);

extern int fdbInputTestDep(long val, long tested_tab, long tested_col);


/* load steps */
extern int f_step_new_change(long id, double time, long val_num, long *values);
extern int f_stepval_change(long id, long set, double val);
extern int f_step_delete(long id);
extern int f_step_list(long id, int header, FILE *fw);
extern int f_step_list_prn(FILE *fw, long from, long to);

extern int fdbTS2D_tensor_scale(long   plane,
	 	                     double px, 
												 double py, 
												 double pz, 
												 long   lines, 
												 double radius, 
												 long   linediv,
												 double *a,
												 double *b,
												 double *angle,
												 long   *mat_type_base);

extern int fdbTS2D_global_tensor_scale(
				                 long   plane,
	 	                     double px_glob, 
												 double py_glob, 
												 double pz_glob, 
												 double step,
												 long   num,
												 long   mat,
												 long   lines, 
												 double radius, 
												 long   linediv,
												 double *a_glob,
												 double *b_glob,
												 double *angle_glob,
												 double *aniso
												 );

extern int fdb_rand_new(long id, long type, long itm, long sitm, long iter);
extern int fdb_rand_delete(long id);
extern int fdb_rand_check(void);
extern int fdb_rand_list(long id, int header, FILE *fw);
extern int fdb_rand_list_prn(FILE *fw, long from, long to);


extern long fdb_user_get_sel(long tab, long tab_id, long id);
extern double fdb_user_get_node_xyz(long id, long dir);
extern long fdb_user_get_elem_prop(long id, long dir);

extern long fdb_user_get_eres_no_rep_num(long elem_id, long res_type);
extern long fdb_user_get_eres_rep_num(long elem_id, long res_type);
extern double fdb_user_get_eres(long elem_id, long res_type, long ipoint, long maxminav);

extern double fdb_user_get_ndisp_val(long node_id, long type);

extern long fdb_user_get_first_selected(long type, long type_id);

extern double fdb_user_get_kp_xyz(long id, long dir);

extern int pick_grid_compute_xyz(long pos, double *x, double *y, double *z);
extern int fdb_set_grid(
    long plane,
    double dx, double dy, 
    long nx, long ny, 
    double x0, double y0, double z0,
    double ang_in,
    double ang_out
    );

extern int fdb_mod_n_l_d(long type, long from, long to, long op, double val);

/* math operations: */
extern void fdbFreeMatrixSpace(void);
extern int fdbResizeMatrixSpace(long m_len);
extern int fdbMatrixPrint(tMatrix *matrix, int type, char *fname, long rewrite);


/* path operations */
extern char *femInputStringFromPathList(long nummm);
#endif

/* end of fdb_fem.h */
