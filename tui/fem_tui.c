/*
   File name: fem_tui.c
   Date:      2003/08/17 16:38
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

	 FEM - text user interface
*/

#include "fem_tui.h"

extern int my_main (int argc, char *argv[]) ;
extern int fem_parse_params(int argc, char *argv[]);
extern int fem_get_ui_config(void);


/** Main block of text user interface */
int main(int argc, char *argv[])
{
	int rv = AF_OK ;
  
  msgout = stdout ; /* required here */

  if ((rv = fem_parse_params(argc, argv)) != AF_OK) {return(rv);}

  if ((rv = fem_tui_init()) != AF_OK) {return(rv);}

  fem_get_ui_config(); /* no testing needed */

	return( my_main (argc, argv)) ;
}

/* end of fem_tui.c */
