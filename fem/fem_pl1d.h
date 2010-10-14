/*
   File name: fem_pl1d.h
   Date:      2006/09/19 10:41
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

   FEM Solver - 3D elasticity and plastitity

   $Id: fem_pl3d.h,v 1.6 2004/11/11 21:39:25 jirka Exp $
*/


#ifndef __FEM_PL1D_H__
#define __FEM_PL1D_H__

#include "fem_mat.h"
#include "fem_elem.h"

extern double fem_pl1d_E(
		long   ePos, 
		double Eprev, 
		double depsilon, 
		double epsilon_old, 
		double sigma_old
		);

#endif

/* end of fem_pl3d.h */
