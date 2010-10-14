/*
   File name: fdb_math.c
   Date:      2009/07/28 11:09
   Author:    Jiri Brozovsky

   Copyright (C) 2009 Jiri Brozovsky

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

	 Database for FEM: matrix and vector support

   Note: only MAT_FULL and VEC_FULL are supported!
*/

#include "fem_math.h"
#include "cint.h"

#ifdef _USE_THREADS_
long      femUseThreads = AF_NO ; /* workaround for fem_math library */
long      femThreadNum  = 1 ; /* workaround for fem_math library */
long      femThreadMin  = 1 ; /* workaround for fem_math library */
#endif

long      fdb_matrix_len = 0 ;
long      fdb_vector_len = 0 ;
tMatrix  *fdb_matrix = NULL ;
tVector  *fdb_vector = NULL ;


/** Frees matrix field */
void fdbFreeMatrixSpace(void)
{
  long i ;

  for (i=0; i< fdb_matrix_len; i++)
  {
    if (fdb_matrix[i].rows > 0)
    {
      femMatFree(&fdb_matrix[i]);
    }
    else
    {
      femMatNull(&fdb_matrix[i]);
    }
  }

  free(fdb_matrix);
  fdb_matrix = NULL ;
}

/** Creates or enlarges/stretches matrix field
 * @param m_len new lenght of matrix field 
 * @return status value */
int fdbResizeMatrixSpace(long m_len)
{
  int rv = AF_OK ;
  long i ;
  tMatrix *tmp ;

  if (m_len == fdb_matrix_len) { return(rv); } /* nothing to do */

  if (m_len <= 0) /* free stuff */
  {
    fdbFreeMatrixSpace() ;
  }

  if (m_len < fdb_matrix_len) /* make it smaller */
  { 
    /* TODO */
    if ((tmp = (tMatrix *)malloc(m_len*sizeof(tMatrix))) == NULL)
    {
      fprintf(msgout,"[E] %s!\n", _("Can not change matrix space - out of memory"));
      return(AF_ERR_MEM);
    }
    for (i=0; i<m_len; i++) { femMatNull(&tmp[i]); }
    for (i=0; i<m_len; i++)
    {
      if (fdb_matrix[i].rows > 0)
      {
        femFullMatInit(&tmp[i], fdb_matrix[i].rows, fdb_matrix[i].cols);
        rv = femMatCloneDiffToEmpty(&fdb_matrix[i],&tmp[i]);
        if (rv != AF_OK)
        {
          fprintf(msgout,"[W] %s: %li!\n", _("Error when clonning matrix"),i);
          break ;
        }
      }
    }
    if (rv != AF_OK)
    {
      fdb_matrix_len = m_len ; /* some memory will be lost but OK */
    }
    else
    {
      fdbFreeMatrixSpace() ;
      fdb_matrix = tmp ;
      tmp = NULL ;
    }

    return(rv); 
  } 

  if (m_len > fdb_matrix_len) /* enlarge */
  { 
    /* TODO */
    if ((tmp = (tMatrix *)malloc(m_len*sizeof(tMatrix))) == NULL)
    {
      fprintf(msgout,"[E] %s!\n", _("Can not enlarge matrix space - out of memory"));
      return(AF_ERR_MEM);
    }
    for (i=0; i<m_len; i++) { femMatNull(&tmp[i]); }
    for (i=0; i<m_len; i++)
    {
      if (fdb_matrix[i].rows > 0)
      {
        femFullMatInit(&tmp[i], fdb_matrix[i].rows, fdb_matrix[i].cols);
        rv = femMatCloneDiffToEmpty(&fdb_matrix[i],&tmp[i]);
        if (rv != AF_OK)
        {
          fprintf(msgout,"[W] %s: %li!\n", _("Error when clonning matrix"),i);
          break ;
        }
      }
    }
    if (rv != AF_OK)
    {
      fprintf(msgout,"[E] %s!\n", _("Can not enlarge matrix space - out of memory"));

      for (i=0; i< fdb_matrix_len; i++)
      {
        if (tmp[i].rows > 0) { femMatFree(&tmp[i]); }
        else { femMatNull(&tmp[i]); }
      }

      free(tmp);
      tmp = NULL ;
      return(rv);
    }
    else
    {
      fdbFreeMatrixSpace() ;
      fdb_matrix = tmp ;
      tmp = NULL ;
    }

    return(rv); 
  } 

  return(rv);
}

int fdbMatrixPrint(tMatrix *matrix, int type, char *fname, long rewrite)
{
  long i, j ;
  char mod[1]="a" ;
  FILE *fw = NULL ;

  switch (type)
  {
    case 1:
    case 2: /* output to file - dense */
    case 3: /* output to file - sparse  */
            if (fname != NULL)
            {
              if (strlen(fname) > 1)
              {
                if (rewrite == AF_YES) {mod[0] = 'w';}
                if ((fw = fopen(fname,mod)) == NULL)
                {
                  fprintf(msgout,"[E] %s:%s!\n", _("Can not open file"),fname);
                  return(AF_ERR_IO);
                }
                break ;
              }
            } 
            fprintf(msgout,"[E] %s!\n", _("Invalid filename"));
            return(AF_ERR_VAL);
            break;
#if 0
    case 4: /* dense output - standard tgfem features */
            /* TODO */
            break;
#endif
    case 0: /* standard message output */
    default: fw = msgout ; type = 0 ; 
             break ;
  }

  if ((matrix->cols > 0)&&(matrix->rows >0))
  {
    for (i=1; i<=matrix->rows; i++)
    {
      for (j=1; j<matrix->cols; j++)
      {
        fprintf(fw," %e", femMatGet(matrix,i,j));
      }
      fprintf(msgout, "\n");
    }
  }

  if (fw != msgout) { fclose(fw); } 

  return(AF_OK);
}


/* end of fdb_math.c */
