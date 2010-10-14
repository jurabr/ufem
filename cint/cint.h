/*
   File name: cint.h
   Date:      2003/06/15 20:53
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
   
	 $Id: cint.h,v 1.13 2004/11/11 21:40:25 jirka Exp $
*/


#ifndef __CINT_H__
#define __CINT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fem_comm.h"

#ifdef _USE_LIB_EVAL_
#include <m_eval.h>
#endif

#ifdef WIN32
#define CI_SLASH '\\'
#else
#define CI_SLASH '/'
#endif

#define CI_OK      0
#define CI_ERR     1
#define CI_ERR_LEN 2
#define CI_ERR_VAL 3

#define CI_STR_LEN 2048

#define CI_MAX_SCRIPTS 99


typedef int (* CI_FUNC)(char *);

typedef struct
{
	char     *cmd;
	CI_FUNC  func;
}tFunc;

typedef struct
{
	char *name ;
	char *value;
	char *group;
}tVar;

typedef struct
{
	char *cmd ;  /* command line            */
	char *var ;  /* variable name for "for" */
	long  type ; /* 0=ordinary, 1=for, 2=if */
	long  end ;  /* end line of for/if      */
	long  from ; /* "from" value for "for"  */
	long  to ;   /* "to" value for "for"    */
}tScLn; 

typedef struct
{
	char  *name ;     /* script name             */
	tScLn *line ;     /* commands                */
	long   len ;      /* number of commands      */
}tScp;


extern tFunc *ciFunction;
extern int    ciFuncLen;

extern FILE  *ciLogFile;
extern int    ciUseLog ;

extern long ciReadingScript ;

extern tScp  ciScript[] ;
extern long  ciScriptLen ;



extern int ciParNum(char *str) ;
extern char *ciGetParStr(char *str, int parNum) ;
extern char *ciGetParStrNoExpand(char *str, int parNum) ;
extern void ciStrCompr(char *str) ;
extern void ciStrClearBegEnd(char *str);
extern long ciGetParInt(char *str, int ParNum) ;
extern double ciGetParDbl(char *str, int ParNum) ;
extern char *ciReplaceText(char *str, char *s_old, char *s_new) ;
extern int ciRunCmd(char *cmd) ;
extern int ciRunCmdHs(char *cmd) ;

extern void ciStrNoNewL(char *str);
extern char *ciSetPath(char *dir, char *fname, char *ext);
extern char *ciStrCat2CmdPers(char *s1, char *s2);
extern char *ciStrCat2ParsNoCommas(char *s1, char *s2);
extern char *ciStrCat3CmdPers(char *s1, char *s2, char *s3);
extern char *ciStrCat4CmdPers(char *s1, char *s2, char *s3, char *s4);
extern char *ciStrCat5CmdPers(char *s1, char *s2, char *s3, char *s4, char *s5);
extern char *ciStrCat6CmdPers(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6);

extern long ciTestStringALL(char *cmd, long pos);
extern char *ciAllocStr(char *str);
extern char *ciInt2Str(long num);
extern char *ciDbl2Str(double num);

extern int ciAddVar(char *name, char *value);
extern int ciAddVarGrp(char *name, char *value, char *group);
extern int ciDelVar(char *name);
extern char *ciGetVarVal(char *name);
extern char *ciGetVarValFromGrp(char *name, char *group);
extern char *ciGetVarNameFromGrp(char *name, char *group);
extern int ciListVars(FILE *fw, long values);
extern char *ciListVarsCSV(char *group);
extern char *ciListVarsCSVpreDef(char *group, long first);
extern char *ciListVarsListedCSV(char *group, long *list, long listlen);


/* scripting: */
extern void ciNullScriptField(void);
extern void ciFreeScriptField(void);
extern int ciAddScript(char *name);
extern int ciAddLineToActiveScript(char *cmd);
extern int ciPreProcActiveScript(void);
extern int ciRunScript (tScp *scp);

/* history: */
extern char **ciHistory     ;
extern long   ciHistorySize ;

extern void ciCmdHsFree(void);
extern int ciCmdHsAlloc(long size);
extern int ciCmdHsAdd(char *str);
extern char *ciCmdHsGet(long n);

#endif

/* end of cint.h */
