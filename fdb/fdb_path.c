/*
   File name: fdb_path.c
   Date:      2010/10/17 18:52
   Author:    Jiri Brozovsky

   Copyright (C) 2010 Jiri Brozovsky

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

   This file defines "path operations": making paths for line graphs
*/

#include "fdb_fem.h"
#include "fdb_res.h"
#include "fdb_edef.h"
#include "cint.h"

/* Data for path: */
tResPath femPath[PATH_NUM] ;

long femActivePath = -1 ;

/** Sets given path to NULL 
 * @param num path index (0..PATH_NUM )
 * */
void resPathNull(int num)
{
  if (num >= PATH_NUM) {return;}
  femPath[num].len  = 0 ;
  femPath[num].node = NULL ;
}

/** Free given path 
 * @param num path index (0..PATH_NUM )
 */
void resPathFree(int num)
{
  int i ;
  if (num >= PATH_NUM) {return;}
  if (femPath[num].node != NULL) { free(femPath[num].node) ; }
  if (femPath[num].desc != NULL )
     { for (i=0; i<265; i++) {femPath[num].desc[i] = '\0' ;} }
  femPath[num].len  = 0 ;

  resPathNull(num);
}

/** Sets all paths to null */
void resPathNullAll(void)
{
  long i ;
  for (i=0; i<PATH_NUM; i++) { resPathNull(i); }
}

/** Frees all paths */
void resPathFreeAll(void)
{
  long i ;
  for (i=0; i<PATH_NUM; i++) { resPathFree(i); }
}

/** Initialises paths (makes them empty with -1 in .len) */
void femPathInit(void)
{
  int i, j ;

  for (i=0; i<PATH_NUM; i++)
  {
    femPath[i].len = -1 ;
    femPath[i].node = NULL ;
    for (j=0; j<265; j++) {femPath[i].desc[j] = '\0';} 
  }
}

/** Allocates path 
 * @param num path index (0..PATH_NUM )
 * @return status
 */
int resPathAlloc(int num)
{
  int i ;
  if (femPath[num].len > -1) { resPathFree(num) ; }

  if ((femPath[num].node = femIntAlloc(PATH_LEN)) == NULL)
  {
    femPath[num].len = -1 ;
    return(AF_ERR_MEM); /* no memory for path !? */
  }
  for (i=0; i<265; i++) {femPath[num].desc[i] = '\0' ;}
  femPath[num].len = 0 ;
  return(AF_OK);
}

/** Sets name for path
 * @param name name to be used
 * @param num path index (0..PATH_NUM )
 */
int resPathSetName(char *name, int num)
{
  long i, len ;

  if (num >= PATH_NUM) {return(AF_ERR_SIZ);}
  if ((len=strlen(name)) < 1) {return(AF_ERR_VAL);}

  for (i=0; i<256; i++) {femPath[num].desc[i] = '\0';}
  for (i=0; i<len; i++) {femPath[num].desc[i] = name[i];}

  strncpy(name, femPath[num].desc, 255);

  return(AF_OK);
}

/** Adda new node to path
 * @param num path number
 * @param node id number of node
 * @return status
 */
int resPathAddNode(int num, long node)
{
	long pos ;

  if (num >= PATH_NUM) {return(AF_ERR_SIZ);}
	if (femPath[num].len < 0) /* must be allocated first */
	   { if ( resPathAlloc(num) != AF_OK) { return(AF_ERR_MEM) ; } }

	/* check if node exists: */
	if (fdbInputCountInt(NODE, NODE_ID, node, &pos) < 1) { return(AF_ERR_VAL); }

	if (femPath[num].len >= (PATH_LEN-1)) {return(AF_ERR_SIZ);} /* path is already full */

	femPath[num].node[femPath[num].len] = node ;
	femPath[num].len++ ;

  return(AF_OK);
}

/** Replaces node in path
 * @param num path number
 * @param node_old id number of node to be replaced
 * @param node_new id number of replacement
 * @return status
 * */
int resPathChangeNode(int num, long node_old, long node_new)
{
	long pos, i ;

  if (num >= PATH_NUM)      {return(AF_ERR_SIZ);}
	if (femPath[num].len < 0) {return(AF_ERR_EMP);}

	/* check if node exists: */
	if (fdbInputCountInt(NODE, NODE_ID, node_new, &pos) < 1) { return(AF_ERR_VAL); }

	for (i=0; i<femPath[num].len; i++)
	{
		if (femPath[num].node[i] == node_old)
		{
			femPath[num].node[i] = node_new ;
			return(AF_OK);
		}
	}

  return(AF_ERR_VAL);
}

/** Deletes last node in path
 * @param num path number
 * @param node id number of node to be replaced
 * @return status
 * */
int resPathDelLastNode(int num, int node)
{
  if (num >= PATH_NUM)      {return(AF_ERR_SIZ);}
	if (femPath[num].len < 0) {return(AF_ERR_EMP);}

	femPath[num].node[femPath[num].len-1] = 0 ;
	femPath[num].len-- ;

  return(AF_OK);
}

/** Lists available paths
 * @param from starting position of path
 * @param to last positionof path
 * @return status
 */
int femPathList(FILE *fw, long from, long to)
{
  int  rv = AF_OK ;
  long i, j, ifrom, ito ;

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }

  fdbPrnBeginTable(fw, 2, _("Paths")) ;

	ifrom = from ;
	if (from < 0) {ifrom = 0 ;}
	if (from >= PATH_NUM) {ifrom = PATH_NUM-1 ;}
	ito = to ;
	if (to < ifrom){ito=ifrom;}
	if (to >= PATH_NUM) {to=PATH_NUM-1;}

  for (i=ifrom; i<=ito; i++)
  {
    if (femPath[i].len > 0)
    {
      fdbPrnTableItemStr(fw,0, femPath[i].desc);
      fdbPrnTableItemInt(fw,1, femPath[i].len);
    }
  }
  fdbPrnEndTable(fw) ;

  for (i=ifrom; i<=ito; i++) 
	{
    fdbPrnBeginTable(fw, 1, femPath[i].desc) ;
    if (femPath[i].len > 0)
		{
			for (j=0; j<femPath[i].len; j++)
			  { fdbPrnTableItemInt(fw,0, femPath[i].node[j]); }
		}
	}
  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);
  return(rv);
}

/** Prints results on path 
 * @param fw poiinter to file
 * @param path_num path number
 * @param res_type list of result types (numbers)
 * @param type_len number of result types (size of res_type)
 * */
int femPathResPrn(FILE *fw, long path_num, long *res_type, long type_len)
{
  int     rv  = AF_OK ;
  long    i, j, pos ;
	long   *set_id = NULL ;
  double  val ;

  if (path_num >= PATH_NUM)      {return(AF_ERR_SIZ);}
	if (femPath[path_num].len < 1) {return(AF_ERR_EMP);}

  if (res_type == NULL) {return(AF_ERR_EMP);}
  if (type_len < 1) 
  {
    fprintf(msgout,"[E] %s!\n", _(""));
    return(AF_ERR_EMP);
  }

  if ((ResLen <= 0) || (ResActStep >= ResLen)) { return(AF_ERR); }

  set_id = (long *)malloc(type_len*sizeof(long)) ;

	for (i=1; i<type_len; i++) 
  { 
    set_id[i] = ResElem[ResActStep].set_id ;
  }
	
	if ((rv=fdbAvResPrepareData(type_len, set_id, res_type)) != AF_OK)
  {
    return(rv) ;
  }

  if ((rv=fdbPrnOpenFile(fw)) != AF_OK)
  {
    return(rv);
  }


  fdbPrnBeginTable(fw, type_len+1, _("# Nodal Solution Results")) ;

  fdbPrnTableHeaderItem(fw, 0, _("# Node")) ;

  for (j=0; j<type_len; j++)
  {
    fdbPrnTableHeaderItem(fw, j+1, 
           ciGetVarNameFromGrp(
              fdbFemStrFromInt(res_type[j]), 
              "result")
        ) ;
  }

  for (i=0; i< femPath[path_num].len; i++)
  {
    fdbPrnTableItemInt(fw, 0, femPath[path_num].node[i]);
		if (fdbInputCountInt(NODE, NODE_ID, femPath[path_num].node[i], &pos) < 1) { continue; }

    for (j=0; j<type_len; j++)
    {
      val = fdbAvResGetVal(res_type[j], pos);
      fdbPrnTableItemDbl(fw,j+1, val);
    }
  }

  fdbPrnEndTable(fw) ;

  rv = fdbPrnCloseFile(fw);

  if (set_id != NULL) { free(set_id); set_id = NULL; }

  return(rv);
}

/* end of fdb_path.c */
