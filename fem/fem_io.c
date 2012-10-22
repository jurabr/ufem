/*
   File name: fem_io.c
   Date:      2003/04/07 09:06
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

   Finite Element Solver - I/O operations

  $Id: fem_io.c,v 1.20 2005/01/01 21:01:26 jirka Exp $
*/

#include "fem_dama.h"
#include "fem_data.h"
#include "fem_mem.h"
#include "fem_math.h"
#include "fem_para.h"

/* from "fem_sol.c": */
extern long  nDOFAct  ;    /* total number of ACTIVE DOFs in structure (== size of "K" matrix) */
extern long  nDOFlen  ;    /* lenght of nDOFfld                        */
extern long *nDOFfld  ; /* description of DOFs in nodes             */
extern tVector u; /* structure displacement vector */

extern int fem_monte_read_data(FILE *fr); /* "monte" data reading */

/** replaces name (long number!) values in "ind" with their positions by "by_imd" field
 * @param ind field to be modified
 * @param indLen lenght of ind
 * @param ind_by 
 * @param ind_byLen lenght of ind_by
 * @return state value
 */
int femRenumIndex(long *ind, long indLen, long *by_ind, long by_indLen)
{
	int rv = AF_ERR_VAL;
	long i,j;

	if (femPreparsedData == AF_YES) { rv = AF_OK ; return(rv); }

	for (i=0; i<indLen; i++)
	{
		for (j=0; j<by_indLen; j++)
		{
			rv = AF_ERR_VAL;
				/*fprintf(msgout,"ind[i] = %li | %li\n",ind[i],by_ind[i]);*/
			if (ind[i] == by_ind[j])
			{
				ind[i] = j;
				rv = AF_OK ;
				break;
			}
		}
		if (rv != AF_OK) 
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s: \"%li\"!\n", _("Stop - name without index found"), ind[i]);
#endif
			return(rv);
		}
	}

	return(rv);
}

/** reads loads data from file - do NOT use it directly
 * @param fname file name
 * @return state value
 */
int femReadInputLoads(FILE *fr)
{
	int   rv = AF_OK ;
  long  i, sum, pos ;

	/* nodal loads: */
	fscanf(fr,"%li", &nlLen);

	if (nlLen > 0)
	{
		if ((nlNode  = femIntAlloc(nlLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((nlType  = femIntAlloc(nlLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((nlDir   = femIntAlloc(nlLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((nlVal   = femDblAlloc(nlLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((nlTrPos = femIntAlloc(nlLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

		for (i=0; i<nlLen; i++)
		{
			fscanf(fr,"%li %li %li %lf",
			&nlNode[i], &nlType[i], &nlDir[i], &nlVal[i]); 
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"nl: n=%li typ=%li dir=%li val=%f\n", 
			nlNode[i], nlType[i], nlDir[i], nlVal[i]); 
#endif
		}
	/* renumbering of "nlNode" (names -> indexes) */
 	if ((rv = femRenumIndex(nlNode, nlLen, nID, nLen)) != AF_OK) {goto memFree;}
	}

	/* element loads: */
	fscanf(fr, "%li", &elLen);
	if (elLen > 0)
	{
		if ((elElem = femIntAlloc(elLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((elType = femIntAlloc(elLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((elFrom  = femIntAlloc(elLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

		if ((elTrPos  = femIntAlloc(elLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

		sum = 0 ;
		for (i=0; i<elLen; i++)
		{
			fscanf(fr,"%li %li %li",
					&elElem[i], &elType[i], &elFrom[i]);
			sum += elFrom[i];
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"el: e = %li: t=%li nn=%li\n",
					elElem[i], elType[i], elFrom[i]);
#endif
		}
	
		/* setting elFrom field */
		sum = 0;
		for (i=0; i<elLen; i++)
		{
			pos = elFrom[i];
			elFrom[i] = sum;
			sum += pos;
		}
	
		fscanf(fr,"%li", &elLenL);
		if (elLenL != sum)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s (%lix%li)!\n",_("Bad number of element load values"),sum, elLenL);
#endif
			rv = AF_ERR_SIZ; goto memFree;
		}
		
		if ((elValL  = femDblAlloc(elLenL)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		for (i=0; i<elLenL; i++)
		{
			fscanf(fr,"%lf", &elValL[i]);
#ifdef DEVEL_VERBOSE
			fprintf(msgout," %f \n", elValL[i]);
#endif
		}

		/* renumbering of "elElem" (names -> indexes) */
 		if ((rv = femRenumIndex(elElem, elLen, eID, eLen)) != AF_OK) {goto memFree;}
	}

memFree:
  return(rv);
}

/** Reads repeating data (dynamics, transient analysis etc.)
 * @param fname file name
 * @return state value
 */
int femReadRepeatData(FILE *fr)
{
	int   rv = AF_OK ;
	long  i, j ;

  transTS = -1 ;

  if (fscanf(fr, "%li %li %lf", &transNum, &dynNum, &dynStp) < 3)
  {
#ifdef RUN_VERBOSE
	  fprintf(msgout,"[E] %s!\n", _("Empty transient data"));
#endif
    femEigenNum = 0 ;
    femEigenModal = AF_NO ;
    femNewmarkEL  = AF_NO ;
    femDynamics   = AF_NO ;
    femThermTrans = AF_NO ;
    return(AF_OK);
  }

  if ((transNum > 0) && (dynNum > 0))
  {
		if ((transType  = femIntAlloc(transNum)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((transPos  = femIntAlloc(transNum)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

		if ((transMult  = (double **)malloc(transNum*sizeof(double *))) == NULL) 
    {
      rv=AF_ERR_MEM; 
      goto memFree; 
    }
    else
    {
      for (i=0; i<transNum; i++)
      {
		    if ((transMult[i] = femDblAlloc(dynNum)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
        for (j=0; j<dynNum; j++) {transMult[i][j] = 0.0 ;}
      }
    }

    /* read data types: */
    for (i=0; i<transNum; i++)
    {
      if (fscanf(fr, "%li %li", &transType[i], &transPos[i]) < 2) 
      {
        rv = AF_ERR_IO ;
#ifdef RUN_VERBOSE
	      fprintf(msgout,"[E] %s!\n", _("Missing transient data description"));
#endif
        goto memFree;
      }
    }

    /* read actual data */
    for (i=0; i<transNum; i++)
    {
      for (j=0; j<dynNum; j++)
      {
        if (fscanf(fr, "%lf", &transMult[i][j]) != 1) 
        {
#ifdef RUN_VERBOSE
	      fprintf(msgout,"[E] %s!\n", _("Missing transient data"));
#endif
          rv = AF_ERR_IO;
          goto memFree;
        }
      }
    }
    /* test of data and filling of elTrPos, nlTrPos...  */
    for (i=0; i<transNum; i++)
    {
      switch (transType[i])
      {
        case 0:  /* absolute time */
          transTS = i ;
          break;
        case 1:  /* gravity ux */
          dynAccX = i ;
          break;
        case 2:  /* gravity uy */
          dynAccY = i ;
          break;
        case 3:  /* gravity uz */
          dynAccZ = i ;
          break;
        case 4:  /* load/support in node */
          if ((transPos[i] >= 0) && (transPos[i] < nlLen))
          {
            nlTrPos[transPos[i]] = i ;
          }
          break;
        case 5:  /* load on element */
          if ((transPos[i] >= 0) && (transPos[i] < elLen))
          {
            elTrPos[transPos[i]] = i ;
          }
          break;
      }
    }
  }
  else
  {
    /* no transient data */
    femEigenNum = 0 ;
    femEigenModal = AF_NO ;
    femNewmarkEL  = AF_NO ;
    femDynamics   = AF_NO ;
    femThermTrans = AF_NO ;
    return(AF_OK);
  }

  return(rv);
memFree:
  femIntFree(transType);
  femIntFree(transPos);
  if (transMult != NULL)
  {
    for (i=0; i<transNum; i++) { femDblFree(transMult[i]) ; }
    free(transMult); transMult = NULL ;
  }
  return(rv);
}

/** reads dynamics (acceleration) data from file - do NOT use it directly
 * @param fname file name
 * @return state value
 */
int femReadInputDynLoads(FILE *fr)
{
	int   rv = AF_OK ;

	rv = fscanf(fr,"%li", &dynNum); rv = AF_OK ;

	if (dynNum <= 0) 
	{ 
		if (femNewmarkEL != AF_NO)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n", _("Empty dynamics data"));
#endif
			dynNum = 0 ;
			dynStp = 0.0 ;
			return(AF_ERR_EMP);
		}
		else { return(AF_OK) ; }
	} 

	if (fscanf(fr,"%lf", &dynStp) < 1) 
	{
		dynNum = 0 ;
		return(AF_ERR_IO);
	}
	if (dynStp < 0.0) 
	{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s: %f!\n", _("Dynamics time step too small"),dynStp);
#endif
		dynNum = 0 ;
		dynStp = 0.0 ;
		return(AF_ERR_VAL);
	}

	if (fscanf(fr,"%lf %lf", &dynAlpha, &dynBeta) < 2)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n", _("Can not read damping data"));
#endif
		dynNum = 0; dynStp = 0.0 ;
		return( AF_ERR_IO) ; 
	}
	if ((dynAlpha < 0.0) || (dynBeta < 0.0))
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s: alpha=%f, beta=%f!\n",
				_("Invalid damping data"),dynAlpha, dynBeta);
#endif
		dynNum = 0; dynStp = 0.0 ;
		return( AF_ERR_IO) ; 
	}

	grVal = 0.0 ; /* should be zero for dynamics! */

  return(rv);
}


/** reads input data from file - THE MAIN READING FUNCTION
 * @param fname file name
 * @return state value
 */
int femReadInput(char *fname)
{
	int   rv = AF_OK ;
	FILE *fr = NULL ;
	long pos, sum;
	long  i ;


	/* open file: ************************************** */
	if (femReadStdInput == AF_YES)
	{
		fr = stdin ;
	}
	else
	{
		if ((fr = fopen(fname, "r")) == NULL)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n", _("Error during input data reading"));
#endif
			return(AF_ERR_IO);
		}
	}


	/* solution control data: */
	fscanf(fr,"%li %li", &solID, &solDepOn );
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"TN: %li DepOn: %li\n", solID, solDepOn );
#endif

	fscanf(fr,"%li %li %li\n", &solLinSol, &solNoLinS, &solNoLinSteps );

  /* always can be overwritten from command line: */
  if (solNoLinS_cmd > -1) { solNoLinS = solNoLinS_cmd ; }
  if (solNoLinSteps_cmd > -1) { solNoLinSteps = solNoLinSteps_cmd ; }
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"Eq. solver: %li Non-lin: %li N.L. Steps: %li\n", solID, solNoLinS, solNoLinSteps );
#endif

	/* real sets: */
	fscanf(fr, "%li", &rsLen);
	if (rsLen > 0)
	{
		if ((rsID   = femIntAlloc(rsLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((rsType = femIntAlloc(rsLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((rsFrom = femIntAlloc(rsLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

		sum = 0 ;
		for (i=0; i<rsLen; i++)
		{
			fscanf(fr,"%li %li %li",
					&rsID[i], &rsType[i], &rsFrom[i]);
			sum += rsFrom[i];
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"rs = %li: t=%li nn=%li\n",
					rsID[i], rsType[i], rsFrom[i]);
#endif
		}
	
		/* setting rsFrom field */
		sum = 0;
		for (i=0; i<rsLen; i++)
		{
			pos = rsFrom[i];
			rsFrom[i] = sum;
			sum += pos;
		}
	
		fscanf(fr,"%li", &rsLenL);
		if (rsLenL != sum)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s (%lix%li)!\n",_("Bad number of real set values"),sum, rsLenL);
#endif
			rv = AF_ERR_SIZ; goto memFree;
		}
		
		if (rsLenL > 0)
		{
			if ((rsValL  = femDblAlloc(rsLenL)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
			for (i=0; i<rsLenL; i++)
			{
				fscanf(fr,"%lf", &rsValL[i]);
#ifdef DEVEL_VERBOSE
				fprintf(msgout," %f \n", rsValL[i]);
#endif
			}
		}
	}
	else
	{
		rsLen  = 0 ;
		rsLenL = 0 ;
		rsID   = NULL ;
		rsType = NULL ;
		rsFrom = NULL ;
		rsValL = NULL ;
	}

	/* material properties: */
	fscanf(fr, "%li", &mpLen);
	if (mpLen > 0)
	{
		if ((mpID = femIntAlloc(mpLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((mpType = femIntAlloc(mpLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		if ((mpFrom  = femIntAlloc(mpLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

		sum = 0 ;
		for (i=0; i<mpLen; i++)
		{
			fscanf(fr,"%li %li %li",
					&mpID[i], &mpType[i], &mpFrom[i]);
			sum += mpFrom[i];
#ifdef DEVEL_VERBOSE
			fprintf(msgout,"mp = %li: t=%li nn=%li\n",
					mpID[i], mpType[i], mpFrom[i]);
#endif
		}
	
		/* setting mpFrom field */
		sum = 0;
		for (i=0; i<mpLen; i++)
		{
			pos = mpFrom[i];
			mpFrom[i] = sum;
			sum += pos;
		}
	
		fscanf(fr,"%li", &mpLenL);
		if (mpLenL != sum)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s (%lix%li)!\n",_("Bad number of material property values"),sum, mpLenL);
#endif
			rv = AF_ERR_SIZ; goto memFree;
		}
		
		if ((mpValL  = femDblAlloc(mpLenL)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
		for (i=0; i<mpLenL; i++)
		{
			fscanf(fr,"%lf", &mpValL[i]);
#ifdef DEVEL_VERBOSE
			fprintf(msgout," %f \n", mpValL[i]);
#endif
		}
	}

	/* nodes: */
	fscanf(fr, "%li", &nLen); if (nLen < 2) {return(AF_ERR_SML);}

	if ((nID = femIntAlloc(nLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

	if ((n_x = femDblAlloc(nLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
	if ((n_y = femDblAlloc(nLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
	if ((n_z = femDblAlloc(nLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

	for (i=0; i<nLen; i++)
	{
		fscanf(fr, "%li %lf %lf %lf",
				&nID[i], &n_x[i], &n_y[i], &n_z[i]);
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"n = %li: %f %f %f\n", nID[i],n_x[i],n_y[i],n_z[i]);
#endif
	}

	/* elements: */
	fscanf(fr, "%li", &eLen); if (eLen < 1) {return(AF_ERR_SML);}
	if ((eID = femIntAlloc(eLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }

	if ((eType = femIntAlloc(eLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
	if ((eRS  = femIntAlloc(eLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
	if ((eMP  = femIntAlloc(eLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
	if ((eFrom  = femIntAlloc(eLen)) == NULL) {rv=AF_ERR_MEM; goto memFree; }


	sum = 0 ;
	for (i=0; i<eLen; i++)
	{
		fscanf(fr,"%li %li %li %li %li",
				&eID[i], &eType[i], &eRS[i], &eMP[i], &eFrom[i]);
		sum += eFrom[i];
#ifdef DEVEL_VERBOSE
		fprintf(msgout,"e = %li: t=%li r=%li m=%li nn=%li\n",
				eID[i], eType[i], eRS[i], eMP[i], eFrom[i]);
#endif
	}

	/* Renumbering eMP  */
 	if ((rv = femRenumIndex(eMP, eLen, mpID, mpLen)) != AF_OK) {goto memFree;}

	/* Renumbering eRS  */
	if (rsLen > 0)
	{
 		if ((rv = femRenumIndex(eRS, eLen, rsID, rsLen)) != AF_OK) {goto memFree;}
	}

	/* setting eFrom field */
	sum = 0;
	for (i=0; i<eLen; i++)
	{
		pos = eFrom[i];
		eFrom[i] = sum;
		sum += pos;
	}

	fscanf(fr,"%li", &eLenL);
	if (eLenL != sum)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s (%lix%li)!\n",_("Bad number of element nodes"),sum, eLenL);
#endif
		rv = AF_ERR_SIZ; goto memFree;
	}
	
	if ((eNodesL  = femIntAlloc(eLenL)) == NULL) {rv=AF_ERR_MEM; goto memFree; }
	for (i=0; i<eLenL; i++)
	{
		fscanf(fr,"%li", &eNodesL[i]);
#ifdef DEVEL_VERBOSE
		fprintf(msgout," %li \n", eNodesL[i]);
#endif
	}

	/* renumbering of "eNodesL" (names -> indexes) */
 	if ((rv = femRenumIndex(eNodesL, eLenL, nID, nLen)) != AF_OK) {goto memFree;}

	/* gravitation: */
	fscanf(fr,"%li %lf", &grDir, &grVal);
#ifdef DEVEL_VERBOSE
	fprintf(msgout,"gr: dir=%li g=%f\n", grDir, grVal);
#endif

  /* all load data: */
  if ((rv = femReadInputLoads(fr)) != AF_OK) {goto memFree;}

  /* Array data (thermal/dynamics) - if applicable */
	if ((rv = femReadRepeatData(fr)) != AF_OK) {goto memFree;}

	/* dynamics (if applicable) */
	if ((rv = femReadInputDynLoads(fr)) != AF_OK) {goto memFree;}

  /* Monte-related data (if any )*/
  if ((rv = fem_monte_read_data(fr)) != AF_OK) {goto memFree;}

	/* close file: ************************************* */
  if ((fclose(fr)) != 0)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s!\n",_("Error during input data reading"));
#endif
		fr = NULL;
		return(AF_ERR_IO);
	}

	if (rv == AF_OK) {return(rv);}
  /* on error: */
memFree:
	femDataFree();
	return(rv);
}



/** Writes results
 * @param fname name of file
 * @return 
 */
int femWriteRes(char *fname)
{
	int rv = AF_OK;
	FILE *fr = NULL ;
	long  i ;

	/* open file: ************************************** */
	if ((fname == NULL) && (femWriteStdOutput == AF_YES) )
	{
		fr =stdout ;
	}
	else
	{
		if ((fr = fopen(fname, "w")) == NULL)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n", _("Error during result data writing"));
#endif
			return(AF_ERR_IO);
		}
	}


	/* Solution identifier and dependence info, (useless) simulation number: */
	fprintf(fr,"S\n %li %li %e \n",solID, solDepOn, solSimNum);



	/* DOF solution: */
	fprintf(fr," %li \n %li \n %li\n",nDOFAct, nDOFlen,nLen);

  for (i=0; i<nLen; i++)
	{
		fprintf(fr, " %li\n", nID[i]); /* node number */
	}

	for (i=0; i<nDOFlen; i++)
	{
		fprintf(fr, " %li\n", nDOFfld[i]); /* DOF description */
	}

  femVecOut(&u, fr); /* DOF solution vector*/


	/* Reactions: */
	fprintf(fr," %li \n",resRLen);

	for (i=0; i< resRLen; i++)
	{
		fprintf(fr," %li %li %li %e\n", resRnode[i], nID[resRnode[i]], resRdof[i], resRval[i]);
	}


	/* Element results: */
	fprintf(fr," %li %li\n",eLen,resELen);

	for (i=0; i< eLen; i++)
	{
		fprintf(fr," %li \n",eID[i]);
	}

	for (i=0; i< eLen; i++)
	{
		fprintf(fr," %li \n", resEFrom[i]);
	}

	/*fprintf(fr," %li \n",resELen);*/

	for (i=0; i< resELen; i++)
	{
		fprintf(fr," %e \n",resEVal[i]);
	}

	/* close file: ************************************* */
	if (fr != stdout)
	{
  	if ((fclose(fr)) != 0)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n",_("Error during closing of result file"));
#endif
			fr = NULL;
			return(AF_ERR_IO);
		}
	}
	return(rv);
}

/** Reads results - result fields have to be already allocated!
 *  Note: values will be ADDED, NOT REWRITTEN
 * @param fname name of file
 * @return 
 */
int femReadRes(char *fname)
{
	int     rv = AF_OK;
	FILE   *fr = NULL ;
	long    i, j ;
  char    type_c ;
  long    solID_tst, solDepOn_tst;
  double  solSimNum_tst ;
  long    nDOFAct_tst, nDOFlen_tst, nLen_tst ;
  long   *n_id_tst = NULL ;
  long   *n_pos_tst = NULL ;
  long   *n_desc_tst = NULL ;
  long   *e_id_tst = NULL ;
  long   *e_pos_tst = NULL ;
  long   *e_len_tst = NULL ;
  long    resRLen_tst, resRnode_tst, resRid_tst, resRdof_tst ;
  double  resRval_tst ;
  long    eLen_tst, resELen_tst ;
  long    dummy, pos ;
  double  val ;

	/* open file: ************************************** */
	if ((fname == NULL) && (femPrevStdInput == AF_YES) )
	{
		fr = stdin ;
	}
	else
	{
		if ((fr = fopen(fname, "r")) == NULL)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n", _("Error during reading of previous results"));
#endif
			return(AF_ERR_IO);
		}
	}

	/* Solution identifier and dependence info, (useless) simulation number: */
	fscanf(fr,"%c %li %li %lf",&type_c,&solID_tst, &solDepOn_tst, &solSimNum_tst);

	/* DOF solution: */
	fscanf(fr,"%li %li %li",&nDOFAct_tst, &nDOFlen_tst, &nLen_tst);

  dummy = (long)(nDOFlen_tst/nLen_tst); /*number of DOF's per node */

  if (dummy != ((long)(nDOFlen/nLen)))
  {
    /* catastrophic failure - different numbers of DOF's pe node! */
    return(AF_ERR_VAL);
  }


  /* testing fields */
  if ((n_id_tst=femIntAlloc(nLen_tst))==NULL)
  {
    return(AF_ERR_MEM) ;
  }

  if ((n_pos_tst=femIntAlloc(nLen_tst))==NULL)
  {
    femIntFree(n_id_tst) ;
    return(AF_ERR_MEM) ;
  }

  if ((n_desc_tst=femIntAlloc(nDOFlen_tst))==NULL)
  {
    femIntFree(n_id_tst) ;
    femIntFree(n_pos_tst) ;
    return(AF_ERR_MEM) ;
  }

  for (i=0; i<nLen_tst; i++)
	{
	  fscanf(fr, "%li", &n_id_tst[i]); /* node number */
    n_pos_tst[i] = -1 ;
	}

  for (i=0; i<nLen_tst; i++)
	{
    if (i < nLen)
    {
      if (n_id_tst[i] == nID[i]) /* nodes are the same - nice! */
      {
        n_pos_tst[i] = i ;
      }
      else /* have to find the node */
      {
        for (j=0; j<nLen; j++)
        {
          if (n_id_tst[i] == nID[j])
          {
            n_pos_tst[i] = j ;
            break ;
          }
        }
      }
    }
    else
    {
      for (j=0; j<nLen; j++) /* have to find the node */
      {
        if (n_id_tst[i] == nID[j])
        {
          n_pos_tst[i] = j ;
          break ;
        }
      }
    }
  }

  femIntFree(n_id_tst) ; /* no longer needed */

  /* DOF description - hope that's OK  */
	for (i=0; i<nDOFlen_tst; i++)
	{
		fscanf(fr, "%li", &n_desc_tst[i]);
	}


  fscanf(fr, "%li", &nDOFAct_tst) ; /* once more.. */

  for (i=0;i<nLen_tst; i++)
  {
    for (j=0; j<dummy; j++)
    {
      if (n_desc_tst[(i*dummy)+j] > 0)
      {
		    fscanf(fr, "%lf", &val); /* there is something - read it */
        if (n_pos_tst[i] != -1)
        {
          if ((pos=nDOFfld[(n_pos_tst[i]*dummy)+j]) > 0)
          {
            femVecAdd(&u, pos, val) ; /* probably useless */
            femVecAdd(&u_tot, pos, val) ; /* it exists, use it! */
          }
        }
      }
    }
  }

  femIntFree(n_pos_tst) ; /* no longer needed */
  femIntFree(n_desc_tst) ; /* no longer needed */


	/* Reactions - we will silently ignore them : */
	fscanf(fr,"%li",&resRLen_tst);

	for (i=0; i< resRLen_tst; i++)
	{
		fscanf(fr,"%li %li %li %lf", &resRnode_tst, &resRid_tst, &resRdof_tst, &resRval_tst);
	}


	/* Element results: */
	fscanf(fr,"%li %li",&eLen_tst, &resELen_tst);

  /* testing fields */
  if ((e_id_tst=femIntAlloc(eLen_tst))==NULL)
  {
    return(AF_ERR_MEM) ;
  }

  if ((e_pos_tst=femIntAlloc(eLen_tst))==NULL)
  {
    femIntFree(e_id_tst) ;
    return(AF_ERR_MEM) ;
  }

  if ((e_len_tst=femIntAlloc(eLen_tst))==NULL)
  {
    femIntFree(e_id_tst) ;
    femIntFree(e_pos_tst) ;
    return(AF_ERR_MEM) ;
  }

	for (i=0; i< eLen; i++)
	{
		fscanf(fr,"%li",&e_id_tst[i]);
    e_pos_tst[i] = -1 ;
	}

  for (i=0; i<eLen_tst; i++)
	{
    if (i < eLen)
    {
      if (e_id_tst[i] == eID[i]) /* nodes are the same - nice! */
      {
        e_pos_tst[i] = i ;
      }
      else /* have to find the node */
      {
        for (j=0; j<eLen; j++)
        {
          if (e_id_tst[i] == eID[j])
          {
            e_pos_tst[i] = j ;
            break ;
          }
        }
      }
    }
    else
    {
      for (j=0; j<eLen; j++) /* have to find the node */
      {
        if (e_id_tst[i] == eID[j])
        {
          e_pos_tst[i] = j ;
          break ;
        }
      }
    }
  }

  femIntFree(e_id_tst) ; /* no longer needed */

	for (i=0; i< eLen_tst; i++)
	{
		fscanf(fr,"%li", &e_len_tst[i]);
	}


  for (i=0; i< (eLen_tst-1); i++)
	{
		e_len_tst[i] = e_len_tst[i+1] - e_len_tst[i];
	}

  e_len_tst[eLen_tst-1] = resELen_tst - e_len_tst[eLen_tst-1] ;
  
  /* there also should be a test if elements have the same number of
   * results - I can imagine that it isn's a problem, but nobody knows.. 
   */

  for (i=0; i<eLen_tst; i++)
  {
    if (e_len_tst[i] > 0)
    {
      if (e_pos_tst[i] != -1)
      {
        pos = resEFrom[e_pos_tst[i]] ;
      }
      else
      {
        pos = -1 ; /* we will not use these values */
      }

      for (j=0; j<e_len_tst[i]; j++)
      {
        fscanf(fr, "%lf", &val) ;
        if (pos != -1) { resEVal[pos+j] += val ; }
      }
    }
  }

  femIntFree(e_pos_tst) ; /* no longer needed */
  femIntFree(e_len_tst) ; /* no longer needed */

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

/* ******************************************************* 
 * *******************************************************
 * *******************************************************
 */ 


/** Openes file for solution norm
 * @param fname name of file to be opened
 */
void femOpenSolNormFile(char *fname)
{

  if (fname == NULL)
  {
    fem_sol_norm_file = NULL ;
    return;
  }

  if (strlen(fname) < 1)
  {
    fem_sol_norm_file = NULL ;
    return;
  }
  
  if ((fem_sol_norm_file=fopen(fname,"w")) == NULL)
  {
    fem_sol_norm_file = NULL ;
    return;
  }
}


/** Writes solution norm data to the file
 * @param iter iteration number
 * @param load_mult load level (0.0..1.0)
 * @param val unballanced forces (or other measured data)
 * @param crit criteria value
 * @param crit criteria value
 */
void femWriteSolNorm(long iter, double load_mult, double val, double crit_min, double crit_max)
{
  if (fem_sol_norm_file != NULL)
  {
    fprintf(fem_sol_norm_file,"%li %e %e %e %e\n", 
        iter, load_mult, val, crit_min, crit_max);
    fflush(fem_sol_norm_file);
  }
}

/** Closed file for solution norm
 */
void femCloseSolNormFile(void)
{
  if (fem_sol_norm_file != NULL)
  {
    fclose(fem_sol_norm_file);
    fem_sol_norm_file = NULL ;
  }
}


/* **********************************************************
 * **********************************************************
 * **********************************************************
 */ 

/** Writes thermal results vector
 * @param fname name of results file
 * @param u vector data to save
 * @return status
 */
int femWriteThermDOFS(char *fname, tVector *u)
{
  FILE *fw = NULL ;
  long  i, len ;

	if ((fname == NULL) && (femWriteStdThrOut == AF_YES) )
	{
		fw =stdout ;
	}
	else
	{
		if ((fw = fopen(fname, "w")) == NULL)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n", _("Error during thermal result data writing"));
#endif
			return(AF_ERR_IO);
		}
	}

  /* write data: */
  len = u->rows ;
  fprintf(fw, "%li\n", len) ;
  for (i=1; i<=len; i++) { fprintf(fw, "%e\n", femVecGet(u, i)); }


	/* close file: ************************************* */
	if (fw != stdout)
	{
  	if ((fclose(fw)) != 0)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n",_("Error during closing of thermal result file"));
#endif
			fw = NULL;
			return(AF_ERR_IO);
		}
	}

  return(AF_OK);
}

/** Reads results - result fields have to be already allocated!
 *  Note: values will be ADDED, NOT REWRITTEN
 * @param fname name of file
 * @param u vector to put data in (must be allocated first!)
 * @return 
 */
int femReadThermRes(char *fname, tVector *u)
{
	int     rv = AF_OK;
	FILE   *fr = NULL ;
  double  val ;
  long    i, len ;

	/* open file: ************************************** */
	if ((fname == NULL) && (femPrevThrStdIn == AF_YES) )
	{
		fr = stdin ;
	}
	else
	{
		if ((fr = fopen(fname, "r")) == NULL)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n", _("Error during reading of previous results"));
#endif
			return(AF_ERR_IO);
		}
	}

  /* read data: */
  
  fscanf(fr, "%li\n", &len) ;

  if (len == u->rows)
  {
    for (i=1; i<=len; i++) 
    { 
      fscanf(fr, "%lf", &val ); 
      femVecAdd(u, i, val );
    }
  }
  else
  {
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s!\n",_("Refuse to read thermal data because of size inconsistence"));
#endif
      rv = AF_ERR_SIZ ;
  }

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

/* end of fem_io.c */
