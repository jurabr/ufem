/*
   File name: fdb_exel.c
   Date:      2008/09/14 12:41
   Author:    Jiri Brozovsky

   Copyright (C) 2008 Jiri Brozovsky

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

	 Database for FEM - export of element loads
*/

#include "fdb_fem.h"
#include "cint.h"
#include "fdb_edef.h"
#include "fdb_mdef.h"

/** (Hopefully) Computes lenght of line
 * @param n1 node 1
 * @param n2 node 2
 * @return area
 */

double fdb_line_len(long n1, long n2)
{
  double x[3] ;
  double y[3] ;
  double z[3] ;

  x[1] = fdbInputGetDbl(NODE, NODE_X, n1) ;
  y[1] = fdbInputGetDbl(NODE, NODE_Y, n1) ;
  z[1] = fdbInputGetDbl(NODE, NODE_Z, n1) ;

  x[2] = fdbInputGetDbl(NODE, NODE_X, n2) ;
  y[2] = fdbInputGetDbl(NODE, NODE_Y, n2) ;
  z[2] = fdbInputGetDbl(NODE, NODE_Z, n2) ;

  return (sqrt(
        pow(x[2]-x[1] ,2) +
        pow(y[2]-y[1] ,2) +
        pow(z[2]-z[1] ,2)
        ) );
}

/** Computes area of triangle (TODO: test it!)
 * @param n1 node 1
 * @param n2 node 2
 * @param n3 node 3
 * @param n4 node 4
 * @return area
 */
double fdb_tri_area(long n1, long n2, long n3)
{
  static double A = 0.0 ;
  double x[4] ;
  double y[4] ;
  double z[4] ;
  double s[4] ;
  double hap = 0.0 ;
  int i ;
  int i1, i2 ;

  x[1] = fdbInputGetDbl(NODE, NODE_X, n1) ;
  y[1] = fdbInputGetDbl(NODE, NODE_Y, n1) ;
  z[1] = fdbInputGetDbl(NODE, NODE_Z, n1) ;

  x[2] = fdbInputGetDbl(NODE, NODE_X, n2) ;
  y[2] = fdbInputGetDbl(NODE, NODE_Y, n2) ;
  z[2] = fdbInputGetDbl(NODE, NODE_Z, n2) ;

  x[3] = fdbInputGetDbl(NODE, NODE_X, n3) ;
  y[3] = fdbInputGetDbl(NODE, NODE_Y, n3) ;
  z[3] = fdbInputGetDbl(NODE, NODE_Z, n3) ;

  hap = 0.0 ;

  for (i=1; i<=3; i++)
  {
    i1 = i ;
    i2 = i+1 ;
    if (i2 > 3) { i2 = 1; }

    s[i] = sqrt(
        pow(x[i2]-x[i1] ,2) +
        pow(y[i2]-y[i1] ,2) +
        pow(z[i2]-z[i1] ,2)
        );
    hap += s[i] ;
  }


  hap /= 2.0 ;

  A = sqrt (
      hap *
      (hap - s[1]) * (hap - s[2]) * (hap - s[3])
      ) ;

  return(A);
}


/** Computes area of rectangle (from 2 triangles)
 * @param n1 node 1
 * @param n2 node 2
 * @param n3 node 3
 * @param n4 node 4
 * @return area
 */
double fdb_rect_area(long n1, long n2, long n3, long n4)
{
  return( fdb_tri_area(n1, n2, n3) + fdb_tri_area(n3, n4, n1) );
}

/** Makes nodal forces from element load
 * @param fw file stream to write result
 * @param eLPos index of applied element load (in ELOAD table)
 * @return status
 */
int fdb_eload_expt_fem(FILE *fw, long eLPos, long *node_index, long preparsed)
{
  int rv = AF_OK ;
  long type, etype, epos, efrom, ettype, etpos ;
  long nval, from, npos ;
  long   dir = 0 ;
  long  face = 0 ;
  long  nodes[129] ; /* no more than 128 nodes per element! */
  double val = 0.0 ;
  double A = 0.0 ;
  double load = 0.0 ;
  int    i ;

  type = fdbInputGetInt(ELOAD, ELOAD_TYPE, eLPos) ;
  epos = fdbInputGetInt(ELOAD, ELOAD_EPOS, eLPos) ;
  efrom = fdbInputGetInt(ELEM, ELEM_FROM, epos);
  ettype = fdbInputGetInt(ELEM, ELEM_TYPE, epos) ;
  if ( fdbInputCountInt(ETYPE, ETYPE_ID, ettype, &etpos) < 0)
  {
    fprintf(msgout, "[E] %s: %li\n", _("Element type michmach for element"), fdbInputGetInt(ELEM, ELEM_ID, epos));
    return(AF_ERR_VAL);
  }
  etype = fdbInputGetInt(ETYPE, ETYPE_TYPE, etpos) ;

  nval = fdbInputGetInt(ELOAD, ELOAD_NVAL, eLPos) ;

  if (nval < 2)
  {
    fprintf(msgout,"[E] %s (%li)!\n",_("Invalid number of element data"), nval);
    return(AF_ERR_SIZ);
  }

  from = fdbInputGetInt(ELOAD, ELOAD_FROM, eLPos) ;

  if (from < 0)
  {
    fprintf(msgout,"[E] %s!\n",_("Invalid position of element data"));
    return(AF_ERR_SIZ);
  }

  dir = (long)fdbInputGetDbl(ELVAL, ELVAL_VAL, from) ;
  val = fdbInputGetDbl(ELVAL, ELVAL_VAL, from+1) ;

  /* element - load compatibility testing should be here */

  switch (type)
  {
    case 0: /* empty - testing load */
      return(AF_OK); break ;
    case 1: /* line load */
      /*fprintf(msgout,"[W] %s: %li!\n", _("This type of element load is not implemented, yet"),type);*/
      switch (etype)
      {
        case 2: 
        case 5:

          face = (long) fdbInputGetDbl(ELVAL, ELVAL_VAL, from+2) ;
          switch (face)
          {
            case 1: /* back */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              break ;
            case 2: /* front */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              break ;
            case 3: /* left */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              break ;
            case 4: /* right */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              break ;
            default: 
                    fprintf(msgout,"[E] %s: %li!\n",_("Invalid element face"), face);
										return(AF_ERR_VAL);
                    break;
          }
          A = fdb_line_len(nodes[1], nodes[2]) ;
          load = 0.5 * A * val ;

          for (i=1; i<=2; i++)
          {
            if (preparsed == AF_YES)
            {
              npos = node_index[nodes[i]] ;
            }
            else
            {
              npos = fdbInputGetInt(NODE, NODE_ID,nodes[i]) ;
            }
            fprintf(fw,"%li 3 %li %e\n",npos,dir,load);
          }
          break;
        case 3:
        case 13:
        case 16:
        case 17:
          nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
          nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;

          A = fdb_line_len(nodes[1], nodes[2]) ;
          load = 0.5 * A * val ;

          for (i=1; i<=2; i++)
          {
            if (preparsed == AF_YES)
            {
              npos = node_index[nodes[i]] ;
            }
            else
            {
              npos = fdbInputGetInt(NODE, NODE_ID,nodes[i]) ;
            }
            fprintf(fw,"%li 3 %li %e\n",npos,dir,load);
          }
          break;
        default:
          fprintf(msgout,"[W] %s: %li!\n", _("Element can not carry this load type"),type);
          return(AF_ERR_TYP);
          break;

      }
      break ;
    case 2: /* 2D area load */
      fprintf(msgout,"[W] %s: %li!\n", _("This type of element load is not implemented, yet"),type);

      switch (etype)
      {
        case 2: /* plane */
        case 5: /* slab */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
          
              A = fdb_rect_area(nodes[1], nodes[2], nodes[3], nodes[4]) ;
              load = 0.25 * A * val ;

              for (i=1; i<=4; i++)
              {
                if (preparsed == AF_YES)
                {
                  npos = node_index[nodes[i]] ;
                }
                else
                {
                  npos = fdbInputGetInt(NODE, NODE_ID,nodes[i]) ;
                }
                fprintf(fw,"%li 3 %li %e\n",npos,dir,load);
              }
          break ;
        case 11: /* only 3 nodes! */
            nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
            nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
            nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;

            A = fdb_tri_area(nodes[1], nodes[2], nodes[3]) ;
            load = (1.0/3.0) * A * val ;

            for (i=1; i<=3; i++)
            {
              if (preparsed == AF_YES)
              {
                npos = node_index[nodes[i]] ;
              }
              else
              {
                npos = fdbInputGetInt(NODE, NODE_ID,nodes[i]) ;
              }
              fprintf(fw,"%li 3 %li %e\n",npos,dir,load);
            }
          break ;
        default:
          fprintf(msgout,"[E] %s: %li!\n",_("Incompatible load on element"), fdbInputGetInt(ELEM, ELEM_ID, epos)) ;
          rv = AF_ERR_TYP ;
          break;
      }
      break ;
    case 3: /* 3d area load */
      switch (etype)
      {
        case 9: 
        case 10:
          if (nval < 2)
          {
            fprintf(msgout,"[E] %s!\n",_("Invalid number of element data"));
            return(AF_ERR_SIZ);
          }

          face = (long) fdbInputGetDbl(ELVAL, ELVAL_VAL, from+2) ;

          switch (face)
          {
            case 1: /* back */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              break ;
            case 2: /* front */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+4) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+5) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+6) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+7) ;
              break ;
            case 3: /* left */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+4) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+7) ;
              break ;
            case 4: /* right */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+5) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+6) ;
              break ;
            case 5: /* top */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+0) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+1) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+5) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+4) ;
              break ;
            case 6: /* bottom */
              nodes[1] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+3) ;
              nodes[2] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+2) ;
              nodes[3] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+6) ;
              nodes[4] = fdbInputGetInt(ENODE, ENODE_NPOS, efrom+7) ;
              break ;
            default: 
                    fprintf(msgout,"[E] %s: %li!\n",_("Invalid element face"), face);
										return(AF_ERR_VAL);
                    break;
          }

          A = fdb_rect_area(nodes[1], nodes[2], nodes[3], nodes[4]) ;
          load = 0.25 * A * val ;

          for (i=1; i<=4; i++)
          {
            if (preparsed == AF_YES)
            {
              npos = node_index[nodes[i]] ;
            }
            else
            {
              npos = fdbInputGetInt(NODE, NODE_ID,nodes[i]) ;
            }
            fprintf(fw,"%li 3 %li %e\n",npos,dir,load);
          }
          /* TODO: code here... */
          break ;
        case 8: /* TODO */
        default:
          fprintf(msgout,"[E] %s: %li!\n",_("Incompatible load on element"), fdbInputGetInt(ELEM, ELEM_ID, epos)) ;
          rv = AF_ERR_TYP ;
          break;
      }

      break ;
    case 4: /* volume load */
      fprintf(msgout,"[W] %s: %li!\n", _("This type of element load is not implemented, yet"),type);
      break ;
    default:
            fprintf(msgout,"[E] %s: %li!", 
                _("Invalid element load type on element"),
                fdbInputGetInt(ELOAD, ELOAD_ELEM, eLPos)
                );
            return(AF_ERR_VAL); break ;
  }

  return(rv);
}


/* end of fdb_exel.c */
