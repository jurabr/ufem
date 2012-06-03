/*
   File name: fem_pl3d.h
   Date:      2004/01/03 17:50
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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


#ifndef __FEM_PL3D_H__
#define __FEM_PL3D_H__

#include "fem_mat.h"
#include "fem_elem.h"

extern int femPrinc3D(tVector *sx, tVector *s1) ;
extern int femPrinc3dTranM(tVector *sx, tVector *s1, tMatrix *M) ;

extern int fem_D_3D(
             long ePos, 
             long e_rep, 
             long eT, 
             long mT, 
             tVector *sigma, 
             tVector *epsilon, 
             long Mode, 
             tMatrix *D);

extern int femD_3D_iso(long ePos, double Ex, double nu, tMatrix *D);

extern int fem_chen_D_3D(long ePos, 
                  long e_rep, 
                  long eT, 
                  long mT, 
                  tVector *sigma, 
                  tVector *epsilon, 
                  long Mode, 
                  tMatrix *D);

extern int fem_dp_D_3D(long ePos, 
                  long e_rep, 
                  long eT, 
                  long mT, 
                  tVector *sigma, 
                  tVector *epsilon, 
                  long Mode, 
                  tMatrix *Dep);

extern int fem_vmis_D_3D(long ePos, 
                  long e_rep, 
                  long eT, 
                  long mT, 
                  tVector *sigma, 
                  tVector *epsilon, 
                  long Mode, 
                  tMatrix *Dep);

extern int fem_D_ccr3D(long ePos, 
                long e_rep, 
                long eT, 
                long mT, 
                tVector *sigma, 
                tVector *epsilon, 
                long Mode, 
                tMatrix *D);

extern int od3d_D(long ePos, long iPoint, long Mode, tVector *epsilon, tMatrix *D);

#endif

/* end of fem_pl3d.h */
