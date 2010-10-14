/*
   File name: fem_paux.c
   Date:      2003/04/26 12:57
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

	 FEM Solver - auxiliary funcions for MPI solver

	 $Id: fem_paux.c,v 1.7 2003/05/31 15:20:06 jirka Exp $
*/

#include "fem_mpi.h"

#ifdef USE_MPI

extern long *nDOFfld ; /* description of DOFs in nodes             */

/** Finds nodes for process with rank myRank
 * @param myRank process rank
 * @param mySize process size
 * @param myFrom starting point in nodal field (result)
 * @param myNum number of nodes (result)
 * @return state value
 */
int femMyNodes(int myRank, int mySize, long *myFrom, long *myNum)
{
	int   rv = AF_OK ;
	div_t divres;

	divres = div(nLen, mySize) ;
	
	if (myRank < (mySize-1))
	{
		*myNum = divres.quot;
	}
	else
	{
		/* last process */
		*myNum = divres.quot + divres.rem;
	}

	if (*myNum < 1)
	{
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("number of nodes is nodes is too small"));
#endif
		exit (AF_ERR_SML);
	}

	*myFrom = divres.quot * myRank ;

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[%i] nodes to be used: %li (index from %li)\n",myRank, *myNum, *myFrom);
#endif

	return(rv);
}

/** Creates list of used elements
 * @param myFrom starting point in nID
 * @param myNum number of used nodes
 * @param myEnum number of elements (result)
 * @return field with element indexes (result)
 */
long *femMyElems(long myFrom, long myNum, long *myEnum)
{
	long *eList = NULL ;
	long *eTmp = NULL ;
	long  sum = 0;
	long  i,j, ii;

	*myEnum = 0 ;

	if ((eTmp = femIntAlloc(eLen)) == NULL) {exit (AF_ERR_MEM);}

	for (i=0; i<eLen; i++)
	{
		for (j=eFrom[i]; j<eFrom[i]+femFldItemLen(i, eFrom, eLen, eLenL); j++)
		{
			for (ii=myFrom; ii<myFrom+myNum; ii++)
			{
				if (eNodesL[j] == ii)
				{
					eTmp[i] = 1 ;
				}
			}
		}
	}

	for (i=0; i<eLen; i++) { sum += eTmp[i] ; }

	if ((eList = femIntAlloc(sum)) == NULL) {exit (AF_ERR_MEM);}

	*myEnum = sum ;

	sum = 0;

	for (i=0; i<eLen; i++)
	{
		if (eTmp[i] == 1)
		{
			eList[sum] = i ;
			sum++ ;
		}
	}

	femIntFree(eTmp);

	return(eList);
}

/** Deletes "foreign" elements
 * @param myEnum lenght of eList
 * @param eList list of wanted elements (these will survive this function)
 * @return state value
 */
int femDelOthElem(long myEnum, long *eList)
{
	int     rv = AF_OK;
	long   *eTmp = NULL;
	long   *eTmpNum = NULL;
	long   *eTmpNodes = NULL;
	long    sum = 0;
	long    i,j;
	
	/* element properties: */
	if ((eTmp = femIntAlloc(myEnum)) == NULL) {exit (AF_ERR_MEM);}
	for (i=0; i<myEnum; i++) { eTmp[i] = eID[eList[i]] ; }
	femIntFree(eID);
	eID = eTmp;
	eTmp = NULL ;

	if ((eTmp = femIntAlloc(myEnum)) == NULL) {exit (AF_ERR_MEM);}
	for (i=0; i<myEnum; i++) { eTmp[i] = eType[eList[i]] ; }
	femIntFree(eType);
	eType = eTmp;
	eTmp = NULL ;

	if ((eTmp = femIntAlloc(myEnum)) == NULL) {exit (AF_ERR_MEM);}
	for (i=0; i<myEnum; i++) { eTmp[i] = eRS[eList[i]] ; }
	femIntFree(eRS);
	eRS = eTmp;
	eTmp = NULL ;

	if ((eTmp = femIntAlloc(myEnum)) == NULL) {exit (AF_ERR_MEM);}
	for (i=0; i<myEnum; i++) { eTmp[i] = eMP[eList[i]] ; }
	femIntFree(eMP);
	eMP = eTmp;
	eTmp = NULL ;

	sum = 0;
	if ((eTmp = femIntAlloc(myEnum)) == NULL) {exit (AF_ERR_MEM);}
	if ((eTmpNum = femIntAlloc(myEnum)) == NULL) {exit (AF_ERR_MEM);}
	for (i=0; i<myEnum; i++) 
	{ 
		eTmp[i] = eFrom[eList[i]];
		sum += femFldItemLen(eList[i], eFrom, eLen, eLenL) ;
		eTmpNum[i] = femFldItemLen(eList[i], eFrom, eLen, eLenL) ;
	}

	eLen = myEnum ;

	if ((eTmpNodes = femIntAlloc(sum)) == NULL) {exit (AF_ERR_MEM);}

	eLenL = sum;

	sum = 0 ;

	for (i=0; i<eLen; i++)
	{
		for (j=eTmp[i]; j<eTmp[i]+eTmpNum[i]; j++)
		{
			eTmpNodes[sum] =  eNodesL[j] ;
			sum++ ;
		}
	}

	sum = 0;
	for (i=0; i<eLen; i++)
	{
		eTmp[i] = sum;
		sum += eTmpNum[i] ;
	}

	femIntFree(eNodesL) ;
	eNodesL = eTmpNodes ;
	eTmpNodes = 0 ;

	/* eFrom */
	femIntFree(eFrom);
	femIntFree(eTmpNum);
	eFrom = eTmp;
	eTmp = NULL ;

#ifdef DEVEL_VERBOSE
	for (i=0; i<eLen; i++)
	{
		fprintf(msgout,"[%i] e=%li (%li) r=%li m=%li n=%li: ",ppRank, 
				eID[i], eType[i], eRS[i], eMP[i], eFrom[i]);

		for (j=eFrom[i]; j<eFrom[i]+femFldItemLen(i, eFrom, eLen, eLenL); j++)
		    { fprintf(msgout," %li ", eNodesL[j]) ; }
		fprintf(msgout,"\n");
	}
		fprintf(msgout,"\n---------------------------------\n");

	for (i=0; i<rsLen; i++) { fprintf(msgout," %li ", rsFrom[i]); } fprintf(msgout,"!!\n");
	for (i=0; i<mpLen; i++) { fprintf(msgout," %li ", mpFrom[i]); } fprintf(msgout,"!!\n");
#endif

	return(rv);
}

/** Computes DOFs (from <= index <=to) related to process
 * @param nFrom index of 1st process's node
 * @param nNum number of process's nodes
 * @param from lowest used row in K (result)
 * @param to   highhest used row in K (result)
 */
void femMyDOFs(long nFrom, long nNum, long *from, long *to)
{
	int i;

	*from = nDOFfld[nFrom * KNOWN_DOFS]        ;

	for (i = ((nFrom+nNum-1)*KNOWN_DOFS) ; i<((nFrom+nNum)*KNOWN_DOFS); i++)
	{
		if (nDOFfld[i] == 0) {break;}
		*to = nDOFfld[i] ;
	}

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"[%i] %s: %li %s: %li (%li..%li)\n", 
			ppRank, _("DOFS from"), *from, _("to"), *to, nFrom, nNum);
#endif
}

/** Creates field with sizes of DOFs in nodes
 * @return field
 */
int *femDOFsizes(void)
{
	int i;
	static int  *tmp = NULL ;
	long n_from, n_num, d_from, d_to ;

	if ((tmp = (int *) malloc (ppSize*sizeof(int))) == NULL)
	{
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"[E] (%i) %s!\n", ppRank, _("Out of memory"));
#endif
		exit(AF_ERR_MEM);
	}

	for (i=0; i<ppSize; i++)
	{
    femMyNodes(i, ppSize, &n_from, &n_num) ;
		femMyDOFs(n_from, n_num, &d_from, &d_to) ;
		
		tmp[i] = d_to - d_from + 1 ;
	}

	return(tmp);
}


long ppDataCpyInt(signed long int *dest, signed long int *src, long len, long from)
{
	long i;
	for (i=0; i<len; i++)
	{
		dest[i+from] = src[i] ;
	}
	return(from+len);
}

long ppDataCpyDbl(double *dest, double *src, long len, long from)
{
	long i;
	for (i=0; i<len; i++)
	{
		dest[i+from] = src[i] ;
	}
	return(from+len);
}

long ppDataCpyIntBack(signed long int *dest, signed long int *src, long len, long from)
{
	long i;
	for (i=0; i<len; i++)
	{
		src[i] = dest[i+from] ;
	}
	return(from+len);
}

long ppDataCpyDblBack(double *dest, double *src, long len, long from)
{
	long i;
	for (i=0; i<len; i++)
	{
		src[i] = dest[i+from] ;
	}
	return(from+len);
}

#endif

/* end of fem_paux.c */
