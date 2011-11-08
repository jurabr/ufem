/*
   File name: fem_hard.c
   Date:      2011/11/06 17:56
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

   FEM solver: hardening rules for plasticity
*/

#include "fem_elem.h"
#include "fem_mat.h"

#ifndef _SMALL_FEM_CODE_

/** Computes Ramberg--Osgood approximation of H 
 */
double fem_plast_H_RO(long ePos, 
                      double k, 
                      double n, 
                      double E,
                      double sigma)
{
  return ( (E / (k * n)) * pow(sigma/E , 1.0-n) );
}


#endif

/* end of fem_hard.c */
