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
    return(AF_ERR_MEM); /* no memory por path !? */
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

/** TODO */
int resPathAddNode(int num, int node)
{
  /* TODO */
  return(AF_OK);
}


/* end of fdb_path.c */
