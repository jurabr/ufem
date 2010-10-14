/*
   File name: fem_comm.c
   Date:      2003/08/24 15:29
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

	 $Id: fem_comm.c,v 1.1 2003/09/14 20:41:59 jirka Exp $
*/

#include "fem_comm.h"

FILE *msgout ;

long  femOutToStd = AF_YES; /* if AF_NO then use stderr */


/** sets msgout */
void femInitOut(void)
{
	if (femOutToStd == AF_YES)
	{
		msgout = stdout ;
	}
	else
	{
		msgout = stderr ;
	}
}

/* end of fem_comm.c */
