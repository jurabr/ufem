/*
   File name: fem_comm.h
   Date:      2003/08/24 15:30
   Author:    Jiri Brozovsky

   Copyright (C) 2003 Jiri Brozovsky

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

	 FEM Support - common things for all "FEM" pieces

	 $Id: fem_comm.h,v 1.1 2003/09/14 20:41:59 jirka Exp $
*/


#ifndef __FEM_COMM_H__
#define __FEM_COMM_H__

#ifdef _USE_L10N_
#define _(str) gettext(str)

#define PACKAGE "test" 
#define LOCALEDIR "/usr/share/locale" /* "is this ok?" */

#include <locale.h>
#include <libintl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "fem.h"

extern FILE *msgout ;
extern long  femOutToStd ;

extern void femInitOut(void) ;

#endif

/* end of fem_comm.h */
