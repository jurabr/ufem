/*
   File name: fem_tcmd.c
   Date:      2003/08/24 14:33
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

	 FEM User Interface - commands

	 $Id: fem_tcmd.c,v 1.61 2005/02/21 22:06:02 jirka Exp $
*/

#include "fem_tui.h"
#include "cint.h"

extern int  femCmdDoNotBreak ;

extern long fdbElementTypeLen ;
extern long fdbMatTypeLen ;

extern int func_help (char *cmd);

extern int func_fem_prepset(char *cmd) ;

extern int func_fem_save (char *cmd) ;
extern int func_fem_resu (char *cmd) ;
extern int func_fem_clean_data (char *cmd) ;

extern int func_fem_export (char *cmd) ;
extern int func_fem_import (char *cmd) ;

extern int func_fem_summ (char *cmd) ;
extern int func_fem_prsumm (char *cmd) ;

extern int func_fem_cslist (char *cmd) ;
extern int func_fem_prcs (char *cmd) ;

extern int func_fem_n (char *cmd) ;
extern int func_fem_n_list (char *cmd) ;
extern int func_fem_n_del (char *cmd) ;
extern int func_fem_n_split(char *cmd) ;
extern int func_fem_n_merge (char *cmd);

extern int func_def_e_prop(char *cmd) ;
extern int func_fem_e_prop (char *cmd) ;
extern int func_fem_e_nodes (char *cmd) ;
extern int func_fem_e_list (char *cmd) ;
extern int func_fem_e_del (char *cmd) ;
extern int func_fem_e_change_set (char *cmd);

extern int func_fem_et_new (char *cmd);
extern int func_fem_et_list (char *cmd);
extern int func_fem_et_del (char *cmd);

extern int func_fem_rs_prop (char *cmd);
extern int func_fem_r_val (char *cmd);
extern int func_fem_rset_list (char *cmd);
extern int func_fem_rset_del (char *cmd);

extern int func_fem_mat(char *cmd);
extern int func_fem_mp_val (char *cmd);
extern int func_fem_mat_list (char *cmd);
extern int func_fem_mat_del (char *cmd);

extern int func_fem_time(char *cmd);

extern int func_fem_disp(char *cmd);
extern int func_fem_disp_list (char *cmd);
extern int func_fem_disp_del (char *cmd);
extern int func_fem_disp_change_set (char *cmd);

extern int func_fem_nl(char *cmd);
extern int func_fem_nl_list (char *cmd);
extern int func_fem_nl_del (char *cmd);
extern int func_fem_nl_change_set (char *cmd);

extern int func_fem_el(char *cmd);
extern int func_fem_el_list (char *cmd);
extern int func_fem_el_del (char *cmd);
extern int func_fem_el_change_set (char *cmd);

extern int func_fem_gl(char *cmd);
extern int func_fem_gl_list (char *cmd);
extern int func_fem_gl_del (char *cmd);

extern int func_fem_set_csys(char *cmd) ;

/* selections: */
extern int func_fem_nsel (char *cmd);
extern int func_fem_esel (char *cmd);
extern int func_fem_dsel (char *cmd);
extern int func_fem_fsel (char *cmd);
extern int func_fem_eload_sel (char *cmd);
extern int func_fem_acel_sel (char *cmd);

extern int func_fem_sel_nsle(char *cmd);
extern int func_fem_sel_nsld(char *cmd);
extern int func_fem_sel_nslf(char *cmd);
extern int func_fem_sel_eslel(char *cmd);
extern int func_fem_sel_elsle(char *cmd);
extern int func_fem_sel_fsln(char *cmd);
extern int func_fem_sel_dsln(char *cmd);
extern int func_fem_sel_esln(char *cmd);
extern int func_fem_sel_allsel(char *cmd);

extern int func_fem_sel_by_step(char *cmd);

/* fem model operations (copying etc.): */
extern int func_fem_ngen(char *cmd);
extern int func_fem_engen(char *cmd);
extern int func_fem_dgen(char *cmd);
extern int func_fem_fgen(char *cmd);
extern int func_fem_n_move(char *cmd);

extern int func_fem_nmirror(char *cmd);
extern int func_fem_enmirror(char *cmd);
extern int func_fem_dmirror(char *cmd);
extern int func_fem_fmirror(char *cmd);

/* load steps */
extern int func_fem_step_new_change (char *cmd);
extern int func_fem_step_val_change (char *cmd);
extern int func_fem_step_delete (char *cmd);
extern int func_fem_step_list (char *cmd);

/* matrix operations */
extern int func_mat_new_matrix(char *cmd);
extern int func_mat_free_matrix_space(char *cmd);

extern int func_mat_list_matrix(char *cmd) ;
extern int func_mat_write_matrix(char *cmd) ;
extern int func_mat_matrix_put(char *cmd) ;
extern int func_mat_matrix_add(char *cmd) ;
extern int func_mat_matrix_get(char *cmd) ;
extern int func_mat_read_matrix(char *cmd) ;
extern int func_mat_matrix_fill_res(char *cmd) ;

/* results */
extern int func_fem_prrs(char *cmd);
extern int func_fem_prdof(char *cmd);
extern int func_fem_pres(char *cmd);
extern int func_fem_prns(char *cmd);


extern int func_fem_res_set_list (char *cmd);
extern int func_fem_res_set_print (char *cmd);

/* gui+gfx: */
extern int func_gui_geom (char *cmd);
extern int func_gui_file_dialog (char *cmd);
extern int func_gui_dialog (char *cmd);
extern int func_gui_rset_data_dlg (char *cmd);
extern int func_gui_mat_data_dlg (char *cmd);
extern int func_gui_replot (char *cmd);
extern int func_gui_vpmode(char *cmd);
extern int func_gui_vp(char *cmd);

extern int func_gfx_hide(char *cmd);
extern int func_gfx_show(char *cmd);
extern int func_gfx_nplot (char *cmd);
extern int func_gfx_eplot (char *cmd);
extern int func_gfx_fplot (char *cmd);
extern int func_gfx_dplot (char *cmd);
extern int func_gfx_kplot (char *cmd);
extern int func_gfx_lplot (char *cmd);
extern int func_gfx_aplot (char *cmd);
extern int func_gfx_vplot (char *cmd);
extern int func_gfx_geplot (char *cmd);
extern int func_gfx_no_plot (char *cmd);
extern int func_gfx_plot_prop (char *cmd);
extern int func_gfx_zoom (char *cmd);
extern int func_gfx_unzoom (char *cmd);
extern int func_gfx_reset (char *cmd);
extern int func_gfx_move (char *cmd);
extern int func_gfx_rot (char *cmd);
extern int func_gfx_view (char *cmd);
extern int func_gfx_plotcol (char *cmd);
extern int func_gfx_gcancel (char *cmd);
extern int func_gfx_gfinish (char *cmd);
extern int func_gfx_gedit (char *cmd);
extern int func_gfx_gcreate (char *cmd);
extern int func_gfx_pick (char *cmd);
extern int func_gfx_pick_sel (char *cmd);
extern int func_gfx_gbox (char *cmd);
extern int func_gfx_plot_to_file (char *cmd);
extern int func_gfx_plot_to_ppm_file (char *cmd);
extern int func_gfx_plot_to_tiff_file (char *cmd);
extern int func_gfx_plot_format (char *cmd) ;
extern int func_gfx_set_grid(char *cmd);

/* postprocessor */
extern int func_fem_prep(char *cmd);
extern int func_fem_post_simple(char *cmd);
extern int func_fem_post_set_set(char *cmd);
extern int func_fem_post_read_set(char *cmd);

extern int func_fem_res_compute_mc(char *cmd);

extern int func_gfx_pldef (char *cmd);
extern int func_gfx_plrs (char *cmd);
extern int func_gfx_ples (char *cmd);
extern int func_gfx_plns (char *cmd);

/* solver */
extern int func_fem_solve(char *cmd);


/* prep+post: */
extern int func_fem_set_solver(char *cmd);
extern int func_fem_set_datadir(char *cmd);
extern int func_fem_set_jobname(char *cmd);

extern int func_fem_gp_load_res(char *cmd);

extern int func_fem_set_num_proc(char *cmd);

/* variables: */
extern int func_var_add (char *cmd);
extern int func_var_gen_add (char *cmd);
extern int func_var_del (char *cmd);
extern int func_var_prn (char *cmd);
extern int func_var_dump (char *cmd);

extern int func_var_write_conf_file (char *cmd);
extern int func_var_write_gfx_conf_file (char *cmd);
extern int func_var_write_gui_conf_file (char *cmd);

extern int func_var_write_rawres_dof(char *cmd) ;

/* command line history */
extern int func_cihs_init (char *cmd);
extern int func_cihs_last (char *cmd);

/* output: */
extern int func_fem_set_output(char *cmd);
extern int func_fem_set_outauto(char *cmd);
extern int func_fem_set_outfile(char *cmd);
extern int func_fem_set_outview(char *cmd);


/* geometric model */
extern int func_fem_k (char *cmd) ;
extern int func_fem_k_list (char *cmd) ;
extern int func_fem_k_del (char *cmd) ;
extern int func_fem_k_split(char *cmd);
extern int func_fem_k_merge (char *cmd);

extern int func_fem_default_div (char *cmd);

extern int func_fem_line_prop (char *cmd);
extern int func_fem_line_kps (char *cmd);
extern int func_fem_line_divs (char *cmd);
extern int func_fem_line_list (char *cmd);
extern int func_fem_line_del (char *cmd);

extern int func_fem_area_prop (char *cmd);
extern int func_fem_area_kps (char *cmd);
extern int func_fem_area_divs (char *cmd);
extern int func_fem_area_list (char *cmd);
extern int func_fem_area_del (char *cmd);

extern int func_fem_volume_prop (char *cmd);
extern int func_fem_volume_kps (char *cmd);
extern int func_fem_volume_divs (char *cmd);
extern int func_fem_volume_list (char *cmd);
extern int func_fem_volume_del (char *cmd);

extern int func_fem_ge_prop (char *cmd);
extern int func_fem_ge_kps (char *cmd);
extern int func_fem_entity_create_dim (char *cmd);
extern int func_fem_ge_divs (char *cmd);
extern int func_fem_ge_list (char *cmd);
extern int func_fem_ge_del (char *cmd);

extern int func_fem_create_mesh (char *cmd);

extern int func_fem_kmirror(char *cmd);
extern int func_fem_entkpmirror(char *cmd);
extern int func_fem_entity_divs(char *cmd);
extern int func_fem_entity_change_set (char *cmd);

extern int func_fem_kgen(char *cmd);
extern int func_fem_gekgen(char *cmd);
extern int func_fem_k_move(char *cmd);
extern int func_fem_k_rotate(char *cmd);

extern int func_fem_ksel(char *cmd);
extern int func_fem_gesel(char *cmd);
extern int func_fem_sel_kslge(char *cmd);
extern int func_fem_sel_geslk(char *cmd);

extern int func_fem_mod_nl_nd (char *cmd) ;

extern int func_mac_eval (char *cmd);
extern int func_mac_ieval (char *cmd);
extern int func_mac_aver_dof_res (char *cmd);

extern int func_mac_new_file (char *cmd);
extern int func_mac_write_newline (char *cmd);
extern int func_mac_write_var (char *cmd);
extern int func_mac_var_strcat(char *cmd);

extern int func_mac_fill_var_sum_react (char *cmd);
extern int func_mac_fill_var_one_react (char *cmd);
extern int func_mac_fill_var_tab_len (char *cmd);
extern int func_mac_fill_var_tab_len_sel (char *cmd);
extern int func_mac_fill_var_jobname (char *cmd);
extern int func_mac_fill_var_file_name (char *cmd);
extern int func_mac_fill_var_data_dir (char *cmd);
extern int func_mac_fill_var_res_time (char *cmd);
extern int func_fill_var_is_selected(char *cmd);
extern int func_fill_var_node(char *cmd);

extern int func_fill_var_eres_num_nrep(char *cmd);
extern int func_fill_var_eres_num_rep(char *cmd);
extern int func_fill_var_eres(char *cmd);
extern int func_fill_var_eres_max(char *cmd);
extern int func_fill_var_eres_min(char *cmd);
extern int func_fill_var_eres_aver(char *cmd);
extern int func_fill_var_nres(char *cmd);
extern int func_fill_var_find_first(char *cmd);

int func_fill_var_eres_max_from_all(char *cmd);
int func_fill_var_eres_min_from_all(char *cmd);

extern int func_fill_var_kp(char *cmd);

/* path operations: */
extern int func_path_create(char *cmd);
extern int func_path_delete(char *cmd);
extern int func_path_node_new(char *cmd);
extern int func_path_list(char *cmd);
extern int func_path_print_res(char *cmd);

/* random variables (monte): */
extern int func_fem_rv (char *cmd);
extern int func_fem_rv_list (char *cmd);
extern int func_fem_rv_del (char *cmd);

/* macros: */
extern int func_mac_tbrick (char *cmd);
extern int func_mac_set_nobreak_input (char *cmd);

/* misc stuff: */
extern int func_fem_comp_vec_scale2D (char *cmd);
extern int func_fem_comp_vec_scale_aniso2D (char *cmd);

/* testing of data:  */
extern int func_data_consist_test (char *cmd);


/* support function - reaction after command */
int tuiCmdReact(char *cmd, int rv)
{
#ifdef RUN_VERBOSE 
	if (rv == AF_OK)
	{
		fprintf(msgout,"[ ] %s (%s)\n", _("OK"), ciGetParStrNoExpand(cmd,(int)0));
	}
	else
	{
		fprintf(msgout,"[E] %s (%s)!\n", _("Command failed"), ciGetParStrNoExpand(cmd,(int)0));
	}
#endif
	return(rv);
}



/* no interactivity for non-interactive program: ***********************  */
#ifdef RUN_VERBOSE 



/* Reaction to error (bad command): */
int func_error (char *cmd)
{
  fprintf (msgout,"\n[E] %s!\n",_("Unknown command"));
  return (tuiCmdReact(cmd,AF_ERR));
}

/** Runs external command - maybe a security hole: */
int func_system (char *cmd)
{
#ifndef _SECURE_CODE_
	int rv = AF_OK ;
  if (ciParNum (cmd) == 1) 
	{ 
		fprintf (msgout,"[E]: %s!\n",_("Command required")); 
  	return (tuiCmdReact(cmd,AF_ERR));
	}
  else 
	{ 
    fprintf(msgout,"[i] %s: \"%s\"\n", _("Running command"), ciGetParStr(cmd,1));
		rv = system (ciGetParStr (cmd, 1)); 
  	return (tuiCmdReact(cmd,rv));
	}
#else
	fprintf (msgout,"[E]: %s!\n",_("Command not available in this version")); 
  return (tuiCmdReact(cmd,AF_ERR));
#endif
}


/** program exit */
int func_exit (char *cmd)
{
  fprintf (msgout,"\n[I] %s .. ",_("Exiting"));
	/* some usefull code should be placed here */
  fprintf (msgout,"%s\n",_("OK"));
  exit (AF_OK);
}


/** closes command log (if opened): "logclose" */
int func_close_log(char *cmd)
{
	int rv = AF_OK ;

	if (ciUseLog == 1)
	{
		ciUseLog = 0 ;
		if (fclose(ciLogFile) !=0)
		{
			fprintf(msgout,"[E] %s!\n",_("Cannot close log file"));
			rv = AF_ERR_IO;
		}
	}
	return(tuiCmdReact(cmd, rv));
}

/** openes command log (must be closed): "logopen,fname,(yes|no)" */
int func_open_log(char *cmd)
{
	int rv = AF_OK ;
	long appe = 0 ;
	char *fname = NULL ;

	if (ciUseLog == 1)
	{
		fprintf(msgout,"[E] %s!\n",_("Close log file first"));
		return(tuiCmdReact(cmd, AF_ERR));
	}

	if (ciParNum(cmd) <= 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Filename required"));
		return(tuiCmdReact(cmd, AF_ERR_EMP));
	}
	fname = ciGetParStr(cmd, 1);
	if (strlen(fname) < 1)
	{
		fprintf(msgout,"[E] %s!\n", _("Invalid filename"));
		return(tuiCmdReact(cmd, AF_ERR_VAL));
	}

	if (ciParNum(cmd) > 2) {appe = ciGetParInt(cmd, 2);}

	if (appe == 1)
	{
		if ((ciLogFile = fopen(fname,"a")) == NULL)
		{
			fprintf(msgout,"[E] %s!\n", _("Cannot open file"));
			rv = AF_ERR_IO ; 
			goto memFree;
		}
	}
	else
	{
		if ((ciLogFile = fopen(fname,"w")) == NULL)
		{
			fprintf(msgout,"[E] %s!\n", _("Cannot open file"));
			rv = AF_ERR_IO ; 
			goto memFree;
		}
	}

	ciUseLog = 1 ;

memFree:
	free(fname); 
	fname = NULL;
	return(tuiCmdReact(cmd, rv));
}


/** runs commands form disk file */
int func_run_file(char *cmd)
{
  int   rv = AF_OK ;
  FILE *fw ;
  char  str[FEM_STR_LEN+1] ;
  long  str_len = FEM_STR_LEN ;
  long  i ;

	if (ciParNum(cmd) > 1) 
  { 
    if (ciGetParStr(cmd,1) == NULL)
    {
      fprintf(msgout,"[E] %s!\n", _("Invalid filename"));
      return(AF_ERR_VAL);
    }

    if (strlen(ciGetParStr(cmd,1)) < 1)
    {
      fprintf(msgout,"[E] %s!\n", _("Too short filename"));
      return(AF_ERR_VAL);
    }

    if ((fw = fopen(ciGetParStr(cmd, 1),"r")) == NULL)
    {
      fprintf(msgout,"[E] %s!\n", _("Cannot open file"));
      return(AF_ERR_IO);
    }
    for (i=0; i<=FEM_STR_LEN; i++) { str[i] = '\0' ; }

    while (fgets(str,str_len,fw) != NULL)
    {
      if ((strcmp(str,"") == 0)||(strcmp(str,"\n") == 0)) {continue;}

      if (ciRunCmd(str) != AF_OK)
      {
        rv = AF_ERR ;
        fprintf(msgout,"[E] %s: \"%s\"!\n",_("Command failed"),str);

				if (femCmdDoNotBreak != AF_YES)
				{
        	break ;
				}
      }
      for (i=0; i<=FEM_STR_LEN; i++) { str[i] = '\0' ; }
    }

    if (fclose(fw) != 0)
    {
      fprintf(msgout,"[E] %s!\n", _("Cannot close file"));
      return(AF_ERR_IO);
    }
    else
    {
      return(rv);
    }
  }
  else
  {
    fprintf(msgout,"[E] %s!\n", _("Filename required"));
    return(AF_ERR);
  }
  return(rv);
}


/* scripts: --------------------- */

/** starts reading of new script: "script,name" */
int func_new_script(char *cmd)
{
  int   rv = AF_OK ;
	char *name = NULL ;

	if (ciParNum(cmd) <=1 )
	{
		fprintf(msgout,"[E] %s!\n",_("Script name required"));
	  return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	if ((name=ciGetParStr(cmd,1))==NULL)
	{
		fprintf(msgout,"[E] %s!\n",_("Script name required"));
	  return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	ciStrCompr(name);

	if (strlen(name)<1)
	{
		fprintf(msgout,"[E] %s!\n",_("Script name too short"));
		free(name); name = NULL;
	  return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	if ((rv=ciAddScript(name))==CI_OK)
	{
	  ciReadingScript = 1 ;
	}

	free(name);
	name = NULL;
	return(tuiCmdReact(cmd, rv));
}

/** stops reading of script: "endscript" */
int func_end_script(char *cmd)
{
	return(AF_OK);
}

/** removes all scripts from memory */
int func_remove_scripts(char *cmd)
{
	ciFreeScriptField();
	return(tuiCmdReact(cmd, AF_OK));
}

/** "for" statement for scripts */
int func_script_for(char *cmd) { return(AF_OK); }

/** "endfor" statement for scripts */
int func_script_endfor(char *cmd) { return(AF_OK); }

/** "if" statement for scripts */
int func_script_if(char *cmd) { return(AF_OK); }

/** "endif" statement for scripts */
int func_script_endif(char *cmd) { return(AF_OK); }

/** executes script: "runscript,name" */
int func_run_script(char *cmd)
{
  int   rv = AF_OK ;
	long  i ;
	long found = 0 ;
	char *name = NULL ;

	if (ciParNum(cmd) <=1 )
	{
		fprintf(msgout,"[E] %s!\n",_("Script name required"));
	  return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	if ((name=ciGetParStr(cmd,1))==NULL)
	{
		fprintf(msgout,"[E] %s!\n",_("Script name required"));
	  return(tuiCmdReact(cmd, AF_ERR_EMP));
	}

	ciStrCompr(name);

	for (i=0; i<ciScriptLen; i++)
	{
		if (ciScript[i].name == NULL)
		{
			rv = AF_ERR_EMP ;
			goto memFree;
		}
		if (strcmp(name,ciScript[i].name)==0)
		{
			found = 1 ; 
			break ;
		}
	}

	if (found == 1)
	{
		if ((rv = ciRunScript(&ciScript[i])) != 0)
		{
			fprintf(msgout,"[E] %s: \"%s\"!\n",_("Script failed"),name);
		}
	}
	else
	{
		fprintf(msgout,"[E] %s: \"%s\"!\n",_("Unexistant script"),name);
		rv = AF_ERR_EMP ;
	}

memFree:
	free(name);
	name = NULL ;

	return(tuiCmdReact(cmd, rv));
}


/* --------------------------------------------------- */

/** Init for commands */ 
int fem_cmd_init (void)
{
  long count = 0 ;

  ciFuncLen = 266;
  ciFunction = (tFunc *) malloc (ciFuncLen * sizeof (tFunc));
  
  /* basic functions */
  ciFunction[count].cmd = "error";
  ciFunction[count].func = func_error;
  count++;

  ciFunction[count].cmd = "system";
  ciFunction[count].func = func_system;
  count++;

  ciFunction[count].cmd = "exit";
  ciFunction[count].func = func_exit;
  count++;

  ciFunction[count].cmd = "help";
  ciFunction[count].func = func_help;
  count++;

  ciFunction[count].cmd = "quit";
  ciFunction[count].func = func_exit;
  count++;

  ciFunction[count].cmd = "logclose";
  ciFunction[count].func = func_close_log;
  count++;

  ciFunction[count].cmd = "logopen";
  ciFunction[count].func = func_open_log;
  count++;

  ciFunction[count].cmd = "input";
  ciFunction[count].func = func_run_file;
  count++;


  ciFunction[count].cmd = "setvar";
  ciFunction[count].func = func_var_add ;
  count++;

  ciFunction[count].cmd = "genvar";
  ciFunction[count].func = func_var_gen_add ;
  count++;

  ciFunction[count].cmd = "unsetvar";
  ciFunction[count].func = func_var_del ;
  count++;

  ciFunction[count].cmd = "printvar";
  ciFunction[count].func = func_var_prn ;
  count++;

  ciFunction[count].cmd = "listvar";
  ciFunction[count].func = func_var_dump ;
  count++;

  ciFunction[count].cmd = "writeconf";
  ciFunction[count].func = func_var_write_conf_file ;
  count++;

  ciFunction[count].cmd = "writegfxconf";
  ciFunction[count].func = func_var_write_gfx_conf_file;
  count++;

  ciFunction[count].cmd = "writeguiconf";
  ciFunction[count].func = func_var_write_gui_conf_file ;
  count++;

  ciFunction[count].cmd = "writentab";
  ciFunction[count].func = func_var_write_rawres_dof ;
  count++;

  /* scripting */
  ciFunction[count].cmd = "script";
  ciFunction[count].func = func_new_script ;
  count++;

  ciFunction[count].cmd = "endscript";
  ciFunction[count].func = func_end_script ;
  count++;

  ciFunction[count].cmd = "rmscripts";
  ciFunction[count].func = func_remove_scripts ;
  count++;

  ciFunction[count].cmd = "for";
  ciFunction[count].func = func_script_for ;
  count++;

  ciFunction[count].cmd = "endfor";
  ciFunction[count].func = func_script_endfor ;
  count++;

  ciFunction[count].cmd = "if";
  ciFunction[count].func = func_script_if ;
  count++;

  ciFunction[count].cmd = "endif";
  ciFunction[count].func = func_script_endif ;
  count++;

  ciFunction[count].cmd = "runscript";
  ciFunction[count].func = func_run_script ;
  count++;

  /* command line history */
  ciFunction[count].cmd = "history";
  ciFunction[count].func = func_cihs_init ;
  count++;

  ciFunction[count].cmd = "last";
  ciFunction[count].func = func_cihs_last ;
  count++;


	/* preprocessor settings */
  ciFunction[count].cmd = "prepset";
  ciFunction[count].func = func_fem_prepset;
  count++;


	/* FEM model: */

  ciFunction[count].cmd = "n";
  ciFunction[count].func = func_fem_n;
  count++;

  ciFunction[count].cmd = "nlist";
  ciFunction[count].func = func_fem_n_list;
  count++;

  ciFunction[count].cmd = "ndel";
  ciFunction[count].func = func_fem_n_del;
  count++;

  ciFunction[count].cmd = "nsplit";
  ciFunction[count].func = func_fem_n_split;
  count++;

  ciFunction[count].cmd = "nmerge";
  ciFunction[count].func = func_fem_n_merge;
  count++;



  ciFunction[count].cmd = "edef";
  ciFunction[count].func = func_def_e_prop;
  count++;

  ciFunction[count].cmd = "ep";
  ciFunction[count].func = func_fem_e_prop;
  count++;

  ciFunction[count].cmd = "e";
  ciFunction[count].func = func_fem_e_nodes;
  count++;

  ciFunction[count].cmd = "elist";
  ciFunction[count].func = func_fem_e_list;
  count++;

  ciFunction[count].cmd = "edel";
  ciFunction[count].func = func_fem_e_del;
  count++;

  ciFunction[count].cmd = "echset";
  ciFunction[count].func = func_fem_e_change_set;
  count++;

  ciFunction[count].cmd = "et";
  ciFunction[count].func = func_fem_et_new;
  count++;

  ciFunction[count].cmd = "etlist";
  ciFunction[count].func = func_fem_et_list;
  count++;

  ciFunction[count].cmd = "etdel";
  ciFunction[count].func = func_fem_et_del;
  count++;


  ciFunction[count].cmd = "rs";
  ciFunction[count].func = func_fem_rs_prop;
  count++;

  ciFunction[count].cmd = "r";
  ciFunction[count].func = func_fem_r_val;
  count++;

  ciFunction[count].cmd = "rslist";
  ciFunction[count].func = func_fem_rset_list;
  count++;

  ciFunction[count].cmd = "rlist";
  ciFunction[count].func = func_fem_rset_list;
  count++;

  ciFunction[count].cmd = "rsdel";
  ciFunction[count].func = func_fem_rset_del;
  count++;

  ciFunction[count].cmd = "rdel";
  ciFunction[count].func = func_fem_rset_del;
  count++;

  ciFunction[count].cmd = "mat";
  ciFunction[count].func = func_fem_mat;
  count++;

  ciFunction[count].cmd = "mp";
  ciFunction[count].func = func_fem_mp_val;
  count++;

  ciFunction[count].cmd = "mplist";
  ciFunction[count].func = func_fem_mat_list;
  count++;

  ciFunction[count].cmd = "matlist";
  ciFunction[count].func = func_fem_mat_list;
  count++;

  ciFunction[count].cmd = "mpdel";
  ciFunction[count].func = func_fem_mat_del;
  count++;

  ciFunction[count].cmd = "matdel";
  ciFunction[count].func = func_fem_mat_del;
  count++;

  ciFunction[count].cmd = "time";
  ciFunction[count].func = func_fem_time;
  count++;

  ciFunction[count].cmd = "d";
  ciFunction[count].func = func_fem_disp;
  count++;

  ciFunction[count].cmd = "dlist";
  ciFunction[count].func = func_fem_disp_list;
  count++;

  ciFunction[count].cmd = "ddel";
  ciFunction[count].func = func_fem_disp_del;
  count++;

  ciFunction[count].cmd = "dchset";
  ciFunction[count].func = func_fem_disp_change_set;
  count++;

  ciFunction[count].cmd = "f";
  ciFunction[count].func = func_fem_nl;
  count++;

  ciFunction[count].cmd = "flist";
  ciFunction[count].func = func_fem_nl_list;
  count++;

  ciFunction[count].cmd = "fdel";
  ciFunction[count].func = func_fem_nl_del;
  count++;

  ciFunction[count].cmd = "fchset";
  ciFunction[count].func = func_fem_nl_change_set;
  count++;

  ciFunction[count].cmd = "el";
  ciFunction[count].func = func_fem_el;
  count++;

  ciFunction[count].cmd = "ellist";
  ciFunction[count].func = func_fem_el_list;
  count++;

  ciFunction[count].cmd = "eldel";
  ciFunction[count].func = func_fem_el_del;
  count++;

  ciFunction[count].cmd = "elchset";
  ciFunction[count].func = func_fem_el_change_set;
  count++;

  ciFunction[count].cmd = "accel";
  ciFunction[count].func = func_fem_gl;
  count++;

  ciFunction[count].cmd = "accellist";
  ciFunction[count].func = func_fem_gl_list;
  count++;

  ciFunction[count].cmd = "acceldel";
  ciFunction[count].func = func_fem_gl_del;
  count++;

  ciFunction[count].cmd = "csys";
  ciFunction[count].func = func_fem_set_csys;
  count++;

  ciFunction[count].cmd = "save";
  ciFunction[count].func = func_fem_save;
  count++;

  ciFunction[count].cmd = "resu";
  ciFunction[count].func = func_fem_resu;
  count++;

  ciFunction[count].cmd = "cleandata";
  ciFunction[count].func = func_fem_clean_data;
  count++;

  ciFunction[count].cmd = "export";
  ciFunction[count].func = func_fem_export;
  count++;

  ciFunction[count].cmd = "import";
  ciFunction[count].func = func_fem_import;
  count++;

  ciFunction[count].cmd = "summ";
  ciFunction[count].func = func_fem_summ;
  count++;

  ciFunction[count].cmd = "prsumm";
  ciFunction[count].func = func_fem_prsumm;
  count++;

  ciFunction[count].cmd = "prcs";
  ciFunction[count].func = func_fem_prcs;
  count++;

  ciFunction[count].cmd = "cslist";
  ciFunction[count].func = func_fem_cslist;
  count++;

  /* results */

  ciFunction[count].cmd = "prrs";
  ciFunction[count].func = func_fem_prrs;
  count++;

  ciFunction[count].cmd = "prdof";
  ciFunction[count].func = func_fem_prdof;
  count++;

  ciFunction[count].cmd = "prdef";
  ciFunction[count].func = func_fem_prdof;
  count++;

  ciFunction[count].cmd = "pres";
  ciFunction[count].func = func_fem_pres;
  count++;

  ciFunction[count].cmd = "prns";
  ciFunction[count].func = func_fem_prns;
  count++;

  ciFunction[count].cmd = "slist";
  ciFunction[count].func = func_fem_res_set_list;
  count++;

  ciFunction[count].cmd = "prslist";
  ciFunction[count].func = func_fem_res_set_print;
  count++;



  /* *** */

  ciFunction[count].cmd = "ggeom";
  ciFunction[count].func = func_gui_geom;
  count++;

  ciFunction[count].cmd = "filedialog";
  ciFunction[count].func = func_gui_file_dialog;
  count++;

  ciFunction[count].cmd = "dialog";
  ciFunction[count].func = func_gui_dialog;
  count++;

  ciFunction[count].cmd = "r_dlg";
  ciFunction[count].func = func_gui_rset_data_dlg;
  count++;

  ciFunction[count].cmd = "mp_dlg";
  ciFunction[count].func = func_gui_mat_data_dlg;
  count++;

  ciFunction[count].cmd = "replot";
  ciFunction[count].func = func_gui_replot;
  count++;

  ciFunction[count].cmd = "vpmode";
  ciFunction[count].func = func_gui_vpmode;
  count++;

  ciFunction[count].cmd = "vp";
  ciFunction[count].func = func_gui_vp;
  count++;

  ciFunction[count].cmd = "hide";
  ciFunction[count].func = func_gfx_hide;
  count++;

  ciFunction[count].cmd = "show";
  ciFunction[count].func = func_gfx_show;
  count++;

  ciFunction[count].cmd = "nplot";
  ciFunction[count].func = func_gfx_nplot ;
  count++;


  ciFunction[count].cmd = "eplot";
  ciFunction[count].func = func_gfx_eplot ;
  count++;

  ciFunction[count].cmd = "fplot";
  ciFunction[count].func = func_gfx_fplot ;
  count++;

  ciFunction[count].cmd = "dplot";
  ciFunction[count].func = func_gfx_dplot ;
  count++;

  ciFunction[count].cmd = "kplot";
  ciFunction[count].func = func_gfx_kplot ;
  count++;

  ciFunction[count].cmd = "lplot";
  ciFunction[count].func = func_gfx_lplot ;
  count++;

  ciFunction[count].cmd = "aplot";
  ciFunction[count].func = func_gfx_aplot ;
  count++;

  ciFunction[count].cmd = "vplot";
  ciFunction[count].func = func_gfx_vplot ;
  count++;

  ciFunction[count].cmd = "geplot";
  ciFunction[count].func = func_gfx_geplot ;
  count++;

  ciFunction[count].cmd = "noplot";
  ciFunction[count].func = func_gfx_no_plot ;
  count++;

  ciFunction[count].cmd = "plotprop";
  ciFunction[count].func = func_gfx_plot_prop ;
  count++;


  ciFunction[count].cmd = "gcancel";
  ciFunction[count].func = func_gfx_gcancel ;
  count++;

  ciFunction[count].cmd = "gfinish";
  ciFunction[count].func = func_gfx_gfinish ;
  count++;

  ciFunction[count].cmd = "gedit";
  ciFunction[count].func = func_gfx_gedit ;
  count++;

  ciFunction[count].cmd = "gcreate";
  ciFunction[count].func = func_gfx_gcreate ;
  count++;

  ciFunction[count].cmd = "pick";
  ciFunction[count].func = func_gfx_pick ;
  count++;

  ciFunction[count].cmd = "gsel";
  ciFunction[count].func = func_gfx_pick_sel ;
  count++;

  ciFunction[count].cmd = "gbox";
  ciFunction[count].func = func_gfx_gbox ;
  count++;

  ciFunction[count].cmd = "zoom";
  ciFunction[count].func = func_gfx_zoom ;
  count++;

  ciFunction[count].cmd = "unzoom";
  ciFunction[count].func = func_gfx_unzoom ;
  count++;

  ciFunction[count].cmd = "reset";
  ciFunction[count].func = func_gfx_reset ;
  count++;

  ciFunction[count].cmd = "move";
  ciFunction[count].func = func_gfx_move ;
  count++;

  ciFunction[count].cmd = "rot";
  ciFunction[count].func = func_gfx_rot ;
  count++;

  ciFunction[count].cmd = "view";
  ciFunction[count].func = func_gfx_view ;
  count++;

  ciFunction[count].cmd = "plotcol";
  ciFunction[count].func = func_gfx_plotcol ;
  count++;

  ciFunction[count].cmd = "gl2ps";
  ciFunction[count].func = func_gfx_plot_to_file ;
  count++;

  ciFunction[count].cmd = "gl2ppm";
  ciFunction[count].func = func_gfx_plot_to_ppm_file ;
  count++;

  ciFunction[count].cmd = "gl2tiff";
  ciFunction[count].func = func_gfx_plot_to_tiff_file ;
  count++;

  ciFunction[count].cmd = "glformat";
  ciFunction[count].func = func_gfx_plot_format ;
  count++;

  ciFunction[count].cmd = "pgrid";
  ciFunction[count].func = func_gfx_set_grid ;
  count++;

	/* selections */
  ciFunction[count].cmd = "nsel";
  ciFunction[count].func = func_fem_nsel ;
  count++;

  ciFunction[count].cmd = "esel";
  ciFunction[count].func = func_fem_esel ;
  count++;

  ciFunction[count].cmd = "dsel";
  ciFunction[count].func = func_fem_dsel ;
  count++;

  ciFunction[count].cmd = "fsel";
  ciFunction[count].func = func_fem_fsel ;
  count++;

  ciFunction[count].cmd = "elsel";
  ciFunction[count].func = func_fem_eload_sel ;
  count++;

  ciFunction[count].cmd = "accelsel";
  ciFunction[count].func = func_fem_acel_sel ;
  count++;


  ciFunction[count].cmd = "nsle";
  ciFunction[count].func = func_fem_sel_nsle ;
  count++;

  ciFunction[count].cmd = "nsld";
  ciFunction[count].func = func_fem_sel_nsld ;
  count++;

  ciFunction[count].cmd = "nslf";
  ciFunction[count].func = func_fem_sel_nslf ;
  count++;

  ciFunction[count].cmd = "elsle";
  ciFunction[count].func = func_fem_sel_elsle ;
  count++;

  ciFunction[count].cmd = "eslel";
  ciFunction[count].func = func_fem_sel_eslel ;
  count++;

  ciFunction[count].cmd = "fsln";
  ciFunction[count].func = func_fem_sel_fsln ;
  count++;

  ciFunction[count].cmd = "dsln";
  ciFunction[count].func = func_fem_sel_dsln ;
  count++;

  ciFunction[count].cmd = "esln";
  ciFunction[count].func = func_fem_sel_esln ;
  count++;

  ciFunction[count].cmd = "allsel";
  ciFunction[count].func = func_fem_sel_allsel ;
  count++;

  ciFunction[count].cmd = "slstep";
  ciFunction[count].func = func_fem_sel_by_step ;
  count++;

	/* copying etc. */
  ciFunction[count].cmd = "ngen";
  ciFunction[count].func = func_fem_ngen ;
  count++;

  ciFunction[count].cmd = "nmove";
  ciFunction[count].func = func_fem_n_move ;
  count++;

  ciFunction[count].cmd = "engen";
  ciFunction[count].func = func_fem_engen ;
  count++;

  ciFunction[count].cmd = "dgen";
  ciFunction[count].func = func_fem_dgen ;
  count++;

  ciFunction[count].cmd = "fgen";
  ciFunction[count].func = func_fem_fgen ;
  count++;

  ciFunction[count].cmd = "nmirror";
  ciFunction[count].func = func_fem_nmirror ;
  count++;

  ciFunction[count].cmd = "enmirror";
  ciFunction[count].func = func_fem_enmirror ;
  count++;

  ciFunction[count].cmd = "dmirror";
  ciFunction[count].func = func_fem_dmirror ;
  count++;

  ciFunction[count].cmd = "fmirror";
  ciFunction[count].func = func_fem_fmirror ;
  count++;

  /* prep/postprocessor */
  ciFunction[count].cmd = "prep";
  ciFunction[count].func = func_fem_prep ;
  count++;

  ciFunction[count].cmd = "solve";
  ciFunction[count].func = func_fem_solve ;
  count++;

  ciFunction[count].cmd = "ssolve";
  ciFunction[count].func = func_fem_solve ;
  count++;

  ciFunction[count].cmd = "gpres";
  ciFunction[count].func = func_fem_gp_load_res ;
  count++;


  ciFunction[count].cmd = "setsolver";
  ciFunction[count].func = func_fem_set_solver ;
  count++;

  ciFunction[count].cmd = "nproc";
  ciFunction[count].func = func_fem_set_num_proc ;
  count++;

  ciFunction[count].cmd = "datadir";
  ciFunction[count].func = func_fem_set_datadir ;
  count++;

  ciFunction[count].cmd = "jobname";
  ciFunction[count].func = func_fem_set_jobname ;
  count++;

  ciFunction[count].cmd = "gpost";
  ciFunction[count].func = func_fem_post_simple ;
  count++;

  ciFunction[count].cmd = "set";
  ciFunction[count].func = func_fem_post_set_set ;
  count++;

  ciFunction[count].cmd = "rread";
  ciFunction[count].func = func_fem_post_read_set ;
  count++;

  ciFunction[count].cmd = "mcres";
  ciFunction[count].func = func_fem_res_compute_mc ;
  count++;


  ciFunction[count].cmd = "pldef";
  ciFunction[count].func = func_gfx_pldef ;
  count++;

  ciFunction[count].cmd = "plrs";
  ciFunction[count].func = func_gfx_plrs ;
  count++;

  ciFunction[count].cmd = "ples";
  ciFunction[count].func = func_gfx_ples ;
  count++;

  ciFunction[count].cmd = "plns";
  ciFunction[count].func = func_gfx_plns ;
  count++;

  /* output: */
  ciFunction[count].cmd = "outform";
  ciFunction[count].func = func_fem_set_output ;
  count++;

  ciFunction[count].cmd = "outauto";
  ciFunction[count].func = func_fem_set_outauto ;
  count++;

  ciFunction[count].cmd = "outfile";
  ciFunction[count].func = func_fem_set_outfile ;
  count++;

  ciFunction[count].cmd = "outview";
  ciFunction[count].func = func_fem_set_outview ;
  count++;

  
  /* geometric model: */
  ciFunction[count].cmd = "k";
  ciFunction[count].func = func_fem_k ;
  count++;

  ciFunction[count].cmd = "klist";
  ciFunction[count].func = func_fem_k_list ;
  count++;

  ciFunction[count].cmd = "kdel";
  ciFunction[count].func = func_fem_k_del ;
  count++;

  ciFunction[count].cmd = "ksplit";
  ciFunction[count].func = func_fem_k_split ;
  count++;

  ciFunction[count].cmd = "kmerge";
  ciFunction[count].func = func_fem_k_merge ;
  count++;



  ciFunction[count].cmd = "ddiv";
  ciFunction[count].func = func_fem_default_div ;
  count++;


  ciFunction[count].cmd = "lp";
  ciFunction[count].func = func_fem_line_prop ;
  count++;

  ciFunction[count].cmd = "l";
  ciFunction[count].func = func_fem_line_kps ;
  count++;

  ciFunction[count].cmd = "ldiv";
  ciFunction[count].func = func_fem_line_divs ;
  count++;

  ciFunction[count].cmd = "llist";
  ciFunction[count].func = func_fem_line_list ;
  count++;

  ciFunction[count].cmd = "ldel";
  ciFunction[count].func = func_fem_line_del ;
  count++;

  ciFunction[count].cmd = "ap";
  ciFunction[count].func = func_fem_area_prop ;
  count++;

  ciFunction[count].cmd = "a";
  ciFunction[count].func = func_fem_area_kps ;
  count++;

  ciFunction[count].cmd = "adiv";
  ciFunction[count].func = func_fem_area_divs ;
  count++;

  ciFunction[count].cmd = "alist";
  ciFunction[count].func = func_fem_area_list ;
  count++;

  ciFunction[count].cmd = "adel";
  ciFunction[count].func = func_fem_area_del ;
  count++;

  ciFunction[count].cmd = "vp";
  ciFunction[count].func = func_fem_volume_prop ;
  count++;

  ciFunction[count].cmd = "v";
  ciFunction[count].func = func_fem_volume_kps ;
  count++;

  ciFunction[count].cmd = "vdiv";
  ciFunction[count].func = func_fem_volume_divs ;
  count++;

  ciFunction[count].cmd = "vlist";
  ciFunction[count].func = func_fem_volume_list ;
  count++;

  ciFunction[count].cmd = "vdel";
  ciFunction[count].func = func_fem_volume_del ;
  count++;


  ciFunction[count].cmd = "gep";
  ciFunction[count].func = func_fem_ge_prop ;
  count++;

  ciFunction[count].cmd = "ge";
  ciFunction[count].func = func_fem_ge_kps ;
  count++;

  ciFunction[count].cmd = "gesize";
  ciFunction[count].func = func_fem_entity_create_dim ;
  count++;

  ciFunction[count].cmd = "gediv";
  ciFunction[count].func = func_fem_ge_divs ;
  count++;

  ciFunction[count].cmd = "gelist";
  ciFunction[count].func = func_fem_ge_list ;
  count++;

  ciFunction[count].cmd = "gedel";
  ciFunction[count].func = func_fem_ge_del ;
  count++;

  ciFunction[count].cmd = "gechset";
  ciFunction[count].func = func_fem_entity_change_set ;
  count++;

  ciFunction[count].cmd = "kmirror";
  ciFunction[count].func = func_fem_kmirror ;
  count++;

  ciFunction[count].cmd = "gekmirror";
  ciFunction[count].func = func_fem_entkpmirror ;
  count++;

  ciFunction[count].cmd = "kgen";
  ciFunction[count].func = func_fem_kgen ;
  count++;

  ciFunction[count].cmd = "kmove";
  ciFunction[count].func = func_fem_k_move ;
  count++;

  ciFunction[count].cmd = "krotate";
  ciFunction[count].func = func_fem_k_rotate ;
  count++;

  ciFunction[count].cmd = "gekgen";
  ciFunction[count].func = func_fem_gekgen ;
  count++;

  ciFunction[count].cmd = "ksel";
  ciFunction[count].func = func_fem_ksel ;
  count++;

  ciFunction[count].cmd = "gesel";
  ciFunction[count].func = func_fem_gesel ;
  count++;

  ciFunction[count].cmd = "kslge";
  ciFunction[count].func = func_fem_sel_kslge ;
  count++;

  ciFunction[count].cmd = "geslk";
  ciFunction[count].func = func_fem_sel_geslk ;
  count++;

  ciFunction[count].cmd = "modify";
  ciFunction[count].func = func_fem_mod_nl_nd ;
  count++;


  ciFunction[count].cmd = "mesh";
  ciFunction[count].func = func_fem_create_mesh ;
  count++;


  /* macros: */
  ciFunction[count].cmd = "eval";
  ciFunction[count].func = func_mac_eval ;
  count++;

  ciFunction[count].cmd = "ieval";
  ciFunction[count].func = func_mac_ieval ;
  count++;

  ciFunction[count].cmd = "tbrick";
  ciFunction[count].func = func_mac_tbrick ;
  count++;

  ciFunction[count].cmd = "newfile";
  ciFunction[count].func = func_mac_new_file ;
  count++;

  ciFunction[count].cmd = "writenl";
  ciFunction[count].func = func_mac_write_newline ;
  count++;

  ciFunction[count].cmd = "writevar";
  ciFunction[count].func = func_mac_write_var ;
  count++;

  ciFunction[count].cmd = "varstrcat";
  ciFunction[count].func = func_mac_var_strcat ;
  count++;


  ciFunction[count].cmd = "fillvarsumreact";
  ciFunction[count].func = func_mac_fill_var_sum_react;
  count++;

  ciFunction[count].cmd = "fillvarreact";
  ciFunction[count].func = func_mac_fill_var_one_react;
  count++;

  ciFunction[count].cmd = "fillvartablen";
  ciFunction[count].func = func_mac_fill_var_tab_len;
  count++;

  ciFunction[count].cmd = "fillvaraverdof";
  ciFunction[count].func = func_mac_aver_dof_res ;
  count++;

  ciFunction[count].cmd = "fillvartablensel";
  ciFunction[count].func = func_mac_fill_var_tab_len_sel;
  count++;

  ciFunction[count].cmd = "fillvarjobname";
  ciFunction[count].func = func_mac_fill_var_jobname ;
  count++;

  ciFunction[count].cmd = "fillvarfilename";
  ciFunction[count].func = func_mac_fill_var_file_name ;
  count++;

  ciFunction[count].cmd = "fillvardatadir";
  ciFunction[count].func = func_mac_fill_var_data_dir ;
  count++;

  ciFunction[count].cmd = "fillvarrestime";
  ciFunction[count].func = func_mac_fill_var_res_time ;
  count++;

  ciFunction[count].cmd = "fillvarissel";
  ciFunction[count].func = func_fill_var_is_selected ;
  count++;

  ciFunction[count].cmd = "fillvarnode";
  ciFunction[count].func = func_fill_var_node ;
  count++;


  ciFunction[count].cmd = "fillvareresnumnrep";
  ciFunction[count].func = func_fill_var_eres_num_nrep ;
  count++;

  ciFunction[count].cmd = "fillvareresnumrep";
  ciFunction[count].func = func_fill_var_eres_num_rep ;
  count++;

  ciFunction[count].cmd = "fillvareresmax";
  ciFunction[count].func = func_fill_var_eres_max ;
  count++;

  ciFunction[count].cmd = "fillvareresmin";
  ciFunction[count].func = func_fill_var_eres_min ;
  count++;

  ciFunction[count].cmd = "fillvareresaver";
  ciFunction[count].func = func_fill_var_eres_aver ;
  count++;

  ciFunction[count].cmd = "fillvareres";
  ciFunction[count].func = func_fill_var_eres ;
  count++;

  ciFunction[count].cmd = "fillvareresselmin";
  ciFunction[count].func = func_fill_var_eres_min_from_all ;
  count++;

  ciFunction[count].cmd = "fillvareresselmax";
  ciFunction[count].func = func_fill_var_eres_max_from_all ;
  count++;

  ciFunction[count].cmd = "fillvarnres";
  ciFunction[count].func = func_fill_var_nres ;
  count++;

  ciFunction[count].cmd = "fillvarfirstsel";
  ciFunction[count].func = func_fill_var_find_first ;
  count++;

  ciFunction[count].cmd = "fillvarkp";
  ciFunction[count].func = func_fill_var_kp ;
  count++;


  ciFunction[count].cmd = "tensorscale";
  ciFunction[count].func = func_fem_comp_vec_scale2D ;
  count++;

  ciFunction[count].cmd = "tensoraniso";
  ciFunction[count].func = func_fem_comp_vec_scale_aniso2D ;
  count++;

  ciFunction[count].cmd = "datatest";
  ciFunction[count].func = func_data_consist_test ;
  count++;

  /* load steps: */
  ciFunction[count].cmd = "step";
  ciFunction[count].func = func_fem_step_new_change ;
  count++;

  ciFunction[count].cmd = "ssmult";
  ciFunction[count].func = func_fem_step_val_change ;
  count++;

  ciFunction[count].cmd = "stepdel";
  ciFunction[count].func = func_fem_step_delete ;
  count++;

  ciFunction[count].cmd = "steplist";
  ciFunction[count].func = func_fem_step_list ;
  count++;

  /* random variables (monte) */
  ciFunction[count].cmd = "rval";
  ciFunction[count].func = func_fem_rv ;
  count++;

  ciFunction[count].cmd = "rvlist";
  ciFunction[count].func = func_fem_rv_list ;
  count++;

  ciFunction[count].cmd = "rvdel";
  ciFunction[count].func = func_fem_rv_del ;
  count++;

	/* input (no)breaking: */
  ciFunction[count].cmd = "nobreakinput";
  ciFunction[count].func = func_mac_set_nobreak_input ;
  count++;


  ciFunction[count].cmd = "matrix";
  ciFunction[count].func = func_mat_new_matrix;
  count++;

  ciFunction[count].cmd = "nomatrix";
  ciFunction[count].func = func_mat_free_matrix_space;
  count++;


  ciFunction[count].cmd = "varmatlist";
  ciFunction[count].func = func_mat_list_matrix;
  count++;

  ciFunction[count].cmd = "varmatwrite";
  ciFunction[count].func = func_mat_write_matrix;
  count++;

  ciFunction[count].cmd = "varmatput";
  ciFunction[count].func = func_mat_matrix_put;
  count++;

  ciFunction[count].cmd = "varmatadd";
  ciFunction[count].func = func_mat_matrix_add;
  count++;

  ciFunction[count].cmd = "varmatget";
  ciFunction[count].func = func_mat_matrix_get;
  count++;

  ciFunction[count].cmd = "varmatread";
  ciFunction[count].func = func_mat_read_matrix;
  count++;

  ciFunction[count].cmd = "varmatfillres";
  ciFunction[count].func = func_mat_matrix_fill_res;
  count++;

  /* path operations */
  ciFunction[count].cmd = "path";
  ciFunction[count].func = func_path_create;
  count++;

  ciFunction[count].cmd = "pathdel";
  ciFunction[count].func = func_path_delete;
  count++;

  ciFunction[count].cmd = "pn";
  ciFunction[count].func = func_path_node_new;
  count++;

  ciFunction[count].cmd = "pathlist";
  ciFunction[count].func = func_path_list;
  count++;

  ciFunction[count].cmd = "prpath";
  ciFunction[count].func = func_path_print_res;
  count++;

  /* test: */
  if (count != ciFuncLen)
  {
    printf("[E] %s!\n",_("Developer's error: Invalid number of commands - exiting"));
    exit(AF_ERR);
  }

  return(AF_OK);
}


/** starts interpreter (name of this funcion isn't very good, is it?) */ 
int my_main (int argc, char *argv[])
{
  char cmd[FEM_STR_LEN+1];
  int cmdlen = FEM_STR_LEN;

	/* Start: */

  /* Some localization */
#ifdef _USE_L10N_
  setlocale(LC_MESSAGES, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif

	/* some fun prints: */
#ifdef RUN_VERBOSE
	fprintf(msgout,"\n");
	if (getenv("USER") != NULL) 
	   { fprintf(msgout, "[ ]  %s: \"%s\"\n", _("User"), getenv("USER")); }
	if (getenv("HOSTNAME") != NULL) 
	   { fprintf(msgout, "[ ]  %s: \"%s\"\n", _("System"), getenv("HOSTNAME")); }
	if (getenv("MACHTYPE") != NULL) 
	   { fprintf(msgout, "[ ]  %s: \"%s\"\n", _("Machine"),getenv("MACHTYPE")); }
	if (getenv("PWD") != NULL) 
	   { fprintf(msgout, "[ ]  %s: \"%s\"\n", _("Directory"), getenv("PWD")); }
#endif

  /* startup message: */
  fprintf (msgout,"\n[I] %s %s", FEM_TUI_RELEASE, _("(terminal mode)"));
  fprintf (msgout,"\n[i] (C) Jiri Brozovsky\n\n");

	/* prompt: */
  fprintf (msgout,"\n%s ",FEM_TUI_PROMPT);

	/* command loop: */
  while (fgets (cmd, cmdlen, stdin))
  {
    ciRunCmdHs (cmd);
  	fprintf (msgout,"\n%s ",FEM_TUI_PROMPT);
  }
  return (AF_OK);
}

#endif

/* end of fem_tcmd.c */
