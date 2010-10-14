/*
   File name: fem_solv.c
   Date:      2003/12/31 14:42
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

   FEM User Interface - solver calling routines

   $Id: fem_solv.c,v 1.3 2004/12/31 00:31:45 jirka Exp $
*/

#include "fem_tui.h"
#include "cint.h"

char *femSolverPath = NULL ;
char *femDataDir    = NULL ; /* directory for data */
char *femJobname    = NULL ; /* jobname            */


/** Sets data directory name
 * @param dir directory to be used
 * @return status
 */
int femSetDataDir(char *dir)
{
  int rv = AF_OK ;
  int i;

  if (dir == NULL) {return(AF_ERR_EMP); }

  if (femDataDir != NULL)
  {
    free(femDataDir) ;
    femDataDir = NULL ;
  }

  if ((femDataDir = (char *)malloc((strlen(dir)+1)*sizeof(char))) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Out of memory - cannot set directory name"));
    return(AF_ERR_MEM) ;
  }

  for (i=0; i<=strlen(dir); i++) { femDataDir[i] = '\0' ; }

  strcpy(femDataDir, dir);
  ciStrNoNewL(femDataDir) ;

  if (femDataDir[strlen(femDataDir)-1] == CI_SLASH)
  {
    femDataDir[strlen(femDataDir)-1] = '\0';
  }

  return(rv);
}

/** Sets jobname
 * @param dir name to be used
 * @return status
 */
int femSetJobname(char *dir)
{
  int rv = AF_OK ;
  int i;

  if (dir == NULL) {return(AF_ERR_EMP); }

  ciStrCompr(dir);
  if (strlen(dir) < 1) {return(AF_ERR_EMP); }

  if (femJobname != NULL)
  {
    free(femJobname) ;
    femJobname = NULL ;
  }

  if ((femJobname = (char *)malloc((strlen(dir)+1)*sizeof(char))) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Out of memory - cannot set name name"));
    return(AF_ERR_MEM) ;
  }

  for (i=0; i<=strlen(dir); i++) { femJobname[i] = '\0' ; }

  strcpy(femJobname, dir);

  return(rv);
}

/** Sets solver path
 * @param dir name to be used
 * @return status
 */
int femSetSolverPath(char *dir)
{
  int rv = AF_OK ;
  int i;

  if (dir == NULL) {return(AF_ERR_EMP); }

  if (femSolverPath != NULL)
  {
    free(femSolverPath) ;
    femSolverPath = NULL ;
  }

  if ((femSolverPath = (char *)malloc((strlen(dir)+1)*sizeof(char))) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Out of memory - cannot set name name"));
    return(AF_ERR_MEM) ;
  }

  for (i=0; i<=strlen(dir); i++) { femSolverPath[i] = '\0' ; }

  strcpy(femSolverPath, dir);
  ciStrNoNewL(femSolverPath) ;

  if (femSolverPath[strlen(femSolverPath)-1] == CI_SLASH)
  {
    femSolverPath[strlen(femSolverPath)-1] = '\0';
  }

  return(rv);
}

/** Runs solver (using "system" command)
 *@param opts options string (or NULL)
 *@param infile name of input file
 *@param outfile name of output file
 *@return status
 * */
int femRunSolver(char *opts, char *infile, char *outfile)
{
  int     rv  = AF_OK ;
  char   *tmp = NULL;
  long    len = 0 ;
  long    optlen = 0 ;
  long    i;

  if (femSolverPath == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Solver name must be defined first"));
    return(AF_ERR_EMP);
  }

  if ((infile == NULL) || (outfile == NULL))
  {
    fprintf(msgout,"[E] %s!\n", _("Solver parameters undefined"));
    return(AF_ERR_EMP);
  }

  if (opts != 0) { optlen = strlen(opts); }
  else           { optlen = 0 ;           }

  len = strlen(femSolverPath)+strlen(" ")+optlen+strlen(" -i ")+strlen(infile)+strlen(" -o ")+strlen(outfile);

  if ((tmp = (char *)malloc((len+1)*sizeof(char))) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("No memory to run solver"));
    return(AF_ERR_MEM) ;
  }

  for (i=0; i<=len; i++) { tmp[i] = '\0'; }

  strcpy(tmp,femSolverPath) ;
  strcat(tmp," ") ;
  if (opts != NULL) {strcat(tmp,opts) ;}
  strcat(tmp," -i ") ;
  strcat(tmp,infile) ;
  strcat(tmp," -o ") ;
  strcat(tmp,outfile) ;

  rv = system(tmp) ;

  free(tmp);
  tmp = NULL ;

  return(rv);
}

/** returnes jobname (without path) - existing or default */
char *femGetJobname(void)
{
  if (femJobname != NULL)
  {
    return(femJobname);
  }
  else
  {
    return("femfile");
  }
}

/** returnes data directory ("." if not set) */
char *femGetDataDir(void)
{
  if (femDataDir != NULL)
  {
    return(femDataDir);
  }
  else
  {
    return(".");
  }
}
/** returnes solver path ("." if not set) */
char *femGetSolverPath(void)
{
  if (femSolverPath != NULL)
  {
    return(femSolverPath);
  }
  else
  {
    return(".");
  }
}



/* end of fem_solv.c */
