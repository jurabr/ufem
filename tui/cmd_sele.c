/*
   File name: cmd_sele.c
   Date:      2004/01/01 12:18
   Author:    Jiri Brozovsky

   Copyright (C) 2004 Jiri Brozovsky

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

	 FEM User Interface - commands for selections

	 $Id: cmd_sele.c,v 1.6 2004/12/30 22:26:58 jirka Exp $
*/

#include "fem_tui.h"
#include "cint.h"
#include "fdb_fem.h"
#include "fdb_edef.h"
#include "fdb_res.h"

/* SELECTIONS */

/** Tests and returns  selection limits (from & to) - integers
 * @param cmd command
 * @param mode mode of selection (result)
 * @param bywhat selection item (result)
 * @param dir coord. direction (if applicable) (result)
 * @return status
 */
int fem_get_sel_limitsInt(char *cmd, long *from, long *to)
{
  int rv = AF_OK;

	if (ciParNum(cmd) > 4) { *from = ciGetParInt(cmd, 4) ; *to = *from ; }
	else { return(AF_ERR_EMP); }
	if (ciParNum(cmd) > 5) { *to   = ciGetParInt(cmd, 5) ; }

  return(rv);
}

/** Tests and returns  selection limits (from & to) - integers
 * @param cmd command
 * @param mode mode of selection (result)
 * @param bywhat selection item (result)
 * @param dir coord. direction (if applicable) (result)
 * @return status
 */
int fem_get_sel_limitsDbl(char *cmd, double *from, double *to)
{
  int rv = AF_OK;

	if (ciParNum(cmd) > 4) { *from = ciGetParDbl(cmd, 4) ; *to = *from ; }
	else { return(AF_ERR_EMP); }
	if (ciParNum(cmd) > 5) { *to   = ciGetParDbl(cmd, 5) ; }

  return(rv);
}

/** Select nodes: "nsel,mode(all|none|invert|select|reselect|unselect|add),what(number/id|loc),dir(x|y|z),from,to"
 * @param cmd command
 * @return status
 */
int func_fem_nsel(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	char  *bywhat = NULL ;
	char  *dir    = NULL ;
	long   imode, ibywhat, idir;
	long   ifrom, ito ;
	double dfrom, dto ;

	imode   = 0 ;
	ibywhat = 0 ;
	idir    = 0 ;

	if (ciParNum(cmd) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NODE, NODE_ID, imode, ifrom, ito))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NODE, NODE_ID, imode, ifrom, ito))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NODE, NODE_ID, imode, ifrom, ito))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

	if (ciParNum(cmd) > 2) 
	{
		if (strlen(bywhat = ciGetParStr(cmd, 2)) < 1) 
		{
			free(bywhat);
			fprintf(msgout,"[E] %s!\n", 
					_("Selection item specification should have at least one character"));
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(bywhat);
			if (strlen(bywhat) < 1) 
			{
				free(bywhat);
				fprintf(msgout,"[E] %s!\n", 
						_("Selection item specification should have at least one character"));
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
		}
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Selection item required"));
		free(bywhat);
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	switch (bywhat[0])
	{
		case 'i':
		case 'n': ibywhat = NODE_ID ; free(bywhat); break ;
		case 'l': ibywhat = NODE_X  ;
							if (ciParNum(cmd) > 3) 
							{
								if ((dir = ciGetParStr(cmd, 3)) == NULL) 
								{
									fprintf(msgout,"[E] %s!\n", ("Direction required"));
									free(bywhat) ; free(dir);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
								else
								{
									ciStrCompr(dir);
									if (strlen(dir) < 1) 
									{
										fprintf(msgout,"[E] %s!\n", _("Invalid direction"));
										free(bywhat) ; free(dir);
										return(tuiCmdReact(cmd, AF_ERR_VAL));
									}
								}
							}
							else
							{
								fprintf(msgout,"[E] %s!\n", ("Direction required"));
								free(bywhat) ; free(dir);
								return(tuiCmdReact(cmd, AF_ERR_EMP));
							}

							switch(dir[0])
							{
								case 'x': idir = NODE_X ; free(dir); break ;
								case 'y': idir = NODE_Y ; free(dir); break ;
								case 'z': idir = NODE_Z ; free(dir); break ;
								default: free(dir); 
												 fprintf(msgout,"[E] %s!\n", 
														 ("Direction required"));
												 return(tuiCmdReact(cmd, AF_ERR_EMP));
												 break ;
							}
							break ;
		case 'r': 
		case 'v': 
  						if ((ResLen <= 0) || (ResActStep >= ResLen)) 
							{ 
								fprintf(msgout,"[E] %s!\n", ("You are NOT in the postprocessor"));
								free(bywhat) ;
								return(tuiCmdReact(cmd, AF_ERR_VAL));
							}
              if (ciParNum(cmd) > 3) 
							{
								if ((idir = ciGetParInt(cmd, 3)) < 1) 
								{
									fprintf(msgout,"[E] %s!\n", ("Invalid type of result"));
									free(bywhat) ;
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							else
							{
								fprintf(msgout,"[E] %s!\n", ("Result type must be specified"));
								free(bywhat) ;
								return(tuiCmdReact(cmd, AF_ERR_EMP));
							}
							
							free(bywhat) ;
							if ((rv=fem_get_sel_limitsDbl(cmd, &dfrom, &dto)) != AF_OK) 
							{
								return ( tuiCmdReact(cmd, rv) ) ;
							}
							rv =  fdbSelectNodeRes(idir, imode, dfrom, dto) ;
						  return ( tuiCmdReact(cmd, rv) ) ;
							break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown item"));
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	if (ibywhat == NODE_ID)
	{
		if ((rv=fem_get_sel_limitsInt(cmd, &ifrom, &ito)) != AF_OK) 
		{
			return ( tuiCmdReact(cmd, rv) ) ;
		}
		rv = (fdbInputSelectIntInterval(NODE, NODE_ID, imode, ifrom, ito)) ;
	}
	else
	{
		if ((rv=fem_get_sel_limitsDbl(cmd, &dfrom, &dto)) != AF_OK) 
		{
			return ( tuiCmdReact(cmd, rv) ) ;
		}
		rv = (fdbInputSelectDblInterval(NODE, idir, imode, dfrom, dto)) ;
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Select elements: "esel,mode(all|none|invert|select|reselect|unselect|add),what(id|etype|rset|mat|set|loc),[dir(x|y|z)],from,to"
 * @param cmd command
 * @return status
 */
int func_fem_esel(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	char  *bywhat = NULL ;
	long   imode, ibywhat;
	long   ifrom, ito ;
	double dfrom, dto ;
	long   collumn ;
	char  *dir    = NULL ;

	imode   = 0 ;
	ibywhat = 0 ;

	if (ciParNum(cmd) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ELEM, ELEM_ID, imode, ifrom, ito))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ELEM, ELEM_ID, imode, ifrom, ito))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ELEM, ELEM_ID, imode, ifrom, ito))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

	if (ciParNum(cmd) > 2) 
	{
		if (strlen(bywhat = ciGetParStr(cmd, 2)) < 1) 
		{
			free(bywhat);
			fprintf(msgout,"[E] %s!\n", 
					_("Selection item specification should have at least one character"));
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(bywhat);
			if (strlen(bywhat) < 1) 
			{
				free(bywhat);
				fprintf(msgout,"[E] %s!\n", 
						_("Selection item specification should have at least one character"));
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
		}
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Selection item required"));
		free(bywhat);
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	switch (bywhat[0])
	{
		case 'i': ibywhat = ELEM_ID ; free(bywhat); break ;
		case 't': 
		case 'e': ibywhat = ELEM_TYPE ; free(bywhat); break ;
		case 'r': ibywhat = ELEM_RS ; free(bywhat); break ;
		case 'm': ibywhat = ELEM_MAT ; free(bywhat); break ;
		case 'g': 
		case 's': ibywhat = ELEM_SET ; free(bywhat); break ;
		case 'l': /* by element centroid - bit complicated */
              if (ciParNum(cmd) > 3) 
							{
								if ((dir = ciGetParStr(cmd, 3)) == NULL) 
								{
									fprintf(msgout,"[E] %s!\n", ("Direction required"));
									free(bywhat) ; free(dir);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
								else
								{
									ciStrCompr(dir);
									if (strlen(dir) < 1) 
									{
										fprintf(msgout,"[E] %s!\n", _("Invalid direction"));
										free(bywhat) ; free(dir);
										return(tuiCmdReact(cmd, AF_ERR_VAL));
									}
								}
							}
							else
							{
								fprintf(msgout,"[E] %s!\n", ("Direction required"));
								free(bywhat) ; free(dir);
								return(tuiCmdReact(cmd, AF_ERR_EMP));
							}
						  switch(dir[0])
							{
								case 'x': collumn = NODE_X ; free(dir); break ;
								case 'y': collumn = NODE_Y ; free(dir); break ;
								case 'z': collumn = NODE_Z ; free(dir); break ;
								default: free(dir); 
												 fprintf(msgout,"[E] %s!\n", 
														 ("Direction required"));
												 return(tuiCmdReact(cmd, AF_ERR_EMP));
												 break ;
							}
							if ((rv=fem_get_sel_limitsDbl(cmd, &dfrom, &dto)) != AF_OK) 
							{
								return ( tuiCmdReact(cmd, rv) ) ;
							}
							rv = fdbSelectElemPos(collumn, imode, dfrom, dto);
						  return ( tuiCmdReact(cmd, rv) ) ;
							break ;
		case 'v': /* by element result - bit complicated */
  						if ((ResLen <= 0) || (ResActStep >= ResLen)) 
							{ 
								fprintf(msgout,"[E] %s!\n", ("You are NOT in the postprocessor"));
								free(bywhat) ;
								return(tuiCmdReact(cmd, AF_ERR_VAL));
							}
              if (ciParNum(cmd) > 3) 
							{
								if ((collumn = ciGetParInt(cmd, 3)) < 1) 
								{
									fprintf(msgout,"[E] %s!\n", ("Invalid type of result"));
									free(bywhat) ;
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							else
							{
								fprintf(msgout,"[E] %s!\n", ("Result type must be specified"));
								free(bywhat) ;
								return(tuiCmdReact(cmd, AF_ERR_EMP));
							}
							
							free(bywhat) ;
							if ((rv=fem_get_sel_limitsDbl(cmd, &dfrom, &dto)) != AF_OK) 
							{
								return ( tuiCmdReact(cmd, rv) ) ;
							}
							rv =  fdbSelectElemRes(collumn, imode, dfrom, dto) ;
						  return ( tuiCmdReact(cmd, rv) ) ;
							break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown item"));
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
  }

	if ((rv=fem_get_sel_limitsInt(cmd, &ifrom, &ito)) != AF_OK) 
	{
		return ( tuiCmdReact(cmd, rv) ) ;
	}
	rv = (fdbInputSelectIntInterval(ELEM, ibywhat, imode, ifrom, ito)) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Select displacements: "dsel,mode(all|none|invert|select|reselect|unselect|add),what(id|node|type|set|value),,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_dsel(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	char  *bywhat = NULL ;
	long   imode, ibywhat;
	long   ifrom, ito ;
	long   itype ;
	double dfrom, dto ;

	imode   = 0 ;
	ibywhat = 0 ;

	if (ciParNum(cmd) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NDISP, NDISP_ID, imode, ifrom, ito))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NDISP, NDISP_ID, imode, ifrom, ito))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NDISP, NDISP_ID, imode, ifrom, ito))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

	if (ciParNum(cmd) > 2) 
	{
		if (strlen(bywhat = ciGetParStr(cmd, 2)) < 1) 
		{
			free(bywhat);
			fprintf(msgout,"[E] %s!\n", 
					_("Selection item specification should have at least one character"));
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(bywhat);
			if (strlen(bywhat) < 1) 
			{
				free(bywhat);
				fprintf(msgout,"[E] %s!\n", 
						_("Selection item specification should have at least one character"));
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
		}
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Selection item required"));
		free(bywhat);
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	itype = 0 ;

	switch (bywhat[0])
	{
		case 'i': ibywhat = NDISP_ID ; free(bywhat); break ;
		case 'n': ibywhat = NDISP_NODE ; free(bywhat); break ;
		case 't': ibywhat = NDISP_TYPE ; free(bywhat); break ;
		case 'g': 
		case 's': ibywhat = NDISP_SET ; free(bywhat); break ;
		case 'v': ibywhat = NDISP_VAL ; free(bywhat); itype = 1 ; break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown item"));
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
  }

	if (itype == 0)
	{
		if ((rv=fem_get_sel_limitsInt(cmd, &ifrom, &ito)) != AF_OK) 
		{
			return ( tuiCmdReact(cmd, rv) ) ;
		}
		rv = (fdbInputSelectIntInterval(NDISP, ibywhat, imode, ifrom, ito)) ;
	}
	else
	{
		if ((rv=fem_get_sel_limitsDbl(cmd, &dfrom, &dto)) != AF_OK) 
		{
			return ( tuiCmdReact(cmd, rv) ) ;
		}
		rv = (fdbInputSelectDblInterval(NDISP, ibywhat, imode, dfrom, dto)) ;
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}


/** Select node loads: "fsel,mode(all|none|invert|select|reselect|unselect|add),what(id|node|type|set|value),,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_fsel(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	char  *bywhat = NULL ;
	long   imode, ibywhat;
	long   itype ;
	long   ifrom, ito ;
	double dfrom, dto ;

	imode   = 0 ;
	ibywhat = 0 ;

	if (ciParNum(cmd) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NLOAD, NLOAD_ID, imode, ifrom, ito))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NLOAD, NLOAD_ID, imode, ifrom, ito))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntInterval(NLOAD, NLOAD_ID, imode, ifrom, ito))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

	if (ciParNum(cmd) > 2) 
	{
		if (strlen(bywhat = ciGetParStr(cmd, 2)) < 1) 
		{
			free(bywhat);
			fprintf(msgout,"[E] %s!\n", 
					_("Selection item specification should have at least one character"));
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(bywhat);
			if (strlen(bywhat) < 1) 
			{
				free(bywhat);
				fprintf(msgout,"[E] %s!\n", 
						_("Selection item specification should have at least one character"));
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
		}
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Selection item required"));
		free(bywhat);
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	itype = 0 ;

	switch (bywhat[0])
	{
		case 'i': ibywhat = NLOAD_ID ; free(bywhat); break ;
		case 'n': ibywhat = NLOAD_NODE ; free(bywhat); break ;
		case 't': ibywhat = NLOAD_TYPE ; free(bywhat); break ;
		case 'g': 
		case 's': ibywhat = NLOAD_SET ; free(bywhat); break ;
		case 'v': ibywhat = NLOAD_VAL ; free(bywhat); itype = 1 ; break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown item"));
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
  }

	if (itype == 0)
	{
		if ((rv=fem_get_sel_limitsInt(cmd, &ifrom, &ito)) != AF_OK) 
		{
			return ( tuiCmdReact(cmd, rv) ) ;
		}
		rv = (fdbInputSelectIntInterval(NLOAD, ibywhat, imode, ifrom, ito)) ;
	}
	else
	{
		if ((rv=fem_get_sel_limitsDbl(cmd, &dfrom, &dto)) != AF_OK) 
		{
			return ( tuiCmdReact(cmd, rv) ) ;
		}
		rv = (fdbInputSelectDblInterval(NLOAD, ibywhat, imode, dfrom, dto)) ;
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Select element loads: "elsel,mode(all|none|invert|select|reselect|unselect|add),what(id|elem|type|set),,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_eload_sel(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	char  *bywhat = NULL ;
	long   imode, ibywhat;
	long   ifrom, ito ;

	imode   = 0 ;
	ibywhat = 0 ;

	if (ciParNum(cmd) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ELOAD, ELOAD_ID, imode, ifrom, ito))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ELOAD, ELOAD_ID, imode, ifrom, ito))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ELOAD, ELOAD_ID, imode, ifrom, ito))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

	if (ciParNum(cmd) > 2) 
	{
		if (strlen(bywhat = ciGetParStr(cmd, 2)) < 1) 
		{
			free(bywhat);
			fprintf(msgout,"[E] %s!\n", 
					_("Selection item specification should have at least one character"));
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(bywhat);
			if (strlen(bywhat) < 1) 
			{
				free(bywhat);
				fprintf(msgout,"[E] %s!\n", 
						_("Selection item specification should have at least one character"));
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
		}
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Selection item required"));
		free(bywhat);
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	switch (bywhat[0])
	{
		case 'i': ibywhat = ELOAD_ID ; free(bywhat); break ;
		case 'e': ibywhat = ELOAD_ELEM ; free(bywhat); break ;
		case 't': ibywhat = ELOAD_TYPE ; free(bywhat); break ;
		case 'g': 
		case 's': ibywhat = ELOAD_SET ; free(bywhat); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown item"));
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
  }

	if ((rv=fem_get_sel_limitsInt(cmd, &ifrom, &ito)) != AF_OK) 
	{
		return ( tuiCmdReact(cmd, rv) ) ;
	}
	rv = (fdbInputSelectIntInterval(ELOAD, ibywhat, imode, ifrom, ito)) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Select gravitation: "accelsel,mode(all|none|invert|select|reselect|unselect|add),what(id/number|dir|set),,from,to"
 * @param cmd command
 * @return status
 */
int func_fem_acel_sel(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	char  *bywhat = NULL ;
	long   imode, ibywhat;
	long   ifrom, ito ;

	imode   = 0 ;
	ibywhat = 0 ;

	if (ciParNum(cmd) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(GRAV, GRAV_ID, imode, ifrom, ito))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(GRAV, GRAV_ID, imode, ifrom, ito))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntInterval(GRAV, GRAV_ID, imode, ifrom, ito))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

	if (ciParNum(cmd) > 2) 
	{
		if (strlen(bywhat = ciGetParStr(cmd, 2)) < 1) 
		{
			free(bywhat);
			fprintf(msgout,"[E] %s!\n", 
					_("Selection item specification should have at least one character"));
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(bywhat);
			if (strlen(bywhat) < 1) 
			{
				free(bywhat);
				fprintf(msgout,"[E] %s!\n", 
						_("Selection item specification should have at least one character"));
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
		}
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Selection item required"));
		free(bywhat);
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	switch (bywhat[0])
	{
		case 'n': 
		case 'i': ibywhat = GRAV_ID ; free(bywhat); break ;
		case 'd': ibywhat = GRAV_DIR ; free(bywhat); break ;
		case 's': ibywhat = GRAV_SET ; free(bywhat); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown item"));
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
  }

	if ((rv=fem_get_sel_limitsInt(cmd, &ifrom, &ito)) != AF_OK) 
	{
		return ( tuiCmdReact(cmd, rv) ) ;
	}
	rv = (fdbInputSelectIntInterval(GRAV, ibywhat, imode, ifrom, ito)) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}

/* --------------------------------------------- */
/** Select keypoints: "ksel,mode(all|none|invert|select|reselect|unselect|add),what(number/id|loc),dir(x|y|z),from,to"
 * @param cmd command
 * @return status
 */
int func_fem_ksel(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	char  *bywhat = NULL ;
	char  *dir    = NULL ;
	long   imode, ibywhat, idir;
	long   ifrom, ito ;
	double dfrom, dto ;

	imode   = 0 ;
	ibywhat = 0 ;
	idir    = 0 ;

	if (ciParNum(cmd) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(KPOINT, KPOINT_ID, imode, ifrom, ito))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(KPOINT, KPOINT_ID, imode, ifrom, ito))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntInterval(KPOINT, KPOINT_ID, imode, ifrom, ito))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

	if (ciParNum(cmd) > 2) 
	{
		if (strlen(bywhat = ciGetParStr(cmd, 2)) < 1) 
		{
			free(bywhat);
			fprintf(msgout,"[E] %s!\n", 
					_("Selection item specification should have at least one character"));
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(bywhat);
			if (strlen(bywhat) < 1) 
			{
				free(bywhat);
				fprintf(msgout,"[E] %s!\n", 
						_("Selection item specification should have at least one character"));
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
		}
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Selection item required"));
		free(bywhat);
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	switch (bywhat[0])
	{
		case 'i':
		case 'n': ibywhat = KPOINT_ID ; free(bywhat); break ;
		case 'l': ibywhat = KPOINT_X  ;
							if (ciParNum(cmd) > 3) 
							{
								if ((dir = ciGetParStr(cmd, 3)) == NULL) 
								{
									fprintf(msgout,"[E] %s!\n", ("Direction required"));
									free(bywhat) ; free(dir);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
								else
								{
									ciStrCompr(dir);
									if (strlen(dir) < 1) 
									{
										fprintf(msgout,"[E] %s!\n", _("Invalid direction"));
										free(bywhat) ; free(dir);
										return(tuiCmdReact(cmd, AF_ERR_VAL));
									}
								}
							}
							else
							{
								fprintf(msgout,"[E] %s!\n", ("Direction required"));
								free(bywhat) ; free(dir);
								return(tuiCmdReact(cmd, AF_ERR_EMP));
							}

							switch(dir[0])
							{
								case 'x': idir = KPOINT_X ; free(dir); break ;
								case 'y': idir = KPOINT_Y ; free(dir); break ;
								case 'z': idir = KPOINT_Z ; free(dir); break ;
								default: free(dir); 
												 fprintf(msgout,"[E] %s!\n", 
														 ("Direction required"));
												 return(tuiCmdReact(cmd, AF_ERR_EMP));
												 break ;
							}
							break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown item"));
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	if (ibywhat == KPOINT_ID)
	{
		if ((rv=fem_get_sel_limitsInt(cmd, &ifrom, &ito)) != AF_OK) 
		{
			return ( tuiCmdReact(cmd, rv) ) ;
		}
		rv = (fdbInputSelectIntInterval(KPOINT, KPOINT_ID, imode, ifrom, ito)) ;
	}
	else
	{
		if ((rv=fem_get_sel_limitsDbl(cmd, &dfrom, &dto)) != AF_OK) 
		{
			return ( tuiCmdReact(cmd, rv) ) ;
		}
		rv = (fdbInputSelectDblInterval(KPOINT, idir, imode, dfrom, dto)) ;
	}

	return ( tuiCmdReact(cmd, rv) ) ;
}

/** Select entities: "gesel,mode(all|none|invert|select|reselect|unselect|add),what(id|type|etype|rset|mat|set|loc),[loc:(x|y|z)],from,to"
 * @param cmd command
 * @return status
 */
int func_fem_gesel(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	char  *bywhat = NULL ;
	long   imode, ibywhat;
	long   ifrom, ito ;
	double dfrom, dto ;
	long   collumn ;
	char  *dir    = NULL ;

	imode   = 0 ;
	ibywhat = 0 ;

	if (ciParNum(cmd) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ENTITY, ENTITY_ID, imode, ifrom, ito))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ENTITY, ENTITY_ID, imode, ifrom, ito))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntInterval(ENTITY, ENTITY_ID, imode, ifrom, ito))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

	if (ciParNum(cmd) > 2) 
	{
		if (strlen(bywhat = ciGetParStr(cmd, 2)) < 1) 
		{
			free(bywhat);
			fprintf(msgout,"[E] %s!\n", 
					_("Selection item specification should have at least one character"));
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(bywhat);
			if (strlen(bywhat) < 1) 
			{
				free(bywhat);
				fprintf(msgout,"[E] %s!\n", 
						_("Selection item specification should have at least one character"));
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
		}
	}
	else
	{
		fprintf(msgout,"[E] %s!\n", _("Selection item required"));
		free(bywhat);
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	switch (bywhat[0])
	{
		case 'i': ibywhat = ENTITY_ID ; free(bywhat); break ;
		case 't': ibywhat = ENTITY_TYPE ; free(bywhat); break ;
		case 'e': ibywhat = ENTITY_ETYPE ; free(bywhat); break ;
		case 'r': ibywhat = ENTITY_RS ; free(bywhat); break ;
		case 'm': ibywhat = ENTITY_MAT ; free(bywhat); break ;
		case 'g': 
		case 's': ibywhat = ENTITY_SET ; free(bywhat); break ;
		case 'l': /* by entity centroid - bit complicated */
              if (ciParNum(cmd) > 3) 
							{
								if ((dir = ciGetParStr(cmd, 3)) == NULL) 
								{
									fprintf(msgout,"[E] %s!\n", ("Direction required"));
									free(bywhat) ; free(dir);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
								else
								{
									ciStrCompr(dir);
									if (strlen(dir) < 1) 
									{
										fprintf(msgout,"[E] %s!\n", _("Invalid direction"));
										free(bywhat) ; free(dir);
										return(tuiCmdReact(cmd, AF_ERR_VAL));
									}
								}
							}
							else
							{
								fprintf(msgout,"[E] %s!\n", ("Direction required"));
								free(bywhat) ; free(dir);
								return(tuiCmdReact(cmd, AF_ERR_EMP));
							}
						  switch(dir[0])
							{
								case 'x': collumn = KPOINT_X ; free(dir); break ;
								case 'y': collumn = KPOINT_Y ; free(dir); break ;
								case 'z': collumn = KPOINT_Z ; free(dir); break ;
								default: free(dir); 
												 fprintf(msgout,"[E] %s!\n", 
														 ("Direction required"));
												 return(tuiCmdReact(cmd, AF_ERR_EMP));
												 break ;
							}
							if ((rv=fem_get_sel_limitsDbl(cmd, &dfrom, &dto)) != AF_OK) 
							{
								return ( tuiCmdReact(cmd, rv) ) ;
							}
							rv = fdbSelectEntPos(collumn, imode, dfrom, dto);
						  return ( tuiCmdReact(cmd, rv) ) ;
							break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown item"));
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
  }

	if ((rv=fem_get_sel_limitsInt(cmd, &ifrom, &ito)) != AF_OK) 
	{
		return ( tuiCmdReact(cmd, rv) ) ;
	}
	rv = (fdbInputSelectIntInterval(ENTITY, ibywhat, imode, ifrom, ito)) ;

	return ( tuiCmdReact(cmd, rv) ) ;
}
/* --------------------------------------------- */



/** Select nodes by elements: "nsle"
 * @param cmd command
 * @return status
 */
int func_fem_sel_nsle(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_nsle()) ) ;
}

/** Select nodes by forces: "nslf"
 * @param cmd command
 * @return status
 */
int func_fem_sel_nslf(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_nslf()) ) ;
}

/** Select nodes by displacements: "nsld"
 * @param cmd command
 * @return status
 */
int func_fem_sel_nsld(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_nsld()) ) ;
}

/** Select elements by element loads: "eslel"
 * @param cmd command
 * @return status
 */
int func_fem_sel_eslel(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_eslel()) ) ;
}

/** Select element loads by elements: "elsle"
 * @param cmd command
 * @return status
 */
int func_fem_sel_elsle(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_elsle()) ) ;
}

/** Select forces by nodes: "fsln"
 * @param cmd command
 * @return status
 */
int func_fem_sel_fsln(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_fsln()) ) ;
}


/** Select displacements by nodes: "dsln"
 * @param cmd command
 * @return status
 */
int func_fem_sel_dsln(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_dsln()) ) ;
}

/** Select elements by nodes: "esln"
 * @param cmd command
 * @return status
 */
int func_fem_sel_esln(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_esln()) ) ;
}

/** Select keypoints by entities: "kslge"
 * @param cmd command
 * @return status
 */
int func_fem_sel_kslge(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_kslge()) ) ;
}

/** Select entities by keypoints: "geslk"
 * @param cmd command
 * @return status
 */
int func_fem_sel_geslk(char *cmd)
{
	return ( tuiCmdReact(cmd, fem_sele_geslk()) ) ;
}

/** Select everything "allsel" */
int func_fem_sel_allsel(char *cmd)
{
  ciRunCmd("ksel,all");
  ciRunCmd("gesel,all");

  ciRunCmd("nsel,all");
  ciRunCmd("esel,all");
  ciRunCmd("dsel,all");
  ciRunCmd("fsel,all");
  ciRunCmd("accelsel,all");
  ciRunCmd("elsel,all");
 
	return ( tuiCmdReact(cmd, AF_OK) ) ;
}

/** Select stuff by load steps: "slstep,mode(all|none|invert|select|reselect|unselect|add),steps..."
 * @param cmd command
 * @return status
 */
int func_fem_sel_by_step(char *cmd)
{
  int    rv     = AF_OK ;
	char  *mode   = NULL ;
	long   imode ;
  long   nvals ;
  long  *values = NULL ;
  long   i ;

	imode   = 0 ;

	if ((nvals=ciParNum(cmd)) > 1) 
	{
		if ((mode = ciGetParStr(cmd, 1)) == NULL) 
		{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
				free(mode);
			return(tuiCmdReact(cmd, AF_ERR_VAL));
		}
		else
		{
			ciStrCompr(mode);
			if (strlen(mode) < 1) 
			{
				fprintf(msgout,"[E] %s!\n", 
						_("Mode specification is invalid"));
				free(mode);
				return(tuiCmdReact(cmd, AF_ERR_VAL));
			}
			else
			{
				if (strlen(mode) < 2) 
				{
					if (mode[0] == 'a')
					{
						fprintf(msgout,"[E] %s!\n", 
								_("Mode specification should have at least two characters"));
						free(mode);
						return(tuiCmdReact(cmd, AF_ERR_VAL));
					}
				}
			}
		}
	}
	else
	{
			fprintf(msgout,"[E] %s!\n", 
					_("Mode specification required"));
		free(mode);
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	switch (mode[0])
	{
		case 'i': imode = FDB_SELE_INVERT ; 
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntList(NODE, NODE_ID, imode, 0, NULL))) ;
							break ;
		case 'n': imode = FDB_SELE_NONE ;
							free(mode);
							return(tuiCmdReact(cmd, fdbInputSelectIntList(NODE, NODE_ID, imode, 0, NULL))) ;
							break;
		case 'a': if (mode[1] == 'l')
							{
								/* all */
								imode = FDB_SELE_ALL ;
								free(mode);
								return(tuiCmdReact(cmd, fdbInputSelectIntList(NODE, NODE_ID, imode, 0, NULL))) ;
							}
							else
							{
								if (mode[1] == 'd')
								{
									imode = FDB_SELE_ADD ;
									free(mode);
								}
								else
								{
									fprintf(msgout,"[E] %s!\n",_("Invalid mode"));
									free(mode);
									return(tuiCmdReact(cmd, AF_ERR_VAL));
								}
							}
							break;
		case 's': imode = FDB_SELE_SELECT; free(mode); break ;
		case 'r': imode = FDB_SELE_RESELE; free(mode); break ;
		case 'u': imode = FDB_SELE_UNSELE; free(mode); break ;
		default:  fprintf(msgout,"[E] %s!\n",_("Unknown mode"));
							free(mode); 
							return(tuiCmdReact(cmd, AF_ERR_VAL));
							break;
	}

	mode = NULL ; /* no longer needed */

  if ((imode == FDB_SELE_ALL) || (imode == FDB_SELE_INVERT) || (imode == FDB_SELE_NONE))
  {
    nvals  = 0 ;
    values = NULL ;
  }
  else
  {
    nvals -= 2 ;

    if (nvals <= 0)
    {
      nvals = 0 ;
		  imode = FDB_SELE_ALL ;
    }
    else
    {
      if ((values=femIntAlloc(nvals)) == NULL)
      {
		    fprintf(msgout,"[E] %s!\n", _("Out of memory"));
		    return ( tuiCmdReact(cmd, AF_ERR_MEM) ) ;
      }

      for (i=0; i<nvals; i++)
      {
        values[i] = ciGetParInt(cmd, i+2) ;
        if (values[i] <= 0) 
        {
          nvals = i ;
          break ;
        }
      }

      if (nvals <= 0)
      {
        nvals = 0 ;
        femIntFree(values) ;
		    imode = FDB_SELE_ALL ;
      }
    }
  }
  

	rv = (fdbInputSelectIntList(ELEM, ELEM_SET, imode, nvals, values)) ;
	rv = (fdbInputSelectIntList(NDISP, NDISP_SET, imode, nvals, values)) ;
	rv = (fdbInputSelectIntList(NLOAD, NLOAD_SET, imode, nvals, values)) ;
	rv = (fdbInputSelectIntList(ELOAD, ELOAD_SET, imode, nvals, values)) ;
	rv = (fdbInputSelectIntList(GRAV, GRAV_SET, imode, nvals, values)) ;

	rv = (fdbInputSelectIntList(ENTITY, ENTITY_SET, imode, nvals, values)) ;

  rv = fem_sele_nsle() ;

  rv = fem_sele_kslge() ;

  if (nvals > 0)
  {
    nvals = 0 ;
    femIntFree(values) ;
  }

	return ( tuiCmdReact(cmd, rv) ) ;
}

/* end of cmd_sele.c */
