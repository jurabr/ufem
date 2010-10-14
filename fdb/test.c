/*
   File name: test.c
   Date:      2003/06/17 21:40
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

	 FEM database - test file
   $Id: test.c,v 1.2 2003/08/04 19:00:30 jirka Exp $
*/

#include "fdb.h"


extern int fdbCreateInputTabHdr(void) ;

int main(int argc, char *argv[])
{
	int rv = AF_OK ;

	printf("Test started:\n");
	rv = (fdbCreateInputTabHdr());
	printf("Test finished, return value is: %i\n",rv);

	return(rv);
}



/* end of test.c */
