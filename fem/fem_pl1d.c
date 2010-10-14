/*
   File name: fem_pl1d.c
   Date:      2006/09/19 10:38
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

   FEM solver (uFEM): 1D non-linear behaviour (plasticity) 
*/

#include "fem_pl1d.h"

/** Computes stiffness for 1D case (tangential only)
 * @param ePes element position
 * @param Eprev previous size of the stiffness
 * @param depsilon current epsilon
 * @param epsilon_old previous total epsilon
 * @param sigma_old previous total stress
 * @return E (stiffness)
 * */
double fem_pl1d_E(
		long   ePos, 
		double Eprev, 
		double depsilon, 
		double epsilon_old, 
		double sigma_old
		)
{
	long   mpos      ;
	double Ed    = 0 ;
	double sigma     ;
	double s_lim     ;
	long   sets  = 1 ;
	long   i         ;

	mpos  = femGetEMPPos(ePos) ;
	sets  = femMPNumPos(mpos) / Mat[4].num ;
	Ed    = femGetMPValPos(ePos, MAT_EX, 0) ;

  if (femTangentMatrix != AF_YES)
  {
    return(Ed); /* doesn't work for non-tangent matrix! */
  }

#ifdef DEVEL_VERBOSE
  fprintf(msgout,"[ ] MP pos: %li x %li\n", eMP[ePos], mpos);
#endif

	sigma = Eprev * depsilon + sigma_old ; /* new sigma */

#ifdef DEVEL_VERBOSE
  fprintf(msgout,"[ ] SIGMA: %e = %e * _%e_ + %e\n",
	sigma , Eprev , depsilon , sigma_old
      );
#endif

	/* unloading test: */
	if ( 
			( (depsilon * epsilon_old) >= 0 ) 
			&& 
			( fabs (depsilon + epsilon_old) < fabs(depsilon) )
			)
	{
		return(Ed) ; /* Ed == E0 here */
	}

	/* limit test: */
	if (sets == 1)
	{
		s_lim = femGetMPValPos(ePos, MAT_F_YC, 0) ;
		if ( fabs(sigma) > fabs(s_lim) )
		{
			Ed = femGetMPValPos(ePos, MAT_HARD, 0) ;
			return(Ed) ;
		}
	}
	else
	{
		for (i=(sets-2); i>= 0; i--)
		{
			s_lim = femGetMPValPos(ePos, MAT_F_YC, i) ;
			
			if ( fabs(sigma) > fabs(s_lim) )
			{
				Ed = femGetMPValPos(ePos, MAT_HARD, i+1) ;
				return(Ed) ;
			}
		}
	}

	return(Ed);
}

/* end of fem_pl1d.c */
