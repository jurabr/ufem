/*
   File name: fdb_path.c
   Date:      2010/10/17 18:52
   Author:    Jiri Brozovsky

   Copyright (C) 2009 Jiri Brozovsky

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

/* Data for path: */
tResPath femPath[PATH_NUM] ;

/** Sets given path to NULL 
 * @param num path index (0..PATH_NUM )
 * */
void resPathNull(int num)
{
  if (num >= PATH_NUM) {return;}
  femPath[num].len  = 0 ;
  femPath[num].node = NULL ;
  femPath[num].desc = NULL ;
}

/** Free given path 
 * @param num path index (0..PATH_NUM )
 * */
void resPathFree(int num)
{
  if (num >= PATH_NUM) {return;}
  if (femPath[num].node != NULL) { free(femPath[num].node) ; }
  if (femPath[num].desc != NULL ){ free(femPath[num].desc) ; }
  femPath[num].len  = 0 ;

  resPathNull(num);
}

/** Sets all paths to null */
void resPathNullAll(void)
{
  long i ;
  for (i=0; i<PATH_NUM; i++) { resPathNull(i); }
}

/** Frees all paths*/
void resPathFreeAll(void)
{
  long i ;
  for (i=0; i<PATH_NUM; i++) { resPathFree(i); }
}

/** Allocates path 
 * */
int resAllocPath(int num)
{
  /* TODO */
  return(AF_OK);
}


/** Defines new path */

/* end of fdb_path.c */
