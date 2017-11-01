/*
   File name: fem_spio.c
   Date:      2009/02/19 17:08
   Author:    Jiri Brozovsky

   Copyright (C) 2009 Jiri Brozovsky

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

   Finite Element Solver - special types of I/O operations
*/

#include "fem_dama.h"
#include "fem_data.h"
#include "fem_mem.h"
#include "fem_math.h"
#include "fem_para.h"

/* SILO library interface: https://wci.llnl.gov/codes/silo */
#ifdef FEM_USE_SILO 
#include <silo.h>
#endif

#define SPIO_RES_NUM 12

extern int femPrinc3D(tVector *sx, tVector *s1);

/** Find is given element type provides given kind of element result
 * @param eT type of element
 * @param type results type
 * @return AF_YES if provides, AF_NO if not
 */
long femFindResTypeET(long eT, long type)
{
  long i ;

  for (i=0; i<Elem[eT].res; i++)
    { if (Elem[eT].nres[i] == type) { return(AF_YES) ; } }

  for (i=0; i<Elem[eT].res_rp; i++)
    { if (Elem[eT].nres_rp[i] == type) { return(AF_YES) ; } }

  return(AF_NO) ;
}

/** Computes average results in nodes
 * @param type result type
 * @param fld field for results 
 * @param intfld field for numbers of results in node
 * @param fldLen length of fld ( must be identical to nLen!)
 * @return status
 * */
int femGetAvNodeRes(long type, double *fld, long *intfld, long fldLen)
{
  int     rv = AF_OK;
  long    i, j, eT, npos ;
  double  val ;

  for (i=0; i<nLen; i++)
  {
    fld[i]    = 0.0 ;
    intfld[i] = 0 ;
  }

  for (i=0; i<eLen; i++)
  {
    eT = femGetETypePos(i) ;

    if ( femFindResTypeET(eT, type) == AF_YES)
    {
      for (j=0; j<Elem[eT].nodes; j++)
      {
        npos = eNodesL[eFrom[i]+j] ;
        val = 0 ;
        Elem[eT].res_node(i, j, type, &val);

        fld[npos] += val ;
        intfld[npos]++ ;
      }
    }
  }

  for (i=0; i<nLen; i++)
  {
    if (intfld[i] > 0)
    {
      fld[i] /= ((double)intfld[i]) ;
    }
  }

  return(rv);
}

/** Writes brick data and results to file
 */
int femWriteNodeResTxt(char *fname)
{
  int     rv = AF_OK;
  FILE   *fr = NULL ;
  long     i, j ;
  double  *resfld[SPIO_RES_NUM] ;
  long    *intfld = NULL ;


  /* space for results in nodes */
  for (i=0; i<SPIO_RES_NUM; i++)
  {
    if ((resfld[i]=femDblAlloc(nLen)) == NULL)
    {
      for (j=0;j<i; j++)
      {
        femDblFree(resfld[j]) ;
      }
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Out of memory during VTK export"));
#endif
      return(AF_ERR_MEM);
    }
  }
  if ((intfld=femIntAlloc(nLen)) == NULL)
  {
    for (j=0;j<SPIO_RES_NUM; j++)
    {
      femDblFree(resfld[j]) ;
    }
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Out of memory during VTK export"));
#endif
      return(AF_ERR_MEM);
  }

  femGetAvNodeRes(RES_EX,  resfld[0], intfld, nLen);
  femGetAvNodeRes(RES_EY,  resfld[1], intfld, nLen);
  femGetAvNodeRes(RES_EZ,  resfld[2], intfld, nLen);
  femGetAvNodeRes(RES_SX,  resfld[3], intfld, nLen);
  femGetAvNodeRes(RES_SY,  resfld[4], intfld, nLen);
  femGetAvNodeRes(RES_SZ,  resfld[5], intfld, nLen);
  

  /* open file: ************************************** */
  if (fname == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid name for listing file"));
    return(AF_ERR_VAL) ;
  }
  else
  {
    if ((fr = fopen(fname, "w")) == NULL)
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n", _("Cannot open data file"));
#endif
      return(AF_ERR_IO);
    }
  }

  /* Write nodes: */
  fprintf(fr,"%li %i\n", nLen, 4+SPIO_RES_NUM);

  for (i=0; i<nLen; i++) 
  { 
    fprintf(fr,"%li %e %e %e", nID[i], n_x[i], n_y[i], n_z[i]); 
    for (j=0; j<SPIO_RES_NUM; j++) { fprintf(fr," %e", resfld[j][i]); }
    fprintf(fr,"\n");
  }

  /* freeing of data files */
  for (j=0;j<SPIO_RES_NUM; j++) { femDblFree(resfld[j]) ; }
  femIntFree(intfld) ;

  /* close file: ************************************* */
  if (fr != stdout)
  {
    if ((fclose(fr)) != 0)
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n",_("Error during closing of previous result file"));
#endif
      fr = NULL;
      return(AF_ERR_IO);
    }
  }
  return(rv);
}

long femToTypeVTK(long e_type)
{
  switch (e_type)
  {
    case 1: return(3); break; /* line */
    case 2: return(9); break; /* rectangle */
    case 3: return(1); break; /* line */
    case 4: return(10); break; /* tetrahedron */
    case 5: return(9); break; /* rectangle */
    case 6: return(1); break; /* line */
    case 7: return(1); break; /* line */
    case 8: return(25); break; /* brick20 */
    case 9: return(12); break; /* brick8 */
    case 10: return(12); break; /* brick8 */
    case 11: return(5); break; /* triangle */

    case 14: return(3); break; /* line */
    case 15: return(3); break; /* line */

    default: return(1); break ; /* point */
  }
  return(1);
}

/** Writes VTK file 
 */
int femWriteNodeResVTK(char *fname)
{
  int      rv = AF_OK;
  FILE    *fr = NULL ;
  long     i, j, num, et ;
  double  *resfld[SPIO_RES_NUM] ;
  long    *intfld = NULL ;
  tVector  sigma, sigma1;

  /* vectors for principal stresses: */
  femVecNull(&sigma);
  femVecNull(&sigma1);
  if ((rv=femVecFullInit(&sigma, 6)) != AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&sigma1, 6)) != AF_OK) { goto memFree; }

  if (nLen < 4)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Dataset is too small"));
#endif
    return(AF_ERR_SIZ) ;
  }

  /* space for results in nodes */
  for (i=0; i<SPIO_RES_NUM; i++)
  {
    if ((resfld[i]=femDblAlloc(nLen)) == NULL)
    {
      for (j=0;j<i; j++)
      {
        femDblFree(resfld[j]) ;
      }
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Out of memory during VTK export"));
#endif
      return(AF_ERR_MEM);
    }
  }
  if ((intfld=femIntAlloc(nLen)) == NULL)
  {
    for (j=0;j<SPIO_RES_NUM; j++)
    {
      femDblFree(resfld[j]) ;
    }
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Out of memory during VTK export"));
#endif
      return(AF_ERR_MEM);
  }

  femGetAvNodeRes(RES_EX,  resfld[0], intfld, nLen);
  femGetAvNodeRes(RES_EY,  resfld[1], intfld, nLen);
  femGetAvNodeRes(RES_EZ,  resfld[2], intfld, nLen);
  femGetAvNodeRes(RES_SX,  resfld[3], intfld, nLen);
  femGetAvNodeRes(RES_SY,  resfld[4], intfld, nLen);
  femGetAvNodeRes(RES_SZ,  resfld[5], intfld, nLen);
  femGetAvNodeRes(RES_SYZ,  resfld[6], intfld, nLen);
  femGetAvNodeRes(RES_SZX,  resfld[7], intfld, nLen);
  femGetAvNodeRes(RES_SXY,  resfld[8], intfld, nLen);

  /* Computing of principal stresses (in 3D): */
  for (i=0; i<nLen; i++) 
  { 
    for (j=3; j<9; j++) {  femVecPut(&sigma,j-2,resfld[j][i]); }
    femPrinc3D(&sigma, &sigma1);
    for (j=0; j<3; j++) {  resfld[9+j][i] = femVecGet(&sigma1,j+1); }
  }
  

  /* open file: ************************************** */
  if (fname == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Invalid name for listing file"));
    return(AF_ERR_VAL) ;
  }
  else
  {
    if ((fr = fopen(fname, "w")) == NULL)
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n", _("Can not open file"));
#endif
      return(AF_ERR_IO);
    }
  }

  fprintf(fr,"# vtk DataFile Version 2.0\n");
  fprintf(fr,"uFEM_model\n");
  fprintf(fr,"ASCII\n");
  fprintf(fr,"DATASET UNSTRUCTURED_GRID\n");
  fprintf(fr,"POINTS %li float\n", nLen);

  for (i=0; i<nLen; i++) { fprintf( fr,"%e %e %e\n", n_x[i], n_y[i], n_z[i] ); }

  fprintf(fr,"\nCELLS %li %li\n", eLen, eLenL+eLenL);

  for (i=0; i<eLen; i++) 
  { 
    et = femGetETypePos(i) ;
    num = Elem[et].nodes ;

    fprintf(fr,"%li", num); 
    for (j=0; j<num; j++) { fprintf(fr, " %li", eNodesL[eFrom[i]+j]); }
    fprintf(fr,"\n"); 
  }

  fprintf(fr,"\nCELL_TYPES %li\n", eLen);
  for (i=0; i<eLen; i++) 
  { 
    et = femGetETypePos(i) ;
    fprintf(fr,"%li\n", femToTypeVTK(et) );
  }

  /* result: */
  fprintf(fr,"\nPOINT_DATA %li\n", nLen);
  fprintf(fr,"SCALARS nodenumbers float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", (double)nID[i]); }

  /* result: */
  /*fprintf(fr,"\nPOINT_DATA %li\n", nLen);*/
  fprintf(fr,"SCALARS u_x float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) 
    { fprintf(fr,"%e \n", femVecGet(&u, femKpos(i, U_X))); }

  /*fprintf(fr,"\nPOINT_DATA %li\n", nLen);*/
  fprintf(fr,"SCALARS u_y float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) 
    { fprintf(fr,"%e \n", femVecGet(&u, femKpos(i, U_Y))); }

  /*fprintf(fr,"\nPOINT_DATA %li\n", nLen);*/
  fprintf(fr,"SCALARS u_z float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) 
    { fprintf(fr,"%e \n", femVecGet(&u, femKpos(i, U_Z))); }

  /* Strains and stresses: */

  fprintf(fr,"SCALARS e_x float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[0][i]); }

  fprintf(fr,"SCALARS e_y float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[1][i]); }

  fprintf(fr,"SCALARS e_z float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[2][i]); }

  fprintf(fr,"SCALARS s_x float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[3][i]); }

  fprintf(fr,"SCALARS s_y float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[4][i]); }

  fprintf(fr,"SCALARS s_z float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[5][i]); }

  fprintf(fr,"SCALARS s_yz float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[6][i]); }

  fprintf(fr,"SCALARS s_zx float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[7][i]); }

  fprintf(fr,"SCALARS s_xy float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[8][i]); }

  /* Principal stresses: */

  fprintf(fr,"SCALARS s_1 float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[9][i]); }

  fprintf(fr,"SCALARS s_2 float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[10][i]); }

  fprintf(fr,"SCALARS s_3 float 1 \n");
  fprintf(fr,"LOOKUP_TABLE default \n");
  for (i=0; i<nLen; i++) { fprintf(fr,"%e \n", resfld[11][i]); }



  /* freeing of data files */
  for (j=0;j<SPIO_RES_NUM; j++) { femDblFree(resfld[j]) ; }
  femIntFree(intfld) ;

memFree:
    femVecFree(&sigma);
    femVecFree(&sigma1);

  /* close file: ************************************* */
  if (fr != stdout)
  {
    if ((fclose(fr)) != 0)
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n",_("Error during closing of previous result file"));
#endif
      fr = NULL;
      return(AF_ERR_IO);
    }
  }
  return(rv);
}

/** Writes SILO (MeshTV/VisIt) file 
 */
#ifdef FEM_USE_SILO
int femWriteNodeResSILO(char *fname)
{
  int      rv = AF_OK;
  DBfile  *file = NULL ;
  double  *coordinates[3]; /* use n_x, n_y, n_z */
  int      shapetype[1]; /* element type data: only BRICK */
  int      shapesize[1] ;
  int      shapecnt[1] ;
  int      *nodelist = NULL ;
  double   *noderes  = NULL ;
  int      *matfld = NULL ;
  int      *matnos  = NULL ;
  int       dims[1] ; 
  DBfacelist *facelist ;

  long     i, j ;
  double  *resfld[SPIO_RES_NUM] ;
  long    *intfld = NULL ;
  tVector  sigma, sigma1;
  
  /* Test for compatible elements: BRICK9, BRICK10 only !!!*/
  for (i=0; i<rsLenL; i++)
  {
    if ((eType[i] < 9) || (eType[i] > 10))
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n",_("Incompatible element type: only BRICK9 and BRICK10 elements can be exported to SILO"));
#endif
      rv = AF_ERR_TYP; goto memFree ;
    }
  }

  /* Create the SILO file ---------------------------- */
  if ((file = DBCreate(fname, DB_CLOBBER, DB_LOCAL, NULL, DB_PDB))==NULL)
  {
    rv = AF_ERR_IO ; goto memFree ;
  }

	DBMkDir (file, "/static"); /* makes a directory inside SILO file   */
	DBSetDir(file, "/static"); /* sets the current directory with path */


  /* PREPARE GEOMETRY DATA --------------------------- */
  if ((nodelist = (int *)malloc(eLenL*sizeof(int)))==NULL){rv=AF_ERR_MEM;goto memFree;} 
  for (i=0; i<eLenL; i++) nodelist[i] = eNodesL[i];
  if ((noderes = (double *)malloc(nLen*sizeof(double)))==NULL){rv=AF_ERR_MEM;goto memFree;} 
  if ((matnos = (int *)malloc(mpLen*sizeof(int)))==NULL){rv=AF_ERR_MEM;goto memFree;} 

  /*if ((matfld = (int **)malloc(1*sizeof(int *)))==NULL) {rv=AF_ERR_MEM; goto memFree;}*/
  if ((matfld = (int *)malloc(eLen*sizeof(int)))==NULL) {rv=AF_ERR_MEM; goto memFree;}

  /* material data: */
  for (i=0; i<eLen; i++)  matfld[i] = mpID[eMP[i]] ;
  for (i=0; i<mpLen; i++) matnos[i] = mpID[i] ;
  dims[0] = eLen ;

  /* nodes: nLen */
  coordinates[0] = n_x;
  coordinates[1] = n_y;
  coordinates[2] = n_z;

  /* elements - the most dumb way: eNodesL, eLenL */
  shapetype[0] = DB_ZONETYPE_HEX ; /* type of shape - cube with 8 nodes */
  shapesize[0] = 8 ; /* number of nodes */
  shapecnt [0] = eLen ; /* bricks */

  /* WRITE NODES and ELEMENTS ------------------------ */
  DBPutZonelist2 (file, "ufem3delements", 
      eLen, /* nzones */
      3, /* ndims */
      /* (int *)eNodesL, */ nodelist,
      eLenL, /* lnodelist - nodelist size */
      0, /* origin starting number for nodes: 0 or 1 */
      0,
      0, 
      shapetype, /* type of shape: HEX=brick*/
      shapesize, /* nodes per shape type */
      shapecnt, /* number of zones defined by shape type */
      1, /* nshapes */
      NULL);

  /* WRITE FACES ------------------------------------- */
  facelist = DBCalcExternalFacelist2 (
    /* (int *)eNodesL, */ nodelist,
    nLen, /* nuber of nodes */
    0, /* ghost low */
    0, /* ghost high */
    0, /* origin, */
    shapetype,
    shapesize,
    shapecnt, 
    1 /* number of shapes */, 
    NULL, /* matlist - TODO ! `*/
    0 /* bnd_method - TODO ! */) ;

  DBPutFacelist (file, "ufem3dfaces",
      facelist->nfaces,
      facelist->ndims, 
      facelist->nodelist, 
      facelist->lnodelist,
      facelist->origin, 
      facelist->zoneno,
      facelist->shapesize, 
      facelist->shapecnt, 
      facelist->nshapes,
      facelist->types, 
      facelist->typelist,
      facelist->ntypes);

  /* Write out the mesh to the file */
  DBPutUcdmesh(file, "ufem3dbrick", 
      3, /* ndims */
      NULL, /* coordnames - names of coordinate axes (useless?)*/
      coordinates, /* X, Y, Z coords. */
      nLen, /* nodes */
      eLen, /* elements */
      "ufem3delements", /* zonel_name  */
      "ufem3dfaces", /* facell_name  */
      DB_DOUBLE, /* datatype */
      NULL /* optlist */
      );

  /* Material stuff: TODO - fix matfld stuff */
#if 1
  DBPutMaterial (file, "ufem3dmat", "ufem3dbrick",
  mpLen, 
  matnos,
  matfld,
  dims, 
  1,
  NULL, NULL, NULL, NULL, 0, DB_FLOAT, NULL);
#endif

  /* Indexes: */
  DBPutUcdvar1 (file, 
      "node_id", /* variablename */
      "ufem3dbrick", /*mesh name*/
      nID, /* scalar variable data */
      nLen, /* number of values of var */
      NULL,  /*mixed data */
      0, 
      DB_LONG, /* datatype */ 
      DB_NODECENT, /* centering, */
      NULL /*DBoptlist const *optlist*/);
  DBPutUcdvar1 (file, 
      "elem_id", /* variablename */
      "ufem3dbrick", /*mesh name*/
      eID, /* scalar variable data */
      eLen, /* number of values of var */
      NULL,  /*mixed data */
      0, 
      DB_LONG, /* datatype */ 
      DB_ZONECENT, /* centering, */
      NULL /*DBoptlist const *optlist*/);

  /* RESULTS ---------------------------------------------- */

  /* Deformations */
  for (i=0; i<nLen; i++) noderes[i] =  femVecGet(&u, femKpos(i, U_X));
  DBPutUcdvar1 (file, 
      "u_x", 
      "ufem3dbrick", noderes, nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  for (i=0; i<nLen; i++) noderes[i] =  femVecGet(&u, femKpos(i, U_Y));
  DBPutUcdvar1 (file, 
      "u_y", 
      "ufem3dbrick", noderes, nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  for (i=0; i<nLen; i++) noderes[i] =  femVecGet(&u, femKpos(i, U_Z));
  DBPutUcdvar1 (file, 
      "u_z", 
      "ufem3dbrick", noderes, nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);

  /* vectors for principal stresses: */
  femVecNull(&sigma);
  femVecNull(&sigma1);
  if ((rv=femVecFullInit(&sigma, 6)) != AF_OK) { goto memFree; }
  if ((rv=femVecFullInit(&sigma1, 6)) != AF_OK) { goto memFree; }

  if (nLen < 8)
  {
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Dataset is too small"));
#endif
    return(AF_ERR_SIZ) ;
  }

  /* space for results in nodes */
  for (i=0; i<SPIO_RES_NUM; i++)
  {
    if ((resfld[i]=femDblAlloc(nLen)) == NULL)
    {
      for (j=0;j<i; j++)
      {
        femDblFree(resfld[j]) ;
      }
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Out of memory during VTK export"));
#endif
      return(AF_ERR_MEM);
    }
  }
  if ((intfld=femIntAlloc(nLen)) == NULL)
  {
    for (j=0;j<SPIO_RES_NUM; j++)
    {
      femDblFree(resfld[j]) ;
    }
#ifdef RUN_VERBOSE
    fprintf(msgout,"[E] %s!\n", _("Out of memory during VTK export"));
#endif
      return(AF_ERR_MEM);
  }

  femGetAvNodeRes(RES_EX,  resfld[0], intfld, nLen);
  femGetAvNodeRes(RES_EY,  resfld[1], intfld, nLen);
  femGetAvNodeRes(RES_EZ,  resfld[2], intfld, nLen);
  femGetAvNodeRes(RES_SX,  resfld[3], intfld, nLen);
  femGetAvNodeRes(RES_SY,  resfld[4], intfld, nLen);
  femGetAvNodeRes(RES_SZ,  resfld[5], intfld, nLen);
  femGetAvNodeRes(RES_SYZ,  resfld[6], intfld, nLen);
  femGetAvNodeRes(RES_SZX,  resfld[7], intfld, nLen);
  femGetAvNodeRes(RES_SXY,  resfld[8], intfld, nLen);

  /* Computing of principal stresses (in 3D): */
  for (i=0; i<nLen; i++) 
  { 
    for (j=3; j<9; j++) {  femVecPut(&sigma,j-2,resfld[j][i]); }
    femPrinc3D(&sigma, &sigma1);
    for (j=0; j<3; j++) {  resfld[9+j][i] = femVecGet(&sigma1,j+1); }
  }

  DBPutUcdvar1 (file, "e_x", "ufem3dbrick", 
      resfld[0], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  DBPutUcdvar1 (file, "e_y", "ufem3dbrick", 
      resfld[1], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  DBPutUcdvar1 (file, "e_z", "ufem3dbrick", 
      resfld[2], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);

  DBPutUcdvar1 (file, "s_x", "ufem3dbrick", 
      resfld[3], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  DBPutUcdvar1 (file, "s_y", "ufem3dbrick", 
      resfld[4], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  DBPutUcdvar1 (file, "s_z", "ufem3dbrick", 
      resfld[5], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);

  DBPutUcdvar1 (file, "s_yz", "ufem3dbrick", 
      resfld[6], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  DBPutUcdvar1 (file, "s_zx", "ufem3dbrick", 
      resfld[7], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  DBPutUcdvar1 (file, "s_xy", "ufem3dbrick", 
      resfld[8], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);

  DBPutUcdvar1 (file, "s_1", "ufem3dbrick", 
      resfld[9], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  DBPutUcdvar1 (file, "s_2", "ufem3dbrick", 
      resfld[10], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
  DBPutUcdvar1 (file, "s_3", "ufem3dbrick", 
      resfld[11], nLen, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);

  /* freeing of data files */
  for (j=0;j<SPIO_RES_NUM; j++) { femDblFree(resfld[j]) ; }
  femIntFree(intfld) ;

memFree:
  if (nodelist != NULL) {free(nodelist); nodelist=NULL;}
  if (noderes != NULL) {free(noderes); noderes=NULL;}
  if (matnos != NULL) {free(matnos); matnos=NULL;}
  /*if (matfld != NULL) {free(matfld[0]); matfld[0]=NULL; free(matfld); matfld=NULL;}*/
  femVecFree(&sigma);
  femVecFree(&sigma1);

  /* close file: ************************************* */
  if (file != NULL)
  {
    if ((DBClose(file)) != 0)
    {
#ifdef RUN_VERBOSE
      fprintf(msgout,"[E] %s!\n",_("Error during closing of previous result file"));
#endif
      file  = NULL;
      return(AF_ERR_IO);
    }
  }
  return(rv);
}
#endif /* FEM_USE_SILO */

/* end of fem_spio.c */
