/*
   File name: m_eval.h
   Date:      2008/06/13 12:47
   Author:    Jiri Brozovsky

   Copyright (C) 2008 Jiri Brozovsky

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
*/


#ifndef __M_EVAL_H__
#define __M_EVAL_H__

#include  <math.h>
#ifndef __CINT_H__
#include  <cint.h>
#endif

#define MAX_ME_ARGS 500
#define MAX_ME_STR  2048

extern double mev_parse(char *str) ;

#endif

/* end of m_eval.h */
