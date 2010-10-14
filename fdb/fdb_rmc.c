/*
   File name: fdb_rmc.c
   Date:      2004/02/26 07:25
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

	 Database for FEM: Monte Carlo results

   $Id: fdb_rmc.c,v 1.1 2004/02/26 17:42:10 jirka Exp $
*/

#define FDB_LONG_STRING 4096

#include "fdb_res.h"

/** Reads data from Monte Carlo solution
 * @param fname name of file with result summary
 * @return status
 */
int fdb_read_monte_res(char *fname)
{
	int   rv = AF_OK ;
	char  name[FDB_LONG_STRING+1];
	FILE *fr  = NULL ;
	double sims, fail_num;
	long save_mode;
	long data_len_node = 0 ;
	long data_len_reac = 0 ;
	long data_len_elem = 0 ;
	int i;

	/* result data stucture allocation */
	if (ResLen > 0) { fdbFreeResSpacePtrs(); }
	ResLen = 8 ;
	if ((rv=fdbAllocResSpacePtrs()) != AF_OK)
	{
		ResLen = 0 ;
		fprintf(msgout,"[E] %s!\n", _("Result initialisation failed"));
		return ( AF_ERR_MEM );
	}

	/* reading of existing data sets: */
	if ( (fr = fopen(fname, "r")) == NULL )
	{
		fprintf(msgout,"[E] %s!\n",_("Cannot open MC summary file"));
		rv = AF_ERR_IO ;
		goto memFree ;
	}

	fscanf(fr, "%lf", &sims) ;
	fscanf(fr, "%lf", &fail_num) ;
	fscanf(fr, "%li", &save_mode) ;

	if (sims < 1) 
	{
		fprintf(msgout,"[E] %s!\n",_("Invalid number of simulations"));
		rv = AF_ERR_VAL ;
		fclose(fr);
		goto memFree ;
	}

	for (i=0; i<=FDB_LONG_STRING; i++) {name[i] = '\0';}
	fscanf(fr, "%s", name) ;

	if ((rv=fdbResReadStep(0, 1.0, name)) != AF_OK) 
	{ 
		fclose(fr);
		goto memFree; 
	}
	ResNode[0].set_id = 1 ;
	ResReac[0].set_id = 1 ;
	ResElem[0].set_id = 1 ;

	for (i=0; i<=FDB_LONG_STRING; i++) {name[i] = '\0';}
	fscanf(fr, "%s", name) ;

	if ((rv=fdbResReadStep(1, 2.0, name)) != AF_OK) 
	{ 
		fclose(fr);
		goto memFree; 
	}
	ResNode[1].set_id = 2 ;
	ResReac[1].set_id = 2 ;
	ResElem[1].set_id = 2 ;


	for (i=0; i<=FDB_LONG_STRING; i++) {name[i] = '\0';}
	fscanf(fr, "%s", name) ;

	if ((rv=fdbResReadStep(6, 7.0, name)) != AF_OK) 
	{ 
		fclose(fr);
		goto memFree; 
	}
	ResNode[6].set_id = 7 ;
	ResReac[6].set_id = 7 ;
	ResElem[6].set_id = 7 ;

	for (i=0; i<=FDB_LONG_STRING; i++) {name[i] = '\0';}
	fscanf(fr, "%s", name) ;

	if ((rv=fdbResReadStep(7, 8.0, name)) != AF_OK) 
	{ 
		fclose(fr);
		goto memFree; 
	}
	ResNode[7].set_id = 8 ;
	ResReac[7].set_id = 8 ;
	ResElem[7].set_id = 8 ;

	fclose(fr);


	/* data manipulations: */

	/* average */
	if ((rv=fdbResCopyStep(0, 2, 3.0))!=AF_OK) { goto memFree; }
	ResNode[2].set_id = 3 ;
	ResReac[2].set_id = 3 ;
	ResElem[2].set_id = 3 ;
	/* dispersion */
	if ((rv=fdbResCopyStep(0, 3, 4.0))!=AF_OK) { goto memFree; }
	ResNode[3].set_id = 4 ;
	ResReac[3].set_id = 4 ;
	ResElem[3].set_id = 4 ;
	/* standard deviation */
	if ((rv=fdbResCopyStep(0, 4, 5.0))!=AF_OK) { goto memFree; }
	ResNode[4].set_id = 5 ;
	ResReac[4].set_id = 5 ;
	ResElem[4].set_id = 5 ;
	/* coefficient of variation */
	if ((rv=fdbResCopyStep(0, 5, 6.0))!=AF_OK) { goto memFree; }
	ResNode[5].set_id = 6 ;
	ResReac[5].set_id = 6 ;
	ResElem[5].set_id = 6 ;

	data_len_node = ResNode[0].data_len ;
	data_len_reac = ResReac[0].data_len ;
	data_len_elem = ResElem[0].data_len ;

	for (i=0; i<data_len_node; i++)
	{
		ResNode[2].data[i] = ResNode[0].data[i] / sims ;
		if (sims > 1)
		{
		ResNode[3].data[i] = 
			(1.0/(sims-1.0))*
			( ResNode[1].data[i] - ((1.0/sims)*ResNode[0].data[i]*ResNode[0].data[i]) );
		}
		else
		{
			ResNode[3].data[i] = 0.0 ;
		}
		ResNode[4].data[i] = sqrt(ResNode[3].data[i]);
		if (ResNode[2].data[i] != 0.0)
		{
			ResNode[5].data[i] = (ResNode[4].data[i] / ResNode[2].data[i]);
		}
		else
		{
			ResNode[5].data[i] = 0.0 ;
		}
	}

	for (i=0; i<data_len_reac; i++)
	{
		ResReac[2].data[i] = ResReac[0].data[i] / sims ;
		if (sims > 1)
		{
		ResReac[3].data[i] = 
			(1.0/(sims-1.0))*
			( ResReac[1].data[i] - ((1.0/sims)*ResReac[0].data[i]*ResReac[0].data[i]) );
		}
		else
		{
		ResReac[3].data[i] = 0.0 ;
		}
		ResReac[4].data[i] = sqrt(ResReac[3].data[i]);
		if (ResReac[2].data[i] != 0.0)
		{
			ResReac[5].data[i] = (ResReac[4].data[i] / ResReac[2].data[i]);
		}
		else
		{
			ResReac[5].data[i] = 0.0 ;
		}
	}

	for (i=0; i<data_len_elem; i++)
	{
		ResElem[2].data[i] = ResElem[0].data[i] / sims ;
		if (sims > 1)
		{
		ResElem[3].data[i] = 
			(1.0/(sims-1.0))*
			( ResElem[1].data[i] - ((1.0/sims)*ResElem[0].data[i]*ResElem[0].data[i]) );
		}
		else
		{
		ResElem[3].data[i] = 0.0 ;
		}
		ResElem[4].data[i] = sqrt(ResElem[3].data[i]);
		if (ResElem[2].data[i] != 0.0)
		{
			ResElem[5].data[i] = (ResElem[4].data[i] / ResElem[2].data[i]);
		}
		else
		{
			ResElem[5].data[i] = 0.0 ;
		}
	}

	ResActStep = 2 ;

	return(rv);
memFree:
	if (ResLen > 0) { fdbFreeResSpacePtrs(); }
	return(rv);
}

/* end of fdb_rmc.c */
