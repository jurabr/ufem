/*
   File name: fem_sb1d.c
   Date:      2007/07/16 10:45
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

   SBETA-like material constitutive model for 1D (for BEAM3)
*/

#include "fem_elem.h"
#include "fem_mat.h"


/** Exponential softwening curve for concrete (type=2)
 * @param E0 initial E (youn modullus)
 * @param ft current tensile strenth
 * @param Gf fracture energy
 * @param eps strain
 * @param Le crack band width
 * @param sigma current stress (pointer to result)
 */
int ft_curve(double E0, double ft, double Gf, double eps, double Le, double  *sigma )
{
  double epso,epsi,epsc,eps_f;
  int i;

	eps_f = Gf / ft / Le;

	epso = ft / E0;

  if (eps<epso)
	{
    *sigma = eps * E0;
  }
	else
	{
    epsi = eps;
    *sigma = ft * exp(-1.0*(eps-epso)/(eps_f));
    epsc = epsi - (*sigma)/E0;

	  for (i=1; i<=20; i++)
		{
	    *sigma = ft*exp(-1.0*(epsc)/(eps_f));
	    epsi = epsc + (*sigma) / E0;
	    epsc = epsc - (epsi-eps);
	  }
  }

	if (*sigma < 0) { *sigma = 0.0 ; }

	return(AF_OK);
}

  
/** Computes Rt for tension (no epsilon-check, no Kupfer, no nothing)
 * @param mPos material type index
 * @epsilon current total strain
 * @return Rt 
 */
double sb1d_get_Rt_tension(long ePos,
    double E0, 
    double smax, 
    double Gf, 
    double L, 
    double epsilon)
{
  double  sigma ;
  long    type = 0 ;

  if (epsilon <= 0.0) {return(E0);} /* nothing to do with non-linearity */

	if (Gf <= 0.0) {return(0.0);}

	if (type <= 1) /* linear interpolation */
	{
		sigma = smax - smax*smax*L / (2.0*Gf/1.0)*(epsilon-(smax/E0)) ;
		if (sigma <= 0.0) {sigma = 0.0 ;}
	}
	else /* curvilinear interpolation */
	{
		ft_curve(E0, smax, Gf, epsilon, L, &sigma ) ;
	}

	if ((sigma/epsilon) < 0.0) {return(0.0);}
	if ((sigma/epsilon) > E0 ) {return(E0); }

  return(sigma / epsilon);
}

/** Tests the type of stress (tension, compression, unloading)
 * @param epsilon current total strain
 * @param prev_epsilon total strain froum previous step
 * @param epsilon_el_tension limit epsilon for elastic tension
 * @param epsilon_el_compression limit epsilon for elastic compression
 * @return type of stress number (see fem_sbet.c for values)
 * */
long sb1d_get_check_epsilon(
    double epsilon, 
    double prev_epsilon,
    double epsilon_el_tension,
    double epsilon_el_compression
    )
{
  if (epsilon > 0.0)
  {
    /* tension */

    if (epsilon <= epsilon_el_tension)
    {
      /* elastic behaviour */
      return(1);
    }
    else
    {
      if (epsilon < prev_epsilon)
      {
        /* unloading - use previous E */
        /* TODO - move this to inelastic section !!! TODO */
#if 1
				return(2);
#endif
        return(-1);
      }
      else
      {
        /* inelastic behaviour */
        return(2);
      }
    }
  }
  else /* epsilon < 0.0 */
  {
    /* compression */

    if (fabs(epsilon) <= fabs(epsilon_el_compression))
    {
      /* elastic behaviour */
      return(3);
    }
    else
    {
      if ((epsilon) > (prev_epsilon)) /* values are negative! */
      {
        /* unloading - use previous E */
#if 1
				return(3);
#endif
        return(-3);
      }
      else
      {
        /* inelastic behaviour */
#if 1
				return(3);
#endif
        return(4);
      }
    }
  }
  
  return(0);
}


/* end of fem_sb1d.c */
