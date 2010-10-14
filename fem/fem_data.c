/*
   File name: fem_data.c
   Date:      2003/04/07 09:13
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

	 FEM Solver - data fields

  $Id: fem_data.c,v 1.6 2004/11/11 21:39:23 jirka Exp $
*/

#include "fem_data.h"

/* SOLUTION DATA */
long   solID           = 0 ;   /* "time node" identifier                       */
long   solDepOn        = 0 ;   /* dependency on prefious "time node"           */
long   solLinSol       = 0 ;   /* linear eq. sys. solve                        */
long   solNoLinS       = 0 ;   /* non-linear solution method                   */
long   solLinStepMax   = 1 ;   /* max. number of steps in linear solution      */
long   solNoLinSteps   = 1 ;   /* number of steps in non-linear solution       */
long   solNoLinIter    = 333 ; /* default max number of iterations in one step */
long   solNoLinStepMax = 1 ;   /* max. number of steps in non-linear solution  */
long   solNoBreakStep  = AF_NO;/* "unbreakable" non-linear solution            */
double solLinPrec      = 1e-6; /* linear precission                            */
double solNoLinPre     = 1e-6; /* nonlinear precission                         */
double solNoLinBig     = 10;   /* nonlinear max. residual size                 */
double solSimNum       = 0 ;   /* simulation identifier or load multiplier */

long solNoLinS_cmd     = -1 ;  /*solNoLinS from command line */
long solNoLinSteps_cmd = -1 ;  /*solNoLinSteps from command line */

long solUseBiCGs       = AF_NO ; /* in non-symetric equation solver is forced to use */
long solUseCGSSOR      = AF_NO ; /* in non-symetric equation solver is forced to use */

/* LENGHTS OF FIELDS: */

long  nLen  = 0; /* number of nodes                */
long  eLen  = 0; /* number of elemnts              */
long  eLenL = 0; /* lenght of element nodes field  */
long rsLen  = 0; /* number or real sets            */
long rsLenL = 0; /* number of real value field     */
long mpLen  = 0; /* number of materials            */
long mpLenL = 0; /* number of materials values     */
long nlLen  = 0; /* number of nodal loads/b.c.s    */
long elLen  = 0; /* number of element loads        */
long elLenL = 0; /* number of element loads values */

long resRLen = 0;/* number of nodes with reactions */
long resELen = 0;/* number of element results      */

long resBackRLen = 0;/* number of nodes with reactions (backup) */
long resBackELen = 0;/* number of element results (backup)      */


/* NODES */
long     *nID = NULL ; /* node number  */
double   *n_x = NULL ; /* x coordinate */
double   *n_y = NULL ; /* y coordinate */
double   *n_z = NULL ; /* z coordinate */


/* ELEMENTS */
long     *eID     = NULL ; /* element number               */
long     *eType   = NULL ; /* element type number          */
long     *eRS     = NULL ; /* real set data position       */
long     *eMP     = NULL ; /* material data position       */
long     *eFrom   = NULL ; /* starting point in eNodesL    */

long     *eNodesL = NULL;  /* list of nodes          */


/* REAL SETS */
long     *rsID     = NULL ; /* real set number              */
long     *rsType   = NULL ; /* element type number          */
long     *rsFrom   = NULL ; /* starting point in rsValL     */

double   *rsValL   = NULL ; /* real set values              */


/* MATERIAL PROPERTIES */
long     *mpID     = NULL ; /* material number              */
long     *mpType   = NULL ; /* material type number         */
long     *mpFrom   = NULL ; /* starting point in mpValL     */

double   *mpValL   = NULL ; /* real set values              */


/* NODAL LOADS AND BOUNDARY CONDITIONS  */
long     *nlNode = NULL ;   /* node to be used (position) */
long     *nlType = NULL ;   /* type of load               */
long     *nlDir  = NULL ;   /* direction of load          */
double   *nlVal  = NULL ;   /* value of load              */


/* ELEMENT LOADS */
long     *elElem = NULL ;   /* node to be used (position) */
long     *elType = NULL ;   /* type and direction of load */
long     *elFrom = NULL ;   /* starting point in elValL   */

double   *elValL = NULL ;   /* values of load             */


/* GRAVITATION */
long   grDir = 0 ; /* direction (none=0, x=1, y=3, z=3)      */
double grVal = 0 ; /* acceleration value (9.81, for example) */


/* RESULTS  - reaction */
long     *resRnode = NULL ; /* node with reaction(s)     */
long     *resRdof  = NULL ; /* dof type     */
double   *resRval   = NULL ; /*  force                   */
double   *resRval0  = NULL ; /*  force from previous iteration (4,5,6,7 type only) */

/* RESULTS ON ELEMENTS */
long     *resEFrom = NULL ;  /* number of results on element */
double   *resEVal = NULL ;  /* results on element (values)  */


/* BACKUP results */
long     *resBackRnode = NULL ; /* node with reaction(s)     */
long     *resBackRdof  = NULL ; /* dof type     */
double   *resBackRval   = NULL ; /*  force                   */
double   *resBackRval0  = NULL ; /*  force from previous iteration (4,5,6,7 type only) */

long     *resBackEFrom = NULL ;  /* number of results on element */
double   *resBackEVal = NULL ;  /* results on element (values)  */

/* end of fem_data.c */
