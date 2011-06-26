/*
   File name: fdb_thst.c
   Date:      2011/06/25 18:30
   Author:    Jiri Brozovsky

   Copyright (C) 2011 Jiri Brozovsky

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

   Transfer of data from thermal to structural solution
*/

#include "fdb_fem.h"
#include "fdb_res.h"

/** Makes structural model from thermal
 * @return status
 */
int fdbFromThermalToStruct(void)
{
  int     rv         = AF_OK ;
  long    len, i, nodepos ;
  long   *tmp_node   = NULL ;
  double *tmp_therm  = NULL ;
  double  tmp_len    = 0 ;

  /* find result: */
  tmp_len = ResNode[ResActStep].i_len ;

  if (tmp_len > 0)
  {
    if ((tmp_node  = femIntAlloc(tmp_len)) == NULL) {rv = AF_ERR_MEM; goto memFree;}
    if ((tmp_therm = femDblAlloc(tmp_len)) == NULL) {rv = AF_ERR_MEM; goto memFree;}

    for (i=0; i<tmp_len; i++)
    {
      nodepos = ResNode[ResActStep].i_pos[i] ;
      tmp_node[i] = fdbInputGetInt(NODE,NODE_ID,nodepos) ;
      tmp_therm[i] = ResNode[ResActStep].data[ResNode[ResActStep].from[i]+(6)] ; /* 6 = KNOWN_DOFS-1 */
    }
  }

	func_fem_prep(NULL) ;

  /* delete non-compatible stuff: */
  len = fdbInputTabLenAll(NDISP);
  fdbInputRemoveTableRow(NDISP, 0, len);

  len = fdbInputTabLenAll(NLOAD);
  fdbInputRemoveTableRow(NLOAD, 0, len);

  len = fdbInputTabLenAll(ELOAD);
  fdbInputRemoveTableRow(ELOAD, 0, len);

  len = fdbInputTabLenAll(ELVAL);
  fdbInputRemoveTableRow(ELVAL, 0, len);

  /* switch element types and real sets */

  len = fdbInputTabLenAll(ETYPE);
  for (i=0; i< len; i++)
  {
    if (fdbInputGetInt(ETYPE, ETYPE_TYPE, i) == 18)
       { fdbInputPutInt(ETYPE, ETYPE_TYPE, i, 11) ; }

    if (fdbInputGetInt(ETYPE, ETYPE_TYPE, i) == 19)
       { fdbInputPutInt(ETYPE, ETYPE_TYPE, i, 4) ; }
  }

  len = fdbInputTabLenAll(RSET);
  for (i=0; i< len; i++)
  {
    if (fdbInputGetInt(RSET, RSET_TYPE, i) == 18)
       { fdbInputPutInt(RSET, RSET_TYPE, i, 11) ; }

    if (fdbInputGetInt(RSET, RSET_TYPE, i) == 19)
       { fdbInputPutInt(RSET, RSET_TYPE, i, 4) ; }
  }

  /* add therm loads to nodes: */
  for (i=0; i<tmp_len; i++)
      { f_nd_new_change(0, tmp_node[i], 7, 0, tmp_therm[i]) ; }

memFree:
  if (tmp_len > 0)
  {
    femIntFree(tmp_node);
    femDblFree(tmp_therm);
    tmp_len = 0 ;
  }

  return(rv);
}


/* end of fdb_thst.c */
