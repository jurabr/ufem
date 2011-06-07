/*
   File name: fem_eres.h
   Date:      2003/04/20 23:24
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

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

	 FEM Solver - element definition - types of results

  $Id: fem_eres.h,v 1.9 2005/07/11 17:56:16 jirka Exp $
*/


#ifndef __FEM_ERES_H_
#define __FEM_ERES_H__

#define RES_EMPTY  0
#define RES_FX     1
#define RES_FY     2
#define RES_FZ     3
#define RES_MX     4
#define RES_MY     5
#define RES_MZ     6
#define RES_SX     7
#define RES_SY     8
#define RES_SZ     9
#define RES_SXY   10
#define RES_SYZ   11
#define RES_SZX   12
#define RES_EX    13
#define RES_EY    14
#define RES_EZ    15
#define RES_EXY   16
#define RES_EYZ   17
#define RES_EZX   18
#define RES_S1    19
#define RES_S2    20
#define RES_S3    21
#define RES_E1    22
#define RES_E2    23
#define RES_E3    24
#define RES_DIR1  25
#define RES_DIR2  26
#define RES_DIR3  27
#define RES_ECR1  28
#define RES_ECR2  29
#define RES_ECR3  30
#define RES_GCR1  31
#define RES_GCR2  32
#define RES_GCR3  33
#define RES_MXY   34
#define RES_QX    35
#define RES_QY    36
#define RES_CR1   37
#define RES_PSI   38
#define RES_P_M11 39
#define RES_P_M12 40
#define RES_P_M13 41
#define RES_P_M21 42
#define RES_P_M22 43
#define RES_P_M23 44
#define RES_P_M31 45
#define RES_P_M32 46
#define RES_P_M33 47
#define RES_S_MX  48 /* slab bending */
#define RES_S_MY  49
#define RES_S_MXY 50
#define RES_S_VXZ 51
#define RES_S_VYZ 52
#define RES_TMAX   53
#define RES_SM1    54
#define RES_SM2    55
#define RES_SMSMAX 56
#define RES_SMXDIM 57
#define RES_SMYDIM 58
#define RES_SVMIS  59 /* von Mises stress */
#define RES_EI     60 /* beam stiffness   */
#define RES_STMAX  61
#define RES_SCMAX  62
#define RES_STAT1  63
#define RES_STAT2  64
#define RES_USX    65
#define RES_USY    66
#define RES_USZ    67
#define RES_USXY   68
#define RES_USYZ   69
#define RES_USZX   70
#define RES_TEMP   71


#endif

/* end of fem_elem.h */
