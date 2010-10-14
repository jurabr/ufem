/*
   File name: math_mviz.c
   Date:      2007/12/06 18:38
   Author:    Jiri Brozovsky

   Copyright (C) 2007 Jiri Brozovsky

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.

   Visualization of fem_math SPARSE matrices and vectors.
*/

#include "fem_math.h"
#include <GL/glut.h>

tMatrix mat ; /* original matrix */
tMatrix mat2 ; /* original matrix */
tMatrix viz ; /* visualization data matrix (if needed) */
int    do_diff = 0 ;
int    do_over = 0 ;
int    mode = 0 ;
int    type = MAT_SPAR ;
double zoom = 1.0 ;

static int w_i    = 600 ;
static int h_i    = 600 ;
static int size_i = 600 ;

void *capFont = GLUT_BITMAP_HELVETICA_12 ;
int   capFontHeight = 12 ;

int mviz_read_matrix(char *fname, int type, tMatrix *mat)
{
  return(femSparseMatReadF(fname, mat));
}

int mviz_viz(int mode, double zoom)
{
  long i, j ;
  long sum_diffs = 0 ;
  double val  = 0.0 ;
  double mult = 1.0 ;

  if (zoom != 1.0)
  {
    /* plot subarrays */
    for (i=1; i<= viz.rows; i++)
    {
      for (j=1; j<=viz.cols; j++)
      {
        /* Plot data for [i][j] */
      }
    }
  }
  else
  {
    /* plot 1:1 */

    glPointSize(1);
    glColor4f(0.1,1.0,0.1,1.0);

    glBegin(GL_POINTS);

    for (i=0; i< mat.rows; i++)
    {
      for (j=0; j<mat.defpos[i]; j++)
      {
        val = (mat.data[j+mat.frompos[i]]) ; 
        if (fabs(val) > 0.000001)
        {
          /* plot data[j+frompos] for (i+1, pos[j+frompos]) */
          glVertex2f( (float)(mat.pos[j+mat.frompos[i]]), (float)(i+1) );
        }
      }
    }

    glEnd();


    /* DIFF ONLY: */

    if (do_diff == 1) 
    { 
      glPointSize(4);
      glBegin(GL_POINTS);
      sum_diffs = 0 ;

      for (i=0; i< mat.rows; i++)
      {
        for (j=0; j<mat.defpos[i]; j++)
        {
          val = (mat.data[j+mat.frompos[i]]) ; 
          if (fabs(val) > 0.000001)
          {
            if ( fabs(femMatGet(&mat2,i+1, mat.pos[j+mat.frompos[i]])
                  - val) > 0.0001)
            {
              sum_diffs++;
              if ( (femMatGet(&mat2,i+1, mat.pos[j+mat.frompos[i]])
                  - val) > 0.0)
                { glColor4f(1.0,0.0,0.0,1.0); }
              else 
                { glColor4f(0.0,0.0,1.0,1.0); }

              glVertex2f(
                (float)(mat.pos[j+mat.frompos[i]]),
                (float)(i+1)
              );
            }
          }
        }
      }

      fprintf(stderr,"Number of diferences: %li\n", sum_diffs);
      glEnd();
    }

    if (do_over == 1) /* matrix over matrix */
    {
      mult = ((float)mat.rows) / ((float)mat2.rows) ;
      glPointSize(1);
      glColor4f(1.0,0.1,0.1,1.0);

      glBegin(GL_POINTS);

      for (i=0; i< mat2.rows; i++)
      {
        for (j=0; j<mat2.defpos[i]; j++)
        {
          val = (mat2.data[j+mat2.frompos[i]]) ; 
          if (fabs(val) > 0.000001)
          {
            glVertex2f(
                (float)(mat2.pos[j+mat2.frompos[i]])*mult, 
                (float)(i+1)*mult
                );
          }
        }
      }

      glEnd();
    }
    
  }
  return(0);
}

void printStringXYi(int x, int y, char *str)
{
  int i ;

  glRasterPos2i(x,y);
  for (i=0; i<strlen(str); i++)
  {
    glutBitmapCharacter(capFont,str[i]);
	}
}


void onDisplay(void)
{
  int rows ;

  if (zoom == 1.0)
  {
    rows = mat.rows;
  }
  else
  {
    rows = viz.rows;
  }

  
  glViewport(5, 5, size_i-10, size_i-10);

  glClearColor(0.6,0.6,0.6,1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, rows, 0, rows, -1, 1);
  glScalef(1, -1, 1);
  glTranslatef(0, -rows, 0);
    
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

 /* Matrix limits: */
  glColor4f(0.3,0.3,0.3,1);
	glBegin(GL_LINE_LOOP);
		glVertex2f(0.0,0.0);
		glVertex2f(rows,0.0);
		glVertex2f(rows,rows);
		glVertex2f(0.0,rows);
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex2f(0.0,0.0);
		glVertex2f(rows,rows);
	glEnd();
    
  mviz_viz(mode, zoom);
#if 0
  glFlush();
#else
  glutSwapBuffers();
#endif

  /* some informational code should be here */
}

void onResize(int w, int h)
{
  int size ;
  
  if (w > h)
  {
    size = h ;
  }
  else
  {
    size = w ;
  }

  w_i = w ; 
  h_i = h ;
  size_i = size ;
  
  /* onDisplay(); */
  glutPostRedisplay();
}

void onKeyboard(unsigned char key, int x, int y)
{
  key=(key>'A' && key<='Z') ? key+'a'-'A':key; /* conversion to lower case */

  switch (key)
  {
    case 'w':
      glutPositionWindow(90, 60);
      glutReshapeWindow(600, 600); 
      break ;
    case 'f': glutFullScreen(); break ;
    case 27 :
    case 'q': exit(0); break ;
  }
}

int main(int argc, char *argv[])
{
  char titlestr[2048];
  int i;

  for (i=0; i<2048; i++){titlestr[i]='\0';}

  femMatNull(&mat) ; 
  femMatNull(&mat2) ; 
  femMatNull(&viz) ; 

  fprintf(stderr,"\n\nMatViz: sparse matrix viewer for uFEM\n\n  (C) Jiri Brozovsky, 2008\n\n");
  
  if (argc < 2)
  {
    fprintf(stderr,"Usage: %s filename - exiting!\n",argv[0]);
    exit(-1);
  }

  fprintf(stderr,"Reading of data: \n");
  if (mviz_read_matrix(argv[1], type, &mat) != 0)
  {
    fprintf(stderr,"Invalid input data - exiting!\n");
    exit(-1);
  }
  fprintf(stderr,"Reading of \"%s\" is finished:\n",argv[1]);

  fprintf(stderr,"  Rows:     %li\n", mat.rows) ;
  fprintf(stderr,"  Collumns: %li\n", mat.cols) ;

  if (argc > 2)
  {
    if (strlen(argv[2]) > 0)
    {
      if (argv[2][0] != '-')
      {
        /* OK, we probably have to do "diff" from two matrices */
        fprintf(stderr,"Reading of second matrix data: \n");
          if (mviz_read_matrix(argv[2], type, &mat2) != 0)
          {
            fprintf(stderr,"Invalid input data - exiting!\n");
            do_diff = 0 ;
          }
          do_diff = 1 ;
          fprintf(stderr,"Reading of \"%s\" is finished:\n",argv[1]);

          fprintf(stderr,"  Rows:     %li\n", mat.rows) ;
          fprintf(stderr,"  Collumns: %li\n", mat.cols) ;

          if ((mat.rows != mat2.rows) || (mat.cols != mat2.cols))
          {
            fprintf(stderr,"Matrices with different sizes - no comparison will be done!\n");
            do_diff = 0 ;
            do_over = 1 ;
          }
      }
    } 
  }

  glutInit(&argc, argv);
  if (do_diff == 1)
  {
    snprintf(titlestr,2048,"MatViz 0.0.2: %s <> %s",argv[1],argv[2]);
  }
  else
  {
    snprintf(titlestr,2048,"MatViz 0.0.2: %s",argv[1]);
  }

  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(600, 600);
  glutInitWindowPosition(90, 60);

  glutCreateWindow(titlestr);

  glutDisplayFunc(onDisplay);
  glutReshapeFunc(onResize);
  glutKeyboardFunc(onKeyboard);

  glutMainLoop();

  femMatFree(&mat) ; 
  femMatFree(&mat2) ; 
  femMatFree(&viz) ; 
  return(0);
}

/* end of math_mviz.c */
