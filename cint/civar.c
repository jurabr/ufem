/*
   File name: civar.c
   Date:      2003/11/18 14:22
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

	 Simple command interpreter to be used in FEM or similar packages
	 Variables and (maybe) history
   
	 $Id: civar.c,v 1.9 2004/11/11 21:40:26 jirka Exp $
*/

#include "cint.h"

tVar *ciVariable    = NULL ;
long  ciVariableLen = 0 ;

/** Adds variable or rewrites existing
 * @param name name of variable
 * @param value value of variable
 * @param group variable's group (somethng like comment ;-)
 * @return status
 */
int ciAddVarGrp(char *name, char *value, char *group)
{
	int  new = 1 ;
  long pos = 0 ;
	long i ;
	char *str_name  = NULL ;
	tVar *var = NULL ;

	var = NULL ;

	if ((strlen(name) <= 0)||(strlen(value) <= 0)) 
	   { return(CI_ERR_LEN) ; }
		
	if ((str_name = (char *) malloc((strlen(name)+1)*sizeof(char))) == NULL)
	   { return(CI_ERR) ; }

	for (i=0; i<=strlen(name); i++) { str_name[i] = '\0'; }

	strcpy(str_name, name) ;
	ciStrCompr(str_name);

	if (ciVariableLen > 0)
	{
		for (i=0; i<ciVariableLen; i++)
		{
			if (strcmp(str_name, ciVariable[i].name) == 0)
			{
				new = 0 ;
				pos = i ;
				break ;
			}
		}
	}

	free(str_name);

  if (new == 0)
  {
	  if ((str_name = (char *) malloc((strlen(group)+1)*sizeof(char))) == NULL)
	    { return(CI_ERR) ; }

	  for (i=0; i<=strlen(group); i++) { str_name[i] = '\0'; }

	  strcpy(str_name, group) ;
	  ciStrCompr(str_name);

	  if (strcmp(str_name, ciVariable[pos].group) != 0)
	  {
		  new = 1 ;
    }

	  free(str_name);
  }

	if (new == 1)
	{
		if ((var = (tVar *) malloc((ciVariableLen+1)*sizeof(tVar))) == NULL)
	   		{ return(CI_ERR) ; }

		pos = ciVariableLen ;

		for (i=0; i<ciVariableLen; i++) { var[i] = ciVariable[i] ; }

		free(ciVariable);
		ciVariable = var ;
		var = NULL ;
		
		if ((ciVariable[ciVariableLen].name = (char *) malloc((strlen(name)+1)*sizeof(char))) == NULL)
		 	{ return(CI_ERR) ; }
  	if ((ciVariable[ciVariableLen].value = (char *) malloc((strlen(value)+1)*sizeof(char))) == NULL)
	   	{ return(CI_ERR) ; }
  	if ((ciVariable[ciVariableLen].group = (char *) malloc((strlen(group)+1)*sizeof(char))) == NULL)
	   	{ return(CI_ERR) ; }

  	ciVariable[ciVariableLen].name[strlen(name)] = '\0' ;
  	ciVariable[ciVariableLen].value[strlen(value)] = '\0' ;
  	ciVariable[ciVariableLen].group[strlen(group)] = '\0' ;

		strcpy(ciVariable[ciVariableLen].name, name) ;
		strcpy(ciVariable[ciVariableLen].value, value) ;
		strcpy(ciVariable[ciVariableLen].group, group) ;


  	ciStrCompr(ciVariable[ciVariableLen].name);
  	ciStrCompr(ciVariable[ciVariableLen].group);

		ciVariableLen++;
	}
	else
	{
		free(ciVariable[pos].value) ; 
		ciVariable[pos].value = NULL ;
  	if ((ciVariable[pos].value = (char *) malloc((strlen(value)+1)*sizeof(char))) == NULL)
	   	{ return(CI_ERR) ; }
		strcpy(ciVariable[pos].value, value) ;
	}
	return(AF_OK) ;
}


/** Adds variable or rewrites existing
 * @param name name of variable
 * @param value value of variable
 * @return status
 */
int ciAddVar(char *name, char *value)
{
  return(ciAddVarGrp(name,value,"global")) ;
}

/** Deletes variable
 * @param name name of variable
 * @return status
 */
int ciDelVar(char *name)
{
	int  found = 0 ;
  long pos = 0 ;
	long i ;
	char *str_name  = NULL ;
	tVar *var = NULL ;

	if ((strlen(name) <= 0)) 
	   { return(CI_ERR_LEN) ; }
		
	if ((str_name = (char *) malloc((strlen(name)+1)*sizeof(char))) == NULL)
	   { return(CI_ERR) ; }

  str_name[strlen(name)] = '\0';

	strncpy(str_name, name, strlen(str_name)) ;
	ciStrCompr(str_name);
		
	if (ciVariableLen > 0)
	{
		for (i=0; i<ciVariableLen; i++)
		{
			if (strcmp(str_name, ciVariable[i].name) == 0)
			{
				found = 1 ;
				pos = i ;
				break ;
			}
		}
	}

	free(str_name);

	if (found == 1)
	{
		if ((var = (tVar *) malloc((ciVariableLen-1)*sizeof(tVar))) == NULL)
	   		{ return(CI_ERR) ; }

		pos = ciVariableLen ;
	}
	else
	{
		return(CI_ERR) ;
	}

	for (i=0; i<pos; i++) 
	{ 
		var[i] = ciVariable[i] ; 
	}

	for (i=pos; i<(ciVariableLen-1); i++) 
	{ 
		var[i] = ciVariable[i+1] ; 
	}

	free(ciVariable);
	ciVariable = var ;
	var = NULL ;
		
	ciVariableLen--;

	return(AF_OK) ;
}

/** Returnes value of variable
 * @param name name of variable
 * @return value ("0" on error)
 */
char *ciGetVarVal(char *name)
{
	int   found = 0 ;
	long  i ;
	long  len = 0 ;
	char *str_name  = NULL ;
  char *tmp = NULL ;

	len = strlen(name) ;

	if ((len <= 0)) 
	   { return(NULL) ; }

	if ((str_name = (char *) malloc((len+1)*sizeof(char))) == NULL)
	   { return(NULL) ; }

	for (i=0; i<len+1; i++) { str_name[i] = '\0'; }

	strncpy(str_name, name, len) ;
	ciStrCompr(str_name);
	
  found = 0 ;
  
	if (ciVariableLen > 0)
	{
		for (i=0; i<ciVariableLen; i++)
		{
			if (strcmp(str_name, ciVariable[i].name) == 0)
			{
				found = 1 ;
				free(str_name);
        if ((tmp = (char *)malloc((strlen(ciVariable[i].value)+1)*sizeof(char))) == NULL)
        {
          return(NULL);
        }
        else
        {
          tmp[strlen(ciVariable[i].value)] = '\0' ;
          strcpy(tmp,ciVariable[i].value);
          return(tmp);
        }
				break ;
			}
		}
	}

	if (found == 0)
	{
		free(str_name);
	}

	return(NULL);
}

/** Returnes value of variable
 * @param name name of variable
 * @return value ("0" on error)
 */
char *ciGetVarValFromGrp(char *name, char *group)
{
	int   found = 0 ;
	long  i ;
	long  len = 0 ;
	long  len_grp = 0 ;
	char *str_name = NULL ;
	char *str_grp  = NULL ;
  char *tmp = NULL ;

	len     = strlen(name) ;
	len_grp = strlen(group) ;

	if ((len <= 0)) { return(NULL) ; }
	if ((len_grp <= 0)) { return(NULL) ; }

	if ((str_name = (char *) malloc((len+1)*sizeof(char))) == NULL)
	   { return(NULL) ; }

	if ((str_grp = (char *) malloc((len_grp+1)*sizeof(char))) == NULL)
	   { return(NULL) ; }

	for (i=0; i<len+1; i++) { str_name[i] = '\0'; }
	for (i=0; i<len_grp+1; i++) { str_grp[i] = '\0'; }

	strncpy(str_name, name, len) ;
	ciStrCompr(str_name);

	strncpy(str_grp, group, len_grp) ;
	ciStrCompr(str_grp);
		
	if (ciVariableLen > 0)
	{
		for (i=0; i<ciVariableLen; i++)
		{
			if (strcmp(str_name, ciVariable[i].name) == 0)
			{
        if (strcmp(str_grp, ciVariable[i].group) == 0)
        {
				  found = 1 ;
				  free(str_name);
				  free(str_grp);

          if ((tmp = (char *)malloc((strlen(ciVariable[i].value)+1)*sizeof(char))) == NULL)
          {
            return(NULL);
          }
          else
          {
            tmp[strlen(ciVariable[i].value)] = '\0' ;
            strcpy(tmp,ciVariable[i].value);
            return(tmp);
          }
				  break ;
        }
			}
		}
	}

	if (found == 0)
	{
		free(str_name);
	  free(str_grp);
	}

	return(NULL);
}

/** Returnes name of variable
 * @param name name of variable
 * @return value ("0" on error)
 */
char *ciGetVarNameFromGrp(char *name, char *group)
{
	int   found = 0 ;
	long  i ;
	long  len = 0 ;
	long  len_grp = 0 ;
	char *str_name = NULL ;
	char *str_grp  = NULL ;
  char *tmp = NULL ;

	len     = strlen(name) ;
	len_grp = strlen(group) ;

	if ((len <= 0)) { return(NULL) ; }
	if ((len_grp <= 0)) { return(NULL) ; }

	if ((str_name = (char *) malloc((len+1)*sizeof(char))) == NULL)
	   { return(NULL) ; }

	if ((str_grp = (char *) malloc((len_grp+1)*sizeof(char))) == NULL)
	   { return(NULL) ; }

	for (i=0; i<len+1; i++) { str_name[i] = '\0'; }
	for (i=0; i<len_grp+1; i++) { str_grp[i] = '\0'; }

	strncpy(str_name, name, len) ;
	ciStrCompr(str_name);

	strncpy(str_grp, group, len_grp) ;
	ciStrCompr(str_grp);
		
	if (ciVariableLen > 0)
	{
		for (i=0; i<ciVariableLen; i++)
		{
			if (strcmp(str_name, ciVariable[i].value) == 0)
			{
        if (strcmp(str_grp, ciVariable[i].group) == 0)
        {
				  found = 1 ;
				  free(str_name);
				  free(str_grp);

          if ((tmp = (char *)malloc((strlen(ciVariable[i].name)+1)*sizeof(char))) == NULL)
          {
            return(NULL);
          }
          else
          {
            tmp[strlen(ciVariable[i].name)] = '\0' ;
            strcpy(tmp,ciVariable[i].name);
            return(tmp);
          }
				  break ;
        }
			}
		}
	}

	if (found == 0)
	{
		free(str_name);
	  free(str_grp);
	}

	return(name);
}

/** List all variables
 * @param fw file to write
 * @param values if==1 then values are printed, too
 * @return status
 */
int ciListVars(FILE *fw, long values)
{
	long i;

	for (i=0; i<ciVariableLen; i++)
	{
		fprintf(fw," %s %s\n", ciVariable[i].name, ciVariable[i].value);
	}
	return(CI_OK);
}

/** Returnes values from given group in string (comma separated)
 * @param group grou of variables (string)
 * @return status
 */
char *ciListVarsCSV(char *group)
{
	long i;
  char *tmp=NULL;
  long num = 0;
  long len=0;

  if (group == NULL) {return(" ");}

  num = 0 ;
  len = 0 ;

	for (i=0; i<ciVariableLen; i++)
	{
    if (strcmp(group, ciVariable[i].group) == 0)
    {
      len += strlen(ciVariable[i].name) ;
      num++ ;
    }
	}

  len += num + 2 ;

  if ((tmp=(char*)malloc((len+1)*sizeof(char))) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Out of memory for variables"));
    return(" ");
  }

  for (i=0; i<=len; i++) {tmp[i] = '\0'; }

  num = 0 ;

  for (i=0; i<ciVariableLen; i++)
	{
    if (strcmp(group, ciVariable[i].group) == 0)
    {
      if (num > 0) {strcat(tmp,",") ;}
      num++;
      strcat(tmp,ciVariable[i].name) ;
    }
	}

  return(tmp);
}


/** Returnes values from given group in string (comma separated)
 * @param group group of variables (string)
 * @param first first variable to use
 * @return status
 */
char *ciListVarsCSVpreDef(char *group, long first)
{
	long i, ii;
  char *tmp=NULL;
  long num = 0;
  long len=0;
  long first_pos = 0 ;
  long pos0 = 0 ;

  if (group == NULL) {return(" ");}

  num = 0 ;
  len = 0 ;
  
	for (i=0; i<ciVariableLen; i++)
	{
    
    if (strcmp(group, ciVariable[i].group) == 0)
    {
      len += strlen(ciVariable[i].name) ;
      num++ ;
    }
	}

  len += num + 2 ;

  if ((tmp=(char*)malloc((len+1)*sizeof(char))) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Out of memory for variables"));
    return(" ");
  }

  for (i=0; i<=len; i++) {tmp[i] = '\0'; }

  num = 0 ;

	for (i=0; i<ciVariableLen; i++)
	{
    if (strcmp(group, ciVariable[i].group) == 0)
    {
      if (atol(ciVariable[i].value) == first)
      {
        first_pos = i ;
        break ;
      }
      num++;
    }
  }

  num = 0 ;

  for (i=0; i<ciVariableLen; i++)
	{
    if (strcmp(group, ciVariable[i].group) == 0)
    {
      if (num > 0) {strcat(tmp,",") ;}
      num++;

      ii = i ;
      if (num == 1) 
      { 
        ii = first_pos ; 
        pos0 = i ;
      }
      if (i == first_pos) { ii = pos0 ; }

      strcat(tmp,ciVariable[ii].name) ;
    }
	}

  return(tmp);
}


/** Returnes values from given group in string (comma separated)
 * @param group group of variables (string)
 * @param list fist of allowed variables
 * @param listlen length of list
 * @return status
 */
char *ciListVarsListedCSV(char *group, long *list, long listlen)
{
  /* TODO */
	long i,j ;
  char *tmp=NULL;
  long num = 0;
  long len=0;
  long found = AF_NO ;

  if (group == NULL) {return(" ");}

  num = 0 ;
  len = 0 ;

	for (i=0; i<ciVariableLen; i++)
	{
    if (strcmp(group, ciVariable[i].group) == 0)
    {
      found = AF_NO ;
      for (j=0; j<listlen; j++)
      {
        if (list[j] == atoi(ciVariable[i].value))
        {
          found = AF_YES ;
          break ;
        }
      }
      if (found == AF_YES)
      {
        len += strlen(ciVariable[i].name) ;
        num++ ;
      }
    }
	}

  if (num <= 0) { return(" "); }

  len += num + 2 ;

  if ((tmp=(char*)malloc((len+1)*sizeof(char))) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Out of memory for variables"));
    return(" ");
  }

  for (i=0; i<=len; i++) {tmp[i] = '\0'; }

  num = 0 ;

  for (i=0; i<ciVariableLen; i++)
	{
    if (strcmp(group, ciVariable[i].group) == 0)
    {
      found = AF_NO ;
      for (j=0; j<listlen; j++)
      {
        if (list[j] == atoi(ciVariable[i].value))
        {
          found = AF_YES ;
          break ;
        }
      }
      if (found == AF_YES)
      {
        if (num > 0) {strcat(tmp,",") ;}
        num++;
        strcat(tmp,ciVariable[i].name) ;
      }
    }
	}

  return(tmp);
}

/* end of civar.c */
