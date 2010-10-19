/*
   File name: cmd_vars.c
   Date:      2003/11/18 21:31
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

	 Variable handling commands
   
	 $Id: cmd_vars.c,v 1.1 2003/11/18 21:53:53 jirka Exp $
*/

#include "fem_tui.h"
#include "cint.h"
#include "fdb_res.h"
#include "fdb_edef.h"

/** Defines/changes variable "setvar,name,value"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_var_add (char *cmd)
{
	int    rv    = AF_OK ;

	if (ciParNum(cmd) < 3) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Name and value required")) ;
		return(AF_ERR_VAL);
	}

	rv = ciAddVar( ciGetParStrNoExpand(cmd, 1), ciGetParStr(cmd, 2) );

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Defines/changes variable "genvar,name,value"
 * Note: the name can be a variable (variable value will be used)
 * @param cmd command
 * @return status
 */
int func_var_gen_add (char *cmd)
{
	int    rv    = AF_OK ;

	if (ciParNum(cmd) < 3) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Name and value required")) ;
		return(AF_ERR_VAL);
	}

	rv = ciAddVar( ciGetParStr(cmd, 1), ciGetParStr(cmd, 2) );

	return ( tuiCmdReact(cmd, rv) ) ;
}



/** Unsets variable "unsetvar,name"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_var_del (char *cmd)
{
	int    rv    = AF_OK ;

	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Variable name required")) ;
		return(AF_ERR_VAL);
	}

	rv = ciDelVar( ciGetParStr(cmd, 1) );

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Prints variable "printvar,name"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_var_prn (char *cmd)
{
	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Variable name required")) ;
		return(AF_ERR_VAL);
	}

  fprintf(msgout, "%s\n", ciGetParStr(cmd, 1) );

  return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** Prints all variables "listvar"
 * Note: no path or extension addition/substitution is done
 * @param cmd command
 * @return status
 */
int func_var_dump (char *cmd)
{
  int rv = AF_OK ;

  rv = ciListVars(msgout, 1) ;
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Starts command history "history,n" (n=0 means history is halted)
 * @param cmd command
 * @return status
 */
int func_cihs_init (char *cmd)
{
  int rv = AF_OK ;
  long num = 0 ;

	if (ciParNum(cmd) < 2) 
  {
    if (ciHistory != NULL)
    {
      ciCmdHsFree(); /* no parameters: stop history */
    }
  }
  else
  {
    if((num = ciGetParInt(cmd,1)) <=0)
    {
      if (ciHistory != NULL) /* zero: stop history */
      {
        ciCmdHsFree();
      }
    }
    else
    {
      if (ciHistory != NULL)
      {
        ciCmdHsFree();
      }
      if ((rv=ciCmdHsAlloc(num)) != AF_OK)
      {
        fprintf(msgout,"[E] %s!\n",
            _("Can not start command history tool"));
      }
      else
      {
        fprintf(msgout,"[i] %s: %li\n",
            _("History activated with given number of records")
            ,num);
      }
    }
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Runs last n-th command from history buffer "last,n"
 * @param cmd command
 * @return status
 */
int func_cihs_last (char *cmd)
{
  int rv = AF_OK ;
  long num = 0 ;

  if (strcmp(ciGetParStr(cmd,0),"last") != 0) 
  {
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

	if (ciParNum(cmd) < 2) 
  {
    num = 1 ;
  }
  else
  {
    if((num = ciGetParInt(cmd,1)) <=0)
    {
      num = 1 ;
    }
  }

  rv = ciRunCmd(ciCmdHsGet(num));

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Writes current (TUI only) config to a selected file: "writeconf,file"
 * @param cmd command
 * @return status
 */
int func_var_write_conf_file (char *cmd)
{
	int    rv    = AF_OK ;

	if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Name of file required")) ;
		return(AF_ERR_VAL);
	}

  if (ciGetParStr(cmd,1) == NULL)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty filename not allowed")) ;
		return(AF_ERR_VAL);
  }

  if (strlen(ciGetParStr(cmd,1)) < 1)
  {
		fprintf(msgout,"[E] %s!\n", _("Filename too short")) ;
		return(AF_ERR_VAL);
  }

	rv = femUIWriteConfigFile(ciGetParStr(cmd,1)) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Writes reaction sum and displacement in selected node and dir. to file: "writentab,file,node,dof"
 * @param cmd command
 * @return status
 */
int func_var_write_rawres_dof(char *cmd)
{
	int    rv = AF_OK ;
	FILE  *fw = NULL ;
	long   n_id, dof_type ;

	if (ciParNum(cmd) < 4) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("All paramerers are required: filename, node, DOF")) ;
		return(AF_ERR_VAL);
	}

	if (ciGetParStr(cmd,1) == NULL)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty filename not allowed")) ;
		return(AF_ERR_VAL);
  }
	if (strlen(ciGetParStr(cmd,1)) <= 0)
  {
		fprintf(msgout,"[E] %s!\n", _("Empty filename not allowed")) ;
		return(AF_ERR_VAL);
  }

	if ((n_id = ciGetParInt(cmd,2)) <= 0) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid node")) ;
		rv = AF_ERR_VAL; 
		goto memFree;
	}

	dof_type = ciGetParInt(cmd,3);
	
	if ((dof_type <= 0) || (dof_type > KNOWN_DOFS)) 
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid result type")) ;
		rv = AF_ERR_VAL; 
		goto memFree;
	}

	if ((fw = fopen(ciGetParStr(cmd,1), "w")) == NULL) {rv = AF_ERR_IO; goto memFree;}

	rv =  fdbGetLoopRawRes_DOF(fw, n_id, dof_type);

	if (fclose(fw) != 0) {rv = AF_ERR_IO;}
memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}


/* ** MATRIX OPERATIONS: ** */

/** Allocates new matrix: "matrix,id,rows[,cols]"
 * @param cmd command
 * @return status
 */
int func_mat_new_matrix(char *cmd)
{
	int    rv = AF_OK ;
  long   id = -1 ;
  long   cols = 0 ;
  long   rows = 0 ;
  long   oldlen=0 ;
  
  if (ciParNum(cmd) < 3) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("These parameters are necessary: matrix number, rows, cols")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

  id   = ciGetParInt(cmd, 1) ;
  rows = ciGetParInt(cmd, 2) ;
  if (ciParNum(cmd) > 3) {cols = ciGetParInt(cmd, 3);}
  if ((cols <= 0) && (rows <= 0))
  {
		fprintf(msgout,"[E] %s: %lix%li!\n", _("Invalid matrix dimensions"),rows, cols) ;
	  return (tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  if (cols <1) {cols = 1;}
  if (rows <1) {rows = 1;}

  if (id < 0)
  {
		fprintf(msgout,"[E] %s: %li!\n", _("Invalid matrix identifier"),id) ;
	  return (tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  if (id == 0) /* new matrix */
  {
    if ((rv = fdbResizeMatrixSpace(fdb_matrix_len+1)) == AF_OK)
    {
      if ((rv = femFullMatInit(&fdb_matrix[fdb_matrix_len], cols, rows)) == AF_OK)
      {
        fprintf(msgout, "[i] %s[%li,%li]: %li\n",_("New matrix created"),rows, cols, fdb_matrix_len+1);
      }
      else
      {
        fdb_matrix_len-- ;
        fprintf(msgout,"[E] %s!\n", _("Unable to create new matrix"));
      }
    }
    else
    {
      fprintf(msgout,"[E] %s!\n", _("Unable to create new matrix"));
    }
  }
  else
  {
    if (id <= fdb_matrix_len) /* existing matrix */
    {
      if ((fdb_matrix[id-1].rows == rows) && (fdb_matrix[id-1].cols == cols))
      {
        /* OK, no change */
        femMatSetZero(&fdb_matrix[id-1]);
        rv = AF_OK ;
      }
      else
      {
        /* new dimensions + matrix erase */
        femMatFree(&fdb_matrix[id-1]);
        rv = femFullMatInit(&fdb_matrix[id-1],rows,cols);
      }

    }
    else /* new matrix */
    {
      /* TODO */
      oldlen = fdb_matrix_len ;
      if ((rv = fdbResizeMatrixSpace(id)) == AF_OK)
      {
        if ((rv = femFullMatInit(&fdb_matrix[id-1], cols, rows)) == AF_OK)
        {
          fprintf(msgout, "[i] %s[%li,%li]: %li\n",_("New matrix created"),rows, cols, fdb_matrix_len+1);
        }
        else
        {
          fdb_matrix_len = oldlen ;
          fprintf(msgout,"[E] %s!\n", _("Unable to create new matrix"));
        }
      }
      else
      {
        fprintf(msgout,"[E] %s!\n", _("Unable to create new matrix"));
      }
    }
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Frees all matrices: "nomatrix"
 * @param cmd command
 * @return status
 */
int func_mat_free_matrix_space(char *cmd)
{
  fdbFreeMatrixSpace();
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** Lists matrix: "varmatlist,id"
 * @param cmd command
 * @return status
 */
int func_mat_list_matrix(char *cmd)
{
  long id, cols, rows ;

  if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Matrix must be specified")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}
  id = ciGetParInt(cmd, 1) ;

  if ((id <= 0)|| (id >= fdb_matrix_len))
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid matrix identifier")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  
  id-- ;

  rows = fdb_matrix[id].rows  ;
  cols = fdb_matrix[id].cols  ;
  
  if ((fdb_matrix[id].cols > 0)&&(fdb_matrix[id].rows >0))
  {
    fprintf(msgout,"[i] %s [%s] %lix%li:\n",_("Matrix"),ciGetParStr(cmd,1),cols,rows);
    fdbMatrixPrint(&fdb_matrix[id], 0, NULL, AF_NO) ;
  }
  else
  {
		fprintf(msgout,"[E] %s!\n", _("Empty or invalid matrix")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** Writes matrix to file: "varmatwrite,id,file"
 * @param cmd command
 * @return status
 */
int func_mat_write_matrix(char *cmd)
{
  long id, cols, rows ;

  if (ciParNum(cmd) < 2) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Matrix must be specified")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}
  id = ciGetParInt(cmd, 1) ;

  if ((id <= 0)|| (id >= fdb_matrix_len))
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid matrix identifier")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  
  id-- ;

  rows = fdb_matrix[id].rows  ;
  cols = fdb_matrix[id].cols  ;
  
  if ((fdb_matrix[id].cols > 0)&&(fdb_matrix[id].rows >0))
  {
    fprintf(msgout,"[i] %s [%s] %lix%li:\n",_("Matrix"),ciGetParStr(cmd,1),cols,rows);
    fdbMatrixPrint(&fdb_matrix[id], 0, NULL, AF_NO) ;
  }
  else
  {
		fprintf(msgout,"[E] %s!\n", _("Empty or invalid matrix")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}


/** Puts data to matrix: "varmatput,id,row,col,value"
 * @param cmd command
 * @return status
 */
int func_mat_matrix_put(char *cmd)
{
  int rv = AF_OK ;
  long id, col, row ;
  double value = 0.0 ;

  if (ciParNum(cmd) < 4) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Must specify: matrix,row,collumn,value")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}
  id    = ciGetParInt(cmd, 1) ;
  row   = ciGetParInt(cmd, 2) ;
  col   = ciGetParInt(cmd, 3) ;
  value = ciGetParDbl(cmd, 4) ;

  

  if ((id <= 0)|| (id >= fdb_matrix_len))
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid matrix identifier")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  
  id-- ;
  
  if ((fdb_matrix[id].cols>0)&&(fdb_matrix[id].rows>0)&&(fdb_matrix[id].rows<=row)&&(fdb_matrix[id].cols<=col)&&(col>1)&&(row>1))
  {
    rv = femMatPut(&fdb_matrix[id], row, col, value) ;
  }
  else
  {
		fprintf(msgout,"[E] %s!\n", _("Empty or invalid matrix")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Adds data to matrix: "varmatadd,id,row,col,value"
 * @param cmd command
 * @return status
 */
int func_mat_matrix_add(char *cmd)
{
  int rv = AF_OK ;
  long id, col, row ;
  double value = 0.0 ;

  if (ciParNum(cmd) < 4) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Must specify: matrix,row,collumn,value")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}
  id    = ciGetParInt(cmd, 1) ;
  row   = ciGetParInt(cmd, 2) ;
  col   = ciGetParInt(cmd, 3) ;
  value = ciGetParDbl(cmd, 4) ;

  

  if ((id <= 0)|| (id >= fdb_matrix_len))
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid matrix identifier")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  
  id-- ;
  
  if ((fdb_matrix[id].cols>0)&&(fdb_matrix[id].rows>0)&&(fdb_matrix[id].rows<=row)&&(fdb_matrix[id].cols<=col)&&(col>1)&&(row>1))
  {
    rv = femMatAdd(&fdb_matrix[id], row, col, value) ;
  }
  else
  {
		fprintf(msgout,"[E] %s!\n", _("Empty or invalid matrix")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Adds data to matrix: "varmatget,variable,id,row,col,value"
 * @param cmd command
 * @return status
 */
int func_mat_matrix_get(char *cmd)
{
  int rv = AF_OK ;
  long id, col, row ;
  double result = 0.0 ;
	char  *var = NULL ;
	char  sres[2048];

  if (ciParNum(cmd) < 4) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Must specify: variable,matrix,row,collumn,value")) ;
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
  
  id    = ciGetParInt(cmd, 2) ;
  row   = ciGetParInt(cmd, 3) ;
  col   = ciGetParInt(cmd, 4) ;
  

  if ((id <= 0)|| (id >= fdb_matrix_len))
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid matrix identifier")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  
  id-- ;
  
  if ((fdb_matrix[id].cols>0)&&(fdb_matrix[id].rows>0)&&(fdb_matrix[id].rows<=row)&&(fdb_matrix[id].cols<=col)&&(col>1)&&(row>1))
  {
    result = femMatGet(&fdb_matrix[id], row, col) ;
	  sprintf(sres,"%e",result);
	  ciAddVar(var, sres);
  }
  else
  {
		fprintf(msgout,"[E] %s!\n", _("Empty or invalid matrix")) ;
    rv = AF_ERR_VAL ;
  }

	free(var); var = NULL ;
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Reads matrix from file (matrix must be allocated first!): "varmatread,id,fname"
 * @param cmd command
 * @return status
 */
int func_mat_read_matrix(char *cmd)
{
  int rv = AF_OK ;
  long id ;
  long i, j ;
  double val = 0.0 ;
  FILE *fw = NULL ;

  if (ciParNum(cmd) < 3) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Matrix must be specified")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}
  id = ciGetParInt(cmd, 1) ;

  if ((id <= 0)|| (id >= fdb_matrix_len))
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid matrix identifier")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
  
  id-- ;
  
  if ((fdb_matrix[id].cols > 0)&&(fdb_matrix[id].rows >0))
  {
    femMatSetZero(&fdb_matrix[id]) ;

    rv = AF_ERR_IO ;
    if (ciGetParStr(cmd,2) != NULL)
    {
      if (strlen(ciGetParStr(cmd,2)) > 1)
      {
        if ((fw=fopen(ciGetParStr(cmd,2),"r")) != NULL)
        {
          for (i=1; i<=fdb_matrix[id].rows; i++)
          {
            for (j=1; j<=fdb_matrix[id].cols; j++)
            {
              if (fscanf(fw,"%lf", &val) <= 0) 
              {
                fprintf(msgout,"[E] %s!\n",_("I/O error when reading matrix"));
                fclose(fw);
                femMatSetZero(&fdb_matrix[id]) ;
	              return ( tuiCmdReact(cmd, AF_ERR_IO) ) ;
              }
              femMatPut(&fdb_matrix[id],i,j, val);
            }
          }
          rv = AF_OK ;
          fclose(fw);
        }
      }
    }
  }
  else
  {
		fprintf(msgout,"[E] %s!\n", _("Empty or invalid matrix")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }
	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Adds result data to matrix: "varmatfillres,id,elem,ipoint,res1[,res2,res3,...,res6]"
 * @param cmd command
 * @return status
 */
int func_mat_matrix_fill_res(char *cmd)
{
  int rv = AF_OK ;
  long id ;
  double res[101] ;
  long elem = 0 ;
  long etype = 0 ;
  long ipoint = 0 ;
  long e_pos ;
  long i, j ;
  int cols, rows ;
  double val ;

	FEM_TEST_POSTPROCESSOR

  /* TODO  - CHYBI NACIT ELEMET, IPOINT !!!!!!!!!!! */ 

  if (ciParNum(cmd) < 5) 
	{ 
		fprintf(msgout,"[E] %s!\n", _("Must specify at least: matrix,time,result")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
	}

  rows = ResLen ;
  cols = ciParNum(cmd) - 4 ;
  if (cols > 100) {cols = 100 ;}

  id    = ciGetParInt(cmd, 2) ;

  if (id <1) 
  {
		fprintf(msgout,"[E] %s!\n", _("Invalid matrix identifier")) ;
	  return ( tuiCmdReact(cmd, AF_ERR_VAL) ) ;
  }

  if (id >= fdb_matrix_len)
  {
    if ((rv = fdbResizeMatrixSpace(id)) == AF_OK)
    {
      fprintf(msgout,"[E] %s!\n", _("Unable to create new matrix"));
      goto memFree;
    }
  }
  
  id-- ;

  elem    = ciGetParInt(cmd, 3) ;
  ipoint  = ciGetParInt(cmd, 4) ;

  if ((fdbInputCountInt(ELEM, ELEM_ID, elem, &e_pos)) < 1)
  {
    rv = AF_ERR_VAL ;
    fprintf(msgout,"[E] %s!\n", _("Invalid element"));
    goto memFree;
  }

  etype = fdbInputGetInt(ETYPE, ETYPE_TYPE, e_pos);

  if ((rows != fdb_matrix[id].rows) ||(cols != fdb_matrix[id].cols))
  {
    femMatSetZero(&fdb_matrix[id]) ;
    if ((rv = femFullMatInit(&fdb_matrix[id], cols, rows)) != AF_OK)
    {
      fprintf(msgout,"[E] %s!\n", _("Unable to create new matrix"));
      goto memFree;
    }
  }

  if (ipoint >  fdbElementType[etype].res_rp)
  {
    rv = AF_ERR_VAL ;
    fprintf(msgout,"[E] %s!\n", _("Invalid element point"));
    goto memFree;
  }


  for (j=1;j<cols; i++) { res[j]=  ciGetParInt(cmd, 4+j) ; }

  for (i=0; i<ResLen; i++)
  {
    femMatPut(&fdb_matrix[id], i+1, 1, ResElem[i].set_id) ;
    for (j=1;j<cols; i++)
    {
	    val = fdbResElemGetVal(e_pos, etype, res[j], ipoint) ;
      femMatPut(&fdb_matrix[id], i+1, j, val) ;
    }
  }


memFree:
	return ( tuiCmdReact(cmd, rv) ) ;
}


/* end of cmd_vars.c */
