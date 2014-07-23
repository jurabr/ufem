/*
   File name: fdb_prn.c
   Date:      2004/01/06 21:49
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

	 FEM Pre/post-processor - printing of data (common functions)
*/

#include "fdb.h"
#include "cint.h"

extern char *femDataDir ; /* from fem_tui.h */
extern char *femGetJobname(void); /* from fem_tui.h */

long  fdbOutputFormat      = FDB_FORMAT_TEXT ;
long  fdbActPrnTabLen      = 0 ;

long  fdbPrnAutoName       = AF_YES ; /* if file is auto-named      */
long  fdbPrnAppendFile     = AF_NO ; /* if file is appended        */
FILE *fdbPrnFile           = NULL ; /* file pointer               */
char *fdbPrnFileName       = NULL ;   /* name of non-automatic file */
long  fdbPrnAppFileCounter = 0 ; /* if file is appended        */
char *fdbPrnViewCommand    = NULL ; /* command to view files */


/** Returnes string in safe way (no NULL)
 * @param str string to be returned
 * @param defstr string to be returned if str is NULL
 * @return str or defstr or "--"
 * */
char *fdbPrnSafeStr(char *str, char *defstr)
{
	if (str == NULL)
	{
		if (defstr == NULL)
		{
			return("--");
		}
		else
		{
			return(defstr);
		}
	}
	else
	{
		return(str);
	}
}

/** Prints table header file
 * @param fdbPrnFile pointer to file
 * @param rows number of rows
 * @param title of table (NULL if not needed)
 * */
void fdbPrnBeginTable(FILE *fw, long rows, char *title)
{
	int i ;

	fdbActPrnTabLen = rows ;

	if (strlen (title) > 0)
	{
		if (title != NULL)
		{
			switch (fdbOutputFormat)
			{
				case FDB_FORMAT_HTML: fprintf(fdbPrnFile, "<h3>%s</h3>\n",title ) ; break ;
				case FDB_FORMAT_LTX:  fprintf(fdbPrnFile, "\\section*{%s}\n",title ) ; break ;
				case FDB_FORMAT_CSV:  fprintf(fdbPrnFile, "%s;\n",title ) ; break ;
				case FDB_FORMAT_TEXT: 
				default:              fprintf(fdbPrnFile, "%s\n",title ) ; break ;
			}
	 	}
  }

	switch (fdbOutputFormat)
	{  
		 case FDB_FORMAT_HTML: fprintf(fdbPrnFile, "<table border=\"1\">\n") ; break ;
		 case FDB_FORMAT_LTX:  fprintf(fdbPrnFile, "\\begin{tabular}{|") ; 
													 for (i=0; i<rows; i++){fprintf(fdbPrnFile,"l|");}
													 fprintf(fdbPrnFile,"}\n");
													 break ;
		 case FDB_FORMAT_CSV: 
		 case FDB_FORMAT_TEXT: 
		 default:              fprintf(fdbPrnFile, "\n" ) ; 
													 break ;
	}
}

/** Prints item of table header
 * @param fdbPrnFile pointer to file
 * @param pos index of row (0,1,2,..)
 * @param text text to be printed
 * */
void fdbPrnTableHeaderItem(FILE *fw, long pos, char *text)
{
	switch (fdbOutputFormat)
	{  
		 case FDB_FORMAT_HTML: if (pos == 0) { fprintf(fdbPrnFile,"<tr>\n"); }
													 fprintf(fdbPrnFile,"<td align=\"right\">");
			 										 fprintf(fdbPrnFile, "<b>%s</b>",fdbPrnSafeStr(text,"&nbsp;")) ;
													 fprintf(fdbPrnFile,"</td>\n");
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"</tr>\n"); }
 													 break ;
		 case FDB_FORMAT_LTX:  fprintf(fdbPrnFile,"{\\bf %s}", fdbPrnSafeStr(text,"~")); 
													 if (pos < (fdbActPrnTabLen-1)) 
													 { 
														 fprintf(fdbPrnFile,"&");
													 }
													 else
													 {
														 fprintf(fdbPrnFile,"\\\\\n \\hline \n \\hline \n"); 
													 }
 													 break ;
		 case FDB_FORMAT_CSV: 
		                       fprintf(fdbPrnFile, " %12s;", fdbPrnSafeStr(text," ") ) ; 
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"\n"); }
													 break ;
		 case FDB_FORMAT_TEXT: 
		 default:              fprintf(fdbPrnFile, " %12s", fdbPrnSafeStr(text," ") ) ; 
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"\n"); }
													 break ;
	}
}

/** Prints item of table 
 * @param fdbPrnFile pointer to file
 * @param pos index of row (0,1,2,..)
 * @param text text to be printed
 * */
void fdbPrnTableItemStr(FILE *fw, long pos, char *text)
{
	switch (fdbOutputFormat)
	{  
		 case FDB_FORMAT_HTML: if (pos == 0) { fprintf(fdbPrnFile,"<tr>\n"); }
													 fprintf(fdbPrnFile,"<td align=\"right\">");
			 										 fprintf(fdbPrnFile, "%s",fdbPrnSafeStr(text,"&nbsp;")) ;
													 fprintf(fdbPrnFile,"</td>\n");
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"</tr>\n"); }
 													 break ;
		 case FDB_FORMAT_LTX:  fprintf(fdbPrnFile,"%s", fdbPrnSafeStr(text,"~")); 
													 if (pos < (fdbActPrnTabLen-1)) 
													 { 
														 fprintf(fdbPrnFile,"&");
													 }
													 else
													 {
														 fprintf(fdbPrnFile,"\\\\\n \\hline \n"); 
													 }
													 break ;
		 case FDB_FORMAT_CSV: 
		                       fprintf(fdbPrnFile, " %12s;", fdbPrnSafeStr(text," ") ) ; 
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"\n"); }
													 break ;
		 case FDB_FORMAT_TEXT: 
		 default:              fprintf(fdbPrnFile, " %12s", fdbPrnSafeStr(text," ") ) ; 
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"\n"); }
													 break ;
	}
}

/** Prints item of table (integer)
 * @param fdbPrnFile pointer to file
 * @param pos index of row (0,1,2,..)
 * @param text text to be printed
 * */
void fdbPrnTableItemInt(FILE *fw, long pos, long text)
{
	switch (fdbOutputFormat)
	{  
		 case FDB_FORMAT_HTML: if (pos == 0) { fprintf(fdbPrnFile,"<tr>\n"); }
													 fprintf(fdbPrnFile,"<td>");
			 										 fprintf(fdbPrnFile, "%li",text) ;
													 fprintf(fdbPrnFile,"</td>\n");
													 break;
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"</tr>\n"); }
		 case FDB_FORMAT_LTX:  fprintf(fdbPrnFile,"%li", text); 
													 if (pos < (fdbActPrnTabLen-1)) 
													 { 
														 fprintf(fdbPrnFile,"&");
													 }
													 else
													 {
														 fprintf(fdbPrnFile,"\\\\\n \\hline \n"); 
													 }
													 break ;
		 case FDB_FORMAT_CSV: 
		                       fprintf(fdbPrnFile, " %12li;", text) ; 
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"\n"); }
													 break ;
		 case FDB_FORMAT_TEXT: 
		 default:              fprintf(fdbPrnFile, " %12li", text) ; 
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"\n"); }
													 break ;
	}
}


/** Prints item of table (double)
 * @param fdbPrnFile pointer to file
 * @param pos index of row (0,1,2,..)
 * @param text text to be printed
 * */
void fdbPrnTableItemDbl(FILE *fw, long pos, double text)
{
	switch (fdbOutputFormat)
	{  
		 case FDB_FORMAT_HTML: if (pos == 0) { fprintf(fdbPrnFile,"<tr>\n"); }
													 fprintf(fdbPrnFile,"<td>");
			 										 fprintf(fdbPrnFile, "%e",text) ;
													 fprintf(fdbPrnFile,"</td>\n");
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"</tr>\n"); }
													 break;
		 case FDB_FORMAT_LTX:  fprintf(fdbPrnFile,"%e", text); 
													 if (pos < (fdbActPrnTabLen-1)) 
													 { 
														 fprintf(fdbPrnFile,"&");
													 }
													 else
													 {
														 fprintf(fdbPrnFile,"\\\\\n \\hline \n"); 
													 }
													 break ;
		 case FDB_FORMAT_CSV: 
		                       fprintf(fdbPrnFile, " %12e;", text) ; 
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"\n"); }
													 break ;
		 case FDB_FORMAT_TEXT: 
		 default:              fprintf(fdbPrnFile, " %12e", text) ; 
													 if (pos >= (fdbActPrnTabLen-1)) { fprintf(fdbPrnFile,"\n"); }
													 break ;
	}
}

/** Prints end of table 
 * @param fdbPrnFile pointer to file
 * */
void fdbPrnEndTable(FILE *fw)
{
	switch (fdbOutputFormat)
	{  
		 case FDB_FORMAT_HTML: fprintf(fdbPrnFile, "</table>\n") ; break ;
		 case FDB_FORMAT_LTX:  fprintf(fdbPrnFile, "\\end{tabular}\n") ; break ;
		 case FDB_FORMAT_CSV: 
		 case FDB_FORMAT_TEXT: 
		 default:              fprintf(fdbPrnFile, "\n" ) ; break ;
	}

	fdbActPrnTabLen = 0 ;
}

/** Prints begin of file
 * @param fdbPrnFile pointer to file
 * */
void fdbPrnBeginFile(FILE *fw)
{
	switch (fdbOutputFormat)
	{  
		 case FDB_FORMAT_HTML: 
			 fprintf(fdbPrnFile, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
			 fprintf(fdbPrnFile,"<html>\n<head>\n");
			 fprintf(fdbPrnFile," <title>%s</title>\n</head>\n<body>\n",_("FEM Output"));
			 break;
		 case FDB_FORMAT_LTX:  fprintf(fdbPrnFile, "\\documentclass{report}\n \\begin{document}") ; break ;
		 case FDB_FORMAT_CSV: 
		 case FDB_FORMAT_TEXT: 
     default: break ;
	}
	
	fdbActPrnTabLen = 0 ;
}

/** Prints end of file
 * @param fdbPrnFile pointer to file
 * */
void fdbPrnEndFile(FILE *fw)
{
	switch (fdbOutputFormat)
	{  
		 case FDB_FORMAT_HTML: fprintf(fdbPrnFile,"</body\n</html>\n"); break;
		 case FDB_FORMAT_LTX:  fprintf(fdbPrnFile, "\\end{document}\n") ; break ;
		 case FDB_FORMAT_CSV: 
		 case FDB_FORMAT_TEXT: 
     default: break ;
	}
	
	fdbActPrnTabLen = 0 ;
}


/** Openes file for writing */
int fdbPrnOpenFile(FILE *fw)
{
  char style[] = "a";
  char count[CI_STR_LEN+1] ;
  char ext[256] ;
  int  i ;

  for (i=0; i<256; i++)
  {
    count[i] = '\0' ;
    ext[i]   = '\0' ;
  }
  
  if (fdbPrnAppendFile == AF_YES)
  {
    style[0] = 'a' ;
  }
  else
  {
    style[0] = 'w' ;
  }
  
  if (fdbPrnFile != NULL)
  {
    /* file opened - no action */
    return(AF_OK);
  }
  else
  {
    if (fdbPrnAutoName == AF_NO)
    {
      if (fdbPrnFileName == NULL)
      {
        fprintf(msgout, "[E] %s!\n", _("Output file name unknown"));
        return(AF_ERR_EMP);
      }
      else
      {
        if ((fdbPrnFile = fopen(fdbPrnFileName, style)) == NULL )
        {
          fprintf(msgout, "[E] %s!\n", _("Unable to open file"));
          return(AF_ERR_IO) ;
        }
        else
        {
          fdbPrnBeginFile(fdbPrnFile);
          return(AF_OK) ;
        }
      }
    }
    else
    {
      switch (fdbOutputFormat)
	    {  
		    case FDB_FORMAT_HTML: sprintf(ext,"html"); break;
		    case FDB_FORMAT_LTX:  sprintf(ext,"tex"); break ;
		    case FDB_FORMAT_CSV:  sprintf(ext,"csv"); break ;
		    case FDB_FORMAT_TEXT: 
		    default:              sprintf(ext,"txt"); break ;
	    }

      if (femGetJobname() != NULL)
      {
        sprintf(count,"%s-out%li",femGetJobname(), fdbPrnAppFileCounter) ;
      }
      else
      {
        sprintf(count,"out%li", fdbPrnAppFileCounter) ;
      }
      fdbPrnAppFileCounter++;
      
      if ((fdbPrnFile = fopen(ciSetPath(femDataDir,count,ext), style)) == NULL )
      {
        fprintf(msgout, "[E] %s!\n", _("Unable to open file"));
        return(AF_ERR_IO) ;
      }
      else
      {
        fdbPrnBeginFile(fdbPrnFile);
        return(AF_OK) ;
      }
    }
  }

  return(AF_OK);
}


/** Closes file */
int fdbPrnCloseFile(FILE *fw)
{
  char count[256] ;
  char ext[256] ;

  if ((fdbPrnFile == NULL)||(fdbPrnFile == stdout)||(fdbPrnFile == stderr))
  {
    return(AF_OK) ;
  }
  else
  {
    fdbPrnEndFile(fw) ;
    fclose(fdbPrnFile) ;
    fdbPrnFile = NULL ;

    /* viewer is called: */
    if ( (fdbPrnViewCommand != NULL) && (fdbPrnAutoName == AF_YES) )
    {
      switch (fdbOutputFormat)
	    {  
		    case FDB_FORMAT_HTML: sprintf(ext,"html"); break;
		    case FDB_FORMAT_LTX:  sprintf(ext,"tex"); break ;
		    case FDB_FORMAT_CSV:  sprintf(ext,"csv"); break ;
		    case FDB_FORMAT_TEXT: 
		    default:              sprintf(ext,"txt"); break ;
	    }

      if (femGetJobname() != NULL)
      {
        sprintf(count,"%s-out%li",femGetJobname(), fdbPrnAppFileCounter-1) ;
      }
      else
      {
        sprintf(count,"out%li", fdbPrnAppFileCounter-1) ;
      }

      system(
          ciStrCat2ParsNoCommas(fdbPrnViewCommand, 
          ciSetPath(femDataDir,count,ext))
          ) ;
    }

  }

  return(AF_OK) ;
}

/* end of fdb_prn.c */
