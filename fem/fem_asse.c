/*
   File name: fem_asse.c
   Date:      2011/01/31 18:27
   Author:    Jiri Brozovsky

   Copyright (C) 2011 Jiri Brozovsky

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

   General assessment routines (mainly for libfem.so)
*/

#include "fem.h"
#include "fem_data.h"


/** ### MATERIAL POINT LEVEL ####  */

/** Max. displacement test
 * @param disp_max maximum allowed displacement (x,y, or z)
 */
long fem_asse_max_disp_simple(double disp_max)
{
  if (femVecMaxAbs(&u) >= fabs(disp_max)) {return(1);} /* failed */

  return(0);
}

/* end of fem_asse.c */
