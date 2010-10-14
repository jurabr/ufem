/*
   File name: cmd_macs.c
   Date:      2004/01/24 10:40
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

   FEM User Interface: "macros" for command interpreter

*/

#include "fem_tui.h"
#include "cint.h"
#include "fdb_fem.h"
#include "fdb_res.h"

#include <math.h>

extern int femCmdDoNotBreak ;

/** Creates brick from tetraedra elements: tbrick,n1,n2,n3,n4,n5,n6,n7,n8
 * @param cmd command
 * @return status
 */
int func_mac_tbrick (char *cmd)
{
	int    rv = AF_OK ;
  int    i ;
  long   node[9] ;
  long   nodes4[4] ;

	FEM_TEST_PREPROCESSOR

	if (ciParNum(cmd) < 9) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Eight node numbers are required")) ;
		return(AF_ERR_VAL);
	}

  for (i=1; i<=8; i++) { node[i] = ciGetParInt(cmd, i) ; }

  nodes4[0] = node[1] ;
  nodes4[1] = node[3] ;
  nodes4[2] = node[8] ;
  nodes4[3] = node[4] ;
	rv = f_en_change(0, nodes4, 4);

  nodes4[0] = node[6] ;
  nodes4[1] = node[3] ;
  nodes4[2] = node[7] ;
  nodes4[3] = node[8] ;
	rv = f_en_change(0, nodes4, 4);

  nodes4[0] = node[6] ;
  nodes4[1] = node[1] ;
  nodes4[2] = node[2] ;
  nodes4[3] = node[3] ;
	rv = f_en_change(0, nodes4, 4);

  nodes4[0] = node[1] ;
  nodes4[1] = node[6] ;
  nodes4[2] = node[8] ;
  nodes4[3] = node[5] ;
	rv = f_en_change(0, nodes4, 4);

  nodes4[0] = node[1] ;
  nodes4[1] = node[6] ;
  nodes4[2] = node[3] ;
  nodes4[3] = node[8] ;
	rv = f_en_change(0, nodes4, 4);

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Evaluates a simple expression (result=value1(operation)value2): "eval,variable_for_result,value1,(+|-|/|*|^),value2"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_eval (char *cmd)
{
	int    rv = AF_OK ;
	double result = 0 ;
	double val1 = 0.0 ;
	double val2 = 0.0 ;
	char  *var = NULL ;
	char  *ops = NULL ;
	char  sres[2048];

	if (ciParNum(cmd) < 5) 
	{ 
		fprintf(msgout,"[E] %s:%s,%s!\n", _("Must be"),ciGetParStr(cmd,0),_("variable,value1,operation,value2")) ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

	if ((var=ciGetParStrNoExpand(cmd,1))==NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  ciStrCompr(var);

	if (strlen(var) < 1) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid name of variable"));
		free(var); var = NULL ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}


	if ((ops=ciGetParStrNoExpand(cmd,3))==NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Operation symbol required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  ciStrCompr(ops);

	if (strlen(ops) < 1) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid symbol of operation"));
		free(ops); ops = NULL ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}


	val1 = ciGetParDbl(cmd,2) ;
	val2 = ciGetParDbl(cmd,4) ;

	switch(ops[0])
	{
		case '+': result = val1+val2 ; break ;
		case '-': result = val1-val2 ; break ;
		case '*': result = val1*val2 ; break ;
		case '/': result = val1/val2 ; break ;
		case '%': result = val1/val2 ; break ;
		case '^': result = pow(val1,val2) ; break ;
    case 's':
    case 'S': result = val1 * sin(FEM_PI*(val2/180)) ; break ;
    case 'c':
    case 'C': result = val1 * cos(FEM_PI*(val2/180)) ; break ;
    case 't':
    case 'T': result = val1 * tan(FEM_PI*(val2/180)) ; break ;
    case 'a':
    case 'A': result = val1 * 180 * atan((val2))/FEM_PI ; break ; /* ! */
    case 'k':
    case 'K': result = val1 * 180 * acos((val2)/FEM_PI) ; break ; /* ! */
    case 'z':
    case 'Z': result = val1 * 180 * asin((val2)/FEM_PI) ; break ; /* ! */

		default: result = 0 ; rv = AF_ERR_VAL ; break;
	}

	if (rv != AF_OK)
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid symbol of operation"));
	}

	sprintf(sres,"%e",result);

	ciAddVar(var, sres);

	free(var); var = NULL ;
	free(ops); var = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Evaluates a simple INTEGER expression (result=value1(operation)value2): "ieval,variable_for_result,value1,(+|-|/|*|^),value2"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_ieval (char *cmd)
{
	int    rv = AF_OK ;
	long result = 0 ;
	long val1 = 0 ;
	long val2 = 0 ;
	char  *var = NULL ;
	char  *ops = NULL ;
	char  sres[2048];
	ldiv_t dval;

	if (ciParNum(cmd) < 5) 
	{ 
		fprintf(msgout,"[E] %s:%s,%s!\n", _("Must be"),ciGetParStr(cmd,0),_("variable,value1,operation,value2")) ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

	if ((var=ciGetParStrNoExpand(cmd,1))==NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  ciStrCompr(var);

	if (strlen(var) < 1) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid name of variable"));
		free(var); var = NULL ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}


	if ((ops=ciGetParStrNoExpand(cmd,3))==NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Operation symbol required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  ciStrCompr(ops);

	if (strlen(ops) < 1) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid symbol of operation"));
		free(ops); ops = NULL ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}


	val1 = ciGetParInt(cmd,2) ;
	val2 = ciGetParInt(cmd,4) ;

	switch(ops[0])
	{
		case '+': result = val1+val2 ; break ;
		case '-': result = val1-val2 ; break ;
		case '*': result = val1*val2 ; break ;
		case '/': 
		case '%': dval = ldiv(val1,val2) ;
							result = dval.quot ; break ;
		case '\\':  dval = ldiv(val1,val2) ;
							result = dval.rem ; break ;
		case '^': result = (long)pow((double)val1,(double)val2) ; break ;
		default: result = 0 ; rv = AF_ERR_VAL ; break;
	}

	if (rv != AF_OK)
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid symbol of operation"));
	}

	sprintf(sres,"%li",result);

	ciAddVar(var, sres);

	free(var); var = NULL ;
	free(ops); var = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Fills variable with lengh of table: "fillvartablensel,variable,(et|mat|rs|n|ele|g|f|d|elo|k|ent)"
 * @param cmd command
 * @return status
 */
int func_mac_fill_var_tab_len_sel (char *cmd)
{
	int    rv = AF_OK ;
	char  *var  = NULL ;
	char  *type = NULL ;
	int    i ;
	char   value[CI_STR_LEN];

	for (i=0; i<CI_STR_LEN; i++) { value[i] = '\0' ; }
	
	if (ciParNum(cmd) < 3) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Variable and type data must be specified"));
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

	if ((var=ciGetParStrNoExpand(cmd,1))==NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  ciStrCompr(var);

	if (strlen(var) < 1) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid name of variable"));
		free(var); var = NULL ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

	if ((type=ciGetParStrNoExpand(cmd,2))==NULL)
	{
		free(var) ; var = NULL ;
		fprintf(msgout,"[E] %s!\n", _("Type of data is required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  ciStrCompr(type);

	if (strlen(type) < 1) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid type of data"));
		free(var) ; var = NULL ;
		free(type); type = NULL ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

	switch (type[0])
	{
		case 'm': sprintf(value,"%li",fdbInputTabLenAll(MAT)); break ;
		case 'r': sprintf(value,"%li",fdbInputTabLenAll(RSET)); break ;
		case 'n': sprintf(value,"%li",fdbInputTabLenAll(NODE)); break ;
		case 'd': sprintf(value,"%li",fdbInputTabLenAll(NDISP)); break ;
		case 'f': sprintf(value,"%li",fdbInputTabLenAll(NLOAD)); break ;
		case 'g': sprintf(value,"%li",fdbInputTabLenAll(GRAV)); break ;
		case 'k': sprintf(value,"%li",fdbInputTabLenAll(KPOINT)); break ;
		case 'l': 
		case 'a': 
		case 'v': sprintf(value,"%li",fdbInputTabLenAll(ENTITY)); break ;
		case 'e': if (strlen(type) == 1)
							{
								sprintf(value,"%li",fdbInputTabLenAll(ELEM)); break ;
							}
							else
							{
								if (type[1] == 'n')
								{
									sprintf(value,"%li",fdbInputTabLenAll(ENTITY)); break ;
								}
								else
								{
									if (type[1] == 't')
									{
										sprintf(value,"%li",fdbInputTabLenAll(ETYPE)); break ;
									}
									else
									{
										if (type[1] == 'l')
										{
											if (strlen(type) < 3)
											{
												rv = AF_ERR ; 
												fprintf(msgout,"[E] %s: \"elem\",\"eload\"!\n",
														_("Type specification too short - please use one of these"));
												goto memFree;
											}
											else
											{
												if ((type[2] !='e')&&(type[2] !='o'))
												{
													rv = AF_ERR ; 
													fprintf(msgout,"[E] %s: \"elem\",\"eload\"!\n",_("Please use one of these"));
													goto memFree;
												}
												else
												{
													if (type[2] =='e')
													{
														sprintf(value,"%li",fdbInputTabLenAll(ELEM)); break ;
													}
													else
													{
														sprintf(value,"%li",fdbInputTabLenAll(ENTITY)); break ;
													}
												}
											}
										}
										else
										{
											rv = AF_ERR ; 
											fprintf(msgout,"[E] %s: \"%s\"!\n",_("Unknown type"),type);
											goto memFree;
										}
									}
								}
							}
		default: 
								fprintf(msgout,"[E] %s: \"%s\"!\n",_("Unknown type"),type);
								rv = AF_ERR_EMP; 
								goto memFree; 
								break;
	}

	rv = ciAddVar(var,value) ;

memFree:
	free(var) ; var = NULL ;
	free(type) ; type = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Fills variable with lengh of table: "fillvartablen,variable,(et|mat|rs|n|ele|g|f|d|elo|k|ent)"
 * @param cmd command
 * @return status
 */
int func_mac_fill_var_tab_len (char *cmd)
{
	int    rv = AF_OK ;
	char  *var  = NULL ;
	char  *type = NULL ;
	int    i ;
	char   value[CI_STR_LEN];

	for (i=0; i<CI_STR_LEN; i++) { value[i] = '\0' ; }
	
	if (ciParNum(cmd) < 3) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Variable and type data must be specified"));
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

	if ((var=ciGetParStrNoExpand(cmd,1))==NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  ciStrCompr(var);

	if (strlen(var) < 1) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid name of variable"));
		free(var); var = NULL ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

	if ((type=ciGetParStrNoExpand(cmd,2))==NULL)
	{
		free(var) ; var = NULL ;
		fprintf(msgout,"[E] %s!\n", _("Type of data is required"));
		return ( tuiCmdReact(cmd, AF_ERR_EMP) ) ;
	}

  ciStrCompr(type);

	if (strlen(type) < 1) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid type of data"));
		free(var) ; var = NULL ;
		free(type); type = NULL ;
		return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

	switch (type[0])
	{
		case 'm': sprintf(value,"%li",fdbInputTabLenSel(MAT)); break ;
		case 'r': sprintf(value,"%li",fdbInputTabLenSel(RSET)); break ;
		case 'n': sprintf(value,"%li",fdbInputTabLenSel(NODE)); break ;
		case 'd': sprintf(value,"%li",fdbInputTabLenSel(NDISP)); break ;
		case 'f': sprintf(value,"%li",fdbInputTabLenSel(NLOAD)); break ;
		case 'g': sprintf(value,"%li",fdbInputTabLenSel(GRAV)); break ;
		case 'k': sprintf(value,"%li",fdbInputTabLenSel(KPOINT)); break ;
		case 'l': 
		case 'a': 
		case 'v': sprintf(value,"%li",fdbInputTabLenSel(ENTITY)); break ;
		case 'e': if (strlen(type) == 1)
							{
								sprintf(value,"%li",fdbInputTabLenSel(ELEM)); break ;
							}
							else
							{
								if (type[1] == 'n')
								{
									sprintf(value,"%li",fdbInputTabLenSel(ENTITY)); break ;
								}
								else
								{
									if (type[1] == 't')
									{
										sprintf(value,"%li",fdbInputTabLenSel(ETYPE)); break ;
									}
									else
									{
										if (type[1] == 'l')
										{
											if (strlen(type) < 3)
											{
												rv = AF_ERR ; 
												fprintf(msgout,"[E] %s: \"elem\",\"eload\"!\n",
														_("Type specification too short - please use one of these"));
												goto memFree;
											}
											else
											{
												if ((type[2] !='e')&&(type[2] !='o'))
												{
													rv = AF_ERR ; 
													fprintf(msgout,"[E] %s: \"elem\",\"eload\"!\n",_("Please use one of these"));
													goto memFree;
												}
												else
												{
													if (type[2] =='e')
													{
														sprintf(value,"%li",fdbInputTabLenSel(ELEM)); break ;
													}
													else
													{
														sprintf(value,"%li",fdbInputTabLenSel(ENTITY)); break ;
													}
												}
											}
										}
										else
										{
											rv = AF_ERR ; 
											fprintf(msgout,"[E] %s: \"%s\"!\n",_("Unknown type"),type);
											goto memFree;
										}
									}
								}
							}
		default: 
								fprintf(msgout,"[E] %s: \"%s\"!\n",_("Unknown type"),type);
								rv = AF_ERR_EMP; 
								goto memFree; 
								break;
	}

	rv = ciAddVar(var,value) ;

memFree:
	free(var) ; var = NULL ;
	free(type) ; type = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Fills variable with average DOF result value: "fillvaraverdof,variable,dof_type"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_aver_dof_res (char *cmd)
{
	int    rv = AF_OK ;
	long   dof;
	char  *var = NULL ;
	char   sres[2048];

	FEM_TEST_POSTPROCESSOR

  if ((ResLen <= 0) || (ResActStep >= ResLen)) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("No results available"));
		return(tuiCmdReact(cmd, AF_ERR_SIZ));
	}

	if (ciParNum(cmd) < 3) 
	{
		fprintf(msgout,"[E] %s!\n", 
					_("Both variable and result type must be specified"));
		return(tuiCmdReact(cmd, AF_ERR_SIZ));
	}

	dof = ciGetParInt(cmd, 2) ;

	if ((dof <= 0)||(dof > KNOWN_DOFS))
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal type of result"));
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	if (ciGetParStrNoExpand(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid name of variable"));
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}
	else
	{
		var = ciGetParStrNoExpand(cmd,1) ;
		ciStrCompr(var);
		if (strlen(var) < 1) 
		{
			fprintf(msgout,"[E] %s!\n", _("Invalid name of variable"));
			rv = AF_ERR_VAL; 
			goto memFree;
		}
	}

	sprintf(sres,"%e", fdbGetAverDOF(dof));
	rv = ciAddVar(var, sres);

memFree:
	free (var); var = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Creates new empty disk file (rewrites existing): "newfile,file_name"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_new_file (char *cmd)
{
	int    rv = AF_OK ;
	
	if (ciParNum(cmd) < 2) 
	{
		fprintf(msgout,"[E] %s!\n", _("Name of file is required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	rv = fdbWriteStrRewrite(ciGetParStr(cmd,1), " ");

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Adds newline character to a file: "writenl,file_name"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_write_newline (char *cmd)
{
	int    rv = AF_OK ;
	
	if (ciParNum(cmd) < 2) 
	{
		fprintf(msgout,"[E] %s!\n", _("Name of file is required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	rv = fdbWriteStrAppend(ciGetParStr(cmd,1), "\n");

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Adds variable to a file: "writevar,file_name,variable_name"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_write_var (char *cmd)
{
	int    rv = AF_OK ;
	
	if (ciParNum(cmd) < 3) 
	{
		fprintf(msgout,"[E] %s!\n", _("Both name of file and varible name are required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Filename must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,2) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	rv = fdbWriteStrAppend(ciGetParStr(cmd,1), ciGetVarVal(ciGetParStrNoExpand(cmd,2)));

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Fills variable with something: "fillvarsumreact,variable_name,type(ux|uy|uz|rotx|roty|rotz)"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_fill_var_sum_react (char *cmd)
{
	int    rv = AF_OK ;
	char  *var = NULL ;
	char   sres[2048];

	if (ciParNum(cmd) < 3) 
	{
		fprintf(msgout,"[E] %s!\n", _("Both varible name and direction are required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	var = ciGetParStrNoExpand(cmd,1) ;
	ciStrCompr(var);
	if (strlen(var)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal name of variable"));
		free(var); var = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

	sprintf(sres,"%e", fdbGetSumReact(ciGetParInt(cmd,2)));
	rv = ciAddVar(var, sres);
	free(var); var = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Fills variable with set time info: "fillvarrestime,variable_name"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_fill_var_res_time (char *cmd)
{
	int    rv = AF_OK ;
	char  *var = NULL ;
	char   sres[2048];

	if (ciParNum(cmd) < 3) 
	{
		fprintf(msgout,"[E] %s!\n", _("Both varible name and set number are required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	var = ciGetParStrNoExpand(cmd,1) ;
	ciStrCompr(var);
	if (strlen(var)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal name of variable"));
		free(var); var = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

	sprintf(sres,"%e", fdbGetSetTime());
	rv = ciAddVar(var, sres);
	free(var); var = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Fills variable with reaction in point and direction: "fillsumreact,variable_name,node,type(ux|uy|uz|rotx|roty|rotz)"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_fill_var_one_react (char *cmd)
{
	int    rv = AF_OK ;
  long   node = 0 ;
	char  *var = NULL ;
	char   sres[2048];

	if (ciParNum(cmd) < 4) 
	{
		fprintf(msgout,"[E] %s!\n", _("All varible name, node and direction are required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	var = ciGetParStrNoExpand(cmd,1) ;
	ciStrCompr(var);
	if (strlen(var)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal name of variable"));
		free(var); var = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

  node = ciGetParInt(cmd,2) ;

  if ( (node < 1) || (node > fdbInputFindMaxInt(NODE, NODE_ID)) )
  {
		fprintf(msgout,"[E] %s!\n", _("Impossible node number"));
    rv = AF_ERR_VAL ;
  }
  else
  {
	  sprintf(sres,"%e", fdbGetOneReact(node, ciGetParInt(cmd,3)));
	  rv = ciAddVar(var, sres);
  }
	free(var); var = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}
	
	
/** Provides strcat() functionality for variables (max. 5 strings!): "varstrcat,variable_name,str,str[,str][,str][,str]"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_mac_var_strcat(char *cmd)
{
	int   rv  = AF_OK ;
	char *str = NULL ;
	char *var = NULL ;
	long  len = 0 ;
	long  ilen = 0 ;
	long  num = 0 ;
	int   i ;

	if (ciParNum(cmd) < 4) 
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name and at least two strings required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	num = 0 ;
	len = 0 ;

	for (i=2; i< ciParNum(cmd); i++)
	{
		if (ciGetParStr(cmd,i) == NULL) {break;}
		if ((ilen=strlen(ciGetParStr(cmd,i))) < 1) {break;}
		len +=ilen ;
		num++ ;
	}

	len++;

	if (num < 2)
	{
		fprintf(msgout,"[E] %s!\n", _("At least two strings required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	var = ciGetParStrNoExpand(cmd,1) ;
	ciStrCompr(var);
	if (strlen(var)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal name of variable"));
		free(var); var = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if ((str=(char *)malloc(len*sizeof(char))) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("No memory for new string"));
		free(var); var = NULL ;
		return(tuiCmdReact(cmd, AF_ERR_MEM));
	}

	for (i=0; i<len; i++) {str[i] = '\0';}

	for (i=0; i<num ; i++)
	{
		strncat(str,ciGetParStr(cmd, i+2), (len-1));
	}

	rv = ciAddVar(var, str);

	free(var); var = NULL ;
	free(str); str = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Fills variable with job name: "fillvarjobname,variable_name"
 * @param cmd command
 * @return status
 */
int func_mac_fill_var_jobname (char *cmd)
{
	int    rv = AF_OK ;
	char  *var = NULL ;

	if (ciParNum(cmd) < 2) 
	{
		fprintf(msgout,"[E] %s!\n", _("Varible name is required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	var = ciGetParStrNoExpand(cmd,1) ;
	ciStrCompr(var);
	if (strlen(var)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal name of variable"));
		free(var); var = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

	rv = ciAddVar(var, femGetJobname());
	free(var); var = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Fills variable with job name: "fillvardatadir,variable_name"
 * @param cmd command
 * @return status
 */
int func_mac_fill_var_data_dir (char *cmd)
{
	int    rv = AF_OK ;
	char  *var = NULL ;

	if (ciParNum(cmd) < 2) 
	{
		fprintf(msgout,"[E] %s!\n", _("Varible name is required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	var = ciGetParStrNoExpand(cmd,1) ;
	ciStrCompr(var);
	if (strlen(var)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal name of variable"));
		free(var); var = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

	rv = ciAddVar(var, femGetDataDir());
	free(var); var = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Fills variable with custom filename (path to working directory is set by auto): "fillvarfilename,variable_name,filename,extension"
 * @param cmd command
 * @return status
 */
int func_mac_fill_var_file_name (char *cmd)
{
	int    rv = AF_OK ;
	char  *var = NULL ;
	char  *fname = NULL ;
	char  *ext = NULL ;

	if (ciParNum(cmd) < 4) 
	{
		fprintf(msgout,"[E] %s!\n", _("Varible name and filename and extension are all required"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciGetParStr(cmd,1) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Variable name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	var = ciGetParStrNoExpand(cmd,1) ;
	ciStrCompr(var);
	if (strlen(var)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal name of variable"));
		free(var); var = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

	/* fname */
	if (ciGetParStr(cmd,2) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("File name must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	fname = ciGetParStrNoExpand(cmd,2) ;
	ciStrCompr(fname);
	if (strlen(fname)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal filename"));
		free(var); var = NULL ;
		free(fname); fname = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

	/* extension */
	if (ciGetParStr(cmd,3) == NULL)
	{
		fprintf(msgout,"[E] %s!\n", _("Extension must not be empty"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	ext = ciGetParStrNoExpand(cmd,3) ;
	ciStrCompr(ext);
	if (strlen(ext)<1)
	{
		fprintf(msgout,"[E] %s!\n", _("Illegal extension"));
		free(var); var = NULL ;
		free(fname); fname = NULL ;
		free(ext); ext = NULL ;
		return(tuiCmdReact(cmd, AF_ERR));
	}

	rv = ciAddVar(var, ciSetPath(femGetDataDir(),fname,ext) );

	free(var); var = NULL ;
	free(fname); fname = NULL ;
	free(ext); ext = NULL ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Sets if infut files breaks on error ("no") or can continue ("yes): "nobreakinput[yes,no]"
 * @param cmd command
 * @return status
 */
int func_mac_set_nobreak_input (char *cmd)
{
	int    rv = AF_OK ;
	char  *var = NULL ;

	if (ciParNum(cmd) < 2) 
	{
		femCmdDoNotBreak = AF_NO ;
		fprintf(msgout,"[i] %s!\n", _("Inputted commands will stop on errors"));
		return(tuiCmdReact(cmd, rv));
	}

	if (ciGetParInt(cmd,1) == 1)
	{
		fprintf(msgout,"[i] %s!\n", _("Inputted commands will ignore errors"));
		femCmdDoNotBreak = AF_YES ;
		return(tuiCmdReact(cmd, rv));
	}

	if (ciGetParStr(cmd,1) != NULL)
	{
		if (strlen(var=ciGetParStr(cmd,1)) > 0)
		{
			if ((var[0] == 'y')||(var[0] == 'Y'))
			{
				fprintf(msgout,"[i] %s!\n", _("Inputted commands will ignore errors"));
				femCmdDoNotBreak = AF_YES ;
				free(var) ; var = NULL ;
				return(tuiCmdReact(cmd, rv));
			}
			else
			{
				free(var) ; var = NULL ;
			}
		}
	}

	femCmdDoNotBreak = AF_NO ;
	fprintf(msgout,"[i] %s!\n", _("Inputted commands will stop on errors"));

	return ( tuiCmdReact(cmd, rv) ) ;
}

/* end of cmd_macs.c */
