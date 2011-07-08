/*
   File name: fem_eini.c
   Date:      2003/04/12 15:26
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

	 FEM Solver - element type initialization

	 $Id: fem_eini.c,v 1.15 2005/07/11 17:56:16 jirka Exp $
*/

#include "fem_elem.h"

extern int addElem_000(void);
extern int addElem_001(void);
#ifndef _SMALL_FEM_CODE_
extern int addElem_002(void);
extern int addElem_003(void);
extern int addElem_004(void);
extern int addElem_005(void);
extern int addElem_006(void);
extern int addElem_007(void);
extern int addElem_008(void);
extern int addElem_009(void);
extern int addElem_010(void);
extern int addElem_011(void);
extern int addElem_012(void);
extern int addElem_013(void);
extern int addElem_014(void);
extern int addElem_015(void);
extern int addElem_016(void);
extern int addElem_017(void);
extern int addElem_018(void);
extern int addElem_019(void);
extern int addElem_020(void);
#endif

int femElemTypeInit(void)
{
	int rv = AF_OK;

	if ((rv = addElem_000()) != AF_OK){goto memFree;}
	if ((rv = addElem_001()) != AF_OK){goto memFree;}
#ifndef _SMALL_FEM_CODE_
	if ((rv = addElem_002()) != AF_OK){goto memFree;}
	if ((rv = addElem_003()) != AF_OK){goto memFree;}
	if ((rv = addElem_004()) != AF_OK){goto memFree;}
	if ((rv = addElem_005()) != AF_OK){goto memFree;}
	if ((rv = addElem_006()) != AF_OK){goto memFree;}
	if ((rv = addElem_007()) != AF_OK){goto memFree;}
	if ((rv = addElem_008()) != AF_OK){goto memFree;}
	if ((rv = addElem_009()) != AF_OK){goto memFree;}
	if ((rv = addElem_010()) != AF_OK){goto memFree;}
	if ((rv = addElem_011()) != AF_OK){goto memFree;}
	if ((rv = addElem_012()) != AF_OK){goto memFree;}
	if ((rv = addElem_013()) != AF_OK){goto memFree;}
	if ((rv = addElem_014()) != AF_OK){goto memFree;}
	if ((rv = addElem_015()) != AF_OK){goto memFree;}
	if ((rv = addElem_016()) != AF_OK){goto memFree;}
	if ((rv = addElem_017()) != AF_OK){goto memFree;}
	if ((rv = addElem_018()) != AF_OK){goto memFree;}
	if ((rv = addElem_019()) != AF_OK){goto memFree;}
	if ((rv = addElem_020()) != AF_OK){goto memFree;}
#endif

	if (rv == AF_OK) {return(rv);}
memFree:
#ifdef RUN_VERBOSE
	fprintf(msgout,"[E] %s!\n", _("Element type initialization failed"));
#endif
#ifdef DEVEL
	exit(rv);
#endif
	return(rv);
}


/* end of fem_eini.c */
