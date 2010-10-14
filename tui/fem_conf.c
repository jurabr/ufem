/*
   File name: fem_conf.c
   Date:      2003/12/01 19:27
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

	 FEM - user interface - reading of configuration

	 $Id: fem_conf.c,v 1.3 2004/04/20 18:23:26 jirka Exp $
*/

#include <errno.h>
#include "fem_tui.h"
#include "cint.h"

extern char *fdbPrnViewCommand;
extern long  fdbPrnAutoName ;
extern long  fdbOutputFormat ;

char *femConfFileCL=NULL;

/** Reads config file. Do NOT use this to parse command files!
 *  (Note that code is simplified.)
 * @param fname name of file
 * @return status
 */
int femUIReadConfigFile(char *fname)
{
  int     rv   = AF_OK ;
  FILE   *fw   = NULL ;
  char    str[FEM_STR_LEN+1] ;
  long    str_len = FEM_STR_LEN ;
  long    i ;

  errno = 0 ;

  if ((fw = fopen(fname,"r")) == NULL)
  {
      return(AF_ERR_IO);
  }

  for (i=0; i<=FEM_STR_LEN; i++) { str[i] = '\0' ; }

  while (fgets(str,str_len,fw) != NULL)
  {
    if ((strcmp(str,"") == 0)||(strcmp(str,"\n") == 0)) {continue;}

    if (ciRunCmd(str) != AF_OK)
    {
      rv = AF_ERR ;
      break ;
    }

    for (i=0; i<=FEM_STR_LEN; i++) { str[i] = '\0' ; }

  }

  if (fclose(fw) != 0) {return(AF_ERR_IO);}

  return(rv);
}

/** Tries to open some config files and read them  */
int fem_get_ui_config(void)
{
  int     rv = AF_OK ;
#ifdef HAVE_HOME_DIR
  char    str[FEM_STR_LEN+1] ;
#endif

  fprintf(msgout,"\n[I] %s:\n", _("Configuration"));

    /* system/default config file: */
#ifdef FEM_SETUP_FILE
  fprintf(msgout,"[ ] %s \"%s\":\n", _("Getting config from"), FEM_SETUP_FILE);
  if (femUIReadConfigFile(FEM_SETUP_FILE) != AF_OK)
  {
    fprintf(msgout, "[E] %s!\n", _("Reading of file failed") );
  }
#endif

  /* file defined in environment variable */
  if (getenv("FEM_SETUP_FILE") != NULL)
  {
    fprintf(msgout,"[ ] %s \"%s\":\n", _("Getting config from"), getenv("FEM_SETUP_FILE"));
    if (femUIReadConfigFile(getenv("FEM_SETUP_FILE")) != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Reading of file failed") );
    }
  }

  /* file defined in home directory */
#ifdef HAVE_HOME_DIR
  if (getenv("HOME") != NULL)
  {
    if (strlen(getenv("HOME")) > (FEM_STR_LEN-1))
    {
      fprintf(msgout,"[E] %s!\n", _("Path to home directory is too long"));
    }
    else
    {
      strncpy(str, getenv("HOME"), FEM_STR_LEN) ;
      strncat(str, "/.tgfemrc", FEM_STR_LEN) ;

      fprintf(msgout,"[ ] %s \"%s\":\n", _("Getting config from"), str );
      if (femUIReadConfigFile(str) != AF_OK)
      {
        fprintf(msgout, "[ ] %s.\n", _("You have no personal configuration"));
      }
    }
  }
#endif

  /* file defined by command line parameter */
	if (femConfFileCL != NULL)
  {
    fprintf(msgout,"[ ] %s \"%s\":\n", _("Getting config from"), femConfFileCL);
    if (femUIReadConfigFile(femConfFileCL) != AF_OK)
    {
      fprintf(msgout, "[E] %s!\n", _("Reading of file failed") );
    }
		free(femConfFileCL); femConfFileCL = NULL ;
  }

  
	/* all done */
  fprintf(msgout,"[i] %s.\n", _("Configuration finished"));

  errno = 0 ;
  return(rv);
}

/** Writes basic config file named "fname"
 * @param fname name of file
 * @return status
 */
int femUIWriteConfigFile(char *fname)
{
  int     rv   = AF_OK ;
  FILE   *fw   = NULL ;

  errno = 0 ;

  if ((fw = fopen(fname,"w")) == NULL) { return(AF_ERR_IO); }

	fprintf(fw,"!* Machine-generated setup file for uFEM GUI\n");
	fprintf(fw,"\n!* Basic settings:\n");

  fprintf(fw, "datadir,%s\n", femGetDataDir() );
  fprintf(fw, "setsolver,%s\n", femGetSolverPath() );

	fprintf(fw,"\n!* Text output settings:\n");

	if (fdbPrnViewCommand != NULL)
	{
  	fprintf(fw, "outview,%s\n", fdbPrnViewCommand );
	}
  fprintf(fw, "outauto,%li !* if\n", fdbPrnAutoName );
  fprintf(fw, "outform,%li\n", fdbOutputFormat );

  if (fclose(fw) != 0) {return(AF_ERR_IO);}

  return(rv);
}

/* end of fem_tui.c */
