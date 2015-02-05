/*
   File name: fem_eini.c
   Date:      2003/04/16 21:00
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

	 FEM Solver - material type initialization

	 $Id: fem_mini.c,v 1.7 2004/11/11 21:39:25 jirka Exp $
*/

#include "fem_mat.h"

extern int addMat_000(void);
extern int addMat_001(void);
extern int addMat_002(void);
extern int addMat_003(void);
extern int addMat_004(void);
extern int addMat_005(void);
extern int addMat_006(void);
extern int addMat_007(void);
extern int addMat_008(void);
extern int addMat_009(void);
extern int addMat_010(void);
extern int addMat_011(void);

int femMatTypeInit(void)
{
	int rv = AF_OK;

	if ((rv = addMat_000()) != AF_OK){goto memFree;}
	if ((rv = addMat_001()) != AF_OK){goto memFree;}
	if ((rv = addMat_002()) != AF_OK){goto memFree;}
	if ((rv = addMat_003()) != AF_OK){goto memFree;}
	if ((rv = addMat_004()) != AF_OK){goto memFree;}
	if ((rv = addMat_005()) != AF_OK){goto memFree;}
	if ((rv = addMat_006()) != AF_OK){goto memFree;}
	if ((rv = addMat_007()) != AF_OK){goto memFree;}
	if ((rv = addMat_008()) != AF_OK){goto memFree;}
	if ((rv = addMat_009()) != AF_OK){goto memFree;}
	if ((rv = addMat_010()) != AF_OK){goto memFree;}
	if ((rv = addMat_011()) != AF_OK){goto memFree;}

	if (rv == AF_OK) {return(rv);}
memFree:
#ifdef RUN_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Material type initialization failed"));
#endif
#ifdef DEVEL
	exit(rv);
#endif
	return(rv);
}

/* end of fem_eini.c */
