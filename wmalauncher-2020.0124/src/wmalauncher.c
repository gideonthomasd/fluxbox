/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>

#include "nls.h"
#include "settings.h"
#include "command.h"
#include "wmutil.h"
#include "wmalhelp.h"
#include "cmdparser.h"

          /*** 
           *    Defines    *
                         ***/

/* Identifiers/indexes of pixmaps for the applications.
 */
#define PXM_MOUT       0
#define PXM_MHOVER     1
#define PXM_MHOVER_LBP 2
#define PXM_MHOVER_RBP 3

/* Number of pixmaps for the applications.
 */
#define PIXMAPS_COUNT PXM_MHOVER_RBP+1

          /*** 
           *    type(defs) and structs      *
                                          ***/
 
/* Keep track, of mouse position relatively to the 
 * application, and, state of the left button.
 */
typedef struct {
  bool is_inside;
  bool lb_pressed;
  bool rb_pressed;
  Time lb_released_time;
} t_mouse_status;

/* Enumeration of timer types which can be used
 * in the main loop.
 */
typedef enum { 
  TM_WAIT_EVENTS, 
  TM_SHOW_TOOLTIP, 
  TM_HIDE_TOOLTIP 
} t_event_timers;

          /*** 
           *    Global variables      *
                                    ***/

/* Array used to store the pixmaps of the application.
 *   
 * dockapp_pixmaps[PXM_MOUT]
 *   Pixmap displayed when the mouse is outside the dockapp.
 * 
 * dockapp_pixmaps[PXM_MHOVER]
 *   pixmap displayed when the mouse is inside the dockapp and
 *   the left mouse button is released.
 * 
 * dockapp_pixmaps[PXM_MHOVER_LBP]
 *   pixmap displayed when the mouse is inside the dockapp, the
 *   left mouse button is pressed and wmalauncher is configured 
 *   to run the command with a single mouse button click.
 */
Pixmap dockapp_pixmaps[PIXMAPS_COUNT];

          /*** 
           *    Functions      *
                             ***/

/* Copies, into the icon window and to the size of this window, the 
 * pixmap dockapp_pixmaps[i] according to the mouse status, where 'i'
 * can be :
 * 
 * PXM_MOUT: 
 *   The mouse is outside the dockapp.
 * 
 * PXM_MHOVER: 
 *   The mouse is inside the dockapp and the left mouse button
 *   is released.
 * 
 * PXMP_HMOVER_LBP:
 *   The mouse is inside the dockapp, the left mouse button is
 *   pressed, and wmalauncher is configured to run the command
 *   with a single mouse button click.
 */
void redraw(t_mouse_status *mouse_status) {
  Display           *display     = get_display();
  Window            iconwin      = get_icon_window();
  GC                gc           = get_gc();
  XWindowAttributes iw_attrs;
  Pixmap            pixmap       = dockapp_pixmaps[PXM_MOUT];
  
  
  XGetWindowAttributes(display,iconwin,&iw_attrs);
  
  if (mouse_status->is_inside) {
    pixmap=dockapp_pixmaps[PXM_MHOVER];
    
    if ((mouse_status->lb_pressed) && (get_config()->single_click)) {
      pixmap=dockapp_pixmaps[PXM_MHOVER_LBP];
    }
      else
    if (mouse_status->rb_pressed) {
      pixmap=dockapp_pixmaps[PXM_MHOVER_RBP];
    }
  }

  XCopyArea(display,
    pixmap,
    iconwin,
    gc,
    0,0,
    iw_attrs.width,iw_attrs.height,
    0,0);
}

/* Init the pixmaps array dockapp_pixmaps.
 */
void init_pixmaps() {

  for (int i=0;i<PIXMAPS_COUNT;i++) {
    dockapp_pixmaps[i]=0;
  }
}

/* Creates the pixmaps for the dockable application according
 * to the current configuration and store them in the array
 * dockapp_pixmaps[].
 */
void create_pixmaps() {
  Window               iconwin   = get_icon_window();
  int                  depth     = get_depth();
  t_config            *config   = get_config();

  t_color_ex          *bg_color       = new_color_ex(config->background_color);
  t_color_ex          *bg_color_hvr   = new_color_ex(config->hover_background_color);
  t_color_ex          *brdr_color     = new_color_ex(config->border_color);
  t_color_ex          *brdr_color_hvr = new_color_ex(config->hover_border_color);

  int                  bsize = config->border_size;
    
  XWindowAttributes    iw_attrs;
  int                  pxid, width, height, isize, icon_x, icon_y;
  XRectangle           irect;
  Imlib_Image         image;
  Imlib_Image         icon;
  Imlib_Load_Error    imlib_error;
  
  char *icon_pathname = NULL;

  XGetWindowAttributes(get_display(),iconwin,&iw_attrs);
  width=iw_attrs.width;
  height=iw_attrs.height;
  
  /* Compute the rectangle in which the icon must be drawn
   */
  irect.width  = width-(2*bsize);
  irect.height = height-(2*bsize);
  irect.x      = (width-irect.width)/2;
  irect.y      = (height-irect.height)/2;
  
  if (is_absolute_path(config->icon_name) ) {
    icon_pathname=strdup(config->icon_name);
  } 
    else 
  if (ends_with_extension(config->icon_name,WMAL_PREFERRED_ICON_TYPES)) {
    icon_pathname = search_file(config->icon_search_path,config->icon_name);
  } else {
    icon_pathname = search_file_ext(config->icon_search_path,
                                    config->icon_name,
				    WMAL_PREFERRED_ICON_TYPES);
  }

  #ifdef DEBUG
    fprintf(stdout,"Loading icon %s\n",icon_pathname);
  #endif

  icon = load_image(icon_pathname,&imlib_error);
  
  if (!icon) {
    fprintf(stderr,_NLS_("Failed to load icon %s. Imlib Error code %d.\n"),
            (icon_pathname) ? icon_pathname : config->icon_name,
	    imlib_error);
    
    icon=create_fail_icon(iw_attrs.width,iw_attrs.height,"lucidasans");
  }
    
  free(icon_pathname);

  
  for (pxid=0;pxid<PIXMAPS_COUNT;pxid++) {
    dockapp_pixmaps[pxid]=XCreatePixmap(get_display(),iconwin,width,height,depth);

    imlib_context_set_drawable(dockapp_pixmaps[pxid]);
    
    image  = imlib_create_image(width,height);
    isize  = (pxid==PXM_MHOVER_LBP || pxid==PXM_MHOVER_RBP) ? config->icon_size-4 : config->icon_size;
    icon_x = (iw_attrs.width-isize)/2;
    icon_y = (iw_attrs.height-isize)/2;
    
    imlib_context_set_image(image);
    imlib_image_clear();
    
    if (bsize > 0) {
      /* Fill the dockapp window with the (right) border 
       * color...
       */
      if ( (pxid==PXM_MHOVER) || (pxid==PXM_MHOVER_LBP) || (pxid==PXM_MHOVER_RBP) ) {
	imlib_fill_rectangle(0,0,width,height,brdr_color_hvr);
      } else {
	imlib_fill_rectangle(0,0,width,height,brdr_color);
      }
    }
    
    /* Fill the rectangle in which the icon must be display
     * with the right background color ...
     */
    if ( (pxid==PXM_MHOVER) || (pxid==PXM_MHOVER_LBP) || (pxid==PXM_MHOVER_RBP) ) {
      imlib_fill_rectangle(irect.x,irect.y,irect.width,irect.height,bg_color_hvr);
    } else {
      imlib_fill_rectangle(irect.x,irect.y,irect.width,irect.height,bg_color);
    }
    
    if (config->frame) {
      t_frame *frame  = config->frame;
      
      t_color hicolor, locolor;
      
      t_color fr_hicolor     = get_color(frame->hicolor);
      t_color fr_hvr_hicolor = get_color(frame->hover_hicolor);

      t_color fr_locolor     = get_color(frame->locolor);
      t_color fr_hvr_locolor = get_color(frame->hover_locolor);
      
      int fr_thick    = frame->thickness;
            
      int fr_x1, fr_y1, fr_x2, fr_y2;
      
      fr_x1=bsize;
      fr_y1=bsize;
      fr_x2=(width-bsize)-1;
      fr_y2=(height-bsize)-1;
      
      switch(pxid) {
	case PXM_MOUT:
	  hicolor=fr_hicolor;
	  locolor=fr_locolor;
	break;
	
	case PXM_MHOVER:
	  hicolor=fr_hvr_hicolor;
	  locolor=fr_hvr_locolor;
	break;
	
	case PXM_MHOVER_LBP:
	  hicolor=fr_hvr_locolor;
	  locolor=fr_hvr_hicolor;
	break;

        case PXM_MHOVER_RBP:
	  hicolor=fr_hvr_hicolor;
	  locolor=fr_hvr_locolor;
        break;

      }
      
      imlib_context_set_color(hicolor._red,
                              hicolor._green,
			      hicolor._blue,
			      255);

      for (int i=0;i<fr_thick;i++) {
        imlib_image_draw_line(fr_x1,fr_y1+i,fr_x2-i,fr_y1+i,0); /* top  */
        imlib_image_draw_line(fr_x1+i,fr_y1,fr_x1+i,fr_y2-i,0); /* left */
      }
      
      imlib_context_set_color(locolor._red,
                              locolor._green,
			      locolor._blue,
			      255);
			      
      for (int i=0;i<fr_thick;i++) {
        imlib_image_draw_line(fr_x1+i,fr_y2-i,fr_x2,fr_y2-i,0); /* bottom */
        imlib_image_draw_line(fr_x2-i,fr_y2,fr_x2-i,fr_y1+i,0); /* right */
      }
    }
           
    /* Render the current image onto the current 
     * pixmap (ie. dockapp_pixmaps[pxid]), then 
     * free the image...
     */
    imlib_render_image_on_drawable(0,0);
    imlib_free_image();
        
    /* Create the icon to render onto the current 
     * pixmap (ie. dockapp_pixmaps[pxid])
     */


    if ( (pxid==PXM_MHOVER) || (pxid==PXM_MHOVER_LBP)  ) {

      image=compose_image(icon,
                          config->hover_icon_grayscale,
                          config->hover_icon_brightness,
                          config->hover_icon_contrast,
                          config->hover_icon_gamma);
    } 
      else 
    if (pxid==PXM_MHOVER_RBP) {
      image=compose_image(icon,true,0.0,1.0,1.0);
    } else { 
      image=compose_image(icon,
                          config->icon_grayscale,
                          config->icon_brightness,
                          config->icon_contrast,
                          config->icon_gamma);
    }


    /* render the generated icon onto the current pixmap
     */
    imlib_context_set_image(image);
    imlib_render_image_on_drawable_at_size(icon_x,icon_y,isize,isize);
    imlib_free_image();

    if (pxid==PXM_MHOVER_RBP) {
      int padding=4;
      int thickness=4;
      int x0,y0,x1,y1;

      x0=padding;
      y0=padding;
      x1=width-padding;
      y1=height-padding;

      image  = imlib_create_image(width,height);
      imlib_context_set_image(image);
      imlib_image_clear();
      imlib_image_set_has_alpha(1);
      imlib_context_set_color(255,0,0,255);

      for (int i=0;i<thickness;i++) {
        imlib_image_draw_line(x0+i,y0,x1,y1-i,0);
        imlib_image_draw_line(x0,y0+(i+1),x1-(i+1),y1,0);

        imlib_image_draw_line(x0+i,y1,x1,y0+i,0);
        imlib_image_draw_line(x0,y1-(i+1),x1-(i+1),y0,0);
      }

      imlib_render_image_on_drawable(0,0);
      imlib_free_image();
    }
  }
  
  imlib_context_set_image(icon);
  imlib_free_image(); 
  
  free_color_ex(bg_color);
  free_color_ex(bg_color_hvr);
  free_color_ex(brdr_color);
  free_color_ex(brdr_color_hvr);
}


/* Handle the incoming event 'event' and update mouse_status 
 * accordingly.
 * 
 * Returns true if the program must continue, false otherwise, 
 * in which case exitcode contains the exit code to return.
 */
static bool handle_event(XEvent           event, 
                         t_mouse_status *mouse_status,
			 int             *exit_code) {
  Time   elapsed_time = 0;
  bool   run_cmd      = false;
  pid_t  cmd_pid     = 0;
  Window mainwin      = get_main_window();
  Window iconwin      = get_icon_window();
  Atom   wm_del_msg;

  switch (event.type) {
    
    case ClientMessage :
      if ( event.xclient.window == mainwin ) {
        wm_del_msg=XInternAtom(get_display(), "WM_DELETE_WINDOW", False);
        if ((Atom)event.xclient.data.l[0] == wm_del_msg ) {
	  /* The application must exit right now. */
	  *exit_code=0;
	  return false;
        }
      }
    break;
    
    case Expose:
      if ( event.xexpose.window == iconwin ) {
        redraw(mouse_status);
      }
    break;
    
    case EnterNotify:
      if ( event.xbutton.window == iconwin ) {
        mouse_status->is_inside=true;
        redraw(mouse_status);
      }
      
    break;
    
    case LeaveNotify:
      if ( event.xbutton.window == iconwin ) {
        mouse_status->is_inside=false;
        redraw(mouse_status);
      }
    break;
    
    case MotionNotify: // nothing to do.
    break;
    
    case ButtonPress:
      if (event.xbutton.window == iconwin) {
        if (event.xbutton.button == Button1 ) {
	  mouse_status->lb_pressed=true;
	  
	  if (get_config()->single_click) {
	    redraw(mouse_status);
	  }
	}
	  else
	if ( (get_config()->exit_on_rclick) && (event.xbutton.button == Button3) ) {
	  mouse_status->rb_pressed=true;
	  redraw(mouse_status);
	}  
      }
    break;
    
    case ButtonRelease:
      if ( event.xbutton.window == iconwin ) {
        if (event.xbutton.button == Button1)  {
	  elapsed_time = event.xbutton.time - mouse_status->lb_released_time;
	  mouse_status->lb_pressed=false;

	  if (mouse_status->is_inside) {
	    if (get_config()->single_click) {
	      run_cmd=true;
	      redraw(mouse_status);
	    }
	      else
	    if (elapsed_time <= get_config()->double_click_delay) {
	      run_cmd = true;
	    } else {
	      mouse_status->lb_released_time = event.xbutton.time;
	    }

	    if (run_cmd) {
	      if ( !exec_command(get_config()->command,&cmd_pid) ) {
		
	        /* The execution of attached command failed.
                 * 
                 * Attention, this code block is run by the child 
                 * process started by exec_command(). This child
                 * must exit, right after the user has been notified
                 * that command execution failed, with a call to
                 * function _exit(), instead of exit(), to prevent 
                 * the call of functions registered with atexit() and/or
                 * on_exit().
                 */
                t_command *xmsgcmd        = new_command();
                char       errno_str[1024] = {0};
                char       error_msg[2048] = {0};
                char      *strcmd          = command_to_str(get_config()->command);
		
		strerror_r(errno,errno_str,2048);

	        snprintf(error_msg,
                         2048,
                         _NLS_("Failed to run %s. Error %d: %s"),
                         strcmd,errno,errno_str);

		free(strcmd);
		
		add_argument(xmsgcmd,"xmessage");
		add_argument(xmsgcmd,"-buttons");
		add_argument(xmsgcmd,"OK");
		add_argument(xmsgcmd,"-nearmouse");
		add_argument(xmsgcmd,"-timeout");
		add_argument(xmsgcmd,"60");
		add_argument(xmsgcmd,error_msg);

		fprintf(stderr,"%s\n",error_msg);
		
		exec_command(xmsgcmd,&cmd_pid);
		
		_exit(1);
	      }
	    }
	  }
	}
	  else
	if ( (get_config()->exit_on_rclick) && (event.xbutton.button == Button3) )  {
	  mouse_status->rb_pressed=false;
	  
	  if (mouse_status->is_inside) {
	    redraw(mouse_status);
	    *exit_code=0;
	    return false;
	  }
	}
      }
    break; // ButtonRelease
  }
  
  return true;
}

/* Cleanup all resources in used before terminating.
 */
void cleanup() {
  Display *display = get_display();
  
  if (display) {    
    for (int i=0;i<PIXMAPS_COUNT;i++) {
      if (dockapp_pixmaps[i]) {
        XFreePixmap(display,dockapp_pixmaps[i]);
      }
    }
    close_connection(display);
  }
  delete_config();  
}

/* Executed in response of signal SIGTERM. 
 */
void terminate_handler(int signal) {
  Display *display = get_display();
  Window  mainwin  = get_main_window();
  XEvent  event;

  if ((signal==SIGTERM) || (signal==SIGINT)) {

    memset(&event,0,sizeof(event));
    
    event.xclient.type         = ClientMessage;
    event.xclient.window       = mainwin;
    event.xclient.message_type = XInternAtom(display,"WM_PROTOCOLS",true);
    event.xclient.format       = 32;
    event.xclient.data.l[0]    = XInternAtom(display,"WM_DELETE_WINDOW",false);
    event.xclient.data.l[1]    = CurrentTime;
    
    XSendEvent(display,mainwin,False,NoEventMask,&event);
  }
}

/* wmalauncher entry point.
 */
int main(int argc, char *argv[])
{
  XEvent           event;
  fd_set          read_fds;
  
  struct          timeval wait_timer;
  struct          timeval show_ttip_timer;
  struct          timeval hide_ttip_timer;
  struct          timeval timer;
  
  t_event_timers timer_id;
  Display         *display;
  int              x11fd;
  int              tt_view_count = 0;
  int              exitcode=0;
  bool              handle_events=true;
  t_config        *config=NULL;
  
  t_mouse_status mouse_status={
    .is_inside        = false,
    .lb_pressed       = false,
    .lb_released_time = 0
  };

  fprintf(stdout,"wmalauncher ver. %s - %s -\n\n",WMAL_VERSION,WMAL_COPYRIGHT);

  setup_i18n_env("wmalauncher",WMAL_LOCALE_PATH,2,LC_MESSAGES,LC_CTYPE);
  
  atexit(cleanup);
  signal(SIGTERM,terminate_handler);
  signal(SIGINT,terminate_handler);
  
  init_pixmaps();
  
  config = create_config();
    
  if ( !parse_command_line(argc,argv,&exitcode) ) {
    return (exitcode);
  }

  #ifdef DEBUG
    print_config();
  #endif
  
  if ( !open_connection(config->display,NULL) ) {
    fprintf(stderr,
      _NLS_("Failed to open display '%s'\n"),
      XDisplayName(config->display));
    return (1);
  }


  if ( !open_dockapp_windows(config->winsize,argc,argv,config->broken_wm,true) ) {
    return (1);
  }
  
  create_pixmaps();

  if (config->tooltip_text) {
    create_tooltip_window(
      config->tooltip_text,
      config->tooltip_bg_color,
      config->tooltip_fg_color,
      config->tooltip_font,
      config->tooltip_border_size,
      config->tooltip_text_padding,
      WMAL_FALLBACK_FONTS);
  }
  
  display = get_display();
  x11fd   = get_connection();
  
  /* Initializes the timers used to wait for events, to trigger the 
   * tooltip, and to hide the tooltip. 
   * 
   * Attention :
   *   1. the fields tv_sec & tv_usec must be initialized. Furthermore, 
   *      tv_usec must be within range [0..1000000[, otherwise, select()
   *      fails with an error 35 (EINVAL) under {Free}BSD, but not under
   *      linux).
   * 
   *   2. On linux, select() modify the timer passed in arguemnt to reflect
   *      the amount of time not slept. Therefore, the timers initialized 
   *      below *are not* directly passed to select().
   */
  wait_timer.tv_sec  = (WAIT_EVENT_TIMEOUT * 1000) / 1000000;
  wait_timer.tv_usec = (WAIT_EVENT_TIMEOUT * 1000) % 1000000;
  
  show_ttip_timer.tv_sec  = (config->tooltip_show_delay * 1000) / 1000000;
  show_ttip_timer.tv_usec = (config->tooltip_show_delay * 1000) % 1000000;
  
  hide_ttip_timer.tv_sec  = (config->tooltip_hide_delay * 1000) / 1000000;
  hide_ttip_timer.tv_usec = (config->tooltip_hide_delay * 1000) % 1000000;
    
  while (handle_events) {
    
    if (!XPending(display)) {
      /*
       * Waits events for at most X ms using a timer. If 
       * events occurs before the timer ends, they are
       * handled immediately. 
       * 
       * If the tooltip is enabled but not visible and has
       * not been displayed since the mouse is entered into
       * the dockable application , the tooltip is displayed
       * when no event occurs until the timer ends.
       * 
       * if the tooltip is enabled and visible, the tooltip 
       * is hidden, (1) when the timer ends, or, (2) if an event
       * occur before the end of the timer.
       */
      FD_ZERO(&read_fds);
      FD_SET(x11fd,&read_fds);
      
      /* By default, setup the timer to wait for events...
       */
      timer.tv_sec  = wait_timer.tv_sec;
      timer.tv_usec = wait_timer.tv_usec;
      timer_id      = TM_WAIT_EVENTS;
      
      if ( is_tooltip_enabled() ) {
        if ((tt_view_count>0) && (!mouse_status.is_inside)) {
	  /* the mouse left the dockapp, and the tooltip
	   * has been displayed once. reset tt_view_count
	   * so that the tooltip can be displayed again
	   * the next time the mouse enters in the dockapp.
	   */
	  tt_view_count=0;
	}
	
	if (is_tooltip_visible()) {
	  /* setup the timer to "hide the tooltip". Note that the
	   * tooltip is also hidden if event(s) occur before the
	   * timer ends.
	   */
	  timer.tv_sec  = hide_ttip_timer.tv_sec;
	  timer.tv_usec = hide_ttip_timer.tv_usec;
	  timer_id      = TM_HIDE_TOOLTIP;
	}
	  else 
	if ( (tt_view_count==0) && 
	     (mouse_status.is_inside) && 
	     (!mouse_status.lb_pressed) &&
             (!mouse_status.rb_pressed) ) {
	  /* setup the timer to "show the tooltip".
	   */
	  timer.tv_sec  = show_ttip_timer.tv_sec;
	  timer.tv_usec = show_ttip_timer.tv_usec;
	  timer_id      = TM_SHOW_TOOLTIP;
	}
      }
      
      if ( !select(x11fd+1,&read_fds,0,0,&timer) ) {
        /* timer fired... */
	
	switch(timer_id) {
	  case TM_SHOW_TOOLTIP:
	    show_tooltip(-1,-1);
	    tt_view_count = 1;
	  break;
	  
	  case TM_HIDE_TOOLTIP:
	    hide_tooltip();
	  break;
	  
	  case TM_WAIT_EVENTS:
	    /* No event occured during the last wait for event(s), so, 
	     * there's nothing else to do than ...to wait again.
	     */
	  break;
	}
      
	continue;
      }
    }
    
    while (XPending(display)) {
      /* invariant: one (or more) event occured... */

      /* hide the tooltip before handling incoming event(s), 
       * when required.
       */
      if (is_tooltip_visible()) {
        hide_tooltip();
      }

      XNextEvent(display,&event);
      if ( !handle_event(event,&mouse_status,&exitcode) ) {
	handle_events=false;
      }
    }
  }
  
  return (exitcode);
}
