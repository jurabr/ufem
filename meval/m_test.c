/*
   File name: m_test.c
   Date:      2008/06/17 23:36
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
*/

#include "m_eval.h"

int main(int argc, char *argv[])
{
	char str[256];
	double a = 0 ;
	int i ;

	for (i=0; i<256; i++) {str[i] = '\0' ; }

	fgets(str,255,stdin) ;
	printf("Original string: %s\n",str);

	a = mev_parse(str);

	printf("Preprocessed string: \"%s\", result = %e\n",str, a);

	printf("Tester for mathematical evaluation library.\n");

	return(CI_OK);
}


/* end of m_test.c */
