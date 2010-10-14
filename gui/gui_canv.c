/*
   File name: gui_canv.c
   Date:      2006/11/15 15:03
   Author:    Jiri Brozovsky

   Copyright (C) 2006 Jiri Brozovsky

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

	 FEM user interface -- Gtk+ canvas
*/

#include "fem_gui.h"

#ifdef _USE_G2D_
extern void g2d_elem_init(void);
extern int g2d_pre_plot(void);

/*
extern int femGfxPickFunc(double x, double y, long mode, long x0, long y0, long x1, long y1);
extern int femGfxPickBox (double x, double y, double w, double h, long x0, long y0, long x1, long y1, long button);
extern int femGfxCanBox(void);
extern int gfxSavePpm(char *filename, int width, int height);
extern int gfxSaveTiff(char *filename, int width, int height);
*/

GtkWidget *garea  = NULL ;
GdkGC     *gc     = NULL ;
GdkPixmap *pixmap = NULL ; /* off-screen pixmap */
extern int use_box ;

void gui_draw_rect(GtkWidget *widget, int x2, int y2, int remove_only, long button)
{
  int x,y, w,h ;
  static int ox,oy, ow,oh ;
  static int x_i,y_i;
  int x1,y1;

#if 0
  printf("SelMode is %li\n", plotProp.SelRectMode);
#endif

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
    gc = gdk_gc_new(garea->window);
    gdk_gc_set_foreground(gc, &(garea->style->white));
    gdk_gc_set_function(gc, GDK_XOR);
  }

  if (plotProp.SelRectPrev == AF_YES)
  {
    gdk_draw_rectangle(garea->window, gc, FALSE, ox, oy, ow, oh);
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
#if 0
      femGfxPickBox (ox, oy, ow, oh,
        0,0,
        widget->allocation.width, 
        widget->allocation.height,
        button 
      ) ;
#endif

    plotProp.SelRectPrev = AF_NO ;
    return ;
  }

  gdk_draw_rectangle(garea->window, gc, FALSE, x, y, w, h);
}

void gui_draw_cross(GtkWidget *widget,int x, int y)
{
  static int old_x = -1 ;
  static int old_y = -1 ;

  if (gc == NULL)
  {
    gc = gdk_gc_new(garea->window);
    gdk_gc_set_foreground(gc, &(garea->style->white));
    gdk_gc_set_function(gc, GDK_XOR);
  }

  gdk_draw_line ( garea->window, gc, 0, old_y, widget->allocation.width, old_y);
  gdk_draw_line ( garea->window, gc, old_x, 0, old_x, widget->allocation.height);

  gdk_draw_line ( garea->window, gc, 0, y, widget->allocation.width, y);
  gdk_draw_line ( garea->window, gc, x, 0, x, widget->allocation.height);

  old_x = x ;
  old_y = y ;
}

/** Tests if and what mouse button was reseased */
gint garea_button_release (GtkWidget* widget, GdkEventButton* event) 
{
  int x ;
  int y ;

  x = event->x;
  y = event->y;

  if (event->button == 1) 
	{
    /* mouse button 1 was released */
    /*g_print ("Button 1 release (%d, %d)\n", x, y);*/
    if (plotProp.SelStuff != GFX_SELE_NONE)
    {
#if 0
			if (use_box == AF_YES)
			{
      	gui_draw_rect(widget, x, y, AF_YES, 1);
			}
			else
			{
		    femGfxPickFunc(x, y, 
        	0,0,
        	widget->allocation.width, 
        	widget->allocation.height,
        	event->button 
        ) ;
			}
#endif
    }

#if 0
		use_box = AF_NO ;
#endif
    return(AF_OK);
  }

  if (event->button == 2) 
	{
    /* mouse button 2 was released */
#if 0
		use_box = AF_NO ;
#endif
    return (AF_OK);
  }

  if (event->button == 3) 
	{
    /* mouse button 2 was released */
#if 0
    if (plotProp.SelStuff != GFX_SELE_NONE)
    {
			if (use_box == AF_YES)
			{
      	gui_draw_rect(widget, x, y, AF_YES, 3);
			}
			else
			{
		    femGfxPickFunc(x, y, 
        	0,0,
        	widget->allocation.width, 
        	widget->allocation.height,
        	event->button 
        ) ;
			}

    }
		use_box = AF_NO ;
    return (AF_OK);
#endif
  }

  return(AF_ERR);
}

/** Pressing of button */
gint garea_button_press (GtkWidget* widget, GdkEventButton* event) 
{
  int x = event->x;
  int y = event->y;

  if (event->button == 2)
	{
#if 0
		use_box = AF_NO ;

  	femGfxPickFunc(x, y, 
        0,0,
        widget->allocation.width, 
        widget->allocation.height,
        event->button 
        ) ;
#endif

    return (AF_OK);
	}
  /* pressing of buttons 1,2,3 cause action */
  if ((event->button >= 1) && (event->button <= 3))
	{
#if 0
		use_box = AF_NO ;
#endif
    return (AF_OK);
  }

  /* other buttons are ignored */
  return (AF_ERR);
}

/** oops, mouse is moving! */
gint garea_motion_notify (GtkWidget* widget, GdkEventMotion* event) 
{
  int x;
  int y;
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

  if (state & GDK_BUTTON1_MASK) 
	{
    /* mouse button 1 is engaged */
    if (plotProp.SelStuff != GFX_SELE_NONE)
    {
#if 0
			if (femGfxCanBox() == AF_YES)
			{
				use_box = AF_YES ;
      	gui_draw_rect(widget, x, y, AF_NO, 1);
			}
#endif
    }
  }

#if 0
  if (state & GDK_BUTTON2_MASK) 
	{
    /* mouse button 2 is engaged */
		use_box = AF_NO ;
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
  }
#endif

  return (AF_OK);
}

gint draw_stuff(GtkWidget* widget, GdkEventExpose* event)
{
  if (pixmap) { gdk_pixmap_unref(pixmap); }
  pixmap = gdk_pixmap_new(widget->window,
                          widget->allocation.width,
                          widget->allocation.height,
                          -1);   
  
  gdk_draw_rectangle (pixmap,
                      widget->style->white_gc,
                      TRUE,
                      0, 0,
                      widget->allocation.width,
                      widget->allocation.height);

	g2d_pre_plot();
#if 0
    /* preparing of plot: */
		femPrePlot(0,0,widget->allocation.width, widget->allocation.height, AF_NO) ;

    /* plotting: */
		femPlotStuff(widget->allocation.width, widget->allocation.height, AF_NO) ;

    /* things after plot: */
		femPostPlot(0,0,widget->allocation.width, widget->allocation.height, AF_NO);
#endif

  gdk_draw_pixmap(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  pixmap,
                  0, 0,
                  0, 0,
                  widget->allocation.width,
                  widget->allocation.height);

  return (AF_OK);
}

/** drawing */
gint garea_draw (GtkWidget* widget, GdkEventExpose* event) 
{
	int rv = AF_OK ;
  /*int one_more = AF_NO ;*/

  if (event != NULL)
  {
    /* draw only on the last expose event */
    if (event->count > 0) 
	  {
      return(AF_OK);
    }
  }

  rv = draw_stuff(widget, event) ;

	return(rv);
}

/** reaction to change of window's size */
int garea_reshape (GtkWidget* widget, GdkEventConfigure* event) 
{
	int rv = AF_OK ;
#if 0

   /* preparing of plot: */
	femPrePlot(0,0,widget->allocation.width, widget->allocation.height, AF_NO) ;

   /* plotting: */
	femPlotStuff(widget->allocation.width, widget->allocation.height, AF_NO) ;

   /* things after plot: */
	femPostPlot(0,0,widget->allocation.width, widget->allocation.height, AF_NO);
 
#endif
  return(rv);
}


/** realize event - init of the drawing board (it may be usefull any day)  */
gint garea_init (GtkWidget* widget) 
{
  /* initialization code (not needed for now) */
  return(AF_OK);
}

/** destruction of garea - a place for cleanup code */
gint garea_destroy (GtkWidget* widget) 
{
  /* Insert any required cleanup code here. */
  return(AF_OK);
}


/** creation of the GL Area */
GtkWidget* gArea_new(void) 
{
	/* creating of garea */
  if ((garea = gtk_drawing_area_new()) == NULL) 
	{
    fprintf(msgout,"[E] %s!\n",_("Can NOT create graphic canvas\n"));
    return NULL;
  }

	/* some boring events */
  gtk_widget_set_events(GTK_WIDGET(garea),
                        GDK_EXPOSURE_MASK|
                        GDK_BUTTON_PRESS_MASK|
                        GDK_BUTTON_RELEASE_MASK|
                        GDK_POINTER_MOTION_MASK|
                        GDK_POINTER_MOTION_HINT_MASK);

  /* button_release_event - mouse button released */
  gtk_signal_connect (GTK_OBJECT(garea), "button_release_event",
                      GTK_SIGNAL_FUNC(garea_button_release), NULL);

  /* button_press_event - mouse button pressed */
  gtk_signal_connect (GTK_OBJECT(garea), "button_press_event",
                      GTK_SIGNAL_FUNC(garea_button_press), NULL);
  
  /* motion_notify_event - mouse is moving */
  gtk_signal_connect (GTK_OBJECT(garea), "motion_notify_event",
                      GTK_SIGNAL_FUNC(garea_motion_notify), NULL);
  
  /* expose_event - window was exposed - redraw probably needed  */
  gtk_signal_connect (GTK_OBJECT(garea), "expose_event",
                      GTK_SIGNAL_FUNC(garea_draw), NULL);
  
  /* configure_event - window resized  */
  gtk_signal_connect (GTK_OBJECT(garea), "configure_event",
                      GTK_SIGNAL_FUNC(garea_reshape), NULL);
  
  /* realize - window has been created */
  gtk_signal_connect (GTK_OBJECT(garea), "realize",
                      GTK_SIGNAL_FUNC(garea_init), NULL);
  
  /* destroy - window has received destroy event - cleanum needed */
  gtk_signal_connect (GTK_OBJECT(garea), "destroy",
                      GTK_SIGNAL_FUNC (garea_destroy), NULL);

	/* default size of area */
  gtk_widget_set_usize(GTK_WIDGET(garea), 200, 100);

	/* garea-specific element types: */
	g2d_elem_init();

  return (garea);
}

/** plots data - this is called from cmd_plot.c etc. */
int femPlot(void)
{
  garea_draw(garea, NULL) ; /* replace with something better*/
  return(AF_OK);
}
#endif /* _USE_G2D_ */

/* end of gui_canv.c */
