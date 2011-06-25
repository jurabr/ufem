/*
   File name: fem_loads.c
   Date:      2003/04/17 09:56
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

	 FEM Solver - loads

	 $Id: fem_loads.c,v 1.16 2004/11/11 21:39:24 jirka Exp $
*/

#include "fem_sol.h"

#ifdef USE_MPI
#include "fem_mpi.h"
#endif

extern tVector u_i;
extern tVector uTemp; /* thermal loads field */
extern long femHaveThermDOFs; /* therm DOFs indicator */

double femRedStiff = 1.0 ;

int femAddThermLoads(void);


/** Find if there is need for thermal loads on elements
 * @return AF_YES if there is at leas one temperature load, AF_NO otherwise
 */
int femTestThermLoads(void)
{
  long i ;
  
  for (i=0; i<nlLen; i++) { if (nlType[i] == 8) { return(AF_YES); } }
  return(AF_NO);
}

/** Find if there is thermal load on structural elements
 * @return AF_YES if there is, AF_NO if there are only thermal elements
 */
int femTestThermStructElems(void)
{
  long i, j, eType, nNum ;

  if ((femHaveThermDOFs=femTestThermLoads()) == AF_NO) {return(AF_NO);}

  for (i=0; i<rsLen; i++) 
  {
    eType = rsType[i] ;
    nNum  = Elem[eType].dofs ;
    for (j=0; j<nNum; j++)
    { 
      if (Elem[eType].ndof[j] != TEMP) 
      {
        return(AF_YES);
      } 
    }
  }
  
  return(AF_NO);
}


/** Boundary conditions realization (structural stifness)
 *  @param nodePos node position
 *  @param dof     degree of freedom
 *  @param Val     value to be added (stiffness)
 */
int femApplyStiff(long nodePos, long dof, double Val)
{
	long Pos = 0;
#ifdef USE_MPI
	long row = 0 ;
#endif

	if ((Pos = femKpos(nodePos, dof)) <= 0)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s (%s: %li, %s: %li)\n",
				_("Cannot add stiffness"), _("node"),femGetNIDPos(nodePos) , _("DOF"), dof
				);
#endif
		return(AF_ERR_VAL);
	}

#ifndef USE_MPI 
	/* standard way: */
	femMatAdd(&K, Pos,Pos, fabs(Val));
#else 
	/* MPI version: */
	row = Pos - (ppDOFfrom-1) ;
	if ((row < 1) || (row > ppDOFlen)) {return(AF_OK);}
	femMatAdd(&K, row,Pos, fabs(Val));
#endif

  return(AF_OK);
}

/** Adds displacements/stifnesses to Ku=F
 * @param
 * @return
 */
int femApplyDisp(long nodePos, long dof, double Val)
{
	int rv = AF_OK;
	double K_Val = 0 ;
	double F_Val = 0 ;
	double u_Val = 0 ;
	double tmpVal = 0;
	long Pos = 0;
#ifndef BLOCK_FOR_MPI
	long rPos = 0;
#endif
	long row = 0 ;
	long i;

	if ((Pos = femKpos(nodePos, dof)) <= 0)
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s (%s: %li, %s: %li)\n",
				_("Cannot add boundary condition"), _("node"),femGetNIDPos(nodePos) , _("DOF"), dof
				);
#endif
		return(AF_ERR_VAL);
	}

#ifdef USE_MPI
	row = Pos - (ppDOFfrom-1) ;
	if ((row >= 1) && (row <= ppDOFlen)) 
	{
#else
	row = Pos;
#endif
	K_Val = femMatGet(&K, row, Pos);
	F_Val = femVecGet(&F, row);
	u_Val = femVecGet(&u, row);
#ifdef USE_MPI
	}
#endif

	if (Val == 0)
	{
#ifdef USE_MPI /* MPI*/
		femMatSetZeroCol(&K, Pos);

		if ((row >= 1) && (row <= ppDOFlen)) 
		{
		  femMatSetZeroRow(&K, row);

			femMatPut(&K, row, Pos, 1);
			femVecPut(&u, row, Val);
			femVecPut(&F, row, Val);
		}
#else /* standard: */
		femMatSetZeroRow(&K, Pos);
		femMatSetZeroCol(&K, Pos);

		femMatPut(&K, Pos, Pos, 1);
		femVecPut(&u, Pos, Val);
		femVecPut(&F, Pos, Val);


#if 1
    if (femDynamics == AF_YES)
    {
		  femMatSetZeroRow(&M, Pos);
		  femMatSetZeroCol(&M, Pos);
		  femMatPut(&M, Pos, Pos, 1);
    }
#endif
#endif
		
#ifndef BLOCK_FOR_MPI
#if 1
		/* add F_Val handling code HERE! */
		if ((rPos = fem2IntPos(nodePos, resRnode, dof, resRdof, resRLen)) < 0)
		{
			if (Val == 0.0)
			{
#ifdef RUN_VERBOSE
# if 0
				fprintf(msgout,"[W] %s [n=%li,dof=%li]!\n", _("Cannot store load on b.c."),nID[nodePos],dof);
#endif
#endif
#if 0
				return(AF_ERR_VAL); /* return is suspended to allow diplacement control for the NRM*/
#endif
			}
		}

		if (rPos > -1) { resRval[rPos] -= F_Val ; }
#endif
#endif
	}
	else
	{
		u_Val = Val ;
#ifdef USE_MPI /* MPI: */
		for (i=1; i<=K.rows; i++)
#else/* standard: */
		for (i=1; i<=K.rows; i++)
#endif
		{
			tmpVal = femMatGet(&K,i, Pos);
			femVecAdd(&F, i, (-1.0 * tmpVal * u_Val));

#ifdef DEVEL_VERBOSE
  printf(" [%li | %li] Val is %e * %e\n",i, Pos, u_Val, tmpVal);
#endif
		}

#ifdef USE_MPI /* MPI: */
		femMatSetZeroCol(&K, Pos); /* why? */

		if ((row >= 1) && (row <= ppDOFlen)) 
		{
		  femMatSetZeroRow(&K, row);

		  femVecPut(&u, row, Val);
		  femMatPut(&K, row, Pos, (femVecGet(&F,row)/u_Val));
    }
#else /* standard: */
		femMatSetZeroRow(&K, Pos);
		femMatSetZeroCol(&K, Pos); /* why? */

		femVecPut(&u, Pos, Val);

    if (fabs(femVecGet(&F,Pos)/u_Val) > FEM_ZERO)
    {
		  femMatPut(&K, Pos, Pos, (femVecGet(&F,Pos)/u_Val));
    }
    else
    {
		  femMatPut(&K, Pos, Pos, 1.0);
		  femVecPut(&F, Pos, Val);
    }

#if 1
    if (femDynamics == AF_YES)
    {
		  femMatSetZeroRow(&M, Pos);
		  femMatSetZeroCol(&M, Pos);
		  femMatPut(&M, Pos, Pos, 1);
    }
#endif
#endif
	}

	return(rv);
}

int femTestDisp(long nodePos, long Type, long dof)
{
	long rPos ;
	long dPos ;

	if ((rPos = fem2IntPos(nodePos, resRnode, dof, resRdof, resRLen)) < 0)
	{
		/* not found => ignore it */
		return(AF_YES);
	}
	else
	{
		dPos = femKpos(nodePos, dof);

		switch(Type)
		{
			case 4: 
			case 6: 
							if (resRval0[rPos] < 0.0) 
							{
								if (femVecGet(&u_i, dPos)  > 0.0)
								{
								}
								else
								{
									femVecPut(&u_i, dPos, 1.0)  ;
									femIterBCchange++;
								}
								return(AF_NO);
							}
							else
							{
								if (femVecGet(&u_i, dPos)  > 0.0)
								{
									femVecPut(&u_i, dPos, 0.0) ;
									femIterBCchange++;
								}
							}
				      break;
			case 5:
			case 7: 
							if (resRval0[rPos] > 0.0) 
							{
								if (femVecGet(&u_i, dPos)  > 0.0)
								{
									/* do nothing */
								}
								else
								{
									femVecPut(&u_i, dPos, 1.0)  ;
									femIterBCchange++;
								}
								return(AF_NO);
							}
							else
							{
								if (femVecGet(&u_i, dPos)  > 0.0)
								{
									femVecPut(&u_i, dPos, 0.0) ;
									femIterBCchange++;
								}
							}
							break;
			default:
              return(AF_YES); break;
		}
	}
	return(AF_YES);
}

/** Uses load on nodes EXCEPT boundary conditions
 * @param nPos node position
 * @param load type
 * @param dir direction (relevant DOF)
 * @param Val value to be used
 * @return state value
 */
int femApplyNLoad(long nPos, long Type, long Dir, double Val)
{
	int rv = AF_OK ;
	long Pos;
#ifdef USE_MPI
	long row = 0 ;
#endif

	if (((Pos = femKpos(nPos, Dir)) <= 0) && (Type != 8))
	{
#ifdef RUN_VERBOSE
		fprintf(msgout,"[E] %s (%s: %li, %s: %li,  %s: %li)\n",
				_("Cannot add nodal load"), _("node"),femGetNIDPos(nPos),_("type"), Type, _("direction"), Dir
				);
#endif
	}

#ifdef USE_MPI
	/* MPI version: */
	row = Pos - (ppDOFfrom-1) ;
	if ((row < 1) || (row > ppDOFlen)) { return(AF_OK); }
	else                               { Pos = row ; }
#endif

	switch (Type)
	{
		case 1:  /* displacement */
		case 2:  /* stifness */
		case 4:
		case 5:
		case 6:
		case 7:
						 return(AF_OK); /* b.c. is added by another function */
						 break;
    case 8:
             if (femHaveThermLoad == AF_YES)
                { rv = femAddThermLoads(); }
             break;
		case 3:  /* force/moment load */
						 rv = femVecAdd(&F, Pos, Val) ;
						 break;
		case 9:  /* heat */
             if (femHaveThermDOFs == AF_YES)
             {
						  rv = femVecAdd(&F, Pos, Val) ;
             }
						 break;
		default: /* error */
						 break;
	}

	return(rv);
}

/** Uses boundary conditions on nodes
 * @param nPos node position
 * @param load type
 * @param dir direction (relevant DOF)
 * @param Val value to be used
 * @return state value
 */
int femApplyNBC(long nPos, long Type, long Dir, double Val)
{
	int rv = AF_OK ;

#ifdef RUN_VERBOSE
	if ((femKpos(nPos, Dir)) <= 0) 
	{
    if (Type != 8) 
    {
		  fprintf(msgout,"[E] %s (%s: %li, %s: %li)\n",
				_("Cannot add nodal b.c."), _("node"),femGetNIDPos(nPos) , _("direction"), Dir
				);
    }
	}
#endif

	switch (Type)
	{
		case 1:  /* displacement */
             rv = femApplyDisp(nPos, Dir, Val); 
             break ;
    case 8:  /* temperature as DOF:  */
             if (femHaveThermLoad != AF_YES)
                { rv = femApplyDisp(nPos, Dir, Val); }
						 break;
		case 2:  /* stifness */
						 rv = femApplyStiff(nPos, Dir, Val) ;
						 break;
		case 3:  /* force/moment load */
						 return(AF_OK);
						 break;

		/* works only in compression */				 
		case 4:  /* displacement - compression only  */
#ifndef BLOCK_FOR_MPI
						if (femTestDisp(nPos, Type, Dir)!=AF_YES)
            {
						  rv = femApplyStiff(nPos, Dir, femRedStiff);
              break;
            }
#endif
						 rv = femApplyDisp(nPos, Dir, Val);
						 break;
		/* works only in tension */				 
		case 5:  /* displacement - tension only */
#ifndef BLOCK_FOR_MPI
						if (femTestDisp(nPos, Type, Dir)!=AF_YES)
            {
						  rv = femApplyStiff(nPos, Dir, femRedStiff);
              break;
            }
#endif
						 rv = femApplyDisp(nPos, Dir, Val);
						 break;
		/* works only in compression */				 
		case 6:  /* stifness */
#ifndef BLOCK_FOR_MPI
						if (femTestDisp(nPos, Type, Dir)!=AF_YES)
            {
						  rv = femApplyStiff(nPos, Dir, femRedStiff);
              break;
            }
#endif
						 rv = femApplyStiff(nPos, Dir, Val) ;
						 break;
		/* works only in tension */				 
		case 7:  /* stifness */
#ifndef BLOCK_FOR_MPI
						if (femTestDisp(nPos, Type, Dir)!=AF_YES)
            {
						  rv = femApplyStiff(nPos, Dir, femRedStiff);
              break;
            }
#endif
						 rv = femApplyStiff(nPos, Dir, Val) ;
						 break;
		case 9:  /* heat power load */
             return(AF_OK);

		default: /* nothing */
						 break;
	}

	return(rv);
}


/** Adds gravitation load to load vector
 * @param Dir direction of acceleration
 * @param Val acceleration size
 * @return state value
 */
int femAddGravLoad(long Dir, double Val)
{
	int rv = AF_OK;
	long eT, MeSz;
	tMatrix M_e;
	tVector u_e;
	tVector F_e;
	long i,j,k;
	long pos;

	if (fabs(Val) <= 0) {return(AF_OK);}

	femMatNull(&M_e);
	femVecNull(&u_e);
	femVecNull(&F_e);
	
	for (i=0; i<eLen; i++)
	{
		eT = eType[i];
		MeSz = Elem[eT].dofs * Elem[eT].nodes ;

		if ((rv=femFullMatInit(&M_e, MeSz, MeSz)) != AF_OK)
		{ 
			femMatNull(&M_e); 
			goto memFree; 
		}

		if ((rv=femVecFullInit(&u_e, MeSz)) != AF_OK)
		{ 
			femMatFree(&M_e); 
			femVecNull(&u_e);
			femVecNull(&F_e);
			goto memFree; 
		}

		if ((rv=femVecFullInit(&F_e, MeSz)) != AF_OK)
		{ 
			femMatFree(&M_e); 
			femVecFree(&u_e);
			femVecNull(&F_e);
			goto memFree; 
		}

		if ((rv=Elem[eT].mass(i, &M_e)) != AF_OK)
		{
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s: %li!", _("Mass matrix creation failed on element"), eID[i]);
#endif
			goto memFree;
		}

		femVecSetZero(&u_e);
		femVecSetZero(&F_e);

		pos = 0 ;

		/* multipliers vector: */
		for (j=0; j<Elem[eT].nodes; j++)
		{
			for (k=0; k<Elem[eT].dofs; k++)
			{
				pos++;
				if (Elem[eT].ndof[k] == Dir)
				{
					rv = femVecPut(&u_e, pos, (-1.0 * Val) );
#ifdef DEVEL_VERBOSE
					if (rv != AF_OK)
					{
					  fprintf(msgout,"[E] %s: %li!\n",_("Problem with gravity matrix on element"), eID[i]);
					}
#endif
				}
			}
		}

		/* load vector: */
		if ((rv = femMatVecMult(&M_e, &u_e, &F_e)) != AF_OK) { goto memFree; }

		femMatPrn(&M_e,"Mass matrix");
		femVecPrn(&u_e,"Mass multiplier");
		femVecPrn(&F_e,"Mass load");

		/* localization of load vector: */
		if((rv = femLocF_e(&F_e, i, &F, AF_YES)) != AF_OK) 
		{ 
#ifdef RUN_VERBOSE
			fprintf(msgout,"[E] %s: %li!\n", _("Localization of gravitation load vectro failed on element"),eID[i]);
#endif
			goto memFree; 
		}
		
			femMatFree(&M_e); 
			femVecFree(&u_e);
			femVecFree(&F_e);
	}

	if (rv == AF_OK) {return(rv);}
memFree:
			femMatFree(&M_e); 
			femVecFree(&u_e);
			femVecFree(&F_e);
	return(rv);
}

/** Adds element loads to load vector
 * @return state value
 */
int femAddElemLoad(void)
{
	int rv = AF_OK;
	tVector F_e;
	long size = 0;
	long eT;
	long i;

	femVecNull(&F_e);

	for (i=0; i<elLen; i++)
	{
		eT = eType[i];
		size = Elem[eT].dofs * Elem[eT].nodes ;

    if ((rv = femVecFullInit(&F_e, size)) != AF_OK) 
       { femVecNull(&F_e); return(rv); }

		if ((rv = Elem[eT].eload(elElem[i], i, &F_e)) != AF_OK) 
       { femVecFree(&F_e); return(rv); }

	  /* add localization of F_e here! */
		rv = femLocF_e(&F_e, i, &F, AF_YES);
	
	  femVecFree(&F_e);
	}
	
	return(rv);
}

/** Adds temperatures as loads to load vector
 * @return state value
 */
int femAddThermLoads(void)
{
	int rv = AF_OK;
	tVector F_e;
	tVector T_e;
	long size = 0;
	long tsize = 0;
	long eT;
	long i, j, pos;
  long use = AF_NO ;

	femVecNull(&F_e);

	for (i=0; i<eLen; i++)
	{
		eT = eType[i];
		tsize = Elem[eT].nodes ;
		size = Elem[eT].dofs * tsize ;

    if ((rv = femVecFullInit(&F_e, size)) != AF_OK) 
       { femVecNull(&F_e); return(rv); }
    if ((rv = femVecFullInit(&T_e, tsize)) != AF_OK) 
       { femVecNull(&F_e); femVecNull(&T_e); return(rv); }

    /* get temperature field: */
    for (j=0; j<tsize; j++)
    {
      pos = 1 + eNodesL[eFrom[i]+j] ;
      femVecPut( &T_e, j+1, femVecGet(&uTemp, pos) ) ;
    }

    /* compute temperature FE: */
		rv = Elem[eT].therm(i, &use, &T_e, &F_e);

	  /* add localization of F_e here! */
    if (use == AF_YES)
    {
		  rv = femLocF_e(&F_e, i, &F, AF_YES);
    }
	
	  femVecFree(&F_e);
	  femVecFree(&T_e);
	}
	
	return(rv);
}


/* end of fem_loads.c */
