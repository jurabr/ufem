/*
   File name: fdb_k.c
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

	 FEM - keypoint handling
*/

#include "fdb_fem.h"

extern double fdbDistTol ; /* from fdb_nops.c*/

/** Creates new kpoint or changes existing
 * @param id number of kpoint (0 == new kpoint with defaukt number)
 * @param x x-coordinate
 * @param y y-coordinate
 * @param z z-coordinate
 * @param new_id new id number (for feedback reasons)
 * @return status
 */
int f_k_new_change(long id, double x, double y, double z, long *new_id)
{
	int  rv    = AF_OK ;
	int  nnew  = AF_NO ;
	long pos   = -1 ;
	long count = 0 ;
	long newid = 0 ;
  double n_x, n_y, n_z ;

	if (id <= 0) /* no ID specified => new */
	{
		newid =  fdbInputFindMaxInt(KPOINT, KPOINT_ID) + 1 ;
		nnew = AF_YES ;
	}
	else       /* ID specified */
	{
		count = fdbInputCountInt(KPOINT, KPOINT_ID, id, &pos);

		if (count > 0) /* existing data */
		{
			newid = fdbInputGetInt(KPOINT, KPOINT_ID, pos) ;
			nnew = AF_NO ;
		}
		else /* new data */
		{
			newid = id ; 
			nnew = AF_YES ;
		}
	}

  if (new_id != NULL) { *new_id = newid ; }

	/*printf("count is %li\n", count);*/

	if (nnew == AF_YES) /* adding of new line */
	{
    /*fdbInputUpdateKPointStats();*/

		if ((rv = fdbInputAppendTableRow(KPOINT, 1, &pos)) != AF_OK) 
		   {return(rv);}

		fdbInputIntChangeVals(KPOINT, KPOINT_ID, pos, 1, &newid, AF_NO);
	}
	else /* changing of existing */
	{
		/* do nothing */
	}

	/* common for all: */
  fdbCSysGetXYZ(x, y, z, &n_x, &n_y, &n_z);

	fdbInputDblChangeVals(KPOINT, KPOINT_X, pos, 1, &n_x, AF_NO);
	fdbInputDblChangeVals(KPOINT, KPOINT_Y, pos, 1, &n_y, AF_NO);
	fdbInputDblChangeVals(KPOINT, KPOINT_Z, pos, 1, &n_z, AF_NO);

#ifdef DEVEL_VERBOSE
	fprintf(msgout,"k(%li) = [%e, %e, %e] ##\n",
					fdbInputGetInt(KPOINT, KPOINT_ID, pos ),
					fdbInputGetDbl(KPOINT, KPOINT_X, pos ),
					fdbInputGetDbl(KPOINT, KPOINT_Y, pos ),
					fdbInputGetDbl(KPOINT, KPOINT_Z, pos )
					);
#endif

	return(rv) ;
}

/** Deletes kpoint
 * @param id number of kpoint
 * @return status
 */
int f_k_delete(long id)
{
	long pos   = -1 ;
	int  rv =  AF_OK ;

  /* tests: */
	if (fdbInputCountInt(KPOINT, KPOINT_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP); /* no kpoint found */
	}

	if (fdbInputTestDep(id, ENTKP, ENTKP_KP) == AF_YES){return(AF_ERR);}

  fdbInputUpdateKPointStats();

	/* removing */
	rv = fdbInputRemoveTableRow(KPOINT, pos, 1);

	fdbInputSyncStats();
	return( rv);
}

/** Lists one kpoint
 * @param id number of kpoint (0 == new kpoint with defaukt number)
 * @param header if AF_YES then header is printed
 * @param fw file for writing
 * @return status
 */
int f_k_list(long id, int header, FILE *fw)
{
	int  rv    = AF_OK ;
	long pos   = -1 ;
	long print_mode = 0 ;

	if (id <= 0) /* no ID specified */
	{
		return(AF_ERR_VAL) ;
	}

	if (fdbInputCountInt(KPOINT, KPOINT_ID, id, &pos) < 1)
	{
		return(AF_ERR_EMP) ;
	}

	if ( fdbTestSelect(&InputTab[KPOINT], &inputData.intfld, pos) != AF_YES) 
	{
		return(AF_OK); /* not selected */
	}

	if (header == AF_YES)
	{
		fprintf(fw, "\n          K          X            Y            Z\n");
	}

	rv = fdbInputPrintTabLine(fw, KPOINT, pos, print_mode);
	fprintf(fw, "\n");

	return(rv) ;
}

int f_k_list_prn(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long id ;
  long i ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 4, _("Key Points")) ;

  fdbPrnTableHeaderItem(fw, 0, _("Number")) ;
  fdbPrnTableHeaderItem(fw, 1, _("X")) ;
  fdbPrnTableHeaderItem(fw, 2, _("Y")) ;
  fdbPrnTableHeaderItem(fw, 3, _("Z")) ;

  for (i=0; i<fdbInputTabLenAll(KPOINT); i++)
  {
    if (fdbInputTestSelect(KPOINT, i) == AF_YES)
    {
      id = fdbInputGetInt(KPOINT,KPOINT_ID,i) ;
      if ( (id < from) || (id > to) ) {continue; }
      fdbPrnTableItemInt(fw,0, id);
      fdbPrnTableItemDbl(fw,1,fdbInputGetDbl(KPOINT,KPOINT_X,i));
      fdbPrnTableItemDbl(fw,2,fdbInputGetDbl(KPOINT,KPOINT_Y,i));
      fdbPrnTableItemDbl(fw,3,fdbInputGetDbl(KPOINT,KPOINT_Z,i));
    }
  }
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}


/** Make 2 KPs from one and link them with separate GEs
 * Note: it assimes one unique KP id per entity!
 * @param kPos position of keypoint
 * @return status
 */
int f_k_split_through_ge_pos(long kPos)
{
  int rv = AF_OK ;
  long i ;
  long id = -1 ;
  long pos = -1 ;
  long num = 0 ;
  long newid, pos1 ;
  double x, y, z ;

  id = fdbInputGetInt(KPOINT, KPOINT_ID, kPos);

	if ( fdbTestSelect(&InputTab[KPOINT], &inputData.intfld, kPos) != AF_YES) 
	{
		return(AF_OK); /* not selected - nothing to do */
	}

  /* number of uses of the KP: */
	if ((num=fdbInputCountInt(ENTKP, ENTKP_KP, id, &pos)) < 2)
  {
    return(AF_OK); /* nothing to do here */
  }
  else
  {
    /* have to make some elements: */
    x = fdbInputGetDbl(KPOINT, KPOINT_X, kPos);
    y = fdbInputGetDbl(KPOINT, KPOINT_Y, kPos);
    z = fdbInputGetDbl(KPOINT, KPOINT_Z, kPos);

    for (i=1; i<num; i++)
    {
      newid = 0 ;
      if ((rv=f_k_new_change(0, x, y, z, &newid)) != AF_OK)
      {
        return(rv);
      }
      if (newid <= 0)
      {
        return(AF_ERR);
      }

      pos1 = fdbInputFindNextInt(ENTKP, ENTKP_KP, pos, id); ;
      if (pos1 > pos)
      {
        fdbInputPutInt(ENTKP,ENTKP_KP,pos1, newid);
      }
      pos = pos1 ;
    }
  }

  fdbInputUpdateKPointStats();
  
  return(rv);
}

/** Make 2 KPs from one and link them with separate GEs
 * Note: it assimes one unique KP id per entity!
 * @param is number (ID) of the keypoint
 * @return status
 */
int f_k_split_through_ge_id(long id)
{
  long kPos ;
  if (fdbInputCountInt(KPOINT, KPOINT_ID, id, &kPos) > 0)
  {
    return( f_k_split_through_ge_pos(kPos) );
  }
  else
  {
    return(AF_ERR_VAL);
  }
  return(AF_ERR);
}

/** Join (merge) keypoints
 * @return status
 */
int f_k_join_all(void)
{
  int rv = AF_OK ;
  long i, j, k ;
  long pos = -1 ;
  long id ,id_r, num ;
  double x, y, z ;
  double nx, ny, nz ;
  double ndist ;
  long count = 0 ;

  for (i=0; i<fdbInputTabLenAll(KPOINT); i++)
  {
	  if (fdbInputTestSelect(KPOINT,i) != AF_YES) {continue;} 

    id  = fdbInputGetInt(KPOINT, KPOINT_ID, i);

    x = fdbInputGetDbl(KPOINT, KPOINT_X, i);
    y = fdbInputGetDbl(KPOINT, KPOINT_Y, i);
    z = fdbInputGetDbl(KPOINT, KPOINT_Z, i);

    for (j=0; j<fdbInputTabLenAll(KPOINT); j++)
    {
/*printf("nmerge: %li,%li\n",i,j);*/
      if  (i == j) {continue;}
	    if (fdbInputTestSelect(KPOINT,j) != AF_YES) {continue;} 

      id_r  = fdbInputGetInt(KPOINT, KPOINT_ID, j);
    
		  nx = fdbInputGetDbl(KPOINT, KPOINT_X, j) ;
		  ny = fdbInputGetDbl(KPOINT, KPOINT_Y, j) ;
		  nz = fdbInputGetDbl(KPOINT, KPOINT_Z, j) ;

		  ndist = sqrt ( (x-nx)*(x-nx) + (y-ny)*(y-ny) + (z-nz)*(z-nz) ) ;
      
      if (ndist >= fabs(fdbDistTol)) { continue ; }

      pos = 0 ;

	    if ((num=fdbInputCountInt(ENTKP, ENTKP_KP, id_r, &pos)) > 0)
      {
        for (k=0; k<num; k++)
        {
          fdbInputPutInt(ENTKP, ENTKP_KP, pos, id);
          fdbInputPutInt(ENTKP, ENTKP_POS, pos, i);

	        if ((fdbInputCountInt(ENTKP, ENTKP_KP, id_r, &pos)) < 1)
          {
            break ;
          }
        }
      }

      if (f_k_delete(id_r) == AF_OK)  /* delete node "j" */
      {
        count++;
        j-- ;
      }
      else
      {
#ifdef RUN_VERBOSE
        fprintf(msgout,"[W] %s: %li!\n",_("KP can NOT be deleted"),id_r);
#endif
      }
      fdbInputUpdateNodeStats(); /* fix dependencies */
    }
  }

  fprintf(msgout,"[ ]    %s: %li.\n",_("Number of removed keypoints"), count);
  return(rv);
}

/** Computes distance between two keypoints
 * @param k1 first keypoint
 * @param k2 second keypoint
 * @param dx,dy,dz distances in x,y,z directions (results)
 * @return status
 */
int f_k_dist(long k1, long k2, double *dx, double *dy, double *dz)
{
  long pos1, pos2;

  *dx = 0 ;
  *dy = 0 ;
  *dz = 0 ;

  if (fdbInputCountInt(KPOINT, KPOINT_ID, k1, &pos1) < 1) {return(AF_ERR_EMP);}
  if (fdbInputCountInt(KPOINT, KPOINT_ID, k2, &pos2) < 1) {return(AF_ERR_EMP);}

  *dx = (fdbInputGetDbl(KPOINT, KPOINT_X, pos2) - fdbInputGetDbl(KPOINT, KPOINT_X, pos1)) ;
  *dy = (fdbInputGetDbl(KPOINT, KPOINT_Y, pos2) - fdbInputGetDbl(KPOINT, KPOINT_Y, pos1)) ;
  *dz = (fdbInputGetDbl(KPOINT, KPOINT_Z, pos2) - fdbInputGetDbl(KPOINT, KPOINT_Z, pos1)) ;
  
  return(AF_OK);
}

/* end of fdb_k.c */
