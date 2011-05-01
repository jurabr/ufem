/*
   File name: fem_pric.c
   Date:      2011/05/01 18:31
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
*/

#include "fem.h"
#include "fem_sol.h"
#include "fem_elem.h"

double fem_get_struct_price(long volume_only)
{
  static double price = 0.0 ;
  double vprice, evol ;
  long i, eT ;

  price = 0 ;

  for (i=0; i<eLen; i++)
  {
    if (volume_only == AF_YES)
    {
      vprice = 1.0 ;
    }
    else
    {
      vprice = femGetMPValPos(i, MAT_PRICE, 0) ;
    }

    if (vprice <= 0) {vprice = 1.0 ;}
	  eT = femGetETypePos(i) ;
    Elem[eT].volume(i, &evol) ;
    price += vprice * evol ;
  }

#ifdef DEVEL_VERBOSE
  fprintf("[ ] %s: %e\n", _("SYSTEM PRICE"), price);
#endif

  return(price);
}

/* end of fem_pric.c */
