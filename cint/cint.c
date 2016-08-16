/*
   File name: cint.c
   Date:      2003/06/15 20:52
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

	 $Id: cint.c,v 1.19 2005/01/20 20:09:54 jirka Exp $
*/

#include "cint.h"
#include <locale.h>

tFunc *ciFunction = NULL ;
int    ciFuncLen  = 0 ;

FILE  *ciLogFile  = NULL ;
int    ciUseLog   = 0 ;

long   ciReadingScript = 0 ;

/** Computes number of parameters in "command" if commas are separators
 *  @param str string
 *  @return number of parameter
 */
int ciParNum(char *str)
{
	static long num = 0 ;
	int len, i,j ;
	int count = 0 ;

	if ((len=strlen(str)) < 1)
	{
		num = 0 ;
	}
	else
	{
		count = 1 ;
		for (i=0; i<len; i++)
		{
			if (str[i] == ',')
			{
				if (i>1) { if (str[i-1] == '\\') {continue;} }
				count++;
			}

			/* if comment is detected then the rest of string is changed to `\0`s */
			if ((str[i] == '!') || (str[i] == '#')) 
			{
				for (j=i; j<len; j++) { str[j] = '\0' ; }
				break;
			}

		}
		num = count ;
	}

	return(num);
}

/** Replaces text in string (for variable expanding etc.)
 * @param str original string
 * @param s_old text to be replaced
 * @param s_new new text
 * @return modified string (result)
 */
char *ciReplaceText(char *str, char *s_old, char *s_new)
{
	char *str_new = NULL ;
	int   len = 0 ;
	int   oldeln = 0;
	int   i,j,k ;
	int   count = 0 ;
	int   pos = 0 ;

	oldeln = strlen(s_old);

	for (i=0; i<strlen(str); i++)
	{
		for (j=i;j<i+oldeln; j++)
		{
			if (str[i] != s_old[j]) { break; }
			if (j == (i+oldeln-1)) {count++;}
		}
	}

	if (count == 0) {return(NULL);}

	len = strlen(str) - (count*(strlen(s_new)-strlen(s_old))) ;
	if ((str_new = (char *)malloc((len+1)*sizeof(char))) == NULL)
	{
		return(NULL);
	}

	for (i=0; i<strlen(str); i++)
	{
		for (j=i;j<i+oldeln; j++)
		{
			if (str[i] != s_old[j]) { break; }
			if (j == (i+oldeln-1)) 
			{
				for (k=pos; k<pos+strlen(s_new); i++)
				{
					str_new[k] = s_new[k-pos];
				}
				pos += strlen(s_new)-1; /* -1 ?? */
				i = j ; /* ?? */
			}
		}
		pos++;
	}

	return(str_new);
}

/** Returnes value of ParNum-th parameter (numbered from 0)
 * @param str string
 * @param ParNum parameter number
 * @param expandVars AF_YES/AF_NO (if variables are expanded)
 * @return parameter (NULL if parameter is empty)
 */
char *ciGetParStrVars(char *str, int parNum, long expandVars)
{
	char *tmp = NULL ;
	char *tmp0 = NULL ;
	char *tmp_mod = NULL ;
	int   len = 0 ;
	int   from, to ;
	int   i ;
	int   count = 0 ;

	from = 0 ;
	to   = -2 ;

	for (i=0; i<strlen(str); i++)
	{
		if ((str[i] == ',')||(str[i]=='\0')||(i == (strlen(str)-1)))
		{
			from = to+2 ;
			to = i-1 ;
			if (i == (strlen(str)-1)) {to++;}
			count++;

			if (count == (parNum+1))
			{
				len = to-from+2 ;
				if ((len-1) < 1) {return(NULL);}
				if ((tmp = (char *)malloc(sizeof(char)*len)) == NULL)
				{
					return(NULL);
				}
				else
				{
          if ((tmp0 = (char *)malloc(sizeof(char)*len)) == NULL)
				  {
					  return(NULL);
				  }
					for (i=from; i<=to; i++)
					{
						tmp[i-from] = str[i] ;
					}
					tmp[len-1] = '\0';

					/* to remove EOL from end of the string: */
					if (len > 1) { if (tmp[len-2] == '\n') {tmp[len-2] = '\0';} }
				}
			}
		}
	}
  
  if (tmp != NULL)
  {
    if (strlen(tmp) < 1) {free(tmp); return(NULL);}
    ciStrClearBegEnd(tmp) ;
    if (strlen(tmp) < 1) {free(tmp); return(NULL);}
  }
  else
  {
    return(NULL); 
  } 

  if (expandVars == AF_NO)
  {
		return(tmp);
  }
  else
  {
	  if ((tmp != NULL)&&(tmp0 != NULL))
	  {
  	  strcpy(tmp0,tmp);
  	  ciStrCompr(tmp0);
  
		  if ((tmp_mod = ciGetVarVal(tmp0)) != NULL)
		  {
			  free(tmp) ;
			  return(tmp_mod) ;
		  }
		  else
		  {
			  return(tmp);
		  }
	  }
	  else
	  {
		  return(NULL);
	  }
  }
}

/** Returnes value of ParNum-th parameter (numbered from 0) -
 * variables are expanded
 * @param str string
 * @param ParNum parameter number
 * @return parameter
 */
char *ciGetParStr(char *str, int parNum)
{
  return( ciGetParStrVars(str, parNum, AF_YES) ) ;
}

/** Returnes value of ParNum-th parameter (numbered from 0) -
 * variables are NOT expanded
 * @param str string
 * @param ParNum parameter number
 * @return parameter
 */
char *ciGetParStrNoExpand(char *str, int parNum)
{
  return( ciGetParStrVars(str, parNum, AF_NO) ) ;
}

/** Removes spaces, newlines etc. from string and converts it to lower case
 * @param str string
 */
void ciStrCompr(char *str)
{
	int i,j;
	int len = 0 ;
	char c;

	if (str == NULL) {return;}

	len = strlen(str);

	for (i=0; i<len; i++)
	{
		if ( (str[i] == ' ')|| (str[i] == '\n')|| (str[i] == '\t'))
		{
			for (j=i; j<len-1; j++) { str[j] = str[j+1] ; }

			i--;
			len--;
			str[len] = '\0' ;
		}
	}

	for (i=0; i<len; i++)
	{
		c = tolower(str[i]);
		str[i] = c ;
	}
}

/** Removes spaces, newlines ONLY from begin and end of string from string
 * @param str string
 */
void ciStrClearBegEnd(char *str)
{
#if 1
	int i,j;
	int len = 0 ;
  int stop = 0 ;
  int start = 0 ;

	if (str == NULL) {return;}

	len = strlen(str);

	for (i=0; i<len; i++)
	{
		if ( (str[i] == ' ')||(str[i] == '\n')||(str[i] == '\t'))
		{
			for (j=i; j<len-1; j++) { str[j] = str[j+1] ; }

			i--;
			len--;
			str[len] = '\0' ;
		}
    else
    {
      stop = i ;
      break ;
    }
	}

  start = len-1 ;
  
  for (i=(len-1) ; i<=(stop+1); i--)
	{
		if ( (str[i] == ' ')|| (str[i] == '\n')|| (str[i] == '\t')||(str[i] == '\0'))
		{
      /* do nothing */
		}
    else
    {
      start = i ;
      break ;
    }
	}
  
  for (i=start ; i<len; i++)
	{
		if ( (str[i] == ' ')|| (str[i] == '\n')|| (str[i] == '\t'))
		{
			for (j=i; j<len-1; j++) { str[j] = str[j+1] ; }

			i--;
			len--;
			str[len] = '\0' ;
		}
	}
#endif
}

/** Returnes integer value of ParNum-th parameter (numbered from 0)
 * @param str string
 * @param ParNum parameter number
 * @return parameter
 */
long ciGetParInt(char *str, int ParNum)
{
	static long val = 0 ;
	char       *tmp = NULL ;

	tmp = ciGetParStr(str, ParNum) ;
	if (tmp == NULL)
	{
		val = 0 ;
	}
	else
	{
		setlocale(LC_NUMERIC,"C");
		val = atoi(tmp) ;
	}
	if (tmp != NULL) { free(tmp); }
	return(val);
}

/** Returnes double value of ParNum-th parameter (numbered from 0)
 * @param str string
 * @param ParNum parameter number
 * @return parameter
 */
double ciGetParDbl(char *str, int ParNum)
{
	static double val = 0 ;
	char         *tmp = NULL ;

	tmp = ciGetParStr(str, ParNum) ;
	if (tmp == NULL)
	{
		val = 0 ;
	}
	else
	{
		setlocale(LC_NUMERIC,"C");
#ifdef _USE_LIB_EVAL_
		val = mev_parse(tmp) ;
#else
		val = atof(tmp) ;
#endif
	}
	free(tmp);
	return(val);
}



/** Identifies command and runs it
 * @param cmd command
 * @return state value
 */
int ciRunCmd(char *cmd)
{
  int   rv ;
	int   count = 0 ;
	int   param = 0 ;
	int   pos   = 0 ;
	char *name = NULL ;
	int   i,j,k;

	if (cmd == NULL) {return(CI_OK);}
	if (strlen(cmd) == 0) {return(CI_OK);}
	if ((cmd[0] == '#')||(cmd[0] == '!')) {return(CI_OK);}

	if ((param = ciParNum(cmd)) < 1) {return(CI_ERR_VAL);}
	if ((name = ciGetParStr(cmd,0)) == NULL){return(CI_ERR_LEN);}

	ciStrCompr(name);
	if (strlen(name) < 1 ){ free(name); name = NULL; return(CI_ERR_LEN);}

	/* scripting support */
  if (ciReadingScript == 1)
	{
		if ((strcmp(name,"endscript")==0)||((strcmp(name,"end")==0)))
		{
			rv = ciPreProcActiveScript();
			ciReadingScript = 0 ;
			if (rv != CI_OK) {free(name); name=NULL; return(AF_ERR);}
		}
		else
		{
		  rv = ciAddLineToActiveScript(cmd);
			if (rv != 0) {ciReadingScript = 0 ;}
		}
	  free(name);
	  name = NULL ;
		return(rv);
	}
	/* end of scripting support */

	for (i=0; i<strlen(name); i++)
	{
		count = 0 ;
		for (j=0; j<ciFuncLen; j++)
		{
      if (strcmp(ciFunction[j].cmd,name) == 0 )
      {
        pos = j ;
        count = 1 ;
        /*printf("[CMD0]: \"%s\" == \"%s\"\n",name, ciFunction[j].cmd);*/
        goto Out ;
      }
			for (k=0; k<=i; k++)
			{
				if (name[k] != ciFunction[j].cmd[k]) {break;}
				if (k == i)
				{
					pos = j ;
					count++ ;

					/*if (strlen(name) == (k+1))*/
					if (strlen(name) == (k-1))
					{
						count = 1 ;
            /*printf("[CMD1]: \"%s\" == \"%s\"\n",name, ciFunction[j].cmd);*/
						goto  Out;
					}
				}
			}
		}
		if (count == 1)
		{
			break;
		}
	}

Out:
	if (count != 1)
	{
		pos = 0 ; /* zero function must be empty/error function */
	}

	free(name);
	name = NULL ;

	/* logging of command: */
	if (ciUseLog == 1)
	   { if (ciLogFile != NULL) { fprintf(ciLogFile,"%s\n", cmd) ; } }
	/* end of logging */

	return(ciFunction[pos].func(cmd));
}

/** Identifies command and runs it and saves it to history buffer (if possible)
 * @param cmd command
 * @return state value
 */
int ciRunCmdHs(char *cmd)
{
  int   rv ;

	rv = ciRunCmd(cmd) ;

 /* check and add to history (if possible): */
	if (rv == AF_OK) 
	   { if ((ciHistorySize > 0)&&(ciHistory != NULL)) { ciCmdHsAdd(cmd); } }

	return(rv);
}


/** Replaces newlines with '\0's 
 * @param chr string to be modified
 * */
void ciStrNoNewL(char *str)
{
  int i;

  for (i=0; i<strlen(str); i++)
  {
    if (str[i] == '\n') { str[i] = '\0' ; }
    if (str[i] == '\t') { str[i] = ' ' ; }
  }
}

/** creates path string
 * @param dir directory
 * @param fname filename
 * @param ext extension
 *
 * */
char *ciSetPath(char *dir, char *fname, char *ext)
{
  char *tmp = NULL ;
  char  slash[2];
  long  len ;
  long  i;

  slash[0] = CI_SLASH ;
  slash[1] = '\0' ;

  len = strlen(dir) + 1 + strlen(fname) + strlen(".") + strlen(ext) ;

  if (strlen(dir) <= 0) { len += strlen(".") ;}

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}

  if (strlen(dir) <= 0) { strcat(tmp,"."); }
  else                  { strcat(tmp,dir); }
  strcat(tmp,slash);
  strcat(tmp,fname);
  strcat(tmp,".");
  strcat(tmp,ext);

  return(tmp);
}

/** Cats two strings divided by comma */
char *ciStrCat2CmdPers(char *s1, char *s2)
{
  char *tmp = NULL ;
  long len  = 0;
  long i ;
  long l1=0;
  long l2=0;

  if (s1 != NULL) {l1 = strlen(s1);}
  if (s2 != NULL) {l2 = strlen(s2);}

  len = l1 + l2 + strlen(",") ;

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}



  len = strlen(s1) + strlen(s2) + strlen(",") ;

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}

  if (s1 != NULL){strcat(tmp,s1);}
  strcat(tmp,",");
  if (s2 != NULL) { strcat(tmp,s2); }

  return(tmp);
}

/** Cats two strings divided by comma */
char *ciStrCat2ParsNoCommas(char *s1, char *s2)
{
  char *tmp = NULL ;
  long len  = 0;
  long i ;
  long l1=0;
  long l2=0;

  if (s1 != NULL) {l1 = strlen(s1);}
  if (s2 != NULL) {l2 = strlen(s2);}

  len = l1 + l2 + strlen(" ") ;

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}



  len = strlen(s1) + strlen(s2) + strlen(",") ;

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}

  if (s1 != NULL){strcat(tmp,s1);}
  strcat(tmp," ");
  if (s2 != NULL) { strcat(tmp,s2); }

  return(tmp);
}

/** Cats three strings divided by commas */
char *ciStrCat3CmdPers(char *s1, char *s2, char *s3)
{
  char *tmp = NULL ;
  long len  = 0;
  long i ;
  long l1=0;
  long l2=0;
  long l3=0;

  if (s1 != NULL) {l1 = strlen(s1);}
  if (s2 != NULL) {l2 = strlen(s2);}
  if (s3 != NULL) {l3 = strlen(s3);}

  len = l1 + l2 + l3 + 2*strlen(",") ;

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}

  if (s1 != NULL){strcat(tmp,s1);}
  strcat(tmp,",");
  if (s2 != NULL){strcat(tmp,s2);}
  strcat(tmp,",");
  if (s3 != NULL) { strcat(tmp,s3); }

  return(tmp);
}

/** Cats five strings divided by commas */
char *ciStrCat4CmdPers(char *s1, char *s2, char *s3, char *s4)
{
  char *tmp = NULL ;
  long len  = 0;
  long i ;
  long l1=0;
  long l2=0;
  long l3=0;
  long l4=0;

  if (s1 != NULL) {l1 = strlen(s1);}
  if (s2 != NULL) {l2 = strlen(s2);}
  if (s3 != NULL) {l3 = strlen(s3);}
  if (s4 != NULL) {l4 = strlen(s4);}

  len = l1 + l2 + l3 + l4 + 3*strlen(",") ;

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}

  if (s1 != NULL){strcat(tmp,s1);}
  strcat(tmp,",");
  if (s2 != NULL){strcat(tmp,s2);}
  strcat(tmp,",");
  if (s3 != NULL) { strcat(tmp,s3); }
  strcat(tmp,",");
  if (s4 != NULL) { strcat(tmp,s4); }

  return(tmp);
}

/** Cats five strings divided by commas */
char *ciStrCat5CmdPers(char *s1, char *s2, char *s3, char *s4, char *s5)
{
  char *tmp = NULL ;
  long len  = 0;
  long i ;
  long l1=0;
  long l2=0;
  long l3=0;
  long l4=0;
  long l5=0;

  if (s1 != NULL) {l1 = strlen(s1);}
  if (s2 != NULL) {l2 = strlen(s2);}
  if (s3 != NULL) {l3 = strlen(s3);}
  if (s4 != NULL) {l4 = strlen(s4);}
  if (s5 != NULL) {l5 = strlen(s5);}

  len = l1 + l2 + l3 + l4 + l5 + 4*strlen(",") ;

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}

  if (s1 != NULL){strcat(tmp,s1);}
  strcat(tmp,",");
  if (s2 != NULL){strcat(tmp,s2);}
  strcat(tmp,",");
  if (s3 != NULL) { strcat(tmp,s3); }
  strcat(tmp,",");
  if (s4 != NULL) { strcat(tmp,s4); }
  strcat(tmp,",");
  if (s5 != NULL) { strcat(tmp,s5); }

  return(tmp);
}

/** Cats six strings divided by commas */
char *ciStrCat6CmdPers(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6)
{
  char *tmp = NULL ;
  long len  = 0;
  long i ;
  long l1=0;
  long l2=0;
  long l3=0;
  long l4=0;
  long l5=0;
  long l6=0;

  if (s1 != NULL) {l1 = strlen(s1);}
  if (s2 != NULL) {l2 = strlen(s2);}
  if (s3 != NULL) {l3 = strlen(s3);}
  if (s4 != NULL) {l4 = strlen(s4);}
  if (s5 != NULL) {l5 = strlen(s5);}
  if (s6 != NULL) {l6 = strlen(s6);}

  len = l1 + l2 + l3 + l4 + l5 + l6 + 5*strlen(",") ;

  if ((tmp=(char *)malloc((len+1)*sizeof(char)) ) == NULL) { return(NULL); } 
  for (i=0; i<=len; i++) {tmp[i] = '\0';}

  strcat(tmp,s1);
  strcat(tmp,",");
  if (s2 != NULL){strcat(tmp,s2);}
  strcat(tmp,",");
  if (s3 != NULL) { strcat(tmp,s3); }
  strcat(tmp,",");
  if (s4 != NULL) { strcat(tmp,s4); }
  strcat(tmp,",");
  if (s5 != NULL) { strcat(tmp,s5); }
  strcat(tmp,",");
  if (s6 != NULL) { strcat(tmp,s6); }

  return(tmp);
}


/** Tests if command parameter at position pos is  equal to constant "ALL"
 * @param cmd command string
 * @param pos position of tested parameter
 * @return AF_YES if is, AF_NO if not
 */
long ciTestStringALL(char *cmd, long pos)
{
  char *str = NULL ;

  if (ciParNum(cmd) <= pos){return(AF_NO);}

  if ((str=ciGetParStr(cmd, pos)) == NULL) {return(AF_NO);}

  ciStrCompr(str) ;
  if (strlen(str) < 3) {free(str); return(AF_NO);}

  if (strcmp(str,"all") == 0)
  {
    free(str);
    return(AF_YES) ;
  }
  
  return(AF_NO);
}

/** Allocates string and sets it to "str" */
char *ciAllocStr(char *str)
{
	char *tmp = NULL;
	int len = 0 ;
	int i;
	
	len=(strlen(str)) ;
	if (len <= 0)
	{
		return(" ") ;
	}
	else
	{
		if ((tmp = (char *)malloc((len+1)*sizeof(char))) == NULL)
		{
			return(" ");
		}
		else
		{
			for (i=0; i<=len; i++) {tmp[i] = '\0';}
		}
		strcpy(tmp,str)  ;
	}

	return(tmp);
}

/** Allocates string and sets it to integer number */
char *ciInt2Str(long num)
{
	char *tmp = NULL;
	int len = 1024 ;
	int i;
	
	if (len <= 0)
	{
		return(" ") ;
	}
	else
	{
		if ((tmp = (char *)malloc((len+1)*sizeof(char))) == NULL)
		{
			return(" ");
		}
		else
		{
			for (i=0; i<=len; i++) {tmp[i] = '\0';}
		}
#ifndef __C99__
		sprintf(tmp,"%li",num)  ; /* it's DANGEROUS - C99's snprintf will be better  */
#else
		snprintf(tmp,"%li",num,1024)  ; /* it's DANGEROUS - C99's snprintf will be better  */
#endif
	}

	return(tmp);
}

/** Allocates string and sets it to fp number */
char *ciDbl2Str(double num)
{
	char *tmp = NULL;
	int len = 1024 ;
	int i;
	
	if (len <= 0)
	{
		return(" ") ;
	}
	else
	{
		if ((tmp = (char *)malloc((len+1)*sizeof(char))) == NULL)
		{
			return(" ");
		}
		else
		{
			for (i=0; i<=len; i++) {tmp[i] = '\0';}
		}
#ifndef __C99__
		sprintf(tmp,"%e",num)  ; /* it's DANGEROUS - C99's snprintf will be better  */
#else
		snprintf(tmp,"%e",num,1024)  ; 
#endif
	}

	return(tmp);
}

/* end of cint.c */
