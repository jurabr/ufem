/*
   File name: fdb_res.c
   Date:      2003/12/21 14:47
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

   $Id: fdb_res.c,v 1.20 2005/07/11 17:57:02 jirka Exp $
*/

#include "fdb_res.h"
#include "fdb_edef.h"
#include "cint.h"

long ResLen = 0 ;
long ResActStep = 0 ; /* 1st step is active by default */

tRes *ResNode = NULL ; /* displacements   */
tRes *ResElem = NULL ; /* element results */
tRes *ResReac = NULL ; /* reactions       */

tResAve *aRes = NULL ; /* average results           */
long aResLen  = 0    ; /* number of average results */ 
long aResAct  = 0    ; /* active average data       */

/* for MPI: */
tResProc *eResProc    = NULL ;
long      eResProcLen = 0 ;

long     *eFrom       = NULL ;
long     *eLenght     = NULL ;
double   *eRVal       = NULL ;
long      eRValLen    = 0 ;
int       fdbNoMPIeRes = AF_NO ; /* speedup if nobody needs element results */

void eResProcFree(void)
{
  long i ;

  if ((eResProc != NULL) && (eResProcLen > 0))
  {
    for (i=0; i<eResProcLen; i++)
    {
      eResProc[i].elen   = 0 ;
      eResProc[i].vallen = 0 ;
      free(eResProc[i].e_pos);  eResProc[i].e_pos  = NULL ;
      free(eResProc[i].e_from); eResProc[i].e_from = NULL ;
      free(eResProc[i].e_rlen); eResProc[i].e_rlen = NULL ;
      free(eResProc[i].e_rval); eResProc[i].e_rval = NULL ;
    }

    free(eResProc); 
    eResProc = NULL ;
  }

  eResProcLen = 0 ;
}

void femResFreeEResAux(void)
{
  femIntFree(eFrom);
  femIntFree(eLenght);
  /*femDblFree(eRVal);*/
  return;
}

/** Alocates fields for the final (sorted) element results
 * @param eLen number of elements
 * @param eValLen number of element results
 * @return status
 */
int femResAllocEResAux(long eLen)
{
  if ((eFrom=femIntAlloc(eLen)) == NULL) {goto memFree;}
  if ((eLenght=femIntAlloc(eLen)) == NULL) {goto memFree;}
  
  return(AF_OK);
memFree:
  femResFreeEResAux();
  return(AF_ERR_MEM);
}
/* end of MPI */

/** Sets result data to NULL 
 * @param Res result structure
 */
void fdbNullResStruct(tRes *Res)
{
	Res->empty = AF_YES ;
  Res->set_id = 0 ;
  Res->set_time = 0 ;

  Res->i_len = 0 ;

  Res->i_id = NULL ;
  Res->i_pos = NULL ;
  Res->from = NULL ;
  Res->len = NULL ;

  Res->data_len = 0 ;
  Res->data = NULL ;
}

void fdbFreeResStruct(tRes *Res)
{
	Res->empty = AF_YES ;
  Res->set_id   = 0 ;
  Res->set_time = 0 ;

  Res->i_len = 0 ;

  free(Res->i_id);  Res->i_id  = NULL ;
  free(Res->i_pos); Res->i_pos = NULL ;
  free(Res->from);  Res->from  = NULL ;
  free(Res->len);   Res->len   = NULL ;

  Res->data_len = 0 ;
  free(Res->data);  Res->data = NULL ;
}

void fdbFreeResSpacePtrs(void)
{
	long i ;
	
	if ( (ResLen > 0)&& (ResNode != NULL) )
	{
		for (i=0; i<ResLen; i++)
		{
			fdbFreeResStruct(&ResNode[i]) ;
			fdbFreeResStruct(&ResElem[i]) ;
			fdbFreeResStruct(&ResReac[i]) ;
		}
		
		free(ResNode) ;
		free(ResElem) ;
		free(ResReac) ;
	}

	ResNode = NULL ;
	ResElem = NULL ;
	ResReac = NULL ;
}

int fdbAllocResSpacePtrs(void)
{
	int rv = AF_OK ;
	int i ;

	if ((ResNode = (tRes *)malloc((ResLen)*sizeof(tRes))) == NULL)
	{
		fprintf(msgout, "[E] %s\n", _("Out of memory - cannot create results"));
		return(AF_ERR_MEM);
	}

	if ((ResElem = (tRes *)malloc((ResLen)*sizeof(tRes))) == NULL)
	{
		fprintf(msgout, "[E] %s\n", _("Out of memory - cannot create results"));
		free(ResNode); ResNode = NULL ;
		return(AF_ERR_MEM);
	}

	if ((ResReac = (tRes *)malloc((ResLen)*sizeof(tRes))) == NULL)
	{
		fprintf(msgout, "[E] %s\n", _("Out of memory - cannot create results"));
		free(ResNode); ResNode = NULL ;
		free(ResReac); ResReac = NULL ;
		return(AF_ERR_MEM);
	}

	for (i=0; i<ResLen; i++)
	{
		fdbNullResStruct(&ResNode[i]);
		fdbNullResStruct(&ResElem[i]);
		fdbNullResStruct(&ResReac[i]);
	}

	return(rv);
}

/** Allocates fields in single tRes structure 
 * @param Res structure
 * @param len allocation size
 * @param set_id identifier of dataset (integer number)
 * @param set_time time data (floating point number)
 * @param data_len allocation size of Res->data field
 * @param 
 * @return status
 */
int fdbAllocResStruct(tRes *Res, long len, long set_id, double set_time, long data_len)
{
	int  rv = AF_OK ;
	long i ;

	fdbNullResStruct(Res) ;

	Res->empty    = AF_YES ;
	Res->set_id   = set_id ;
	Res->set_time = set_time ;

	Res->i_len    = len ;

	Res->data_len = data_len ;

	if ((Res->i_id = (long *)malloc(len*sizeof(long))) == NULL)
	   { rv = AF_ERR_MEM ; goto memFree ; }

	if ((Res->i_pos = (long *)malloc(len*sizeof(long))) == NULL)
	   { rv = AF_ERR_MEM ; goto memFree ; }

	if ((Res->from = (long *)malloc(len*sizeof(long))) == NULL)
	   { rv = AF_ERR_MEM ; goto memFree ; }

	if ((Res->len = (long *)malloc(len*sizeof(long))) == NULL)
	   { rv = AF_ERR_MEM ; goto memFree ; }


	if ((Res->data = (double *)malloc(data_len*sizeof(double))) == NULL)
	   { rv = AF_ERR_MEM ; goto memFree ; }

	for (i=0; i<len; i++)
	{
		Res->i_id[i]  = 0 ;
		Res->i_pos[i] = 0 ;
		Res->from[i]  = 0 ;
		Res->len[i]   = 0 ;
	}

	for (i=0; i<data_len; i++) { Res->data[i] = 0.0 ; }

	if (rv == AF_OK) { return(rv) ; }
memFree:
	fprintf(msgout,"[E] %s!\n", _("Out of memory - cannot alocate results") );
	fdbFreeResStruct(Res) ;
	return(rv) ;
}

/** Reads one dataset from file
 * @param step_pos position (index) in Res(Node|Elem|Reac)[]
 * @param set_time time date (floating point number)
 * @param fname name of disk file
 * @return status
 */
int fdbResReadStep(long step_pos, double set_time, char *fname)
{
	int     rv       = AF_OK ;
	FILE   *fr ;
	long    set_id   = 0 ;
	long    dep_info = 0 ;
	long    all_dof  = 0 ;
	long    act_dof  = 0 ;
	long    res_len  = 0 ;
	long    len      = 0 ;
	long    r_node, r_id, r_dof, e_len, pos; 
	double  val ;
	long    ival ;
	long    lastpos  = 0 ;
	long    sum      = 0 ;
	long   *itmp     = NULL ;
	long   *nid     = NULL ;
	double  sim_number ;
	long    i, j, k ;
  long    nnodes ;
	long   *eid     = NULL ;
	char    type_char ;

	errno = 0 ;

	if (ResNode[step_pos].empty == AF_YES)
	{
		fdbNullResStruct(&ResNode[step_pos]);
		fdbNullResStruct(&ResElem[step_pos]);
		fdbNullResStruct(&ResReac[step_pos]);
	}
	else
	{
		fdbFreeResStruct(&ResNode[step_pos]);
		fdbFreeResStruct(&ResElem[step_pos]);
		fdbFreeResStruct(&ResReac[step_pos]);
	}

	if ((fr = fopen(fname, "r")) == NULL)
	{
		fprintf(msgout, "[E] %s: \"%s\"\n", _("Cannot open file with results"), fname);
		return(AF_ERR_IO) ; 
	}


	/* general info */
	fscanf(fr,"%c", &type_char);

	fscanf(fr,"%li %li %lf", &set_id, &dep_info, &sim_number ) ;

  if (sim_number == 0.0) {sim_number = set_time ;}
	
	/* displacements */
	fscanf(fr,"%li %li %li", &act_dof, &all_dof, &nnodes ) ;

#if 0
	if (len*KNOWN_DOFS != all_dof)
	{
		printf(" %li %li !\n", len*KNOWN_DOFS, all_dof);
		fprintf(msgout,"[E] %s!\n", _("Invalid/unsupported number of DOF results"));
		rv = AF_ERR ;
		goto memFree;
	}
#endif

	if ((itmp=(long *)malloc(all_dof*sizeof(long))) == NULL) {rv=AF_ERR_MEM; goto memFree;}
	if ((nid =(long *)malloc(all_dof*sizeof(long))) == NULL) {free(itmp); rv=AF_ERR_MEM; goto memFree;}

	for (i=0; i<nnodes; i++) { fscanf(fr,"%li", &nid[i]); } /* currently unused, will be good for testing */

	for (i=0; i<all_dof; i++) { fscanf(fr,"%li", &itmp[i]); }
	
	/* !! Proper nodes (and elements) should be selected before result reading */
	len = fdbInputTabLenAll(NODE) ;

	if ((rv=fdbAllocResStruct(&ResNode[step_pos], len, set_id, sim_number, len*KNOWN_DOFS)) != AF_OK)
	   { goto memFree ; }

	fscanf(fr,"%li", &act_dof); /* next time reading - should be the same number */

	sum   = 0 ;
  k = 0 ;

	for (i=0; i<fdbInputTabLenAll(NODE); i++)
	{
		ResNode[step_pos].i_id[i] = fdbInputGetInt(NODE, NODE_ID, i) ;
		ResNode[step_pos].i_pos[i] = i ;

#if 0
		if (fdbInputTestSelect(NODE, i) != AF_YES) 
#else
    if ((k >= all_dof)||(ResNode[step_pos].i_id[i] != nid[k]))
#endif
		{ 
			/* no results */
			ResNode[step_pos].from[i] = 0 ;
			ResNode[step_pos].len[i]  = 0 ;
		}
		else
		{
			ResNode[step_pos].from[i] = (sum*KNOWN_DOFS) ;
			ResNode[step_pos].len[i] = KNOWN_DOFS ;

			for (j=(sum*KNOWN_DOFS); j<((sum+1)*KNOWN_DOFS); j++)
			{
				if (itmp[j] > 0) { fscanf(fr,"%lf", &val) ; }
				else             { val = 0.0 ; }

				ResNode[step_pos].data[j] = val ;
			}
		
      k++ ;
			sum++ ;
		}
	}

	free(itmp) ; 
	itmp = NULL ;
	free(nid) ; 
	nid = NULL ;
	ResNode[step_pos].empty = AF_NO ;


	/* reactions */
	fscanf(fr, "%li", &res_len) ;

	if (res_len > 0)
	{
		if ((rv=fdbAllocResStruct(&ResReac[step_pos], res_len, set_id, sim_number, res_len)) != AF_OK)
	   	 { goto memFree ; }

		for (i=0; i<res_len; i++)
		{
			fscanf(fr, "%li %li %li %lf", &r_node, &r_id, &r_dof, &val);

      if (r_id != fdbInputGetInt(NODE, NODE_ID, r_node) )
      {
        if (fdbInputCountInt(NODE, NODE_ID, r_id, &pos) > 0)
        {
          r_node = pos ;
        }
        else
        {
          fprintf(msgout, "[w] %s: %li\n", _("Invalid node requested"),r_id);
          r_node = 0 ;
          continue ;
        }
      }
      else
      {
			  ResReac[step_pos].i_id[i] = r_id ;
      }
        ;
			ResReac[step_pos].i_pos[i]= r_node ;
			ResReac[step_pos].from[i] = i ;
			ResReac[step_pos].len[i]  = r_dof ; /* !!! "len" field is used for "DOF" !!! */
			ResReac[step_pos].data[i] = val ;
		}

		ResReac[step_pos].empty = AF_NO ;
	}
	else
	{
		ResReac[step_pos].empty = AF_YES ;
	}


	/* results on elements */
	fscanf(fr, "%li %li", &e_len, &res_len) ;

	if ((eid =(long *)malloc(e_len*sizeof(long))) == NULL) {free(itmp); rv=AF_ERR_MEM; goto memFree;}

	/* !! Proper elements should be selected before result reading */
	len = fdbInputTabLenAll(ELEM) ;

#if 0
	if (e_len != len)
	{
		fprintf(msgout, "[E] %s!\n", _("Invalid/unsupported number of elements"));
		rv = AF_ERR;
		goto memFree ;
	}
#endif

	if ((rv=fdbAllocResStruct(&ResElem[step_pos], fdbInputTabLenAll(ELEM), set_id, sim_number, res_len)) != AF_OK)
   	 { goto memFree ; }

	sum = 0 ;

	for (i=0; i<e_len; i++)
  {
		fscanf(fr,"%li", &eid[i]) ; 
  }

  k = 0 ;

	for (i=0; i<fdbInputTabLenAll(ELEM); i++)
	{
		ResElem[step_pos].i_id[i]  = fdbInputGetInt(ELEM, ELEM_ID, i) ;
		ResElem[step_pos].i_pos[i] = i ;   

#if 0
		if (fdbInputTestSelect(ELEM, i) != AF_YES)
#else
    if ((ResElem[step_pos].i_id[i] != eid[k]))
#endif
		{
			/* unused */
			ResElem[step_pos].from[i] = 0 ;
			ResElem[step_pos].len[i]  = 0 ;
		}
		else
		{
			/* used */
			fscanf(fr,"%li", &ival) ;

			ResElem[step_pos].from[i] = ival ;

#if 0
			if (sum > 0) { ResElem[step_pos].len[i-1]  = ival -  ResElem[step_pos].from[i-1] ; }
#else
			if (sum > 0) { ResElem[step_pos].len[lastpos]  = ival -  ResElem[step_pos].from[lastpos] ; }
#endif
			/* last element: */
			if (sum == (e_len-1)) { ResElem[step_pos].len[i]  = res_len- ival ; }

      lastpos = i ;

      sum++;
      k++ ;
		}


	}

	sum = 0 ;
  k = 0 ;

	for (i=0; i<fdbInputTabLenAll(ELEM); i++)
	{
#if 0
		if (fdbInputTestSelect(ELEM, i) == AF_YES) 
#else
    if ((ResElem[step_pos].i_id[i] == eid[k]))
#endif
		{
			for (j=ResElem[step_pos].from[i]; j<ResElem[step_pos].from[i]+ResElem[step_pos].len[i];j++)
			{
				fscanf(fr,"%lf", &ResElem[step_pos].data[sum] ) ;
				sum++ ;

				if (sum > res_len)
				{
					fprintf(msgout,"[E] %s!\n", _("Out of array when reading element results"));
					rv = AF_ERR ;
					goto memFree;
				}
			}

      k++;
		}
	}

  free(eid); eid = NULL ;

  ResElem[step_pos].empty = AF_NO ;


	if (fclose(fr) != 0)
	{
		fprintf(msgout, "[E] %s: \"%s\"\n", _("Cannot close file with results"), fname);
		rv = AF_ERR_IO ; 
	}
	fr = NULL ;

	if (rv == AF_OK) { return(rv); }

memFree:
	if (fr != NULL) {fclose(fr);}
	fprintf(msgout, "[E] %s!\n",_("Error during data reading"));
	fdbFreeResStruct(&ResNode[step_pos]);
	fdbFreeResStruct(&ResElem[step_pos]);
	fdbFreeResStruct(&ResReac[step_pos]);
	return(rv);
}

/** Reads one dataset from file created by MPI solver
 * @param step_pos position (index) in Res(Node|Elem|Reac)[]
 * @param set_time time date (floating point number)
 * @param fname name of disk file
 * @return status
 */
int fdbResReadStepFromMPI(long step_pos, double set_time, char *fname)
{
	int     rv       = AF_OK ;
	FILE   *fr ;
	long    set_id   = 0 ;
	long    dep_info = 0 ;
	long    all_dof  = 0 ;
	long    act_dof  = 0 ;
	long    res_len  = 0 ;
	long    len      = 0 ;
	long    r_node, r_dof, e_len; 
	double  val ;
	long    sum      = 0 ;
	long   *itmp     = NULL ;
	double  sim_number ;
	long    i,j,i1 ;
	char    type_char ;
  /* MPI-related: */
  long    last_dof = 0 ;
  long    prev_dof = 0 ;
	long    ii,jj,jjj ;
	long    p_num_useless = 0 ;
	long    dsum = 0 ;
	long    dof_sum = 0 ;
	double  mult = 0 ;

	errno = 0 ;

	if (ResNode[step_pos].empty == AF_YES)
	{
		fdbNullResStruct(&ResNode[step_pos]);
		fdbNullResStruct(&ResElem[step_pos]);
		fdbNullResStruct(&ResReac[step_pos]);
	}
	else
	{
		fdbFreeResStruct(&ResNode[step_pos]);
		fdbFreeResStruct(&ResElem[step_pos]);
		fdbFreeResStruct(&ResReac[step_pos]);
	}

	if ((fr = fopen(fname, "r")) == NULL)
	{
		fprintf(msgout, "[E] %s: \"%s\"\n", _("Cannot open file with results"), fname);
		return(AF_ERR_IO) ; 
	}

	fscanf(fr,"%c", &type_char);

	/* general info */
	fscanf(fr,"%li %li %lf", &set_id, &dep_info, &mult ) ;
	sim_number = mult ;

  if (sim_number == 0.0) {sim_number = set_time ;}

	/* number of processes */
  fscanf(fr,"%li", &eResProcLen);

  if (eResProcLen < 1)
  {
    rv = AF_ERR_VAL ;
    goto memFree;
  }


  /* after the number of processes become known */
  if ((eResProc=(tResProc*)malloc(eResProcLen*sizeof(tResProc)))==NULL)
  {
    eResProcLen = 0 ;
    rv = AF_ERR_VAL ;
    goto memFree;
    /* maybe more code will be here */
  }

  for (ii=0; ii<eResProcLen; ii++)
  {
    fscanf(fr,"%li", &p_num_useless); /* it's crazy! */
	
	/* displacements */
	fscanf(fr,"%li", &all_dof ) ;

/*printf("XXX 0: [%li | %li] --------------------------- \n",ii, p_num_useless);*/

  if (ii == 0)
  { 
	  if ((itmp=(long *)malloc(all_dof*sizeof(long))) == NULL) {rv=AF_ERR_MEM; goto memFree;}
  }

	for (i=0; i<all_dof; i++) { fscanf(fr,"%li", &itmp[i]); }

/*printf("XXX 1 \n");*/
	
	/* !! Proper nodes (and elements) should be selected before result reading */
	len = fdbInputTabLenAll(NODE) ;

  if (ii == 0)
  { 
		if (len*KNOWN_DOFS != all_dof)
		{
			/*printf(" %li %li !\n", len*KNOWN_DOFS, all_dof);*/
			fprintf(msgout,"[E] %s!\n", _("Invalid/unsupported number of DOF results"));
			rv = AF_ERR ;
			goto memFree;
		}

		if ((rv=fdbAllocResStruct(&ResNode[step_pos], len, set_id, sim_number, len*KNOWN_DOFS)) != AF_OK)
	   	{ goto memFree ; }
  }
/*printf("XXX 2 \n");*/

	fscanf(fr,"%li", &act_dof);

/*printf("XXX 3 (act=%li) \n", act_dof);*/

	if (ii == 0) { dof_sum   = 0 ; }
	dsum  = 0 ;

/*printf("XXX 4 (last=%li) \n", last_dof);*/

	if (ii > 0) {last_dof++;}

	for (i=last_dof; i<fdbInputTabLenAll(NODE); i++)
	{
		ResNode[step_pos].i_id[i] = fdbInputGetInt(NODE, NODE_ID, i) ;
		ResNode[step_pos].i_pos[i] = i ;

		if (fdbInputTestSelect(NODE, i) != AF_YES) 
		{ 
			/* no results */
			ResNode[step_pos].from[i] = 0 ;
			ResNode[step_pos].len[i]  = 0 ;
		}
		else
		{
			ResNode[step_pos].from[i] = (dof_sum*KNOWN_DOFS) ;
			ResNode[step_pos].len[i] = KNOWN_DOFS ;

			for (j=(dof_sum*KNOWN_DOFS); j<((dof_sum+1)*KNOWN_DOFS); j++)
			{
				if (itmp[j] > 0) 
				{
				  fscanf(fr,"%lf", &val) ; 
					dsum++;
				}
				else             { val = 0.0 ; }

				ResNode[step_pos].data[j] = val ;
			}

			dof_sum++ ;
		

      if (dsum == act_dof)
      {
				 prev_dof = last_dof ;
         last_dof = i ;

/*printf("XXX 5 (prev=%li | last=%li) \n",prev_dof, last_dof);*/

         break ;
      }
		}
	}

  if (ii == (eResProcLen-1))
  {
	  free(itmp) ; 
	  itmp = NULL ;
	  ResNode[step_pos].empty = AF_NO ;
  }

	/* reactions: */
#if 0
	ResReac[step_pos].empty = AF_YES ;
#else
	fscanf(fr, "%li", &res_len) ;

	if (res_len > 0)
	{
		if (ii == 0)
		{
			if ((rv=fdbAllocResStruct(&ResReac[step_pos], res_len, set_id, sim_number, res_len)) != AF_OK)
	   	 	{ goto memFree ; }
		}

		for (i=0; i<res_len; i++)
		{
			fscanf(fr, "%li %li %lf", &r_node, &r_dof, &val);

			if (ii == 0)
			{
				ResReac[step_pos].i_id[i] = fdbInputGetInt(NODE, NODE_ID, r_node) ;
				ResReac[step_pos].i_pos[i]= r_node ;
				ResReac[step_pos].from[i] = i ;
				ResReac[step_pos].len[i]  = r_dof ; /* !!! "len" field is used for "DOF" !!! */
			}
			ResReac[step_pos].data[i] += val ;
		}

		ResReac[step_pos].empty = AF_NO ;
	}
	else
	{
		ResReac[step_pos].empty = AF_YES ;
	}
#endif

	/* results on elements */

  eResProc[ii].elen   = 0 ;
  eResProc[ii].vallen = 0 ;
  eResProc[ii].e_pos  = NULL ;
  eResProc[ii].e_from = NULL ;
  eResProc[ii].e_rlen = NULL ;
  eResProc[ii].e_rval = NULL ;

	fscanf(fr, "%li", &e_len) ;

  eResProc[ii].elen   = e_len ;

	/* !! Proper elements should be selected before result reading */
	len = fdbInputTabLenSel(ELEM) ;

    if (eResProc[ii].elen > 0)
    {
      if ((eResProc[ii].e_pos=femIntAlloc(eResProc[ii].elen))==NULL)
         { rv = AF_ERR_MEM ; goto memFree ; }  
      if ((eResProc[ii].e_from=femIntAlloc(eResProc[ii].elen))==NULL)
         { rv = AF_ERR_MEM ; goto memFree ; }
      if ((eResProc[ii].e_rlen=femIntAlloc(eResProc[ii].elen))==NULL)
         { rv = AF_ERR_MEM ; goto memFree ; }

/*printf("XXX 7\n");*/

      for (j=0; j<eResProc[ii].elen; j++)
      {
        fscanf(fr,"%li %li", &eResProc[ii].e_pos[j], &eResProc[ii].e_from[j]);
      }

/*printf("XXX 8\n");*/

      for (j=0; j<(eResProc[ii].elen-1); j++)
      {
        eResProc[ii].e_rlen[j]
					=
					eResProc[ii].e_from[j+1]
					-
					eResProc[ii].e_from[j];
      }

/*printf("XXX 9\n");*/
  
      fscanf(fr,"%li", &eResProc[ii].vallen);

/*printf("XXX 10 (vallen=%li)\n",eResProc[ii].vallen);*/

      eResProc[ii].e_rlen[eResProc[ii].elen-1]=
      eResProc[ii].vallen 
      -
      eResProc[ii].e_from[eResProc[ii].elen-1] 
			;

      if ((eResProc[ii].e_rval=femDblAlloc(eResProc[ii].vallen))==NULL)
         { rv = AF_ERR_MEM ; goto memFree ; }

      for (j=0; j<eResProc[ii].vallen; j++)
      {
        fscanf(fr,"%lf", &eResProc[ii].e_rval[j]);
      }

      /* computing of element position */
      for (j=0; j<eResProc[ii].elen; j++)
      {
        for (i1=0; i1<fdbInputTabLenAll(ELEM); i1++)
        {
          if (eResProc[ii].e_pos[j] == fdbInputGetInt(ELEM, ELEM_ID,i1))
          {
            eResProc[ii].e_pos[j] = i1 ;
            break ;
          }
        }
      }
    }
    else
    {
      /* nothing to do here */
      eResProc[ii].elen   = 0 ;
      fscanf(fr,"%li", &eResProc[ii].vallen);
      eResProc[ii].vallen = 0 ;
    } 

  } /* for ii */


#if 1
	if (fdbNoMPIeRes != AF_YES)
	{

  	if ((rv=femResAllocEResAux(fdbInputTabLenAll(ELEM))) != AF_OK) {goto memFree;}

  	for (j=0; j<eResProcLen; j++)
    {
      for (jj=0; jj<eResProc[j].elen; jj++)
      {
        eLenght[eResProc[j].e_pos[jj]]
					=
					eResProc[j].e_rlen[jj];
      }
    }

   	eRValLen = eLenght[0] ;
   	eFrom[0] = 0 ;

	 	len = fdbInputTabLenAll(ELEM) ;

   	for (j=1; j<len; j++)
   	{
     	eFrom[j] = eLenght[j] + eFrom[j-1] ;
     	eRValLen += eLenght[j] ;
   	}

   	if ((eRVal=femDblAlloc(eRValLen)) == NULL) 
     	 {rv = AF_ERR_MEM; goto memFree;}

		/* fill eRVal field: */
    for (j=0; j<eResProcLen; j++)
    {
     	for (jj=0; jj<eResProc[j].elen; jj++)
     	{
       	for (jjj=eResProc[j].e_from[jj]; jjj<(eResProc[j].e_from[jj]+eResProc[j].e_rlen[jj]); jjj++)
       	{
         	eRVal[eFrom[eResProc[j].e_pos[jj]]+(jjj-eResProc[j].e_from[jj])]
         	= 
         	eResProc[j].e_rval[jjj] ; /* it is crazy, isn't it? */
					sum++;
				}
			}
		}


  	/* Now copy these values to proper result data structures: */ 
  	len = fdbInputTabLenAll(ELEM) ;
		res_len = eRValLen ;

		if ((rv=fdbAllocResStruct(&ResElem[step_pos], len, set_id, sim_number, res_len)) != AF_OK)
   	 	{ goto memFree ; }

		sum = 0 ;

		for (i=0; i<len; i++)
		{
			ResElem[step_pos].i_id[i]  = fdbInputGetInt(ELEM, ELEM_ID, i) ;
			ResElem[step_pos].i_pos[i] = i ;   

			if (fdbInputTestSelect(ELEM, i) != AF_YES)
			{
				/* unused */
				ResElem[step_pos].from[i] = 0 ;
				ResElem[step_pos].len[i]  = 0 ;
			}
			else
			{
				/* used */
				ResElem[step_pos].from[i] = eFrom[i] ;
				ResElem[step_pos].len[i]  = eLenght[i] ;

				sum++;
			}
		}

		for (i=0; i<res_len; i++)
		{
			ResElem[step_pos].data[i] = eRVal[i] ;
		}
	}
#endif

	if (fclose(fr) != 0)
	{
		fprintf(msgout, "[E] %s: \"%s\"\n", _("Cannot close file with results"), fname);
		rv = AF_ERR_IO ; 
	}
	fr = NULL ;

  /* MPI code: */
  eResProcFree();
  femResFreeEResAux();
  femDblFree(eRVal);

	if (rv == AF_OK) { return(rv); }

memFree:
	if (fr != NULL) {fclose(fr);}
	fprintf(msgout, "[E] %s!\n",_("Error during data reading"));
	fdbFreeResStruct(&ResNode[step_pos]);
	fdbFreeResStruct(&ResElem[step_pos]);
	fdbFreeResStruct(&ResReac[step_pos]);

  /* MPI code: */
  eResProcFree();
  
  femResFreeEResAux();
  femDblFree(eRVal);
  
	return(rv);
}

/** Copies result step from existing one
 * @param orig_pos original step position (index) in Res(Nede|Elem|Reac)[]
 * @param step_pos position (index) in Res(Node|Elem|Reac)[]
 * @param set_time time date (floating point number)
 * @param fname name of disk file
 * @return status
 */
int fdbResCopyStep(long orig_pos, long step_pos, double set_time)
{
	int     rv       = AF_OK ;
	long    set_id   = 0 ;
	long    res_len  = 0 ;
	long    len      = 0 ;
	long    i,j ;

	errno = 0 ;

	if (ResNode[step_pos].empty == AF_YES)
	{
		fdbNullResStruct(&ResNode[step_pos]);
		fdbNullResStruct(&ResElem[step_pos]);
		fdbNullResStruct(&ResReac[step_pos]);
	}
	else
	{
		fdbFreeResStruct(&ResNode[step_pos]);
		fdbFreeResStruct(&ResElem[step_pos]);
		fdbFreeResStruct(&ResReac[step_pos]);
	}

  set_id = ResNode[orig_pos].set_id ;

  len     = ResNode[orig_pos].i_len ;
  res_len = ResNode[orig_pos].data_len ;

	if ((rv=fdbAllocResStruct(&ResNode[step_pos], len, set_id, set_time, len*KNOWN_DOFS)) != AF_OK)
	   { goto memFree ; }


	for (i=0; i<fdbInputTabLenAll(NODE); i++)
	{
		ResNode[step_pos].i_id[i]  = ResNode[orig_pos].i_id[i] ;
		ResNode[step_pos].i_pos[i] = ResNode[orig_pos].i_pos[i] ;


	  ResNode[step_pos].from[i] = ResNode[orig_pos].from[i] ;
		ResNode[step_pos].len[i]  = ResNode[orig_pos].len[i] ;

	}

	for (j=0; j<ResNode[orig_pos].data_len; j++)
	{
		ResNode[step_pos].data[j] = ResNode[orig_pos].data[j] ;
	}

	ResNode[step_pos].empty = AF_NO ;


	/* reactions */
  res_len = ResReac[orig_pos].i_len ;
    

	if (res_len > 0)
	{
		if ((rv=fdbAllocResStruct(&ResReac[step_pos], res_len, set_id, set_time, res_len)) != AF_OK)
	   	 { goto memFree ; }

		for (i=0; i<res_len; i++)
		{
			ResReac[step_pos].i_id[i] = ResReac[orig_pos].i_id[i] ;
			ResReac[step_pos].i_pos[i]= ResReac[orig_pos].i_pos[i] ;
			ResReac[step_pos].from[i] = ResReac[orig_pos].from[i] ;
			ResReac[step_pos].len[i]  = ResReac[orig_pos].len[i] ;
			ResReac[step_pos].data[i] = ResReac[orig_pos].data[i] ;
		}

		ResReac[step_pos].empty = AF_NO ;
	}
	else
	{
		ResReac[step_pos].empty = AF_YES ;
	}


	/* results on elements */
  len     = ResElem[orig_pos].i_len ;
  res_len = ResElem[orig_pos].data_len ;

	if ((rv=fdbAllocResStruct(&ResElem[step_pos], len, set_id, set_time, res_len)) != AF_OK)
   	 { goto memFree ; }


	for (i=0; i<len; i++)
	{
		ResElem[step_pos].i_id[i]  = ResElem[orig_pos].i_id[i] ;
		ResElem[step_pos].i_pos[i] = ResElem[orig_pos].i_pos[i] ;

		ResElem[step_pos].from[i] = ResElem[orig_pos].from[i] ;
		ResElem[step_pos].len[i]  = ResElem[orig_pos].len[i] ;
    ResElem[step_pos].from[i] = ResElem[orig_pos].from[i] ;
    ResElem[step_pos].len[i] = ResElem[orig_pos].len[i] ;
	}


	for (i=0; i<res_len; i++)
	{
    ResElem[step_pos].data[i] = ResElem[orig_pos].data[i] ;
  }

	ResElem[step_pos].empty = AF_NO ;

	if (rv == AF_OK) { return(rv); }

memFree:
	fdbFreeResStruct(&ResNode[step_pos]);
	fdbFreeResStruct(&ResElem[step_pos]);
	fdbFreeResStruct(&ResReac[step_pos]);
	return(rv);
}


/** Frees one set or results
 * @param step_pos position (index) in Res(Nede|Elem|Reac)[]
 */
void fdbResFreeStep(long step_pos)
{
	if (ResNode[step_pos].empty == AF_YES)
	{
		fdbNullResStruct(&ResNode[step_pos]);
		fdbNullResStruct(&ResElem[step_pos]);
		fdbNullResStruct(&ResReac[step_pos]);
	}
	else
	{
		fdbFreeResStruct(&ResNode[step_pos]);
		fdbFreeResStruct(&ResElem[step_pos]);
		fdbFreeResStruct(&ResReac[step_pos]);
	}
}


/** Returnes max and min values from result data
 * @param Res results
 * @param max pointer to maximum value
 * @param min pointer to minimum value
 * @return status
 */
int fdbResMaxMinAny(tRes *Res, double *max, double *min)
{
  long i;

  if (Res->data_len >= 1)
  {
    *max = Res->data[0] ;
    *min = Res->data[0] ;

    for (i=1; i<Res->data_len; i++)
    {
      if (*max < Res->data[i]) {*max = Res->data[i];}
      if (*min > Res->data[i]) {*min = Res->data[i];}
    }
  }
  else /* nothing to do */
  { 
    return(AF_ERR_EMP) ; 
  } 

  return(AF_OK);
}


/** Found max and min for result on nodes in given direction
 * @param dir_index index of direction (0..ux, 1..uy etc. )
 * @param max pointer to maximum value
 * @param min pointer to minimum value
 * @return status
 */
int fdbResMaxMinNode(long dir_index, double *max, double *min)
{
  long i, j ;
  long count ;

  if (ResNode[ResActStep].i_len <= 0) {return(AF_ERR_EMP);}

  count = 0 ;

  for (i=0; i<ResNode[ResActStep].i_len; i++)
  {
    if (fdbInputTestSelect(NODE, ResNode[ResActStep].i_pos[i]) != AF_YES) 
       {continue;}

    for (j=ResNode[ResActStep].from[i];j<(ResNode[ResActStep].from[i]+ResNode[ResActStep].len[i]); j++ )
    {
      if ((j-ResNode[ResActStep].from[i]) == dir_index)
      {
        count++;

        if (*max < ResNode[ResActStep].data[j]) {*max = ResNode[ResActStep].data[j];}
        if (*min > ResNode[ResActStep].data[j]) {*min = ResNode[ResActStep].data[j];}
      }
    }
  }
  if (count > 0) { return(AF_OK);      }
  else           { return(AF_ERR_EMP); }
}


double fdbResElemGetVal0(long res_pos, long etype, long type, long set)
{
	long   i ;
	long   rpos;
  double val = 0.0;

  val = 0.0 ;
	if (set == 0)
	{
		for (i=0; i<fdbElementType[etype].res; i++)
		{
			if (i >= ResElem[ResActStep].len[res_pos]){return(0.0);} /* we are out of field! */

			if (fdbElementType[etype].nres[i] == type)
			{
#if 0
				printf("(et=%li) %li: %f\n", etype,
i+ResElem[ResActStep].from[res_pos],
ResElem[ResActStep].data[i+ResElem[ResActStep].from[res_pos]]
						);
#endif
				val = ResElem[ResActStep].data[i+ResElem[ResActStep].from[res_pos]];
        break ;
			}
		}
	}
	else
	{
		for (i=0; i<fdbElementType[etype].res_rp; i++)
		{
			rpos = i+((set-1)*fdbElementType[etype].res_rp)+fdbElementType[etype].res ;

			if (rpos >= (ResElem[ResActStep].len[res_pos]+ResElem[ResActStep].from[res_pos]))
      {return(0.0);} /* we are out of field! */

			if (fdbElementType[etype].nres_rp[i] == type)
			{
				val = ResElem[ResActStep].data[rpos+ResElem[ResActStep].from[res_pos]];
        break ;
			}
		}
	}

	return(val);
}



/** Returnes result value
 * @param res_pos position (index) of results
 * @etype type of element (fdbElementType)
 * @type type of value
 * @set set (0..non-repeating, 1,2,.. .. repeating)
 * @return value
 */
double fdbResElemGetVal(long res_pos, long etype, long type, long set)
{

  /* it will make things slower: */
  if (fdbResComp_IsComp(type) == AF_YES)
  {
    return(fdbResComp_Val(res_pos, set, etype, type));
  }
  else
  {
    return(fdbResElemGetVal0(res_pos, etype, type, set));
  }
}



/** Returnes number of existing _repeating_ sets of results 
 * @param res_pos position of checked result
 * @param etype type of element
 * @return number of sets
 */
long fdbResElemNumRepSets(long res_pos, long etype)
{
	long    sets = 0 ;
	long    num ;
	ldiv_t  val ;

  if (res_pos < 0) { return(0); }

	num = ResElem[ResActStep].len[res_pos] - fdbElementType[etype].res ;
	if (num <= 0) {return(0);}

	val = ldiv(num, fdbElementType[etype].res_rp) ;
	sets = val.quot ;
	return(sets);
}

/** Found max and min for result on elements in given direction
 * @param dir_index index of direction (0..ux, 1..uy etc. )
 * @param max pointer to maximum value
 * @param min pointer to minimum value
 * @return status
 */
int fdbResMaxMinElem(long type, double *max, double *min)
{
  long i, j, k ;
  long count ;
  long pos, num, rnum, rrep;
  long etype,etype_usr ;
  long computed = AF_NO ;

  count = 0 ;
  *max = 0.0 ;
  *min = 0.0 ;

  if (ResElem[ResActStep].i_len <= 0) {return(AF_ERR_EMP);}

  computed = fdbResComp_IsComp(type) ; 

  for (i=0; i<ResElem[ResActStep].i_len; i++)
  {
    if (fdbInputTestSelect(ELEM, ResElem[ResActStep].i_pos[i]) != AF_YES) 
       {continue;}

    etype_usr = fdbInputGetInt(ELEM, ELEM_TYPE, ResElem[ResActStep].i_pos[i]) ;
    if (fdbInputCountInt(ETYPE, ETYPE_ID, etype_usr, &pos) < 1) 
       {continue;} /* invalid etype */
    etype = fdbInputGetInt(ETYPE, ETYPE_TYPE, pos) ;

    num  = fdbElementType[etype].res ;
    rnum = fdbElementType[etype].res_rp ;
    rrep = fdbResElemNumRepSets(i, etype);

    if (computed == AF_YES)
    {
      for (j=0; j<=rrep; j++)
      {
        if (*max < fdbResComp_Val( i, j, etype, type))
        {
          *max = fdbResComp_Val( i, j, etype, type) ;
        }
        if (*min > fdbResComp_Val( i, j, etype, type))
        {
          *min = fdbResComp_Val( i, j, etype, type) ;
        }
      }
      count++ ;
    }
    else
    {

      for (j=0; j<num; j++)
      {
        if (fdbElementType[etype].nres[j] == type)
        {
          pos = j + ResElem[ResActStep].from[i] ;
          count++;
  
          if (*max < ResElem[ResActStep].data[pos]) {*max = ResElem[ResActStep].data[pos];}
          if (*min > ResElem[ResActStep].data[pos]) {*min = ResElem[ResActStep].data[pos];}
        }
      }

      if (rrep > 0)
      {
        for (j=0; j<rnum; j++)
        {
          if (fdbElementType[etype].nres_rp[j] == type)
          {
            for (k=0; k<rrep; k++)
            {
              pos = j + ResElem[ResActStep].from[i] + num + k*rnum ;
              count++;
    
              if (*max < ResElem[ResActStep].data[pos]) {*max = ResElem[ResActStep].data[pos];}
              if (*min > ResElem[ResActStep].data[pos]) {*min = ResElem[ResActStep].data[pos];}
            }
          }
        }
      }
    }
  }

#if 0
  printf("Count is %li (max=%f min=%f, res=%li)\n",count, *max, *min, type);
#endif

  if (count > 0) { return(AF_OK);      }
  else           { return(AF_ERR_EMP); }
}

/** Returnes result on node 
 * @param node_pos - position of node
 * @param dof  Degree of freedom type
 * @return result value (or 0.0 on error)
 */
double fdbResNodeGetVal(long node_pos, long dof)
{
	long   dof0 ;
  
  dof0 = dof - 1 ;
	if ((dof0 < 0)||(dof0 >= KNOWN_DOFS))
	{
    fprintf(msgout,"[E] %s!\n", _("Unknown degree of freedom requested"));
		return(0.0);
	}
  
  if ((ResLen <= 0) || (ResActStep >= ResLen)) 
	{ 
		return(0.0);
	}
  else
  {
    if ((node_pos >= 0)&&(node_pos < ResNode[ResActStep].i_len))
    {
      if (fdbInputTestSelect(NODE, node_pos) != AF_YES) 
	  	{ 
        fprintf(msgout,"[W] %s!\n", _("Node is unselected"));
        return(0.0);
    	}
    	else
    	{
      	return(ResNode[ResActStep].data[(node_pos*KNOWN_DOFS) + dof0]) ;
    	}

    }
    else
    {
      fprintf(msgout,"[E] %s!\n", _("Node not found"));
      return(0.0);
    }
  }


  return(0.0);
}

/* ------------------------------------------------- */

/** NULLs one average results set 
 * @param pos index of set
 * */
void fdbAvResNull(long pos)
{
  if ((pos < aResLen)&&(pos >= 0))
  {
    aRes[pos].empty   = AF_YES ;
    aRes[pos].set_id  = 0 ;
    aRes[pos].type    = 0 ; ;
    aRes[pos].len     = 0 ;
    aRes[pos].data    = NULL ;
  }
}

/** Frees one average results set 
 * @param pos index of set
 * */
void fdbAvResFree(long pos)
{
  if ((pos < aResLen)&&(pos >= 0))
  {
    if (aRes[pos].empty != AF_YES)
    {
      free(aRes[pos].data) ; 
    }
    fdbAvResNull(pos) ;
  }
}

/** Allocates one average results set 
 * @param pos index of set
 * @param set_id identifier of data set ("time")
 * @param type type of data
 * @return status
 * */
int fdbAvResAlloc(long pos, long set_id, long type)
{
  long len ;
  long i ;
  
  if (pos < aResLen)
  {
    fdbAvResNull(pos) ;

    len = fdbInputTabLenAll(NODE) ;

    if ((aRes[pos].data = (double *)malloc(len*sizeof(double))) == NULL)
    {
      fprintf(msgout, "[E] %s!\n", _("Cannot alocate space for results") );
      fdbAvResNull(pos) ;
      return(AF_ERR_MEM) ;
    }

    for (i=0; i<len; i++) { aRes[pos].data[i] = 0.0 ; }

    aRes[pos].empty  = AF_NO ;
    aRes[pos].set_id = set_id ;
    aRes[pos].type   = type ;
    aRes[pos].len    = len  ;

    return(AF_OK);
  }
  else
  {
    return(AF_ERR_VAL) ; 
  }
}


/** Allocates space for average results
 * @param len lenght of "types" and "times"
 * @types list of wanted types of results
 * @types list of wanted times (the same lenght as types!)
 * @return status
 * */
int fdbAvResReAllocSpace(long len, long *types, long *times)
{
  long i ;

  if (len <= 0) {return(AF_ERR_EMP);}

  aResAct = 0 ;

  /* clean previous data: */
  if (aResLen > 0)
  {
    for (i=0; i<aResLen; i++) { fdbAvResFree(i) ; }

		free(aRes);
  }

	aRes = NULL ;

  aResLen = 0 ;

  if ((aRes=(tResAve *)malloc(len*sizeof(tResAve))) == NULL)
  {
      fprintf(msgout, "[E] %s!\n", _("Cannot alocate space for results") );
      return(AF_ERR_MEM) ;
  }
  else
  {
    for (i=0; i<len; i++) { fdbAvResNull(i) ; }

    aResLen = len ;

    for (i=0; i<len; i++) 
    { 
      if (fdbAvResAlloc(i, times[i], types[i]) != AF_OK) { goto memFree; }
    }
  
    return(AF_OK) ;
  }

memFree:
  for (i=0; i<len; i++) { fdbAvResFree(i) ; }
  free(aRes) ;
  aRes = NULL ;
  aResLen = 0 ;
  fprintf(msgout, "[E] %s!\n", _("Result space allocation failed") );
  return(AF_ERR_MEM) ;
}


/** Removes averge rsults from memory */
void fdbAvResFreeSpace(void)
{
  long i;

  for (i=0; i<aResLen; i++) { fdbAvResFree(i) ; }
  free(aRes) ;
  aRes = NULL ;
  aResLen = 0 ;
  aResAct = 0 ;
}

/** Fills one set of average results from element data 
 * data should be allocated first! 
 * @param pos pos in avRes
 * @param type type of result
 * @return status
 * */
int fdbAvResFillNormal(long pos, long type)
{
  long  nlen ;
  long  npos ;
  long  len ;
  long  e_pos;
  long  etpos ;
  long  etype ;
  long  etype_prog ;
  long *nhit = NULL ; /* number of items in field */
  long  i,j ;

  nlen = fdbInputTabLenAll(NODE);

  if ((nhit=(long *)malloc(nlen*sizeof(long))) == NULL)
  {
    fprintf(msgout, "[E] %s!\n", _("No space to average results") );
    return(AF_ERR_MEM);
  }
  for(i=0; i<nlen; i++) { nhit[i] = 0 ; }


  len = ResElem[ResActStep].i_len ;

  for (i=0; i<len; i++)
  {
    e_pos = ResElem[ResActStep].i_pos[i] ;

    if (fdbInputTestSelect(ELEM, e_pos) != AF_YES) {continue ;}
    
    etype = fdbInputGetInt(ELEM, ELEM_TYPE, e_pos);
	  if (fdbInputCountInt(ETYPE, ETYPE_ID, etype, &etpos) < 1)
       { continue; /* no such element type */ }
    etype_prog = fdbInputGetInt(ETYPE, ETYPE_TYPE, etpos);

    for (j=0; j<fdbInputGetInt(ELEM,ELEM_NODES, e_pos); j++)
    {
      npos = fdbEnodePos(e_pos, j) ;
      aRes[pos].data[npos] += fdbElementType[etype_prog].node_res(i, etype_prog, j, type) ;
      nhit[npos]++ ;
    }
  }

  for (i=0; i<nlen; i++)
  {
    if (nhit[i] > 0)
    {
      aRes[pos].data[i] = aRes[pos].data[i] / ((double)nhit[i]) ;
    }
  }

  free(nhit);
  nhit = NULL ;
    
  return(AF_OK) ;
}

/** Prepares data for plotting
 * @param len lenght of "types" and "times"
 * @types list of wanted types of results
 * @times list of wanted times (the same lenght as types!)
 * @return status
 * */
int fdbAvResPrepareData(long len, long *times, long *types)
{
  int   rv = AF_OK ;
  int   specdata = AF_NO ;
  int   dataok = AF_YES ;
  long  i ;

  /* place test here: data may be special,
   * if not then search inside normal data */

  if (specdata == AF_YES)
  {
    /* CODE NEEDED HERE! */
  }
  else
  {
    if (len == aResLen)
    {
      for (i=0; i<len; i++)
      {
        if ( (types[i] != aRes[i].type) || (times[i] != aRes[i].set_id) )
        {
          dataok = AF_NO ;
          break ;
        }
      }
    }
    else
    {
      dataok = AF_NO ;
    }

    if (dataok != AF_YES) /* there are no valid data - have to create new */
    {
      if ((rv=fdbAvResReAllocSpace(len, types, times)) != AF_OK){return(rv);}
      for (i=0; i<len; i++) 
      { 
        fdbAvResFillNormal(i, types[i]) ; 
      }
    }
  }

  return(AF_OK) ;
}

/* Computes max and min values from average result set
 * @param pos position of set
 * @max pointer to max (result)
 * @max pointer to min (result)
 * @return status
 * */
int fdbAvResMaxMin(long pos, double *max, double *min)
{
  long i;

  if (pos >= aResLen) {return(AF_ERR_VAL);}

  for (i=0; i<aRes[pos].len; i++)
  {
    if (*max < aRes[pos].data[i]) {*max = aRes[pos].data[i] ;}
    if (*min > aRes[pos].data[i]) {*min = aRes[pos].data[i] ;}
  }
  return(AF_OK) ;
}

/** Returns value from averaged results
 * @param type type of result
 * @param type node position
 * @return value (or 0.0)
 * */
double fdbAvResGetVal(long type, long n_pos)
{
  int    i ;

  for (i=0; i<aResLen; i++)
  {
    if (aRes[i].type == type)
    {
      return(aRes[i].data[n_pos]) ;
    }
  }
  return(0.0) ;
}

/* ================================================= */

int fdbResListReacts(FILE *fw)
{
  int  rv = AF_OK ;
  long nodepos ;
  long i,j ;
  double *sums = NULL ;

  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_ERR); }

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }


  sums = (double *) malloc(KNOWN_DOFS*sizeof(double));

  fdbPrnBeginTable(fw, 1+KNOWN_DOFS, _("Reactions")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Node")) ;
  for (j=1; j<=KNOWN_DOFS; j++)
  {
    fdbPrnTableHeaderItem(fw, j, 
            ciGetVarNameFromGrp(
              fdbFemStrFromInt(j),"load")
        ) ;

    if (sums != NULL) { sums[j-1] = 0.0 ; }
  }


  for (i=0; i<ResReac[ResActStep].i_len; i++)
  {

    nodepos = ResReac[ResActStep].i_pos[i] ;

    if (fdbInputTestSelect(NODE, nodepos) != AF_YES) 
	  { 
      continue ; 
    }
    else
    {
      fdbPrnTableItemInt(fw,0,
          fdbInputGetInt(NODE,NODE_ID,nodepos));

      for (j=1; j<=KNOWN_DOFS; j++)
      {
        if (ResReac[ResActStep].len[i] == (j))
        {
          fdbPrnTableItemDbl(fw,j,
		 			ResReac[ResActStep].data[i] );

          if (sums != NULL)
          {
            sums[j-1] += ResReac[ResActStep].data[i] ;
          }
  
        }
        else
        {
          fdbPrnTableItemDbl(fw,j, 0.0);
        }
      }
    }
  }

  fdbPrnTableItemStr(fw,0,_("Total:"));

  for (j=1; j<=KNOWN_DOFS; j++)
  {
    fdbPrnTableItemDbl(fw,j, sums[j-1] );
  }

  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);

  if (sums != NULL) { free(sums); sums = NULL; }
  return(rv);
}

int fdbResListDOFSolu(FILE *fw)
{
  int  rv = AF_OK ;
  long nodepos ;
  double val ;
  double max[KNOWN_DOFS] ;
  double min[KNOWN_DOFS] ;
  long i,j ;

  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_ERR); }

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  for (i=0; i<KNOWN_DOFS; i++)
  {
    max[i] = 0.0 ;
    min[i] = 0.0 ;
  }

  fdbPrnBeginTable(fw, 1+KNOWN_DOFS, _("DOF Solution")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Node")) ;
  for (j=1; j<=KNOWN_DOFS; j++)
  {
    fdbPrnTableHeaderItem(fw, j, 
            ciGetVarNameFromGrp(
              fdbFemStrFromInt(j),"disp")
        ) ;
  }


  for (i=0; i<ResNode[ResActStep].i_len; i++)
  {

    nodepos = ResNode[ResActStep].i_pos[i] ;

    if (fdbInputTestSelect(NODE, nodepos) != AF_YES) 
	  { 
      continue ; 
    }
    else
    {
      fdbPrnTableItemInt(fw,0,
          fdbInputGetInt(NODE,NODE_ID,nodepos));

      for (j=1; j<=KNOWN_DOFS; j++)
      {
        val = ResNode[ResActStep].data[ResNode[ResActStep].from[i]+(j-1)] ;

        fdbPrnTableItemDbl(fw,j, val );

        if (max[j-1] < val) {max[j-1] = val;}
        if (min[j-1] > val) {min[j-1] = val;}
      }
    }
  }

  fdbPrnTableItemStr(fw,0, _("Maximum") );
  for (j=0; j<KNOWN_DOFS; j++)
    { fdbPrnTableItemDbl(fw,j+1, max[j]); }

  fdbPrnTableItemStr(fw,0, _("Minimum") );
  for (j=0; j<KNOWN_DOFS; j++)
    { fdbPrnTableItemDbl(fw,j+1, min[j]); }

  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);

  return(rv);
}

/** Tests if element type "etype" knows about results "res_type" */
long femCheckHaveResult(long etype, long res_type)
{
  int i;

  for (i=0; i<fdbElementType[etype].res; i++)
  {
    if (fdbElementType[etype].nres[i] == res_type){return(AF_YES);}
  }

  for (i=0; i<fdbElementType[etype].res_rp; i++)
  {
    if (fdbElementType[etype].nres_rp[i] == res_type){return(AF_YES);}
  }

  return(fdbResComp_IsComp(res_type));

  return(AF_NO);
}

/** Tests if element type "etype" knows about results "res_type" for NON-REPATING */
long femCheckHaveResultNoRep(long etype, long res_type)
{
  int i;

  for (i=0; i<fdbElementType[etype].res; i++)
  {
    if (fdbElementType[etype].nres[i] == res_type){return(AF_YES);}
  }

  return(fdbResComp_IsComp(res_type));

  return(AF_NO);
}


/** Tests if element type "etype" knows about results "res_type" for REPEATING */
long femCheckHaveResultRep(long etype, long res_type)
{
  int i;

  for (i=0; i<fdbElementType[etype].res_rp; i++)
  {
    if (fdbElementType[etype].nres_rp[i] == res_type){return(AF_YES);}
  }



  return(fdbResComp_IsComp(res_type));

  return(AF_NO);
}





/** Print results on elements
 *
 * @return status
 */
int fdbResListElemSolu(FILE *fw, long *res_type, long type_len)
{
  int     rv  = AF_OK ;
  long    i, j,k ;
  long    num, norep, rep;
	double *max = NULL ;
	double *min = NULL ;
  double  val ;

  long    epos ;
  long    etype ;
  long    etype_prog ;
  long    id ;

  if (res_type == NULL) {return(AF_ERR_EMP);}
  if (type_len < 1) 
  {
    fprintf(msgout,"[E] %s!\n", _(""));
    return(AF_ERR_EMP);
  }

  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_ERR); }

  max = (double *)malloc(type_len*sizeof(double)) ;
  min = (double *)malloc(type_len*sizeof(double)) ;

  for (i=0; i<type_len; i++)
  {
    max[i] = 0.0 ;
    min[i] = 0.0 ;
  }

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }


  fdbPrnBeginTable(fw, type_len+1, _("Element Solution Results")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Element")) ;

  for (j=0; j<type_len; j++)
  {
    fdbPrnTableHeaderItem(fw, j+1, 
           ciGetVarNameFromGrp(
              fdbFemStrFromInt(res_type[j]), 
              "result")
        ) ;
  }

  for (i=0; i< ResElem[ResActStep].i_len; i++)
  {
    if ( fdbInputTestSelect(ELEM, ResElem[ResActStep].i_pos[i]) != AF_YES) 
	  { 
      continue ; 
    }
    else
    {
      /* finding type of stuff to be plotted*/
      etype = fdbInputGetInt(ELEM, ELEM_TYPE, ResElem[ResActStep].i_pos[i]);

	    if (fdbInputCountInt(ETYPE, ETYPE_ID, etype, &epos) < 1)
         { return (AF_ERR_VAL); }

      id = fdbInputGetInt(ELEM,ELEM_ID, ResElem[ResActStep].i_pos[i]);

      etype_prog = fdbInputGetInt(ETYPE, ETYPE_TYPE, epos);

      norep = fdbElementType[etype_prog].res ;
      rep   = fdbElementType[etype_prog].res_rp ;

      num = ResElem[ResActStep].len[i] ;

      if (rep > 0) { num = 1 + (long) ( (num - norep) / rep ) ; }
      else         { num = 1 ; }


      for (k=0; k<num; k++)
      {
        if ( (norep <= 0) && (k == 0) ) { continue; }
        fdbPrnTableItemInt(fw, 0, id );

        for (j=0; j<type_len; j++)
        {
          if (femCheckHaveResult(etype_prog, res_type[j]) == AF_YES)
          {
            val = fdbResElemGetVal(i, etype_prog, res_type[j], k);

            if (val > max[j]) { max[j] = val ; }
            if (val < min[j]) { min[j] = val ; }
          }
          else
          {
            val = 0.0 ;
          }

          fdbPrnTableItemDbl(fw,j+1, val);
        }
      }
    }
  }

  fdbPrnTableItemStr(fw,0, _("Maximum") );
  for (j=0; j<type_len; j++)
    { fdbPrnTableItemDbl(fw,j+1, max[j]); }

  fdbPrnTableItemStr(fw,0, _("Minimum") );
  for (j=0; j<type_len; j++)
    { fdbPrnTableItemDbl(fw,j+1, min[j]); }

  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);

  if (max != NULL) { free(max); max = NULL; }
  if (min != NULL) { free(min); min = NULL; }

  return(rv);
}

/** Print AVERAGED results on nodes
 *
 * @return status
 */
int fdbResListAvSolu(FILE *fw, long *res_type, long type_len)
{
  int     rv  = AF_OK ;
  long    i, j ;
	long   *set_id = NULL ;
	double *max    = NULL ;
	double *min    = NULL ;
  double  val ;

  if (res_type == NULL) {return(AF_ERR_EMP);}
  if (type_len < 1) 
  {
    fprintf(msgout,"[E] %s!\n", _(""));
    return(AF_ERR_EMP);
  }

  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_ERR); }

  max = (double *)malloc(type_len*sizeof(double)) ;
  min = (double *)malloc(type_len*sizeof(double)) ;
  set_id = (long *)malloc(type_len*sizeof(long)) ;

	for (i=1; i<type_len; i++) 
  { 
    set_id[i] = ResElem[ResActStep].set_id ;
  }
	
	if ((rv=fdbAvResPrepareData(type_len, set_id, res_type)) != AF_OK)
  {
    return(rv) ;
  }

  for (i=0; i<type_len; i++)
  {
    max[i] = 0.0 ;
    min[i] = 0.0 ;
  }

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }


  fdbPrnBeginTable(fw, type_len+1, _("Nodal Solution Results")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Node")) ;

  for (j=0; j<type_len; j++)
  {
    fdbPrnTableHeaderItem(fw, j+1, 
           ciGetVarNameFromGrp(
              fdbFemStrFromInt(res_type[j]), 
              "result")
        ) ;
  }

  for (i=0; i< fdbInputTabLenAll(NODE); i++)
  {
    if ( fdbInputTestSelect(NODE, i) != AF_YES) 
	  { 
      continue ; 
    }
    else
    {
        fdbPrnTableItemInt(fw, 0, fdbInputGetInt(NODE,NODE_ID, i) );

        for (j=0; j<type_len; j++)
        {
      			val = fdbAvResGetVal(res_type[j], i);

            if (val > max[j]) { max[j] = val ; }
            if (val < min[j]) { min[j] = val ; }

          fdbPrnTableItemDbl(fw,j+1, val);
        }
    }
  }

  fdbPrnTableItemStr(fw,0, _("Maximum") );
  for (j=0; j<type_len; j++)
    { fdbPrnTableItemDbl(fw,j+1, max[j]); }

  fdbPrnTableItemStr(fw,0, _("Minimum") );
  for (j=0; j<type_len; j++)
    { fdbPrnTableItemDbl(fw,j+1, min[j]); }

  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);

  if (max != NULL) { free(max); max = NULL; }
  if (min != NULL) { free(min); min = NULL; }
  if (set_id != NULL) { free(set_id); set_id = NULL; }

  return(rv);
}



/** Tests if element results are >=min and <=max
 * @param ePos element position
 * @param res_type type of result
 * @param min minimum
 * @param max maximum
 * @return status AF_YES (they are) or AF_NO (they aren't)
 */
int fdbTestResElemSolu(long e_pos, long res_type, double min, double max)
{
  long    i, k ;
  long    num, norep, rep;
  double  val ;
  long    epos ;
  long    etype ;
  long    etype_prog ;

  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_NO); }

	i = e_pos ;

  etype = fdbInputGetInt(ELEM, ELEM_TYPE, ResElem[ResActStep].i_pos[i]);

	if (fdbInputCountInt(ETYPE, ETYPE_ID, etype, &epos) < 1)
     { return (AF_NO); }

  etype_prog = fdbInputGetInt(ETYPE, ETYPE_TYPE, epos);

  norep = fdbElementType[etype_prog].res ;
  rep   = fdbElementType[etype_prog].res_rp ;

  num = ResElem[ResActStep].len[i] ;

  if (rep > 0) { num = 1 + (long) ( (num - norep) / rep ) ; }
  else         { num = 1 ; }

  for (k=0; k<num; k++)
  {
    if ( (norep <= 0) && (k == 0) ) { continue; }

    if (femCheckHaveResult(etype_prog, res_type) == AF_YES)
    {
      val = fdbResElemGetVal(i, etype_prog, res_type, k);

      if ((val >= min)&&(val <= max))
			{
				return(AF_YES);
			}
    }
  }

  return(AF_NO);
}

/** Select node by DOF result  */
int fdbTestResSelNodesDOF(long npos, long dof, double from, double to)
{
  double val ;

  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_NO); }
	if ((dof <= 0) || (dof > KNOWN_DOFS)) {return(AF_NO);}

  val = ResNode[ResActStep].data[(npos*KNOWN_DOFS) + (dof-1)];

	if ((from <= val)&&(val <= to))
	{
		return(AF_YES);
	}
	else
	{
		return(AF_NO);
	}
}

/** Computes average value from DOF results
 * @param dof DOF type (1=ux, 2=uy etc.)
 * @return average value
 */
double fdbGetAverDOF(long dof)
{
	long   i ;
	double sum = 0.0 ;
	long   dof0, nodepos ;
	long   len = 0 ;

	if ((ResLen <= 0) || (ResActStep >= ResLen)) 
	{ 
		return(0.0);
	}
	else
	{
		dof0 = dof - 1 ;
		if ((dof0 < 0)||(dof0 >= KNOWN_DOFS))
		{
			return(0.0);
		}
		sum = 0.0 ;
		len = 0 ;

		for (i=0; i<ResNode[ResActStep].i_len; i++)
  	{
    	nodepos = ResNode[ResActStep].i_pos[i] ;
	
    	if (fdbInputTestSelect(NODE, nodepos) != AF_YES) 
	  	{ 
      	continue ; 
    	}
    	else
    	{
      	sum += ResNode[ResActStep].data[ResNode[ResActStep].from[i]+dof0] ;
				len++ ;
    	}
  	}
		return(sum/(double)len) ;
	}
}

/** Computes sum of reactions
 * @param dof DOF type (1=ux, 2=uy etc.)
 * @return average value
 */
double fdbGetSumReact(long dof)
{
	long   i ;
	double sum = 0.0 ;
	long   nodepos ;

	if ((ResLen <= 0) || (ResActStep >= ResLen)) 
	{ 
		return(0.0);
	}
	else
	{
		if ((dof <= 0)||(dof > KNOWN_DOFS))
		{
			return(0.0);
		}

		sum = 0.0 ;

		for (i=0; i<ResReac[ResActStep].i_len; i++)
  	{
    	nodepos = ResReac[ResActStep].i_pos[i] ;
	
    	if (fdbInputTestSelect(NODE, nodepos) != AF_YES) 
	  	{ 
      	continue ; 
    	}
    	else
    	{
        if (ResReac[ResActStep].len[i] == (dof))
        {
		 			sum += ResReac[ResActStep].data[i] ;
        }
    	}
  	}

		return(sum);
	}
}

/** Returnes current "set_time" value (or zero on error)
 * @return set_time data
 */
double fdbGetSetTime(void)
{
	if ((ResLen <= 0) || (ResActStep >= ResLen)) 
	{ 
		return(0.0);
	}
	else
	{
    return(ResReac[ResActStep].set_time);
	}
}

/** Some auxiliary listings: ----------------------- */

/** Prints summary of result sets
 */
int fdbResListSets(int header, FILE *fw)
{	
	long i ;

	if (header == AF_YES)
	{
		fprintf(fw, "\n    %s (%li):\n\n",_("Available results sets"), ResLen);
	}

	fprintf(fw," %10s: %s\n", _("Identifier"), _("Time") ) ;
	for (i=0; i<ResLen; i++)
	{
		if (ResNode[i].empty != AF_YES)
		{
			fprintf(fw," %10li: %e\n", ResNode[i].set_id, ResNode[i].set_time) ;
		}
		else
		{
			fprintf(fw," %s.\n",_("Not available"));
		}
	}

	return(AF_OK);
}


/** Prints formatted summary of result sets
 */
int fdbResListSetsPrn(FILE *fw)
{	
	int rv = AF_OK ;
	long i ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 2, _("Available result sets")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Identifier")) ;
  fdbPrnTableHeaderItem(fw, 1, _("Time")) ;

	for (i=0; i<ResLen; i++)
	{
		if (ResNode[i].empty != AF_YES)
		{
			fdbPrnTableItemInt(fw,0, ResNode[i].set_id);
  		fdbPrnTableItemDbl(fw,1, ResNode[i].set_time);
		}
	}

  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);

	return(rv);
}

/** Returnes one particular reaction 
 * @param node id of the node with reaction
 * @param dof DOF type (1=ux, 2=uy etc.)
 * @return average value
 */
double fdbGetOneReact(long node, long dof)
{
	long   i ;
	double sum = 0.0 ;
	long   nodepos ;

	if ((ResLen <= 0) || (ResActStep >= ResLen)) 
	{ 
		return(0.0);
	}
	else
	{
		if ((dof <= 0)||(dof > KNOWN_DOFS))
		{
			return(0.0);
		}

		sum = 0.0 ;

		for (i=0; i<ResReac[ResActStep].i_len; i++)
  	{
    	nodepos = ResReac[ResActStep].i_pos[i] ;
      
      if (fdbInputTestSelect(NODE, nodepos) != AF_YES) 
	  	{ 
      	continue ; 
    	}

      if (node == fdbInputGetInt(NODE,NODE_ID, nodepos))
      {
        if (ResReac[ResActStep].len[i] == (dof))
        {
		 			return ( ResReac[ResActStep].data[i] ) ;
        }
      }
  	}

		return(0.0);
	}
}

/** Writes node result to stream 
 * @param fw stream to write data
 * @param n_id ID of node
 * @param dof_type type of result (degree of freedom)
 * @return status
 *
 * TODO: test and use it!
 */
int fdbGetLoopRawRes_DOF(FILE *fw, long n_id, long dof_type)
{
  long i,j ;
  long pos = -1 ;
  long ipos = -1 ;
	long act ;

  if ((dof_type > KNOWN_DOFS) ||(dof_type < 1))
  {
    fprintf(msgout,"[W] %s!\n",_("Requested data type is invalid"));
    return(AF_ERR_EMP);
  }

	act = ResActStep ;

  for (i=0; i<ResLen; i++)
  {
    if (ResNode[i].empty == AF_YES) {continue; }
    if (pos == -1)
    {
      for (j=0; j<ResNode[i].i_len; j++)
      {
        if (ResNode[i].i_id[j] == n_id)
        {
          pos = j ;
          break ;
        }
        if ((ipos=(ResNode[i].from[j]+dof_type-1)) > ResNode[i].data_len)
        {
          fprintf(msgout,"[W] %s!\n",_("Requested value not available"));
          return(AF_ERR_EMP);
        }
      }
      if (pos == -1)
      {
        fprintf(msgout,"[W] %s!\n",_("Requested node not found"));
        return(AF_ERR_EMP);
      }
    }

		ResActStep = i ;

    fprintf(fw, "%li %e, %e, %e\n",
        ResNode[i].set_id,
        ResNode[i].set_time,
				fdbGetSumReact(dof_type),
        ResNode[i].data[ipos]
        );
  }

	ResActStep = act ;

  return(AF_OK);
}

/* end of fdb_res.c */
