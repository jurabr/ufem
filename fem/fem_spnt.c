/*
   File name: fem_spnt.c
   Date:      2003/06/08 16:40
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

	 FEM Solver - routines to save results in selected points (for NRM etc.)

	 $Id: fem_spnt.c,v 1.4 2004/11/11 21:39:26 jirka Exp $
*/

#include "fem_spnt.h"

#ifndef _SMALL_FEM_CODE_

#ifdef USE_MPI
#undef USE_FAST_SAPO
#endif

char  *femSaPoFName = NULL ; /* filename      */
long   femSaPoNode = 0; /* node number        */
long   femSaPoLen = 0 ; /* lenght of femSaPo  */
long   femSaPoActual = 0;/* actual item in femSaPo  */
tSaPo *femSaPo = NULL ; /* structure for saving of data in ONE selected point */
int    femSaPoHaveIt = AF_NO ; /* rellevent only for USE_MPI */

char  *femSumReactName = NULL ; /* filename for reactions */
double femSumReactVal[KNOWN_DOFS] ; /* reaction data */
long   femSumReactStdOut = AF_NO ; /* use stdout? */


/** Allocates space for SaPo 
 * @param NewLen lenght of allocated field
 * @param OldLen current lenght of allocated field (0 for new )
 * @return status
 */
int femSaPoAlloc(long NewLen, long OldLen)
{
	int    rv = AF_OK ;
	int    i ;
	tSaPo *tmp = NULL ;

	if (femUseSaPo != AF_YES) 
	{
		femSaPo = NULL ;
		return(rv);
	}

	if ((tmp = (tSaPo *) malloc (NewLen * sizeof(tSaPo))) == NULL )
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("Failed to allocate result tracking structure"));
#endif
		return(AF_ERR_MEM);
	}

	if ( (OldLen > 0) && (NewLen > OldLen) )
	{
		for (i=0; i<OldLen; i++)
		{
			tmp[i].mult = femSaPo[i].mult ;
			tmp[i].x    = femSaPo[i].x ;
			tmp[i].y    = femSaPo[i].y ;
			tmp[i].z    = femSaPo[i].z ;
			tmp[i].unused  = femSaPo[i].unused ;
		}
	}
	for (i=OldLen; i<NewLen; i++)
	{
			tmp[i].mult = 0 ;
			tmp[i].x    = 0 ;
			tmp[i].y    = 0 ;
			tmp[i].z    = 0 ;
			tmp[i].unused = AF_NO ;
	}

	if (OldLen > 0 ) { free (femSaPo); }
	femSaPo = tmp ;
	tmp     = NULL ;

	return(rv);
}

/** Frees SaPo field
 */
void femSaPoFree(void)
{
	if (femUseSaPo != AF_YES) {return;}
	free(femSaPo);
	femSaPo = NULL ;
}

/** Fills line in femSaPo
 * @param mult load multiplier (total)
 * @param x x-displacement (total)
 * @param y y-displacement (total)
 * @param z z-displacement (total)
 * @param append if AF_YES multiplier size is appended, not rewrited
 * @param unused if AF_YES data will not be saved
 * @return status
 */
int femSaPoInput(double mult, double x, double y, double z, long append, int unused)
{
	int rv = AF_OK ;
#ifdef USE_FAST_SAPO
	FILE  *fw = NULL ;
#endif

	if (femUseSaPo != AF_YES) {return(rv);}

	if (femSaPoActual >= femSaPoLen)
	{
		if ((rv =  femSaPoAlloc(femSaPoActual+2, femSaPoLen) ) != AF_OK) 
		   { return(rv); }
		femSaPoLen = femSaPoActual+1 ;
	}


#ifdef USE_MPI
	if (femSaPoHaveIt == AF_YES)
	{
#endif

#ifdef USE_MPI_LOG
#if 0
		printf("I have data (%i:%li): [%e] %e %e %e\n", ppRank,femSaPoNode, mult, x,y,z);
#endif
#endif

	if (append == AF_YES)
	{
		if (femSaPoActual > 0)
		{
			femSaPo[femSaPoActual].mult = femSaPo[femSaPoActual-1].mult ;
			femSaPo[femSaPoActual].x    = femSaPo[femSaPoActual-1].x;
			femSaPo[femSaPoActual].y    = femSaPo[femSaPoActual-1].y;
			femSaPo[femSaPoActual].z    = femSaPo[femSaPoActual-1].z;
		}
		femSaPo[femSaPoActual].mult += mult ;
		femSaPo[femSaPoActual].x    += x ;
		femSaPo[femSaPoActual].y    += y ;
		femSaPo[femSaPoActual].z    += z ;
	}
	else
	{
		femSaPo[femSaPoActual].mult = mult ;
		femSaPo[femSaPoActual].x    = x ;
		femSaPo[femSaPoActual].y    = y ;
		femSaPo[femSaPoActual].z    = z ;
	}
	
	femSaPo[femSaPoActual].unused = unused ;

#ifdef USE_FAST_SAPO
if ((fw = fopen(femSaPoFName, "a")) == NULL)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("Failed to write result-tracking data"));
#endif
		return(AF_ERR_IO);
	}
	else
	{
		fprintf(fw, "%e %e %e %e\n",
		femSaPo[femSaPoActual].mult,
		femSaPo[femSaPoActual].x   ,
		femSaPo[femSaPoActual].y   ,
		femSaPo[femSaPoActual].z );
		fclose(fw);
	}
#endif

#if 0
  printf("NRM [%li:%li] Append = %li: [%e|%e|%e]<[%e|%e|%s]\n",femSaPoActual, femSaPoLen, append,
	x, femSaPo[femSaPoActual].x ,
	y, femSaPo[femSaPoActual].y ,
	z, femSaPo[femSaPoActual].z 
      );
#endif

#ifdef USE_MPI
	}
	else
	{

		if (append == AF_YES)
		{
			if (femSaPoActual > 0)
			{
				femSaPo[femSaPoActual].mult = femSaPo[femSaPoActual-1].mult ;
			}
			femSaPo[femSaPoActual].mult += mult ;
		}
		else
		{
			femSaPo[femSaPoActual].mult = mult ;
		}

		femSaPo[femSaPoActual].x    = 0.0 ;
		femSaPo[femSaPoActual].y    = 0.0 ;
		femSaPo[femSaPoActual].z    = 0.0 ;
		femSaPo[femSaPoActual].unused = 0.0 ;
	}
#endif


	femSaPoActual++;
	return(rv);
}

/** Save SaPo data to file
 * @return status
 */
int femSaPoSave(void)
{
	int rv = AF_OK ;
	int i ;
	FILE *fw = NULL ;
#ifdef USE_MPI
	double *fld = NULL ;
	double *fldsum = NULL ;
	long   *unused = 0 ;
	long   *unusedsum = 0 ;
#endif

	if (femUseSaPo != AF_YES) {return(rv);}

#ifdef USE_MPI
	/* */
	if ((fld = femDblAlloc(femSaPoLen)) == NULL) {return(AF_ERR_MEM);}
	if ((fldsum = femDblAlloc(femSaPoLen)) == NULL) {return(AF_ERR_MEM);}

	if ((unused = femIntAlloc(femSaPoLen)) == NULL) {return(AF_ERR_MEM);}
	if ((unusedsum = femIntAlloc(femSaPoLen)) == NULL) {return(AF_ERR_MEM);}

	for (i=0; i<femSaPoLen; i++) { fld[i] = 0; fldsum[i] = 0; }

	if (femSaPoHaveIt == AF_YES) {for (i=0; i<femSaPoLen; i++) {fld[i] = femSaPo[i].x; }}
	MPI_Reduce(fld, fldsum, femSaPoLen, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	for (i=0; i<femSaPoLen; i++) {femSaPo[i].x = fldsum[i]; fldsum[i]=0; fld[i]=0;}
	/*MPI_Barrier(MPI_COMM_WORLD);*/

	if (femSaPoHaveIt == AF_YES) {for (i=0; i<femSaPoLen; i++) {fld[i] = femSaPo[i].y; }}
	MPI_Reduce(fld, fldsum, femSaPoLen, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	for (i=0; i<femSaPoLen; i++) {femSaPo[i].y = fldsum[i]; fldsum[i]=0; fld[i]=0;}
	/*MPI_Barrier(MPI_COMM_WORLD);*/

	if (femSaPoHaveIt == AF_YES) {for (i=0; i<femSaPoLen; i++) {fld[i] = femSaPo[i].z; }}
	MPI_Reduce(fld, fldsum, femSaPoLen, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	for (i=0; i<femSaPoLen; i++) {femSaPo[i].z = fldsum[i]; fldsum[i]=0; fld[i]=0;}
	/*MPI_Barrier(MPI_COMM_WORLD);*/


	if (femSaPoHaveIt == AF_YES) {for (i=0; i<femSaPoLen; i++) {unused[i] = femSaPo[i].unused; }}
	MPI_Reduce(unused, unusedsum, femSaPoLen, MPI_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
	for (i=0; i<femSaPoLen; i++) {femSaPo[i].unused = unusedsum[i]; unusedsum[i]=0; unused[i]=0;}
	MPI_Barrier(MPI_COMM_WORLD);


	free(fld);    fld    = NULL ;
	free(fldsum); fldsum = NULL ;
	/**/
	if (ppRank != 0) {return(AF_OK);}
#endif

	if ((fw = fopen(femSaPoFName, "w")) == NULL)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("Failed to write result-tracking data"));
#endif
		return(AF_ERR_IO);
	}
	else
	{
		fprintf(fw,"0.0 0.0 0.0 0.0\n");

		for (i=0; i<femSaPoActual; i++)
		{
			if (femSaPo[i].unused == AF_YES) {continue;}
			fprintf(fw,"%e %e %e %e\n", femSaPo[i].mult, 
					femSaPo[i].x, femSaPo[i].y, femSaPo[i].z
					);
		}

		if (fclose(fw) != 0) {return(AF_ERR_IO);}
	}

	return(rv);
}

/** Eliminates all SaPo-related stuff */
void femSaPoDestroy(void)
{
	if (femUseSaPo != AF_YES) {return;}

	femSaPoFree();

	free(femSaPoFName); 
	femSaPoFName  = NULL ;
	femSaPoNode   = 0 ;
	femSaPoLen    = 0 ;
	femSaPoActual = 0 ;
}

/** Sets filename for femSaPo data 
 *  @param fname name of file
 *  @return status
 */
int femSaPoFileName(char *fname)
{
#ifdef USE_FAST_SAPO
	FILE  *fw = NULL ;
#endif

	if ((femSaPoFName = (char *)malloc(sizeof(char)*(strlen(fname)+1))) == NULL)
	{
		femUseSaPo = AF_NO ;
		return(AF_ERR_IO);
	}
	else
	{
		strcpy(femSaPoFName, fname);
		femSaPoFName[strlen(fname)] = '\0' ;
		femUseSaPo = AF_YES ;

#ifdef USE_FAST_SAPO
if ((fw = fopen(femSaPoFName, "w")) == NULL)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("Failed to write result-tracking data"));
#endif
		return(AF_ERR_IO);
	}
	else
	{
		fprintf(fw, "0.0 0.0 0.0 0.0\n");
		fclose(fw);
	}
#endif

		return(AF_OK);
	}
}

/** Sets SaPo node
 * @param str node number (string)
 * @return status
 */
int femSaPoSetNode(long from, long num)
{
	int found = AF_NO ;
	int i;

	if (femUseSaPo != AF_YES) {return(AF_OK);}

	if (femSaPoNode <=  0) 
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n",_("Invalid logging node"));
#endif
		femSaPoDestroy();
		goto memFree;
	}

	for (i=from; i<from+num; i++)
	{
		if (femSaPoNode == nID[i])
		{
			femSaPoNode = i ;
			found = AF_YES ;
			break ;
		}
	}

memFree:
	if (found == AF_NO)
	{
#ifdef USE_MPI /* mpi only */
		femSaPoHaveIt = AF_NO ;
		printf("have no node (%i)\n", ppRank);
		return(AF_OK);
#else
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n",_("Invalid logging node"));
#endif
		return(AF_ERR_VAL);
#endif
	}
	else
	{
#ifdef USE_MPI /* mpi only */
		femSaPoHaveIt = AF_YES ;
		printf("logging at process %i (node %li)\n",ppRank,  femSaPoNode);
#endif
		return(AF_OK);
	}
}


/*** SUM OF REACTIONS ***/

/* Makes empty file "fname" for reactions */
int femGetSumReactInit(char  *fname)
{
#ifndef USE_MPI
  long i ;
  FILE *fw = NULL ;

  if (fname == NULL) {return(AF_OK);}

  if ((fw=fopen(fname,"w")) == NULL) { return(AF_ERR_IO) ; }

  fprintf(fw,"0 0 0 ");
  for (i=0; i<KNOWN_DOFS; i++) 
	{
		femSumReactVal[i] = 0.0 ;
		fprintf(fw," 0") ; 
	}
  fprintf(fw,"\n");
  fclose(fw);

#endif
  return(AF_OK);
}

/* Writes sum of reactions to file (with given step/iter/multiplier) */
int femGetSumReact(long step, long iter, double mult, char *fname, long TangentMatrix)
{
#ifndef USE_MPI
  long i ;
  double val[KNOWN_DOFS] ;
  FILE *fw = NULL ;

  if (femSumReactStdOut != AF_YES)
  {
    if (fname == NULL) {return(AF_OK);}

    if ((fw=fopen(fname,"a")) == NULL) { return(AF_ERR_IO) ; }
  }
  else
  {
    fw = stdout ;
  }

  for (i=0; i<KNOWN_DOFS; i++) { val[i] = 0.0 ; }

  for (i=0; i<resRLen; i++)
  {
    if (resRdof[i] <= KNOWN_DOFS)
    {
      val[resRdof[i]-1] += resRval[i] ;
    }
  }

  fprintf(fw,"%li %li %e ",step, iter,mult);
  for (i=0; i<KNOWN_DOFS; i++) 
  { 
    fprintf(fw," %e",val[i]) ; 
  }
	fprintf(fw," \n");

  if (femSumReactStdOut != AF_YES)
  {
    fclose(fw);
  }
  else
  {
    fw = NULL ;
  }
#endif
  return(AF_OK);
}

/** Sets filename for sum of reactions data 
 *  @param fname name of file
 *  @return status
 */
int femSumReactFileName(char *fname)
{
	if ((femSumReactName = (char *)malloc(sizeof(char)*(strlen(fname)+1))) == NULL)
	{
    femSumReactName = NULL ;
		return(AF_ERR_IO);
	}
	else
	{
		strcpy(femSumReactName, fname);
		femSumReactName[strlen(fname)] = '\0' ;
		return(AF_OK);
	}
}

#endif /* end of _SMALL_FEM_CODE_ */

/* end of fem_spnt.c */
