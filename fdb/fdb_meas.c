/*
   File name: fdb_meas.c
   Date:      2004/10/03 11:00
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

	 FEM Postprocessor - some useless measurements:

	 Tensor scale:
	 Saha P.K., Wehrli, F.W.: A robust method for measuring trabecular
	 bone orienatation anisotropy at in vivo resolution using tensor scale,
	 "Pattern Recognition" - The Journal of the Pattern Recognition Society,
	 Vol. 37 (2004) pp 1935-1944, Pergamon

	 $Id: fdb_meas.c,v 1.4 2005/07/11 17:57:02 jirka Exp $
*/

#include "fdb_fem.h"
#include "cint.h"
#include <math.h>

/* Tensor scale measurement in 2D ----------------------- */

long meas_plane = 1 ;  /* plane, XY is default */
long x_dir = NODE_X ;  /* "X" direction */
long y_dir = NODE_Y ;  /* "Y" direction */
long z_dir = NODE_Z ;  /* "Z" direction */
char x_name[] = "x" ;
char y_name[] = "y" ;
char z_name[] = "z" ;

long   number_of_lines = 12 ; /* number of lines */
double angle_delta     = 0  ; /* angle between lines */
long   test_points     = 0  ; /* number of testing points on each line */

double  ray_rad  = 1.0 ;  /* radius of circle - lenght of lines */

double  x_p      = 0.0 ;  /* center of circle */
double  y_p      = 0.0 ;
double  z_p      = 0.0 ;

double *rad_comp = NULL ; /* points computed on structure */
double *ang_comp = NULL ;
double *x_mod    = NULL ; /* modified points */
double *y_mod    = NULL ;

double  a ; /* ellipse properties - results */
double  b ;
double  e_angle ;

double *x_gl ;
double *y_gl ;
double *rad_gl ;


/* Tensor scale measurement in 2D ----------------------- */

/** Translates cyllindrical coordinates (r,angle) to cartessian ones
 * @param r radius
 * @param angle angle
 * @param x pointer to x (result)
 * @param y pointer to y (result)
 */
void fdbTS2D_cyl2xy(double r, double angle, double *x, double *y)
{
	*x = x_p + ( r * cos(angle) ) ;
	*y = y_p + ( r * sin(angle) ) ;
}


/** Translates cyllindrical coordinates (r,angle) to cartessian ones
 * @param px x-coordinate of center
 * @param py y-coordinate of center
 * @param r radius
 * @param angle angle
 * @param x pointer to x (result)
 * @param y pointer to y (result)
 */
void fdbTS2D_cyl2xy_center(double px, double py, double r, double angle, double *x, double *y)
{
	*x = px + ( r * cos(angle) ) ;
	*y = py + ( r * sin(angle) ) ;
}

/** Sets plane where measurement will be done - XY is default
 * @param plane plane: 1=XY, 2=YZ, 3=ZX
 * @return status
 */
int fdbTS2D_set_plane(long plane)
{
	switch (plane)
	{
		case 1: /* XY */
						meas_plane = 1 ;
						x_dir = NODE_X ;
						y_dir = NODE_Y ;
						z_dir = NODE_Z ;
						x_name[0] = 'x';
						y_name[0] = 'y';
						z_name[0] = 'z';
						break ;
		case 2: /* YZ */
						meas_plane = 2 ;
						x_dir = NODE_Y ;
						y_dir = NODE_Z ;
						z_dir = NODE_X ;
						x_name[0] = 'y';
						y_name[0] = 'z';
						z_name[0] = 'x';
						break ;
		case 3: /* ZX */
						meas_plane = 3 ;
						x_dir = NODE_Z ;
						y_dir = NODE_X ;
						z_dir = NODE_Y ;
						x_name[0] = 'z';
						y_name[0] = 'x';
						z_name[0] = 'y';
						break ;
		default:/* default - XY */
						meas_plane = 1 ;
						x_dir = NODE_X ;
						y_dir = NODE_Y ;
						z_dir = NODE_Z ;
						x_name[0] = 'x';
						y_name[0] = 'y';
						z_name[0] = 'z';
						return(AF_ERR_VAL);
						break ;
	}

	return(AF_OK);
}

/** sets all data to NULL or 0 */
void fdbTS2D_null(void)
{
	number_of_lines = 0 ;
  angle_delta     = 0 ;
  test_points     = 0 ;

  ray_rad  = 0.0 ;

  x_p      = 0.0 ;
  y_p      = 0.0 ;

  rad_comp = NULL ;
  ang_comp = NULL ;
  x_mod    = NULL ;
  y_mod    = NULL ;

  a       = 0.0 ; 
  b       = 0.0 ;
	e_angle = 0.0 ;
}

/** frees fields */
void fdbTS2D_free(void)
{
  free(rad_comp) ;
  free(ang_comp) ;
  free(x_mod) ;
  free(y_mod) ;

	fdbTS2D_null();
}

/** Prepares data for computations
 * @param px  x-coordinate of center
 * @param py  y-coordinate of center
 * @param pz  z-coordinate of center
 * @param lines number of lines
 * @param radius lenght of each line (==radius of circle)
 * @param linediv number of line divisions (==testing points on line)
 * @return status
 */
int fdbTS2D_init(double px, double py, double pz, long lines, double radius, long linediv)
{
	div_t res ;

	fdbTS2D_null() ;

	/* center of ellipse position */
	switch (meas_plane)
	{
		case 1:
						x_p = px ;
						y_p = py ;
						z_p = pz ;
						break ;
		case 2:
						x_p = py ;
						y_p = pz ;
						z_p = px ;
						break ;
		case 3:
						x_p = pz ;
						y_p = px ;
						z_p = py ;
						break ;
	}

	/* number of lines: */
	res = div(lines, 2);

	number_of_lines = (long)(res.quot * 2) ;

	if (number_of_lines < 2)
	{
		fprintf(msgout,"[E] %s\n",_("Number of lines is too small"));
		return(AF_ERR_VAL);
	}

	if (res.rem != 0)
	{
		fprintf(msgout,"[w] %s: %li\n",
				_("Number of lines can't be odd. Modified to:"),
				number_of_lines);
	}

	/* angle between lines: */
	angle_delta = 2.0*FEM_PI / (double)number_of_lines ;

	/* radius of circle */
	ray_rad = radius ;

	/* number of testing points on each line */
	test_points = linediv ;

	if (test_points < 1) {test_points = 1 ; }

	if ((rad_comp = (double *)malloc(lines*sizeof(double))) == NULL) { goto memFree ; }
	if ((ang_comp = (double *)malloc(lines*sizeof(double))) == NULL) { goto memFree ; }
	if ((x_mod = (double *)malloc(lines*sizeof(double))) == NULL) { goto memFree ; }
	if ((y_mod = (double *)malloc(lines*sizeof(double))) == NULL) { goto memFree ; }

	return(AF_OK);
memFree:
	fprintf(msgout,"[E] %s!\n", _("Out of memory"));
	fdbTS2D_free();
	return(AF_ERR_MEM);
}

/** Tests elements in given direction for material type
 *  @param x x-coordinate of tested point
 *  @param y y-coordinate of tested point
 *  @param z z-coordinate of tested point
 *  @param mat pointer to material type (result, 0 if none found)
 */
void fdbTS2D_mat_test(double x, double y, double z, long *mat)
{
	long i, len ;
	long cmd_len=2048 ;
	char cmd[2048] ;
	double size ;
	double from, to ;


	size = 0.5*(ray_rad / (double)test_points) ;

	for (i=0; i<cmd_len; i++) { cmd[i] = '\0' ; }
	from = x-size ;
	to   = x+size ;
	sprintf(cmd, "esel,s,loc,%s,%e,%e",x_name,from,to);
	ciRunCmd(cmd) ;

	for (i=0; i<cmd_len; i++) { cmd[i] = '\0' ; }
	from = y-size ;
	to   = y+size ;
	sprintf(cmd, "esel,r,loc,%s,%e,%e",y_name,from,to);
	ciRunCmd(cmd) ;

	for (i=0; i<cmd_len; i++) { cmd[i] = '\0' ; }
	from = z-size ;
	to   = z+size ;
	sprintf(cmd, "esel,r,loc,%s,%e,%e",z_name,from,to);
	ciRunCmd(cmd) ;

	len = fdbInputTabLenAll(ELEM) ;

	*mat = 0 ;

	for (i=0; i<len; i++)
	{
		if (fdbInputTestSelect(ELEM, i) == AF_YES)
		{
			*mat = fdbInputGetInt(ELEM, ELEM_MAT, i) ;
			break ;
		}
	}

	ciRunCmd("esel,all") ;
}

/** Computes edge points
 * @return status
 */
int fdbTS2D_do_tests(long *mat_center)
{
	long i, j ;
	long mat_type_base ;
	long mat_type ;
	double rad_len, angle ;
	double x,y ;

	fdbTS2D_mat_test(x_p, y_p, z_p, &mat_type_base);

	*mat_center = mat_type_base ;

	if (mat_type_base == 0)
	{
		fprintf(msgout, "[E] %s!\n", _("No valid material found at center point"));
		return(AF_ERR_VAL);
	}

	for (i=0; i<number_of_lines; i++)
	{
		angle   = (double)i * angle_delta ;
		ang_comp[i] = angle ;

/*printf("angle[%li * %e(%e)] = %e\n", i,angle_delta,2.0*FEM_PI, ang_comp[i]);*/
		
		for (j=1; j<=(test_points+1); j++)
		{
			rad_len = ((double)j/(double)test_points) * ray_rad ;

			fdbTS2D_cyl2xy(rad_len, angle, &x, &y) ;

			fdbTS2D_mat_test(x, y, z_p, &mat_type);

/*printf("[%li,%li]: %li <> %li(base)\n",i,j, mat_type,mat_type_base);*/

			/* if found the different material or are outside structure: */
			if ((mat_type != mat_type_base)||(mat_type == 0)||(j==(test_points+1)))
			{
				rad_comp[i] = rad_len ;
				break ; /* break "j" and continue in "i" */
			}
		}
	}

	return(AF_OK);
}


/** Modifies edge points according to ellipse requirements
 * @return status
 */
int fdbTS2D_process_points(void)
{
	long i ;
	long half ;
	double radius ;
	double x1, y1, x2, y2 ;

	half = (long)(number_of_lines/2) ;

	for (i=0; i<half; i++)
	{
		/* selecting of smaller radius */
		if (rad_comp[i] < rad_comp[i+half])
		{
			radius = rad_comp[i] ;
		}
		else
		{
			radius = rad_comp[i+half] ;
		}

		fdbTS2D_cyl2xy(radius, ang_comp[i], &x1, &y1) ;
		fdbTS2D_cyl2xy(radius, ang_comp[i+half], &x2, &y2) ;

/*printf("phi=(%e,%e)[%e,  %e,%e]\n",x_p,y_p, radius,ang_comp[i],ang_comp[half+i]);*/
/*printf("x=[%e,%e,%e,%e]\n", x1,y1,x2,y2);*/

		x_mod[i] = x1 ;
		y_mod[i] = y1 ;

		x_mod[i+half] = x2 ;
		y_mod[i+half] = y2 ;
	}

	return(AF_OK);
}

/** Computes tensor scale parameters (axes of ellipse)
 * @param x x-coordinates
 * @param y y-coordinates
 * @param len lenght of x and y
 * @param a pointer to the "a" values (result - axis size)
 * @param b pointer to the "b" values (result - axis size)
 * @return status
 */
int fdbTS2D_compute_a_b(double *x, double *y, long len, double *a, double *b)
{
	long   i ;
	double x4   = 0.0 ;
	double y4   = 0.0 ;
	double x2   = 0.0 ;
	double y2   = 0.0 ;
	double x2y2 = 0.0 ;
	double a_down, a_top ;
	double b_down, b_top ;

	*a = 0.0 ;
	*b = 0.0 ;

	for (i=0; i<len; i++)
	{
		x2   += pow(x[i],2) ;
		x4   += pow(x[i],4) ;

		y2   += pow(y[i],2) ;
		y4   += pow(y[i],4) ;

		x2y2 += (pow(x[i],2)*pow(y[i],2)) ;
	}

	a_top  = (x2y2*x2y2) - (x4*y4) ;
	a_down = (x2y2*y2)   - (x2*y4) ;

	if ((a_down == 0.0) || ((a_down*a_top) < 0.0))
	{
		fprintf(msgout,"[E] %s: a =%f / %f!\n",
				_("Computation of tensor scale failed"),
			 	a_top, a_down);
	}

	b_top  = (x2y2*x2y2) - (x4*y4) ;
	b_down = (x2*x2y2)   - (x4*y2) ;

	if ((b_down == 0.0) || ((b_down*a_top) < 0.0))
	{
		fprintf(msgout,"[E] %s: b =%f / %f!\n",
				_("Computation of tensor scale failed"),
			 	a_top, a_down);
	}

/*printf("\na_top=%e\na_down=%e\nb_top=%e\nb_down=%e\n", a_top, a_down, b_top, b_down);*/

	*a = sqrt ( a_top / a_down ) ;
	*b = sqrt ( b_top / b_down ) ;

	return(AF_OK);
}


int fdbTS2D_compute_angle(double xp, 
												  double yp, 
													double *x, 
													double *y, 
													long    len, 
													double *angle)
{
	double m11 = 0.0 ;
	double m12 = 0.0 ;
	double m21 = 0.0 ;
	double m22 = 0.0 ;
	double n11, n12, n22 ;
	double r1, r2, rs1, rs2 ;
	double lambda, lambda_small, lambda1, lambda2, q1,q2 ;
	long   i ;
  
	/* covariance matrix */
	
	for (i=0; i<len; i++)
	{
		m11 += pow((x[i] - xp),2) ;
		m22 += pow((y[i] - yp),2) ;

		m12 += (pow((y[i] - yp),2)*pow((y[i] - yp),2)) ;
	}

	m11 = m11 / len ;
	m22 = m22 / len ;
	m12 = m12 / len ;
	m21 = m12 ;

	/* eigen values: */
	q1 = (m11+m22) ;
	q2 = ( pow((m11+m22),2) + 4*m12*m21 ) ;
	lambda1 = 0.5 * (q1 + q2 ) ;
	lambda2 = 0.5 * (q1 - q2 ) ;

	/* getting larger one: */
	if (lambda2 > lambda1) 
	{ 
		lambda       = lambda2 ; 
		lambda_small = lambda1 ; 
	}
	else                   
	{ 
		lambda       = lambda1 ; 
		lambda_small = lambda2 ; 
	}

	/* eigenvector for larger eigenvalue */
	n11 = m11 - lambda ;
	n22 = m22 - lambda ;
	n12 = m12 ;

	r1 = (n11*n22)/n12 - n12 ;
	r2 = (-1.0)* (n11/n12) ;

	/* eigenvector for smaller eigenvalue */
	n11 = m11 - lambda_small ;
	n22 = m22 - lambda_small ;
	n12 = m12 ;

#if 0
	rs1 = (n11*n22)/n12 - n12 ;
	rs2 = (-1.0)* (n11/n12) ;
#else
	rs1 = 1 ;
	rs2 = 0 ;
#endif

printf("r=[%e,%e], rs=[%e,%e]\n",r1,r2,rs1,rs2);

	*angle = acos (
		 	(r1*rs1 + r2*rs2) / ( sqrt(r1*r1 + r2*r2) * sqrt(rs1*rs1 + rs2*rs2) )
			) ;

	return(AF_OK);
}


/** Computes tensor scale in 2D
 * @param plane 1=XY, 2=YZ, 3=ZX
 * @param px x-coordinate of center point
 * @param py y-coordinate of center point
 * @param pz z-coordinate of center point
 * @param lines number of lines (rays)
 * @param radius lenght of each line (radius of circle)
 * @param linediv (number of divisions on each line)
 * @param a axis a (result)
 * @param b axis b (result)
 * @param angle (result - not yet available)
 * @return status
 */
int fdbTS2D_tensor_scale(long   plane,
	 	                     double px, 
												 double py, 
												 double pz, 
												 long   lines, 
												 double radius, 
												 long   linediv,
												 double *a,
												 double *b,
												 double *angle,
												 long   *mat_type_base
												 )
{
	int rv = AF_OK ;

	fdbTS2D_null();

	if ((fdbTS2D_set_plane(plane)) != AF_OK) {goto memFree;}

	if ((rv = fdbTS2D_init(px, py, pz, lines, radius, linediv)) != AF_OK) {goto memFree;}
	if ((rv = fdbTS2D_do_tests(mat_type_base)) != AF_OK) {goto memFree;}
	if ((rv = fdbTS2D_process_points()) != AF_OK) {goto memFree;}

	/* angle */
	fdbTS2D_compute_angle(x_p, y_p, x_mod, y_mod, number_of_lines, angle);

	/* axes */
	fdbTS2D_compute_a_b(x_mod, y_mod, number_of_lines, a, b) ;

	fprintf(msgout, "[I] Tensor Scale: a= %e b= %e (%e) angle= %e\n ",
			*a, *b, *a / *b, *angle);

memFree:
	if (rv != AF_OK) 
	   { fprintf(msgout, "[E] %s\n ", _("Tensor scale computation failed")); }

	fdbTS2D_free();
	return(rv);
}

void fdbTS2D_null_global(void)
{
	x_gl   = NULL ;
	y_gl   = NULL ;
	rad_gl = NULL ;
}


void fdbTS2D_free_global(void)
{
	free(x_gl);
	free(y_gl);
	free(rad_gl);
	fdbTS2D_null_global();
}

int fdbTS2D_alloc_global(void)
{
	long i ;
	if ((x_gl = (double *)malloc(number_of_lines*sizeof(double))) == NULL) { goto memFree ; }
	if ((y_gl = (double *)malloc(number_of_lines*sizeof(double))) == NULL) { goto memFree ; }
	if ((rad_gl = (double *)malloc(number_of_lines*sizeof(double))) == NULL) { goto memFree ; }

	for (i=0; i<number_of_lines; i++)
	{
		x_gl[i]   = 0.0 ;
		y_gl[i]   = 0.0 ;
		rad_gl[i] = 0.0 ;
	}

	return(AF_OK);
memFree:
	fdbTS2D_free_global();
	return(AF_ERR_MEM);
}



/** Computes tensor scale in 2D
 * @param plane 1=XY, 2=YZ, 3=ZX
 * @param px x-coordinate of center point
 * @param py y-coordinate of center point
 * @param pz z-coordinate of center point
 * @param lines number of lines (rays)
 * @param radius lenght of each line (radius of circle)
 * @param linediv (number of divisions on each line)
 * @param a axis a (result)
 * @param b axis b (result)
 * @param angle (result - not yet available)
 * @return status
 */
int fdbTS2D_global_tensor_scale(
				                 long   plane,
	 	                     double px_glob, 
												 double py_glob, 
												 double pz_glob, 
												 double step,
												 long   num,
												 long   mat,
												 long   lines, 
												 double radius, 
												 long   linediv,
												 double *a_glob,
												 double *b_glob,
												 double *angle_glob,
												 double *aniso
												 )
{
	int    rv = AF_OK ;
	long   mat_type_base ;
	double a, b, angle ;
	double px = 0 ;
	double py = 0 ;
	double pz = 0 ;
	/* double kappa ; */
	double angle_i ;
	long   i, j, k ;
	long   half ;
	long   number = 0 ;
	double x1,y1,x2,y2;
	
	fdbTS2D_null_global();
	if ((rv=fdbTS2D_alloc_global()) != AF_OK) {return(rv);}

	for (i=0; i<num; i++)
	{

		for (j=0; j<num; j++)
		{
			switch (plane) /* hope that's OK.. */
			{
				case 1: 
								px = px_glob + ((double)i * step) ;
								py = py_glob + ((double)j * step) ;
								pz = pz_glob ;
							 	break ;
				case 2: 
								px = py_glob + ((double)i * step) ;
								py = pz_glob + ((double)j * step) ;
								pz = px_glob ;
							 	break ;
				case 3: 
								px = pz_glob + ((double)i * step) ;
								py = px_glob + ((double)j * step) ;
								pz = py_glob ;
							 	break ;
			}

			/* compute data for a single point */
			if ((rv = fdbTS2D_tensor_scale(
										   plane,
	 	                   px, py, pz, 
											 lines, radius, linediv,
											 &a, &b, &angle,
											 &mat_type_base
											 )) != AF_OK) {goto memFree;}

			if (mat_type_base == mat) /* only bone is interesting */
			{
				/* compute global anisotropy parameters */
				number++ ;


				for (k=0; k<number_of_lines; k++)
				{
					angle_i = angle_delta * k - angle ;
					rad_gl[k] += sqrt( (a*a * b*b) / (b*b*cos(angle_i) + a*a*sin(angle_i)) ) / a ;
				}
			}
		}
	}

	half = (long)(number_of_lines/2) ;
	for (i=0; i<half; i++)
	{
		/* selecting of smaller radius */
		if (rad_gl[i] < rad_gl[i+half]) { radius = rad_gl[i] ; }
		else                            { radius = rad_gl[i+half] ; }

		angle_i = angle_delta * i ;
		fdbTS2D_cyl2xy_center(0,0,radius, angle_i, &x1, &y1) ;
		fdbTS2D_cyl2xy_center(0,0,radius, angle_i+(FEM_PI/2), &x2, &y2) ;

		x_gl[i] = x1 ;
		y_gl[i] = y1 ;

		x_gl[i+half] = x2 ;
		y_gl[i+half] = y2 ;
	}

	fdbTS2D_compute_angle(0, 0, x_gl, y_gl, number_of_lines, angle_glob);
	fdbTS2D_compute_a_b(x_gl, y_gl, number_of_lines, a_glob, b_glob) ;

	if ((b*b) > (a*a))
	{
		*aniso = sqrt (1.0 - (a*a)/(b*b)) ;
	}
	else
	{
		*aniso = sqrt (1.0 - (b*b)/(a*a)) ;
	}

memFree:
	if (rv != AF_OK) 
	   { fprintf(msgout, "[E] %s\n ", _("Tensor scale computation failed")); }
	fdbTS2D_free_global();
	return(rv);
}

/* end of fdb_meas.c */
