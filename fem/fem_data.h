/*
   File name: fem_data.h
   Date:      2003/04/07 09:24
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

	 FEM Solver - data

  $Id: fem_data.h,v 1.6 2004/11/11 21:39:23 jirka Exp $
*/


#ifndef __FEM_DATA_H__
#define __FEM_DATA_H__

#include "fem.h"

/* SOLUTION DATA */
extern long   solID           ;   /* "time node" identifier                       */
extern long   solDepOn        ;   /* dependency on prefious "time node"           */
extern long   solLinSol       ;   /* linear eq. sys. solve                        */
extern long   solNoLinS       ;   /* non-linear solution method                   */
extern long   solNoLinSteps   ;   /* number of steps in non-linear solution       */
extern long   solLinStepMax   ;   /* max. number of steps in linear solution      */
extern long   solNoLinStepMax ;   /* max. number of steps in non-linear solution  */
extern long   solNoLinIter    ;   /* default max number of iterations in one step */
extern long   solNoBreakStep  ;   /* "unbreakable" non-linear solution            */
extern double solLinPrec      ;   /* linear precission                            */
extern double solNoLinPre     ;   /* nonlinear precission                         */
extern double solNoLinBig     ;   /* nonlinear max. residual size                 */
extern double solSimNum       ;   /* simulation identifier or load multiplier */

extern long solNoLinS_cmd     ;  /*solNoLinS from command line */
extern long solNoLinSteps_cmd ;  /*solNoLinSteps from command line */

extern long solUseBiCGs       ; /* in non-symetric equation solver is forced to use */
extern long solUseCGSSOR      ; /* in non-symetric equation solver is forced to use */

extern long   resRLen ; /* number of nodes with reactions */
extern long   resELen ; /* number of element results      */

extern long   resBackRLen ; /* number of nodes with reactions BACKUP */
extern long   resBackELen ; /* number of element results BACKUP      */

/* LENGHTS OF FIELDS: */

extern long  nLen  ; /* number of nodes                */
extern long  eLen  ; /* number of elemnts              */
extern long  eLenL ; /* lenght of element nodes field  */
extern long rsLen  ; /* number or real sets            */
extern long rsLenL ; /* number of real value field     */
extern long mpLen  ; /* number of materials            */
extern long mpLenL ; /* number of materials values     */
extern long nlLen  ; /* number of nodal loads/b.c.s    */
extern long elLen  ; /* number of element loads        */
extern long elLenL ; /* number of element loads values */



/* NODES */
extern long     *nID  ; /* node number  */
extern double   *n_x  ; /* x coordinate */
extern double   *n_y  ; /* y coordinate */
extern double   *n_z  ; /* z coordinate */


/* ELEMENTS */
extern long     *eID      ; /* element number               */
extern long     *eType    ; /* element type number          */
extern long     *eRS      ; /* real set data position       */
extern long     *eMP      ; /* material data position       */
extern long     *eFrom    ; /* starting point in eNodesL    */

extern long     *eNodesL ;  /* list of nodes          */


/* REAL SETS */
extern long     *rsID      ; /* real set number              */
extern long     *rsType    ; /* element type number          */
extern long     *rsFrom    ; /* starting point in rsValL     */

extern double   *rsValL    ; /* real set values              */


/* MATERIAL PROPERTIES */
extern long     *mpID      ; /* material number              */
extern long     *mpType    ; /* material type number         */
extern long     *mpFrom    ; /* starting point in mpValL     */

extern double   *mpValL    ; /* real set values              */


/* NODAL LOADS AND BOUNDARY CONDITIONS  */
extern long     *nlNode  ;   /* node to be used (position) */
extern long     *nlType  ;   /* type of load               */
extern long     *nlDir   ;   /* direction of load          */
extern double   *nlVal   ;   /* value of load              */


/* ELEMENT LOADS */
extern long     *elElem  ;   /* node to be used (position) */
extern long     *elType  ;   /* type and directin of load  */
extern long     *elFrom  ;   /* starting point in elValL   */

extern double   *elValL  ;   /* values of load             */


/* GRAVITATION */
extern long   grDir  ; /* direction (none=0, x=1, y=3, z=3)      */
extern double grVal  ; /* acceleration value (9.81, for example) */


/* RESULTS  - reaction */
extern long     *resRnode ; /* node with reaction(s)     */
extern long     *resRdof  ;   /* x force                   */
extern double   *resRval   ;   /* y force                   */
extern double   *resRval0  ;   


/* RESULTS ON ELEMENTS */
extern long     *resEFrom  ;  /* number of results on element */
extern double   *resEVal  ;  /* results on element (values)  */


/* BACKUP of RESULTS: */

extern long     *resBackRnode ; /* node with reaction(s)     */
extern long     *resBackRdof  ;   /* x force                   */
extern double   *resBackRval   ;   /* y force                   */
extern double   *resBackRval0  ;   

extern long     *resBackEFrom  ;  /* number of results on element */
extern double   *resBackEVal  ;  /* results on element (values)  */



#endif

/* end of fem_data.h */
