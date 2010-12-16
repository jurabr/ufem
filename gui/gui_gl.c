/*
   File name: gui_gl.c
   Date:      2003/10/20 19:13
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

	 FEM user interface -- OpenGL canvas
*/

#include "fem_gui.h"

#ifdef _USE_GFX_
#ifdef _USE_GUI_
extern int femGfxPickFunc(double x, double y, long mode, long x0, long y0, long x1, long y1);
extern int femGfxPickBox (double x, double y, double w, double h, long x0, long y0, long x1, long y1, long button);
extern int femGfxCanBox(void);
extern int gfxSavePpm(char *filename, int width, int height);
extern int gfxSaveTiff(char *filename, int width, int height);
extern int  femComputeViewPortProps( long ix, long iy, long *x0, long *y0, long *x, long *y) ;
GtkWidget *glarea;
GdkGC     *gc = NULL;
extern int use_box ;


static int last_x = -1;
static int last_y = -1 ;

void gui_draw_rect(GtkWidget *widget, int x2, int y2, int remove_only, long button)
{
  int x,y, w,h ;
  static int ox,oy, ow,oh ;
  static int x_i,y_i;
  int x1,y1;
  long x0,y0, xm,ym, xi,yi;

  xi = widget->allocation.width ;
  yi = widget->allocation.height ;
  femComputeViewPortProps( xi, yi, &x0, &y0, &xm, &ym);

  if (plotProp.SelRectMode != AF_YES) 
  {
    ox = 0 ;
    oy = 0 ;
    ow = 0 ;
    oh = 0 ;

    x_i = 0 ;
    y_i = 0 ;

    plotProp.SelRectPrev = AF_NO ;
    return;
  }
  
  if (gc == NULL)
  {
    gc = gdk_gc_new(glarea->window);
    gdk_gc_set_foreground(gc, &(glarea->style->white));
    gdk_gc_set_function(gc, GDK_XOR);
  }

  if (plotProp.SelRectPrev == AF_YES)
  {
    gdk_draw_rectangle(glarea->window, gc, FALSE, ox, oy, ow, oh);
  }
  else
  {
    if (remove_only == AF_YES)
    {
      /* no picking here */
      plotProp.SelRectPrev = AF_NO ;
      return ;
    }
    else
    {
      x_i = x2 ;
      y_i = y2 ;

      ox = x2 ;
      oy = y2 ;
      ow = 0 ;
      oh = 0 ;

      plotProp.SelRectPrev = AF_YES ;
      return;
    }
  }
  
  x1 = x_i ;
  y1 = y_i ;

  w = abs(x1 - x2) ;
  h = abs(y1 - y2) ;

  if (x1 <= x2) { x = x1 ; }
  else          { x = x2 ; }

  if (y1 <= y2) { y = y1 ; }
  else          { y = y2 ; }

  ox = x ;
  oy = y ;

  ow = w ;
  oh = h ;
  
  /* picking function */
  if (remove_only == AF_YES)
  {
      femGfxPickBox (ox, oy, ow, oh,
        x0,y0,
        xm,
        ym,
        button 
      ) ;

    plotProp.SelRectPrev = AF_NO ;
    return ;
  }

  gdk_draw_rectangle(glarea->window, gc, FALSE, x, y, w, h);
}


/** Tests if and what mouse button was reseased */
gint glarea_button_release (GtkWidget* widget, GdkEventButton* event) 
{
  int x ;
  int y ;
  long x0,y0, xm,ym, xi,yi;

  xi = widget->allocation.width ;
  yi = widget->allocation.height ;
  femComputeViewPortProps( xi, yi, &x0, &y0, &xm, &ym);

  x = event->x;
  y = event->y;


  if (event->button == 1) 
	{
    /* mouse button 1 was released */
    /*g_print ("Button 1 release (%d, %d)\n", x, y);*/
    if (plotProp.SelStuff != GFX_SELE_NONE)
    {
			if (use_box == AF_YES)
			{
      	gui_draw_rect(widget, x, y, AF_YES, 1);
			}
			else
			{
		    femGfxPickFunc(x, y, 
        	x0,y0,
        	xm, 
        	ym,
        	event->button 
        ) ;
			}
    }

		use_box = AF_NO ;
    return(AF_OK);
  }

  if (event->button == 2) 
	{
    /* mouse button 2 was released */
    /*g_print ("Button 2 release (%d, %d)\n", x, y);*/
		use_box = AF_NO ;
    return (AF_OK);
  }

  if (event->button == 3) 
	{
    /* mouse button 2 was released */
    /*g_print ("Button 3 release (%d, %d)\n", x, y);*/
    if (plotProp.SelStuff != GFX_SELE_NONE)
    {
			if (use_box == AF_YES)
			{
      	gui_draw_rect(widget, x, y, AF_YES, 3);
			}
			else
			{
		    femGfxPickFunc(x, y, 
        	x0,y0,
        	xm, 
        	ym,
        	event->button 
        ) ;
			}

    }
		use_box = AF_NO ;
    return (AF_OK);
  }

  return(AF_ERR);
}

/** Pressing of button */
gint glarea_button_press (GtkWidget* widget, GdkEventButton* event) 
{
  long x0,y0, xm,ym, xi,yi;
  int x = event->x;
  int y = event->y;

  xi = widget->allocation.width ;
  yi = widget->allocation.height ;
  femComputeViewPortProps( xi, yi, &x0, &y0, &xm, &ym);

	if (plotProp.ActMot == AF_YES)
	{
    last_x = event->x;
    last_y = event->y;
	}

  if (event->button == 2)
	{
		use_box = AF_NO ;

  	femGfxPickFunc(x, y, 
        x0,y0,
        xm, 
        ym,
        event->button 
        ) ;

    return (AF_OK);
	}
  /* pressing of buttons 1,2,3 cause action */
  if ((event->button >= 1) && (event->button <= 3))
	{
		use_box = AF_NO ;
#if 0
		/* This probably should be removed: */
    femGfxPickFunc(x, y, 
        x0,y0,
        xm, 
        ym,
        event->button 
        ) ;
#endif

    return (AF_OK);
  }

  /* other buttons are ignored */
  return (AF_ERR);
}

/* ****************************** */
#ifndef _USE_GTK_GL_AREA_
gint glarea_mouse_wheel (GtkWidget* widget, GdkEventScroll* event, gpointer data) 
{
	char str[CI_STR_LEN] ;

  if (event->direction)
	{
		sprintf(str,"unzoom,%e",0.2);
		ciRunCmd(str); 
    return (AF_OK);
	}
	else
	{
		sprintf(str,"zoom,%e",0.2);
		ciRunCmd(str); 
    return (AF_OK);
	}

  /* other buttons are ignored */
  return (AF_ERR);
}
#endif
/* ****************************** */

/** oops, mouse is moving! */
gint glarea_motion_notify (GtkWidget* widget, GdkEventMotion* event) 
{
  int x;
  int y;
	double valx, valy;
	char str[CI_STR_LEN] ;

  GdkModifierType state;

  if (event->is_hint) 
	{
    gdk_window_get_pointer(event->window, &x, &y, &state);
  } 
	else 
	{
    x = event->x;
    y = event->y;
    state = event->state;
  }

	if (last_x <= -1) { last_x = x ; }
	if (last_y <= -1) { last_y = y ; }
  
  if (state & GDK_BUTTON1_MASK) 
	{
    /* mouse button 1 is engaged */
    if (plotProp.SelStuff != GFX_SELE_NONE)
    {
			if (femGfxCanBox() == AF_YES)
			{
				use_box = AF_YES ;
      	gui_draw_rect(widget, x, y, AF_NO, 1);
			}
    }
		else
		{
			/* Moving */
    	if (plotProp.ActMot == AF_YES)
    	{
				plotStuff.autoreplot = AF_NO ;

				valx = 10*(((double)x-(double)last_x) / (double)widget->allocation.width) ;
				if (fabs(valx) > 0.001)
				{
					sprintf(str,"move,x,%e",valx);
					ciRunCmd(str); 
				}

				valy = -10*(((double)y-(double)last_y) / (double)widget->allocation.height) ;
				if (fabs(valy) > 0.001)
				{
					sprintf(str,"move,y,%e",valy);
					ciRunCmd(str); 
				}

				plotStuff.autoreplot = AF_YES ;
				ciRunCmd("replot"); 
				last_x = x ;
				last_y = y ;
			}
		}
  }

  if (state & GDK_BUTTON2_MASK) 
	{
    /* mouse button 2 is engaged */
		use_box = AF_NO ;

			/* Moving */
    	if (plotProp.ActMot == AF_YES)
    	{
				valy = -10*(((double)y-(double)last_y) / (double)widget->allocation.height) ;

				if (fabs(valy) > 0.005)
				{
					if (valy > 0)
					{
						sprintf(str,"zoom,%e",valy);
					}
					else
					{
						sprintf(str,"unzoom,%e",fabs(valy));
					}
					ciRunCmd(str); 
				}
          
          /* rotating along Z axis: */
          valx = 100*(((double)x-(double)last_x) / (double)widget->allocation.width) ;
				  if (fabs(valx) > 0.001)
				  {
					  sprintf(str,"rot,z,%e",valx);
					  ciRunCmd(str); 
				  }

				ciRunCmd("replot"); 
				last_x = x ;
				last_y = y ;
			}
  }

  if (state & GDK_BUTTON3_MASK) 
	{
    /* mouse button 3 is engaged */
    if (plotProp.SelStuff != GFX_SELE_NONE)
    {
			if (femGfxCanBox() == AF_YES)
			{
				use_box = AF_YES ;
      	gui_draw_rect(widget, x, y, AF_NO, 3);
			}
    }
		else
		{
			/* Rotating */
    	if (plotProp.ActMot == AF_YES)
    	{
				plotStuff.autoreplot = AF_NO ;

				valx = 100*(((double)x-(double)last_x) / (double)widget->allocation.width) ;
				if (fabs(valx) > 0.001)
				{
					sprintf(str,"rot,y,%e",valx);
					ciRunCmd(str); 
				}

				valy = 100*(((double)y-(double)last_y) / (double)widget->allocation.height) ;
				if (fabs(valy) > 0.001)
				{
					sprintf(str,"rot,x,%e",valy);
					ciRunCmd(str); 
				}

				plotStuff.autoreplot = AF_YES ;
				ciRunCmd("replot"); 
				last_x = x ;
				last_y = y ;
			}
		}
  }

  return (AF_OK);
}

gint draw_stuff(GtkWidget* widget, GdkEventExpose* event)
{
  long x0,y0, x,y, xi,yi, i, was;
    xi = widget->allocation.width ;
    yi = widget->allocation.height ;
    was = activePlotView ;
    for (i=0; i<maxPlotView; i++)
    {
      activePlotView = i ;

      femComputeViewPortProps( xi, yi, &x0, &y0, &x, &y);

      /* preparing of plot: */
		  femPrePlot(x0,y0,x, y, AF_NO) ;

      /* plotting: */
		  femPlotStuff(x, y, AF_NO) ;

      /* things after plot: */
		  femPostPlot(x0,y0, x, y, AF_NO);
    }
    activePlotView = was ;

  return (AF_OK);
}

/** drawing */
gint glarea_draw (GtkWidget* widget, GdkEventExpose* event) 
{
	int rv = AF_OK ;
  int one_more = AF_NO ;
#ifndef _USE_GTK_GL_AREA_
	/* for gtkglext: */
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
#endif
#ifdef USE_GL2PS
  FILE *fp = NULL ;
  int buffsize = 0;
  int state = GL2PS_OVERFLOW;
#endif

  if (event != NULL)
  {
    /* draw only on the last expose event */
    if (event->count > 0) 
	  {
      return(AF_OK);
    }
  }

#ifdef _USE_GTK_GL_AREA_
	/* make_current: */
  if (gtk_gl_area_make_current(GTK_GL_AREA(widget))) 
	{
#else
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
  return (AF_ERR_IO);
#endif

  	if ( (plotProp.PlotToFile == AF_YES) && (plotProp.saveBitmap != AF_YES) )
  	{
#ifdef USE_GL2PS
      one_more = AF_YES ;
    	/*setlocale(LC_NUMERIC,"C");*/ /* workaround for non-english locales + PS */
    
	  	if (femPlotFile == NULL) {return(AF_ERR_VAL);}

    	if ( (fp = fopen(femPlotFile,"w")) == NULL )
    	{
      	plotProp.PlotToFile = AF_NO ;
      	fprintf(msgout,"[E] %s!\n", _("Cannot open file for writing"));
      	return(AF_ERR_IO);
    	}
  
    	errno=0;
    	while (state == GL2PS_OVERFLOW)
    	{
      	buffsize +=1024*1024;
		  	gl2psBeginPage ( plotProp.PlotTitle, FEM_TUI_RELEASE, 
               	plotProp.viewport,
               	plotProp.plotFormat, 
							 	GL2PS_BSP_SORT,
               	GL2PS_SIMPLE_LINE_OFFSET|GL2PS_BEST_ROOT|GL2PS_SILENT|GL2PS_OCCLUSION_CULL,
               	GL_RGBA, 0, NULL, 0,0,0, buffsize, fp, NULL );

      	draw_stuff(widget, event) ;
      	state = gl2psEndPage();
    	}
    	if (errno != 0)
    	{
      	fprintf(msgout,"[E] %s!\n", _("writing problems"));
    	}
    	plotProp.PlotToFile = AF_NO ;

    	if (fclose(fp) != 0)
    	{
      	fprintf(msgout,"[E] %s!\n", _("cannot close file"));
      	rv = AF_ERR_IO;
    	}

#endif
  	}
    else
    {
      if (plotProp.saveBitmap == AF_YES)
      {
    	  plotProp.PlotToFile = AF_YES ;
        draw_stuff(widget, event);
        draw_stuff(widget, event);

#ifdef _USE_LIB_TIFF_
        if (plotProp.bitmapType == GFX_PPM)
        {
          rv = gfxSavePpm(femPlotFile,
                          widget->allocation.width, 
                          widget->allocation.height);
        }
        else
        {
          rv = gfxSaveTiff(femPlotFile,
                           widget->allocation.width, 
                           widget->allocation.height);
        }
#else
        rv = gfxSavePpm(femPlotFile,
                        widget->allocation.width, 
                        widget->allocation.height);
#endif

      	if (rv != AF_OK)
        {
          fprintf(msgout,"[E] %s!\n", _("Image saving failed"));
        }
        plotProp.saveBitmap = AF_NO ;
    	  plotProp.PlotToFile = AF_NO ;
      }

      draw_stuff(widget, event);
    }
#ifdef _USE_GTK_GL_AREA_
		/* swapping of buffers: */
   	gtk_gl_area_swapbuffers (GTK_GL_AREA(widget));
  }
#else
	if (gdk_gl_drawable_is_double_buffered(gldrawable))
  {
     gdk_gl_drawable_swap_buffers(gldrawable);
  }
  else
  {
		glFlush();
  }
  gdk_gl_drawable_gl_end (gldrawable);
#endif
  
	/* make_current: */
  if (one_more == AF_YES) /* only after gl2ps  */
  {
#ifdef _USE_GTK_GL_AREA_
    if (gtk_gl_area_make_current(GTK_GL_AREA(widget))) 
	  {
  	  rv = draw_stuff(widget, event) ;
		  /* swapping of buffers: */
   	  gtk_gl_area_swapbuffers (GTK_GL_AREA(widget));
    }
#else
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return (AF_ERR_IO);
 	  rv = draw_stuff(widget, event) ;
	  if (gdk_gl_drawable_is_double_buffered(gldrawable))
        gdk_gl_drawable_swap_buffers(gldrawable);
    else
			  glFlush();
    gdk_gl_drawable_gl_end (gldrawable);
#endif
  }

	return(rv);
}


/** reaction to change of window's size */
gint glarea_reshape (GtkWidget* widget, GdkEventConfigure* event) 
{
  long x0,y0, x,y, xi,yi;
	gint rv = AF_OK ;
#ifndef _USE_GTK_GL_AREA_
	/* for gtkglext: */
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
#endif

#ifdef _USE_GTK_GL_AREA_
  if (gtk_gl_area_make_current (GTK_GL_AREA(widget))) 
	{
#else
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
  return (AF_ERR_IO);
#endif
    xi = widget->allocation.width ;
    yi = widget->allocation.height ;
    femComputeViewPortProps( xi, yi, &x0, &y0, &x, &y);
    
    /* preparing of plot: */
		femPrePlot(x0,y0,x, y, AF_NO) ;

    /* plotting: */
		femPlotStuff(x, y, AF_NO) ;

    /* things after plot: */
		femPostPlot(x0,y0, x, y, AF_NO);

#ifdef _USE_GTK_GL_AREA_
		/* swapping of buffers: */
    gtk_gl_area_swapbuffers (GTK_GL_AREA(widget));
  }
#else
	if (gdk_gl_drawable_is_double_buffered(gldrawable))
       gdk_gl_drawable_swap_buffers(gldrawable);
  else
			 glFlush();
  gdk_gl_drawable_gl_end (gldrawable);
#endif
  
  return(rv);
}


/** realize event - init of the drawing board (it may be usefull any day)  */
gint glarea_init (GtkWidget* widget) 
{
#ifdef _USE_GTK_GL_AREA_
  if (gtk_gl_area_make_current (GTK_GL_AREA(widget))) 
	{
    /* OpenGL initialization code (not needed for now) */
  }
#endif
  return(AF_OK);
}

/** destruction of glarea - a place for cleanup code */
gint glarea_destroy (GtkWidget* widget) 
{
  /* Insert any required cleanup code here. */

  return(AF_OK);
}


/** creation of the GL Area */
GtkWidget* glArea_new(void) 
{
#ifdef _USE_GTK_GL_AREA_	
  int attrlist[] = {
    GDK_GL_RGBA,
    GDK_GL_DOUBLEBUFFER,
    GDK_GL_DEPTH_SIZE, 1,
    GDK_GL_NONE
  };
#else
  GdkGLConfig *glconfig;

  glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
                                        GDK_GL_MODE_DEPTH |
                                        GDK_GL_MODE_DOUBLE);
  if (glconfig == NULL)
  {
    glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
                                           GDK_GL_MODE_DEPTH);
    if (glconfig == NULL)
    {
      fprintf(msgout,"[E] %s!\n", "Cannot use OpenGL - EXITING");
      exit (AF_ERR);
    }
	}
#endif

#ifdef _USE_GTK_GL_AREA_
	/* OpenGL (tm) support: */
  if(gdk_gl_query() == AF_ERR) 
	{
    fprintf(msgout,"[E] %s!!\n",_("OpenGL not supported\n"));
    return NULL;
  }
#endif

	/* creating of glarea */
#ifdef _USE_GTK_GL_AREA_
  if ((glarea = GTK_WIDGET(gtk_gl_area_new(attrlist))) == NULL) 
#else
  if ((glarea = GTK_WIDGET(gtk_drawing_area_new())) == NULL) 
#endif
	{
    fprintf(msgout,"[E] %s!\n",_("Can NOT create graphics canvas\n"));
    return NULL;
  }

#ifndef _USE_GTK_GL_AREA_
  gtk_widget_set_gl_capability (glarea,
                                glconfig,
                                NULL,
                                TRUE,
                                GDK_GL_RGBA_TYPE);
#endif

	/* some boring events */
  gtk_widget_set_events(GTK_WIDGET(glarea),
                        GDK_EXPOSURE_MASK|
                        GDK_BUTTON_PRESS_MASK|
#ifndef _USE_GTK_GL_AREA_
												GDK_SCROLL_MASK |
#endif
                        GDK_BUTTON_RELEASE_MASK|
                        GDK_POINTER_MOTION_MASK|
                        GDK_POINTER_MOTION_HINT_MASK);

  /* button_release_event - mouse button released */
  gtk_signal_connect (GTK_OBJECT(glarea), "button_release_event",
                      GTK_SIGNAL_FUNC(glarea_button_release), NULL);

  /* button_press_event - mouse button pressed */
  gtk_signal_connect (GTK_OBJECT(glarea), "button_press_event",
                      GTK_SIGNAL_FUNC(glarea_button_press), NULL);
  
  /* motion_notify_event - mouse is moving */
  gtk_signal_connect (GTK_OBJECT(glarea), "motion_notify_event",
                      GTK_SIGNAL_FUNC(glarea_motion_notify), NULL);
  
#ifndef _USE_GTK_GL_AREA_
  /* scroll_event - mouse wheel is scrolling */
  gtk_signal_connect (GTK_OBJECT(glarea), "scroll_event",
                      GTK_SIGNAL_FUNC(glarea_mouse_wheel), NULL);
#endif
  
  /* expose_event - window was exposed - redraw probably needed  */
  gtk_signal_connect (GTK_OBJECT(glarea), "expose_event",
                      GTK_SIGNAL_FUNC(glarea_draw), NULL);
  
  /* configure_event - window resized  */
  gtk_signal_connect (GTK_OBJECT(glarea), "configure_event",
                      GTK_SIGNAL_FUNC(glarea_reshape), NULL);
  
  /* realize - window has been created */
  gtk_signal_connect (GTK_OBJECT(glarea), "realize",
                      GTK_SIGNAL_FUNC(glarea_init), NULL);
  
  /* destroy - window has received destroy event - cleanum needed */
  gtk_signal_connect (GTK_OBJECT(glarea), "destroy",
                      GTK_SIGNAL_FUNC (glarea_destroy), NULL);

	/* default size of area */
  gtk_widget_set_usize(GTK_WIDGET(glarea), 200, 100);

  return (glarea);
}

/** plots data - this is called from cmd_plot.c etc. */
int femPlot(void)
{
  glarea_draw(glarea, NULL) ;
  return(AF_OK);
}
#else /* end of _USE_GUI_ */
#ifdef _USE_GLUT_UI_

int draw_stuff(int width, int height)
{
  long x,y,x0,y0,xi,yi ;
  width  = glutGet(GLUT_WINDOW_WIDTH);
  height = glutGet(GLUT_WINDOW_HEIGHT);

  xi = width ;
  yi = height ;
  femComputeViewPortProps( xi, yi, &x0, &y0, &x, &y);

  /* preparing of plot: */
	femPrePlot(x0,y0,x, y, AF_NO) ;

  /* plotting: */
	femPlotStuff(x, y, AF_NO) ;

  /* things after plot: */
	femPostPlot(x0,y0,x, y, AF_NO);

  return (AF_OK);
}

void femPlot(void)
{
	int rv = AF_OK ;
  int one_more = AF_NO ;
  int width;
  int height;
  long x,y,x0,y0,xi,yi ;
#ifdef USE_GL2PS
  FILE *fp = NULL ;
  int buffsize = 0;
  int state = GL2PS_OVERFLOW;
#endif
  
  width  = glutGet(GLUT_WINDOW_WIDTH);
  height = glutGet(GLUT_WINDOW_HEIGHT);

  xi = width ;
  yi = height ;
  femComputeViewPortProps( xi, yi, &x0, &y0, &x, &y);

  	if ( (plotProp.PlotToFile == AF_YES) && (plotProp.saveBitmap != AF_YES) )
  	{
#ifdef USE_GL2PS
      one_more = AF_YES ;
    	/*setlocale(LC_NUMERIC,"C");*/ /* workaround for non-english locales + PS */
    
	  	if (femPlotFile == NULL) {return(AF_ERR_VAL);}

    	if ( (fp = fopen(femPlotFile,"w")) == NULL )
    	{
      	plotProp.PlotToFile = AF_NO ;
      	fprintf(msgout,"[E] %s!\n", _("Cannot open file for writing"));
      	return(AF_ERR_IO);
    	}
  
    	errno=0;
    	while (state == GL2PS_OVERFLOW)
    	{
      	buffsize +=1024*1024;
		  	gl2psBeginPage ( plotProp.PlotTitle, FEM_TUI_RELEASE, 
               	plotProp.viewport,
               	plotProp.plotFormat, 
							 	GL2PS_BSP_SORT,
               	GL2PS_SIMPLE_LINE_OFFSET|GL2PS_BEST_ROOT|GL2PS_SILENT|GL2PS_OCCLUSION_CULL,
               	GL_RGBA, 0, NULL, 0,0,0, buffsize, fp, NULL );

      	draw_stuff(x,y) ;
      	state = gl2psEndPage();
    	}
    	if (errno != 0)
    	{
      	fprintf(msgout,"[E] %s!\n", _("writing problems"));
    	}
    	plotProp.PlotToFile = AF_NO ;

    	if (fclose(fp) != 0)
    	{
      	fprintf(msgout,"[E] %s!\n", _("cannot close file"));
      	rv = AF_ERR_IO;
    	}

#endif
  	}
    else
    {
      if (plotProp.saveBitmap == AF_YES)
      {
    	  plotProp.PlotToFile = AF_YES ;
        draw_stuff(x,y);
        draw_stuff(x,y);

#ifdef _USE_LIB_TIFF_
        if (plotProp.bitmapType == GFX_PPM)
        {
          rv = gfxSavePpm(femPlotFile, x, y);
        }
        else
        {
          rv = gfxSaveTiff(femPlotFile, x, y);
        }
#else
        rv = gfxSavePpm(femPlotFile, x, y);
#endif

      	if (rv != AF_OK)
        {
          fprintf(msgout,"[E] %s!\n", _("Image saving failed"));
        }
        plotProp.saveBitmap = AF_NO ;
    	  plotProp.PlotToFile = AF_NO ;
      }

      draw_stuff(x,y);
    }
		
#if 0
		glFlush();
#else
		glutSwapBuffers();
#endif

	/* make_current: */
  if (one_more == AF_YES) /* only after gl2ps  */
  {
  	rv = draw_stuff(x,y) ;
		glFlush();
  }

  return;
}

void reshape_func(int width, int height)
{
	draw_stuff(width,height);
}

void idlefunc(void)
{
	char cmd[FEM_STR_LEN+1];
  int cmdlen = FEM_STR_LEN;

#if 0
  ciRunCmd (cmd);
#else
  while (fgets (cmd, cmdlen, stdin))
  {
    ciRunCmd (cmd);
  	fprintf (msgout,"\n%s ",FEM_TUI_PROMPT);
  }
#endif
}

void timer_func(int param)
{
	char cmd[FEM_STR_LEN+1];
  int cmdlen = FEM_STR_LEN;

#if 0
	if (feof(stdin))
	{
    if (fgets(cmd, cmdlen, stdin))
	  {
  	  ciRunCmd (cmd);
  	  fprintf (msgout,"\n%s ",FEM_TUI_PROMPT);
	  }
	}
  glutTimerFunc(2000,timer_func,param);
#else
  while (fgets (cmd, cmdlen, stdin))
  {
    ciRunCmd (cmd);
  	fprintf (msgout,"\n%s ",FEM_TUI_PROMPT);
  }
#endif
}

int guiMainWin(void)
{
#if 1
  glutInitWindowSize(1024,768);
#else
  glutInitWindowSize(788,540);
#endif
  glutInitWindowPosition(-1,1);
  glutCreateWindow(FEM_TUI_RELEASE);
  glutDisplayFunc(femPlot);
  glutReshapeFunc(reshape_func);
  /*glutIdleFunc(idlefunc);*/
  glutTimerFunc(2000,timer_func,1234);
	femPlot();
  return(AF_OK);
}

void guiResizeMainWin(void)
{
	glutReshapeWindow(guiProp.width,guiProp.height);
}

#endif
#endif
#endif /* _USE_GFX_*/

/* end of gui_gl.c */
