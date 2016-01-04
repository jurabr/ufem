/*
   File name: fem_alia.c
   Date:      2003/11/22 20:00
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

	 FEM - user interface - command line aliases for misc. number

	 $Id: fem_alia.c,v 1.21 2005/07/11 17:57:09 jirka Exp $
*/

#include "cint.h"

/** Creates default variables - aliases for some properties
 * @return status
 */
int fem_create_cmd_aliases(void)
{
  int rv = AF_OK ;

  /* euclidian geometry values */
  if ((rv=ciAddVarGrp("yz","1","euclide")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("zy","1","euclide")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("xz","2","euclide")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("zx","2","euclide")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("xy","3","euclide")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("yx","3","euclide")) != AF_OK) {goto memFree;}
  
	/* names of element types */
  if ((rv=ciAddVarGrp("link1","1","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("plane2","2","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("beam3","3","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("solid4","4","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("slab5","5","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("link6","6","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("link7","7","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("solid8","8","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("solid9","9","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("solid10","10","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("plane11","11","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("solid12","12","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("beam14","14","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("therm18","18","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("therm19","19","etype")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("therm20","20","etype")) != AF_OK) {goto memFree;}

  /* names of material types */
  if ((rv=ciAddVarGrp("hooke1","1","mattype")) != AF_OK) {goto memFree;}
  /*if ((rv=ciAddVarGrp("crack2","2","mattype")) != AF_OK) {goto memFree;}*/
  /*if ((rv=ciAddVarGrp("chen3","3","mattype")) != AF_OK) {goto memFree;}*/
  if ((rv=ciAddVarGrp("plastic4","4","mattype")) != AF_OK) {goto memFree;}
  /*if ((rv=ciAddVarGrp("hooke5","5","mattype")) != AF_OK) {goto memFree;}*/
  /*if ((rv=ciAddVarGrp("crack8","8","mattype")) != AF_OK) {goto memFree;}*/
  /*if ((rv=ciAddVarGrp("clay9","9","mattype")) != AF_OK) {goto memFree;}*/

  /* names of boundary conditions */
  if ((rv=ciAddVarGrp("ux","1","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("uy","2","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("uz","3","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rotx","4","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("roty","5","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rotz","6","disp")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("temp","7","disp")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("stiffx","8","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffy","9","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffz","10","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffrx","11","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffry","12","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffrz","13","disp")) != AF_OK) {goto memFree;}

	/* one-directional boundary conditions (p=plus only, m=minus only): */
  if ((rv=ciAddVarGrp("ux+","15","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("uy+","16","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("uz+","17","disp")) != AF_OK) {goto memFree;}
#if 1
  if ((rv=ciAddVarGrp("rotx+","18","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("roty+","19","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rotz+","20","disp")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("stiffx+","22","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffy+","23","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffz+","24","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffrx+","25","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffry+","26","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffrz+","27","disp")) != AF_OK) {goto memFree;}
#endif

  if ((rv=ciAddVarGrp("ux-","29","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("uy-","30","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("uz-","31","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rotx-","32","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("roty-","33","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rotz-","34","disp")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("stiffx-","36","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffy-","37","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffz-","38","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffrx-","39","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffry-","40","disp")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stiffrz-","41","disp")) != AF_OK) {goto memFree;}



  /* degrees of freedom */
  if ((rv=ciAddVarGrp("ux",  "1","degree_of_freedom")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("uy",  "2","degree_of_freedom")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("uz",  "3","degree_of_freedom")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rotx","4","degree_of_freedom")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("roty","5","degree_of_freedom")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rotz","6","degree_of_freedom")) != AF_OK) {goto memFree;}

  /* names of loads */
  if ((rv=ciAddVarGrp("fx","1","load")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fy","2","load")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fz","3","load")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("mx","4","load")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("my","5","load")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("mz","6","load")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("heat","7","load")) != AF_OK) {goto memFree;}

	/* names of real set properties */
  if ((rv=ciAddVarGrp("area","1","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("width","2","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("height","3","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("angle1","4","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rreinf1","5","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("angle2","6","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rreinf2","7","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("ix","8","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("iy","9","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("iz","10","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("hinge_a","11","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("hinge_b","12","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("winkler","13","real")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("hinge_x_1","17","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("hinge_y_1","18","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("hinge_z_1","19","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("hinge_x_2","20","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("hinge_y_2","21","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("hinge_z_2","22","real")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("excentr_y","25","real")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("excentr_z","26","real")) != AF_OK) {goto memFree;}

	/* names of material properties */
  if ((rv=ciAddVarGrp("dens","1","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("ex","2","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("ey","3","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("ez","4","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("prxy","5","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("g","6","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("ftens", "7","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fcompr", "8","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("crcken", "9","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("ereduc", "10","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fyc", "12","material")) != AF_OK) {goto memFree;} /* chen's concrete */
  if ((rv=ciAddVarGrp("fybc","13","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fyt", "14","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fuc", "15","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fubc","16","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fut", "17","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("cohes", "18","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("frict", "19","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("hard", "20","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("nuxy", "21","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("nuyz", "22","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("nuzx", "23","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("gxy", "24","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("gyz", "25","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("gzx", "26","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("model", "29","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rambk", "30","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rambn", "31","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("shred", "32","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("fctype", "33","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("crtype", "34","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("crshen", "35","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("price", "36","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("kxx", "37","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("alpha", "38","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("c", "39","material")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("angle", "40","material")) != AF_OK) {goto memFree;}

  /* names of results */
  if ((rv=ciAddVarGrp("f_x","1","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("f_y","2","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("f_z","3","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("m_x","4","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("m_y","5","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("m_z","6","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_x","7","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_y","8","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_z","9","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_xy","10","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_yz","11","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_zx","12","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_x","13","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_y","14","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_z","15","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_xy","16","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_yz","17","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_zx","18","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_1","19","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_2","20","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_3","21","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_1","22","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_2","23","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_3","24","result")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("e_dir1","25","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_dir2","26","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("e_dir3","27","result")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("m_xy","34","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("v_x","35","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("v_y","36","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("sm_x","48","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("sm_y","49","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("sm_xy","50","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("sv_xz","51","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("sv_yz","52","result")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("s_tmax","53","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_m1","54","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_m2","55","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_msmax","56","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_mxdim","57","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_mydim","58","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("s_vmis","59","result")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("stat1","63","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("stat2","64","result")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("s_temp","71","result")) != AF_OK) {goto memFree;}

  if ((rv=ciAddVarGrp("u_x","-1","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("u_y","-2","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("u_z","-3","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rot_x","-4","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rot_y","-5","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rot_z","-6","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("u_temp","-7","result")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("u_sum","-99","result")) != AF_OK) {goto memFree;}

  /* gfx view parameters  */
  if ((rv=ciAddVarGrp("move_pos","1","view")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("move_neg","-1","view")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rot_pos","15","view")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rot_neg","-15","view")) != AF_OK) {goto memFree;}

  /* geometry */
  if ((rv=ciAddVarGrp("line","1","geom_ent")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("rectangle","2","geom_ent")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("brick","3","geom_ent")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("cvbrick","4","geom_ent")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("cvrectangle","5","geom_ent")) != AF_OK) {goto memFree;}

  /* plane */
  if ((rv=ciAddVarGrp("pxy","0","pplane")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("pyz","1","pplane")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("pzx","2","pplane")) != AF_OK) {goto memFree;}

  /* solver settings */
  if ((rv=ciAddVarGrp("linear","1","solver")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("lin","1","solver")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("nrm","2","solver")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("alm","3","solver")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("dds","6","solver")) != AF_OK) {goto memFree;}
  if ((rv=ciAddVarGrp("modal","7","solver")) != AF_OK) {goto memFree;}

  /*if ((rv=ciAddVarGrp("","","")) != AF_OK) {goto memFree;}*/

memFree:
  if (rv != AF_OK)
  {
    fprintf(msgout,"[E] %s!\n",_("Aliases are NOT created"));
  }
  return(rv);
}

/* end of fem_alia.c */
