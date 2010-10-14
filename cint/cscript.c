/*
   File name: cscript.c
   Date:      2004/09/01 22:51
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

	 Simple command interpreter to be used in FEM or similar packages
	 Support for scripting.
   
	 $Id: cscript.c,v 1.1 2004/11/11 21:42:27 jirka Exp $
*/

#include "cint.h"

tScp  ciScript[CI_MAX_SCRIPTS] ;
long  ciScriptLen = 0 ;
long  ciActScriptID = -1 ;

/* prototype - this thing is called by another functions */
int ciRunScriptFor (tScp *scp, long beg, long end, long from, long to, long *skip_to);

/** Sets line of script to NULL
 * @param line line
 */
void ciNullScrLine(tScLn *line)
{
	line->cmd  = NULL ;
	line->var  = NULL ;
	line->type = 0 ;
	line->end  = (-1) ;
	line->from = 0 ;
	line->to   = 0 ;
}

/** Frees line of script
 * @param line line
 */
void ciFreeScrLine(tScLn *line)
{
	free(line->cmd);
	free(line->var);

	ciNullScrLine(line);
}

/** Sets script structure to NULL
 * @param scp script 
 */
void ciNullScript(tScp *scp)
{
	scp->name = NULL ;
	scp->line = NULL ;
	scp->len  = 0 ;
}

/** Frees script structure
 * @param scp script 
 * @return status
 */
int ciFreeScript(tScp *scp)
{
	long i ;

	free (scp->name) ;

	if (scp->len > 0)
	{
		for (i=0; i<scp->len; i++)
		{
			ciFreeScrLine(&scp->line[i]);
		}
	}

	free(scp->line);
	ciNullScript(scp);

	return(CI_OK);
}

/** Script field initialization
 * @param scp script structure
 * @param name script name (required, must be unique and LONGER than 1)
 * @return status
 */
int ciNewScript(tScp *scp, char *name)
{
	ciNullScript(scp);

	if (name == NULL) {return(CI_ERR);}
	if (strlen(name) < 1) {return(CI_ERR_LEN);}

	if ((scp->name=(char *)malloc(sizeof(char)*(strlen(name)+1)))==NULL)
	{
		ciNullScript(scp);
		return(AF_ERR);
	}
	else
	{
		strcpy(scp->name, name);
	}

  ciActScriptID = ciScriptLen ;
	ciScriptLen++ ;

	return(CI_OK);
}

/** Script field initialization (for script that is replaced)
 * @param scp script structure
 * @param name script name (required, must be unique and LONGER than 1)
 * @return status
 */
int ciModScript(tScp *scp, char *name, long pos)
{
  ciFreeScript(scp);

	if (name == NULL) {return(CI_ERR);}
	if (strlen(name) < 1) {return(CI_ERR_LEN);}

	if ((scp->name=(char *)malloc(sizeof(char)*(strlen(name)+1)))==NULL)
	{
		ciNullScript(scp);
		return(AF_ERR);
	}
	else
	{
		strcpy(scp->name, name);
	}

  ciActScriptID = pos ;

	return(CI_OK);
}


/** Adds new line to script
 * @param scp script structure
 * @param cmd line (command) to be added
 * @return status
 */
int ciAddScriptLine(tScp *scp, char *cmd)
{
	tScLn *tmp = NULL ;
	long   i,j ;

	if (cmd == NULL) {return(CI_ERR);}
	if (strlen(cmd) <1) {return(CI_ERR_LEN);}
	
	if (scp->len > 0)
	{
		if ((tmp = (tScLn *) malloc((scp->len)*sizeof(tScLn)))==NULL)
		   { return(CI_ERR); }

		tmp = scp->line ;
		scp->line = NULL ;
	}

	/* realloc scp->line ; fill it; .. */
	if ((scp->line = (tScLn *) malloc((scp->len+1)*sizeof(tScLn)))==NULL)
	{
		if (scp->len > 0)
		{
			scp->line = tmp ;
			tmp = NULL ;
		}
		return(CI_ERR);
	}

	if (scp->len > 0)
	{
		for (i=0; i<scp->len; i++)
		{
			if ((scp->line[i].cmd = (char *) malloc((strlen(tmp[i].cmd)+1)*sizeof(char)))==NULL)
		 	{
				for (j=0; j<i; j++) 
				{ 
					ciFreeScrLine(&scp->line[j]);
				}
				free(scp->line) ;
				scp->line = tmp ;
				tmp = NULL ;
				return(AF_ERR); 
			}
		}
	}

	/* copy the data back (hope this will work..) */
	if (scp->len > 0)
	{
		for (i=0; i<scp->len; i++) 
		{ 
			strcpy(scp->line[i].cmd, tmp[i].cmd); 
			free(tmp[i].cmd);
			tmp[i].cmd = NULL ;
			
			scp->line[i].var  =  NULL ;
			scp->line[i].type =  0 ;
			scp->line[i].end  = -1 ;
			scp->line[i].from =  0 ;
			scp->line[i].to   =  0 ;
		}
		free(tmp) ;
		tmp = NULL ;
	}

	/* fill last line */
	if ((scp->line[scp->len].cmd = (char *) malloc((strlen(cmd)+1)*sizeof(char)))==NULL)
		 { return(AF_ERR); }

	strcpy(scp->line[scp->len].cmd, cmd) ;
	scp->line[scp->len].var  = NULL ;
	scp->line[scp->len].type = 0 ;
	scp->line[scp->len].end  = 0 ;
	scp->line[scp->len].from = 0 ;
	scp->line[scp->len].to   = 0 ;
	scp->len++ ;

	return(CI_OK);
}

/** Preprocesses a script (check for "for", "if" etc.)
 * @param scp script structure
 * @return status
 */
int ciPreProcScript(tScp *scp)
{
  int rv = CI_OK ;
  long  i,j;
	long  fors     = 0 ;
	long  ifs      = 0 ;
	long  endfors  = 0 ;
	long  endifs   = 0 ;
	char *cmdstr   = NULL ;

	for (i=0; i<scp->len; i++)
	{
		if (ciParNum(scp->line[i].cmd) > 0)
		{
			 if ( ciGetParStrNoExpand(scp->line[i].cmd, 0) != NULL)
			 {
			   cmdstr = ciGetParStrNoExpand(scp->line[i].cmd, 0) ;
				 if (strlen(cmdstr)<1) { rv = CI_ERR ; goto memFree ; }
				 ciStrCompr(cmdstr);

				 if (strcmp(cmdstr,"for") == 0)
				 {
			 		 /* set "for" data here */

			     scp->line[i].type = 1 ;
			     scp->line[i].end = (-1) ; 

					 /* variable name should be set here  */
					 if (ciParNum(scp->line[i].cmd) > 1) 
					 {
			       if ( ciGetParStrNoExpand(scp->line[i].cmd, 1) != NULL)
			       {
				 			 if (strlen(ciGetParStrNoExpand(scp->line[i].cmd,1))<1) 
									{ rv = CI_ERR ; goto memFree ; }
							 else
							 {
								 scp->line[i].var = ciGetParStrNoExpand(scp->line[i].cmd,1);
							 }
						 }
					 }

					 if (ciParNum(scp->line[i].cmd) > 2) 
					 {
						 scp->line[i].from = ciGetParInt(scp->line[i].cmd,2);
					 }

					 if (ciParNum(scp->line[i].cmd) > 3) 
					 {
						 scp->line[i].to = ciGetParInt(scp->line[i].cmd,3);
					 }

					 fors++;
				 }
				 else
				 {
				   if (strcmp(cmdstr,"if") == 0)
				   {
					 		/* set "if" data here */

			         scp->line[i].type = 2 ;
			         scp->line[i].var  = NULL ;
			         scp->line[i].end = (-1) ; 
							ifs++;
				   }
					 else
					 {
				     if (strcmp(cmdstr,"endif") == 0)
				     {
			         scp->line[i].type = (-2) ; /* end of "if" */
			         scp->line[i].end = (-1) ; 
							 endifs++;
						 }
						 else
						 {
				       if (strcmp(cmdstr,"endfor") == 0)
				       {
			           scp->line[i].type = (-1) ; /* end of "for" */
			           scp->line[i].end = (-1) ; 
							   endfors++;
							 }
							 else
							 {
			           scp->line[i].type = 0 ; /* nothing exciting */
							 }
						 }
					 }
				 }

				 free(cmdstr); cmdstr = NULL ;
			 }
			 else { rv = CI_ERR ; goto memFree ; }
		}
		else
		{
			scp->line[i].type = (0) ; /* nothing exciting */
		}
	}

	if ( (ifs != endifs)||(fors != endfors) )
	   { rv = CI_ERR_LEN ; goto memFree; }

  /* now sets "for" and "if" positions (may cause infinite loop) */

	while (ifs > 0)
	{
		for (i=0; i<scp->len; i++)
		{
			if (scp->line[i].type == (2)) /* if */
			{
				if (scp->line[i].end == (-1)) /* still not set */
				{
					for (j=(i+1); j<scp->len; j++)
					{
						if ((scp->line[j].type == (2))&&(scp->line[j].end == (-1))) 
						{
						  break;
						}
						if ((scp->line[j].type == (-2))&&(scp->line[j].end == (-1)))
						{
						  scp->line[i].end = j ; /* if    */
						  scp->line[j].end = i ; /* endif */
							ifs-- ;
						  break;
						}
					}
				}
			}
		}
	}

	while (fors > 0)
  {
		for (i=0; i<scp->len; i++)
		{
			if (scp->line[i].type == (1)) /* for */
			{
				if (scp->line[i].end == -1) /* still not set */
				{
					for (j=(i+1); j<scp->len; j++)
					{
						if ((scp->line[j].type == (1))&&(scp->line[j].end == (-1))) 
						{
						  break;
						}
						if ((scp->line[j].type == (-1))&&(scp->line[j].end == (-1)))
						{
						  scp->line[i].end = j ; /* for    */
						  scp->line[j].end = i ; /* endfor */
							fors-- ;

/*printf("adding for: b=%li e=%li [it=%li jt=%li](%s %s)\n",i,j,scp->line[i].type,scp->line[j].type, scp->line[i].cmd,scp->line[j].cmd	);*/
							
						  break;
						}
					}
				}
			}
		}
	}

	/* final check */
	for (i=0; i<scp->len; i++)
	{

/*printf("for test: %li..%li %li..%li [%s]\n",i, scp->line[i].end, scp->line[i].from, scp->line[i].to,scp->line[i].cmd);*/

		if ((scp->line[i].type == 1)&&(scp->line[i].end == (-1)))
		   { rv = CI_ERR; goto memFree; }
		if ((scp->line[i].type == 2)&&(scp->line[i].end == (-1)))
		   { rv = CI_ERR; goto memFree; }

		if ((scp->line[i].type == (-1))&&(scp->line[i].end == (-1)))
		   { rv = CI_ERR; goto memFree; }
		if ((scp->line[i].type == (-2))&&(scp->line[i].end == (-1)))
		   { rv = CI_ERR; goto memFree; }
	}

	return(rv);
memFree:
	/* Destroys whole script: !!! */
	ciFreeScript(scp); 
	return(rv);
}



/** Runs one line of script
 * @param scp script structure
 * @param i line number
 * @param skip_to "go to" number
 * @return status
 */
int ciRunScriptLine(tScp *scp, long i, long *skip_to)
{
  int    rv   = CI_OK ;
	long   arg1 = 0 ;
	long   arg2 = 0 ;
	long   result = 0 ;
	char  *act  = NULL ;

	switch (scp->line[i].type)
  {
		case 2: /* if */
						arg1 = ciGetParInt(scp->line[i].cmd,1) ;
						arg2 = ciGetParInt(scp->line[i].cmd,3) ;
						act  = ciGetParStrNoExpand(scp->line[i].cmd,2) ;
						if (act == NULL)   {*skip_to = i ; return(CI_ERR);}
						if (strlen(act)<1) {*skip_to = i ; return(CI_ERR_LEN);}
						ciStrCompr(act);
						if (strlen(act)<1) {*skip_to = i ; return(CI_ERR_LEN);}

						result = 0 ;

						if (strcmp(act,"=")==0) { if (arg1 == arg2){result=1;} }
						if (strcmp(act,"==")==0) { if (arg1 == arg2){result=1;} }
						if (strcmp(act,">")==0) { if (arg1 >  arg2){result=1;} }
						if (strcmp(act,"<")==0) { if (arg1 <  arg2){result=1;} }
						if (strcmp(act,">=")==0){ if (arg1 >= arg2){result=1;} }
						if (strcmp(act,"<=")==0){ if (arg1 <= arg2){result=1;} }
						if (strcmp(act,"<>")==0){ if (arg1 != arg2){result=1;} }

            if (result == 1) { *skip_to = i ; }
						else             { *skip_to = scp->line[i].end ; }
						break ;

		case 1: /* for */
						rv=ciRunScriptFor(scp,i,scp->line[i].end,scp->line[i].from,scp->line[i].to, skip_to) ;
						*skip_to = scp->line[i].end ;
						break ;

		case 0: rv = ciRunCmd(scp->line[i].cmd) ;
						*skip_to = i ;
						break ;

		default: /* including ends: */
						*skip_to = i ;
						break ;
	}

  return(rv);
}


/** Runs "for" loop
 * @param scp script structure
 * @param beg 1st line to be executed (for,,)
 * @param end last line to be executed (endfor)
 * @param from starting number for loop (for i=from i<=to i++)
 * @param to end number for loop
 * @param skip_to "go to" number
 * @return status
 */
int ciRunScriptFor (tScp *scp, long beg, long end, long from, long to, long *skip_to)
{
  int  rv = CI_OK ;
	long i,j ;
	char varstr[CI_STR_LEN+1];

/*printf("VAR=%s beg=%li end=%li\n",scp->line[beg].var,beg,end);*/

	for (i=from; i<=to; i++)
	{
		/* count variable is updated  */
		for (j=0; j<CI_STR_LEN; j++) { varstr[i] = '\0' ; }
		sprintf(varstr,"%li",i);

		ciAddVarGrp(
		  scp->line[beg].var,
			varstr,
			ciStrCat2ParsNoCommas("script__",scp->name)
			) ;

		/* one run of loop: */
		for (j=(beg+1); j<end; j++)
		{
		  rv = ciRunScriptLine(scp, j, skip_to); 
			if (scp->line[j].type == 2) 
			{
				j = *skip_to ;
				if (j >= end) break ;
			}
#if 0
		  if (rv != CI_OK) {return(rv);}
#endif
		}
	}

	return(rv);
}

/** Runs script
 * @param scp script structure
 * @return status
 */
int ciRunScript (tScp *scp)
{
  int  rv = CI_OK ;
	long i ;
	long skip_to ;

	for (i=0; i<scp->len; i++)
	{
		rv = ciRunScriptLine(scp, i, &skip_to); 
		if (rv != CI_OK) {return(rv);}
		i = skip_to ;
		continue;
	}

	return(rv);
}

/* --------------------------------------------- */

/** Sets whole script field to NULL */ 
void ciNullScriptField(void)
{
	long i ;

	for (i=0; i<CI_MAX_SCRIPTS; i++) { ciNullScript(&ciScript[i]); }
	ciScriptLen = 0 ;
  ciActScriptID = -1 ;
}

/** Frees whole script field */ 
void ciFreeScriptField(void)
{
	long i ;

	for (i=0; i<CI_MAX_SCRIPTS; i++) { ciFreeScript(&ciScript[i]); }
  ciNullScriptField();
}

/** Adds new script to script field 
 * @param name name of script
 * @return status
 */
int ciAddScript(char *name)
{
	long i ;
  long pos   = 1 ;

	if (ciScriptLen >= CI_MAX_SCRIPTS) {return(CI_ERR_LEN);}

	if (name == NULL){return(CI_ERR);}
	ciStrCompr(name);

	for (i=0; i<ciScriptLen; i++)
	{
		if ((strcmp(name,ciScript[i].name)) == 0)  /* already exists */
    {
      pos = i ;
      /* free old one and replace with new: */
	    return(ciModScript(&ciScript[pos], name, pos));
    }
	}

	return(ciNewScript(&ciScript[ciScriptLen], name));
}

/** Adds new line (command) to the last script
 * @param name name of script
 * @return status
 */
int ciAddLineToActiveScript(char *cmd)
{
  if (ciActScriptID < 0) {return(AF_ERR_VAL);}
  return(ciAddScriptLine(&ciScript[ciActScriptID], cmd));
}

int ciPreProcActiveScript(void)
{
  if (ciActScriptID < 0) {return(AF_ERR_VAL);}
	return( ciPreProcScript(&ciScript[ciActScriptID]));
}

/* end of cscript.c */
