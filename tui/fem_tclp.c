/*
   File name: fem_tclp.c
   Date:      2003/08/24 11:44
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

	 FEM User Interface - command line parameters

	 $Id: fem_tclp.c,v 1.8 2004/11/11 21:41:36 jirka Exp $
*/

#include "fem_tui.h"
#include "cint.h"

extern char *femConfFileCL;
extern int   fdbNoMPIeRes ;


int femCleanGUIbutt     = AF_NO ;
int femCleanGUItree     = AF_NO ;
int femGUIlearningTree  = AF_NO ;
int femCmdDoNotBreak    = AF_YES ;
int femCmdNumProcSolver = 1 ;     /* number of processes for solver */
int femCmdAutoResu      = AF_NO ; /* automatically resume after start */

/** Prints simple help to stdout
 * @param argc the same as "argc" from main
 * @param argv the same as "argv" from main
 */
void tui_help(int argc, char *argv[])
{
#ifdef RUN_VERBOSE
	fprintf(msgout,"\n%s: %s\n\n", argv[0], _("a user interface of the uFEM code"));
	fprintf(msgout,"%s: %s [%s]\n\n %s:\n", _(" Usage"), argv[0], _("arguments"),
	  _("Arguments"));
	fprintf(msgout,"   -c  FILE ... %s\n", _("read configuration from FILE"));

	fprintf(msgout,"   -d  DIR  ... %s\n", _("set working directory to DIR"));
	fprintf(msgout,"   -j  NAME ... %s\n", _("start with jobname NAME"));
	fprintf(msgout,"   -rs      ... %s\n", _("resume database at startup"));

	fprintf(msgout,"   -nt NUM  ... %s\n", _("allow use of NUM processes in user interface"));
	fprintf(msgout,"   -np NUM  ... %s\n", _("allow use of NUM processes in solve"));
	fprintf(msgout,"   -l  FILE ... %s\n", _("log commands to FILE (rewrite it)"));
	fprintf(msgout,"   -la FILE ... %s\n", _("log commands to FILE (append it)"));
	fprintf(msgout,"   -e       ... %s\n", _("send all messages to stderr"));
	fprintf(msgout,"   -b       ... %s\n", _("manipulation buttons off (for GUI only)"));
	fprintf(msgout,"   -r       ... %s\n", _("stop command scrips on errors"));
	fprintf(msgout,"   -t       ... %s\n", _("command tree off (for GUI only)"));
	fprintf(msgout,"   -h       ... %s\n", _("print this help and exit"));
#endif
}


/** Parses command line parameteres and sets some variables
 * @param argc the same as "argc" from main
 * @param argv the same as "argv" from main
 * @return state value (AF_OK)
 */
int fem_parse_params(int argc, char *argv[])
{
	int rv = AF_OK;
	int i,j;


	for (i=1; i<argc; i++)
	{
		/* send messages to stderr */
		if (strcmp(argv[i],"-e") == 0)
		{
#ifdef RUN_VERBOSE
			msgout = stderr ;
#endif
		}
		/* logging */
		if (strcmp(argv[i],"-l") == 0)
		{
			if (ciUseLog == 0)
			{
				if (argc <= i)
				{
					fprintf(msgout, "[E] %s!\n", _("Log file name required"));
					exit(AF_ERR_VAL);
				}
				if ((ciLogFile = fopen(argv[i+1],"w")) != NULL)
				{
					ciUseLog = 1 ;
          i++;
				}
				else
				{
					fprintf(msgout, "[E] %s!\n", _("Cannot open log file"));
					exit(AF_ERR_IO);
				}
			}
		}
		
		if (strcmp(argv[i],"-la") == 0)
		{
			if (ciUseLog == 0)
			{
				if (argc <= i)
				{
					fprintf(msgout, "[E] %s!\n", _("Log file name required"));
					exit(AF_ERR_VAL);
				}
				if ((ciLogFile = fopen(argv[i+1],"a")) != NULL)
				{
					ciUseLog = 1 ;
          i++;
				}
				else
				{
					fprintf(msgout, "[E] %s!\n", _("Cannot open log file"));
					exit(AF_ERR_IO);
				}
			}
		}
		
		/* config file  */
		if (strcmp(argv[i],"-c") == 0)
		{
			if (argc <= i)
			{
				fprintf(msgout, "[E] %s!\n", _("Configuration file name required"));
				exit(AF_ERR_VAL);
			}
			else
			{
				if ((femConfFileCL = (char *)malloc(sizeof(char)*(strlen(argv[i+1])+1))) == NULL)
				{
					fprintf(msgout, "[E] %s!\n", _("Cannot allocate space for configuration file name"));
					exit(AF_ERR_VAL);
				}
				else
				{
					for (j=0; j<(strlen(argv[i+1])+1); j++) { femConfFileCL[j] = '\0'; }
					strcpy(femConfFileCL,argv[i+1]);
				}
			}
		}

    /* jobname  */
		if (strcmp(argv[i],"-j") == 0)
		{
    	if (argc < (i+2)) 
			{
				fprintf(msgout, "[E] %s!\n", _("Jobname must be specified"));
				exit(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
				  fprintf(msgout, "[E] %s!\n", _("Jobname must be specified"));
					exit(AF_ERR_VAL);
				}

        if (femSetJobname(argv[i+1]) != AF_OK)
        {
				  fprintf(msgout, "[E] %s!\n", _("Invalid jobname"));
					exit(AF_ERR_VAL);
        }
			}
		}

    /* data directory  */
		if (strcmp(argv[i],"-d") == 0)
		{
    	if (argc < (i+2)) 
			{
				fprintf(msgout, "[E] %s!\n", _("Directory name must be specified"));
				exit(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
				  fprintf(msgout, "[E] %s!\n", _("Directory name must be specified"));
					exit(AF_ERR_VAL);
				}

        if (femSetDataDir(argv[i+1]) != AF_OK)
        {
				  fprintf(msgout, "[E] %s!\n", _("Invalid directory name"));
					exit(AF_ERR_VAL);
        }
			}
		}

    /* number of processes in tgfem */
		if (strcmp(argv[i],"-nt") == 0)
		{
    	if (argc < (i+2)) 
			{
				fprintf(msgout, "[E] %s!\n", _("Number of processes is required"));
				exit(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
				  fprintf(msgout, "[E] %s!\n", _("Number of processes is required"));
					exit(AF_ERR_VAL);
				}

        if (atoi(argv[i+1]) < 1)
        {
				  fprintf(msgout, "[E] %s!\n", _("Invalid number of processes (must be greater than 0)"));
					exit(AF_ERR_VAL);
        }
        else
        {
          femCmdNumProcUI = atoi(argv[i+1]);
        }
			}
		}

    /* number of processes in fem solver */
		if (strcmp(argv[i],"-np") == 0)
		{
    	if (argc < (i+2)) 
			{
				fprintf(msgout, "[E] %s!\n", _("Number of processes is required"));
				exit(AF_ERR_SIZ);
			}
			else
			{
				if (argv[i+1][0] == '-')
				{
				  fprintf(msgout, "[E] %s!\n", _("Number of processes is required"));
					exit(AF_ERR_VAL);
				}

        if (atoi(argv[i+1]) < 1)
        {
				  fprintf(msgout, "[E] %s!\n", _("Invalid number of processes (must be greater than 0)"));
					exit(AF_ERR_VAL);
        }
        else
        {
          femCmdNumProcSolver = atoi(argv[i+1]);
        }
			}
		}
    
    /* no buttons in GUI */
		if (strcmp(argv[i],"-rs") == 0)
		{
			femCmdAutoResu = AF_YES ;
		}


		
    /* no buttons in GUI */
		if (strcmp(argv[i],"-b") == 0)
		{
			femCleanGUIbutt = AF_YES ;
		}

    /* no command tree in GUI */
		if (strcmp(argv[i],"-t") == 0)
		{
			femCleanGUItree = AF_YES ;
		}

		/* break command files on errors */
		if (strcmp(argv[i],"-r") == 0)
		{
			femCmdDoNotBreak = AF_NO ;
		}

    /* show commands in command tree */
		if (strcmp(argv[i],"-tc") == 0)
		{
			femGUIlearningTree = AF_YES ;
		}


    /* ignore element results from MPI */
		if (strcmp(argv[i],"--no-mpi-eres") == 0)
		{
			fdbNoMPIeRes = AF_YES ;
		}

		/* help line */
		if (strcmp(argv[i],"-h") == 0)
		{
			tui_help(argc, argv);
			exit(AF_OK);
		}
	}

	return(rv);
}

/* end of fem_tclp.c */
