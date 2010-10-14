/*
   File name: test.c
   Date:      2003/06/15 20:59
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

	 Example "command interpreter" that uses "cint" library

	 $Id: test.c,v 1.2 2003/09/14 20:39:05 jirka Exp $
*/

#include "cint.h"

int func_error(char *cmd)
{
	printf("Bad command!\n");
	return(CI_OK);
}

int func_hello(char *cmd)
{
	if (ciParNum(cmd) == 1)
	{
		printf("Hello world!");
	}
	else
	{
		printf("Hello: %s",ciGetParStr(cmd,1));
	}
	return(CI_OK);
}

int func_system(char *cmd)
{
	if (ciParNum(cmd) == 1)
	{
		printf("I need your command!");
	}
	else
	{
		system(ciGetParStr(cmd,1));
	}
	return(CI_OK);
}


int func_exit(char *cmd)
{
	printf("Exit!\n");
	exit(CI_OK);
}

int main(int argc, char *argv[])
{
	char cmd[256];
	int  cmdlen=255;

	ciFuncLen = 4 ;
	ciFunction = (tFunc *)malloc(ciFuncLen*sizeof(tFunc));

	ciFunction[0].cmd = "error";
	ciFunction[0].func = func_error;

	ciFunction[1].cmd = "hello";
	ciFunction[1].func = func_hello;

	ciFunction[2].cmd = "system";
	ciFunction[2].func = func_system;

	ciFunction[3].cmd = "exit";
	ciFunction[3].func = func_exit;


	printf("\nSmall Command Interpreter\n\n(c) 2003 Jiri Brozovsky\n\n> ");
	while (fgets(cmd,cmdlen,stdin))
	{
		ciRunCmd(cmd);
		printf("\n\n> ");
	}
	return(0);
}

/* end of test.c */
