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

/* end of fem_spio.c */
