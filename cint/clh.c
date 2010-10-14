/*
   File name: clh.c
   Date:      2007/05/25 21:20
   Author:    Jiri Brozovsky

   Copyright (C) 2007 Jiri Brozovsky

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

   Simple command interpreter: command line history
*/

#include "cint.h"

char **ciHistory     = NULL ;
long   ciHistorySize = 0 ;

/* Erases ciHistory data */
void ciCmdHsFree(void)
{
  long i ;

  if (ciHistory != NULL)
  {
    for (i=0; i<ciHistorySize; i++)
    {
      if (ciHistory[i] != NULL) 
      {
        free(ciHistory[i]);
        ciHistory[i] = NULL ;
      }
    }
    free(ciHistory);
    ciHistory = NULL ;
  }

  ciHistorySize = 0 ;
}

/** Allocates ciHistory field
 * @param size number of commands in a history
 * @return status
 */
int ciCmdHsAlloc(long size)
{
  long i,j ;

  ciHistorySize = 0 ;

  if ((ciHistory = (char **)malloc(sizeof(char *)*size)) == NULL)
  {
    return(AF_ERR_MEM);
  }

  for (i=0; i<size; i++)
  {
    if ((ciHistory[i] = (char *)malloc(sizeof(char)*(CI_STR_LEN+10))) == NULL)
    {
      for (j=0; j<i; j++)
      {
        free(ciHistory[j]); ciHistory[j] = NULL ;
      }
      free(ciHistory);
      ciHistory = NULL ;
      return(AF_ERR_MEM);
    }
    else
    {
      for (j=0; j<CI_STR_LEN+10; j++) { ciHistory[i][j]='\0'; }
    }
  }

  ciHistorySize = size ;

  return(AF_OK);
}

/** Adds command to history
 * @param str command to add
 * @return status
 */
int ciCmdHsAdd(char *str)
{
  long i;

  if (str != NULL)
  {
    if (
        (str[0] =='l')&&
        (str[1] =='a')&&
        (str[2] =='s')&&
        (str[3] =='t')
       )
    {
      return(AF_OK);
    }

    if (strlen(str) > 0)
    {
      if (
          (str[0] != '!')&&
          (str[0] != '#')&&
          (str[0] != '\0')
          )
      {
        if (str[strlen(str)-1] == '\n') {str[strlen(str)-1]='\0';} /* for command files */

        for (i=ciHistorySize-1; i>0; i--)
        {
          strncpy(ciHistory[i],ciHistory[i-1],CI_STR_LEN);
        }
        strncpy(ciHistory[0],str,CI_STR_LEN);
      }
    }

#if 0
printf("HHH %s\n",str);
        for (i=0; i<ciHistorySize; i++)
        {
          fprintf(msgout,"HS %4i: %s !!!\n",i,ciHistory[i]);
        }
#endif
  }
  
  return(AF_OK);
}

/** Returnes n-th command 
 * @param n number of command to return (1=last one)
 * @return the command (string)
 */
char *ciCmdHsGet(long n)
{
  if ((n >= 0) && (n < ciHistorySize))
  {
    if (
        (ciHistory[n-0][0] != '\0') &&
        (ciHistory[n-0][0] != '#') &&
        (ciHistory[n-0][0] != '!') 
        )
    {
      return(ciHistory[n-0]);
    }
  }
  /* if everything fails: */
  return(" ");
}

/* end of clh.c */
