/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <wordexp.h>
#include <librsvg/rsvg.h>
#include <X11/extensions/shape.h>

#include "nls.h"
#include "wmutil.h"


          /*** 
           *    Typedefs and Structs      *
                                        ***/

/* Type to store tooltip data.
 */
typedef struct {
  bool        enabled;
  char       *text;
  t_color    fg_color;
  t_color_ex *bg_color;
  Imlib_Font  font;
  int         border;
  int         text_padding;
  bool        visible;
  int         xpos;
  int         ypos;
  int         width;
  int         height;
  Pixmap      pixmap;
} t_tooltip_data;


          /*** 
           *    Variables      *
                             ***/

static Display *display = NULL;

static GC default_gc = 0;

static Window root_window = 0;
static Window main_window = 0;
static Window icon_window = 0;
static Window tooltip_window = 0;

static t_tooltip_data tooltip_data;
 
          /*** 
           *    Functions      *
                             ***/

/* Adds the given font paths to the imlib font path.
 * 
 * font_paths must be a colon separated list of paths.
 */
static void set_imlib_font_path(char *font_paths) {
  if (!font_paths) {
    return;
  }
  
    /* Duplicates font_paths  to prevent strtok_r() to modify it. */
  char       *wfont_paths = strdup(font_paths);
  wordexp_t   exp_path;
  char       *cpath;
  char       *stok_saveptr=NULL;
  
  cpath = strtok_r(wfont_paths,":",&stok_saveptr);
  
  while(cpath) {
    wordexp(cpath,&exp_path,0);
    imlib_add_path_to_font_path(exp_path.we_wordv[0]);
    wordfree(&exp_path);
    
    cpath=strtok_r(NULL,":",&stok_saveptr);
  }
  
  free(wfont_paths);
}

/* Open a connection to the X server and setup the imlib2
 * library (when the connection to X succeed). 
 * 
 * This function must be called first. Returns true on 
 * success, false otherwise. 
 * 
 * display_name
 *   The name of the display to use, or NULL to default
 *   to the value of DISPLAY environment variable.
 * 
 * font_search_paths
 *   The (initial) font search paths as a colon separated list 
 *   of paths, or NULL if no initial font search path must be
 *   set.
 */
bool open_connection(char *display_name, char *font_search_paths) {

  unsigned long gc_value_mask;
  XGCValues      gc_values;
  bool           status=false;
  
  if (display) {
    return true;
  }

  if ( (display = XOpenDisplay(display_name)) ) {
    status = true;

    root_window = RootWindow(display, get_screen());
  
    gc_value_mask = GCForeground | GCBackground | GCGraphicsExposures;

    gc_values.foreground         = get_color("Black").pixel;
    gc_values.background         = get_color("White").pixel;
    gc_values.graphics_exposures = 0;
  
    default_gc = XCreateGC(display,root_window,gc_value_mask,&gc_values);
        
    imlib_context_set_dither(1);
    imlib_context_set_anti_alias(1);
    imlib_context_set_display(display);
    imlib_context_set_visual(DefaultVisual(display,get_screen()));
  }
  
     /* setup tooltip_data to its defaults. */
  tooltip_data.enabled      = false;
  tooltip_data.text         = NULL;
  tooltip_data.fg_color     = get_color("Black");
  tooltip_data.bg_color     = new_color_ex("White");
  tooltip_data.font         = NULL;
  tooltip_data.border       = 0;
  tooltip_data.text_padding = 0;
  tooltip_data.visible      = false;
  tooltip_data.xpos         = -1;
  tooltip_data.ypos         = -1;
  tooltip_data.width        = 0;
  tooltip_data.height       = 0;
  tooltip_data.pixmap       = 0;  
  
  if (font_search_paths) {
    set_imlib_font_path(font_search_paths); 
  }
  
  return status;
}

/* Clears the imlib font path when required.
 */
static void clear_imlib_font_path() {
  int  count;
  char **path=imlib_list_font_path(&count);
  char *cpath;
  int  clen;

  /* As stated in the documentation, the list returned by
   * imlib_list_font_path must not be freed (or changed).
   * 
   * Therefore, to clear the font path without any trouble
   * it is required to proceed as below: 
   *   1. copy the first path to cpath
   *   2. call imlib_remove_path_from_font_path(cpath)
   *   3. reload the path list.
   */
  while (count>0) {
    clen=strlen(path[0]);
    cpath=calloc(clen+1,sizeof(char));
    strncpy(cpath,path[0],clen);

    imlib_remove_path_from_font_path(cpath);
    free(cpath);

    path=imlib_list_font_path(&count);
  }  
}

/* Close the connection to the X server, and cleanup 
 * resources used by imlib2 library.
 * 
 * This is the counterpart of open_connection(), and this
 * should be the last function called when the program 
 * is about to terminate.
 */
void close_connection() {
  
  clear_imlib_font_path();  
  imlib_flush_font_cache();

  if (display) {
      
    if (default_gc) {
      XFreeGC(display,default_gc);
    }
    
    XCloseDisplay(display);

    free(tooltip_data.text);
    free_color_ex(tooltip_data.bg_color);

    imlib_flush_loaders();
    
    display=NULL;
  }
}

/* Sets the shape of the dockable application windows
 * to a simple rectangle.
 */
static void set_dockapp_windows_shape(int wsize) {
  Pixmap shape_mask;
  GC     shape_gc;
  
  /* Att: Under X11 1.6.4, the following call to XCreatePixmap()
   * leads to a memory leak which is reported by valgrind
   * with the call stack below:
   *
   *   192 (16 direct, 176 indirect) bytes in 1 blocks are definitely lost in loss record 47 of 62
   *     at 0x4C2FD8F: realloc (vg_replace_malloc.c:785)
   *     by 0x52AEDCC: ??? (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x52AF2C6: ??? (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x52B09DE: ??? (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x52B11A7: _XlcCreateLC (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x52CCE5F: _XlcDefaultLoader (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x52B83C5: _XOpenLC (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x52B85DA: _XrmInitParseInfo (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x52A145F: ??? (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x52A498D: XrmGetStringDatabase (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x5282850: ??? (in /usr/lib64/libX11.so.6.3.0)
   *     by 0x5282AED: XGetDefault (in /usr/lib64/libX11.so.6.3.0)
   * 
   * This issue is not encountered under  X11 1.6.5.
   */
  shape_mask = XCreatePixmap(display,main_window,wsize,wsize,1);
  
  if (shape_mask) {
    shape_gc = XCreateGC(display,shape_mask,0,NULL);

    XSetForeground(display,shape_gc,1);
    XFillRectangle(display,shape_mask,shape_gc,0,0,wsize,wsize);

    XShapeCombineMask(display,main_window,ShapeBounding,0,0,shape_mask,ShapeSet);
    XShapeCombineMask(display,icon_window,ShapeBounding,0,0,shape_mask,ShapeSet);

    XFreeGC(display,shape_gc);
    XFreePixmap(display,shape_mask);
  }
}

/* Creates the main and icon windows for the running 
 * application, then maps the main window, and the
 * icon window when broken_wm is true.
 * 
 * Returns true on success, false otherwise.
 * 
 * wsize
 *   The size of the windows.
 * 
 * argc
 *   The number of argument passed to the application on 
 *   the command line.
 * 
 * argv 
 *   The arguments passed to the application on the command
 *   line.
 * 
 * broken_wm
 *   Must be true to enable fix for "broken" window managers, false
 *   otherwise.
 * 
 * support_del_msg
 *   Must be true to register interest in the delete window
 *   message (ie. WM_DELETE_WINDOW), false otherwise.
 */
bool open_dockapp_windows(int wsize, 
			  int argc, 
			  char *argv[], 
			  bool broken_wm,
			  bool support_del_msg) {
  int           mainwin_wsize = 1;
  int           iconwin_xypos = 0;
  
  char          *appname     = get_file_basename(argv[0]);
  unsigned int	 border_width = 1;

  XWMHints*     win_hints;
  XClassHint    class_hint;
  XTextProperty name_property;
  Pixel         fcolor, bcolor;
  
  Atom          wm_del_msg;
  
  long          event_mask= ButtonPressMask     |
                            ExposureMask        | 
			    ButtonReleaseMask   |
			    PointerMotionMask   | 
			    StructureNotifyMask |
			    EnterWindowMask     | 
			    LeaveWindowMask;
  
  fcolor = get_color("Black").pixel;
  bcolor = get_color("White").pixel;

  win_hints = XAllocWMHints();
  
  if (!win_hints) {
    fprintf(stderr,
      _NLS_("Failed to allocate XWMHints instance.\n"));
    return false;
  }
  
  if (broken_wm) {
    /* In case of "broken" window manager like kde/xfce, the
     * main window size must be set to wsize instead of 1, and,
     * the icon window must be at (-1,-1) inside the main window
     * instead of (0,0).
     */
    mainwin_wsize = wsize;
    iconwin_xypos = -1;
  }

  main_window = XCreateSimpleWindow(
    display, 
    root_window,                    /* parent         */
    0,0,                            /* x & y position */
    mainwin_wsize,mainwin_wsize,    /* width & height */
    border_width,
    fcolor,                         /* border color   */
    bcolor                          /* bkgnd color    */
  );

  icon_window = XCreateSimpleWindow(
    display, 
    main_window,                  /* parent         */
    iconwin_xypos,iconwin_xypos,  /* x & y position */
    wsize,wsize,                  /* width & height */
    border_width,                  
    fcolor,                       /* border color   */
    bcolor                        /* bkgnd color    */
  );

  class_hint.res_name  = appname;
  class_hint.res_class = appname;
  
  XSetClassHint(display, main_window, &class_hint);

  XSelectInput(display,main_window,event_mask);
  XSelectInput(display,icon_window,event_mask);

  if (XStringListToTextProperty(&appname,1,&name_property) == 0) {
    fprintf(stderr, 
      _NLS_("Failed to allocate window name '%s'\n"), appname);
    free(appname);
    return false;
  }

  XSetWMName(display,main_window,&name_property);

  win_hints->initial_state = WithdrawnState;
  win_hints->icon_window   = icon_window;
  win_hints->icon_x        = 0;
  win_hints->icon_y        = 0;
  win_hints->window_group  = main_window;
  win_hints->flags         = StateHint        | 
                             IconWindowHint   | 
			     IconPositionHint | 
			     WindowGroupHint;

  XSetWMHints(display,main_window,win_hints);
  XSetCommand(display,main_window,argv,argc);
  
  if (support_del_msg) {
    wm_del_msg=XInternAtom(display,"WM_DELETE_WINDOW",False);
    XSetWMProtocols(display,main_window,&wm_del_msg,1);
    XSetWMProtocols(display,icon_window,&wm_del_msg,1);
  }
  
  /* Sets windows shapes. This might help the application to
   * work properly under some window manager (ex. fvwm,
   * kde,...)
   */
  set_dockapp_windows_shape(wsize);
      
  /* Maps the main windows. 
   */
  XMapRaised(display,main_window);
  
  if (broken_wm) {
    /* On "broken" window managers, it is required to maps 
     * the icon window otherwise the dockapp will not be
     * visible nor accessible.
     */
    XMapRaised(display,icon_window);
  }
  
  XFree(win_hints);
  XFree(name_property.value);
  
  free(appname);
  
  return true;
}

/* Returns the connection number for the display get_display().
 */
int get_connection() {
  return ConnectionNumber(display);
}

/* Returns the current connection to the X server as a 
 * *Display.
 */			     
Display *get_display() {
  return display;
}

/* Returns the default screen.
 */
int get_screen() {
  return DefaultScreen(display);
}

/* Returns the depth of the root window for the default screen.
 */
int get_depth() {
  return DefaultDepth(display,get_screen());
}

/* Returns the default GC in use.
 */
GC get_gc() {
  return default_gc;
}

/* Parses the string 'colorstr' and returns the corresponding 
 * t_color structure.
 * 
 * colorstr can be an X11 color name or an hex-triplet to 
 * the format '#rrggbb' (ex: #ff00ff).
 * 
 * If colorstr cannot be parsed, get_color() return a
 * t_color initialized as below :
 * 
 * t_color color = { 
 *   .pixel=0,
 *   .red=0,  .green=0,  .blue=0,
 *   ._red=0, ._green=0, ._blue=0 
 * }
 */
t_color get_color(char *colorstr) {
  XColor            x_color;
  XWindowAttributes attributes;

  t_color color = { .pixel=0,
                     .red=0,  .green=0,  .blue=0,
		     ._red=0, ._green=0, ._blue=0 } ;

  if (display) {
    XGetWindowAttributes(display, root_window, &attributes);

    x_color.pixel = 0;
    if (!XParseColor(display, attributes.colormap, colorstr, &x_color)) {
      fprintf(stderr, 
        _NLS_("get_color(): Can't parse %s.\n"), colorstr);
    } 
     else
    if  (!XAllocColor(display, attributes.colormap, &x_color)) {
      fprintf(stderr, 
        _NLS_("get_color(): can't allocate %s.\n"), colorstr);
    }
    color.pixel=x_color.pixel;
  
    color.red   = x_color.red;
    color.green = x_color.green;
    color.blue  = x_color.blue;
  
    color._red   = ( color.red==0   ? 0 : ((color.red+1)/256)-1 ) ;
    color._green = ( color.green==0 ? 0 : ((color.green+1)/256)-1 );
    color._blue  = ( color.blue==0  ? 0 : ((color.blue+1)/256)-1 );
  }
  return color;
}

/* Parse the string str and returns the corresponding *t_color_ex,
 * NULL when str is NULL.
 * 
 * str can be :
 *   * a color name
 *   * an hex-triplet to the format #rrggbb
 *   * a gradient, specified using the syntax :
 *       <HGRAD|VGRAD|DGRAD>:<COLOR_1>,<COLOR_2>[,<COLOR_i> ...]
 * 
 *     COLOR_x can be a color name or an hex-triplet to the
 *     format #rrggbb
 * 
 * It is up to the user to free the memory used by the returned 
 * *t_color_ex by calling function free_color_ex()
 */
t_color_ex *new_color_ex(char *str) {
  char *HGRAD_TOKEN="HGRAD:";
  char *VGRAD_TOKEN="VGRAD:";
  char *DGRAD_TOKEN="DGRAD:";
  char *cname_list=NULL;

  if (!str) {
    return NULL;
  }
  
  t_color_ex *new_color_ex=calloc(1,sizeof(t_color_ex));
  
  new_color_ex->ctype  = PLAIN_COLOR;
  new_color_ex->ccount = 0;
  new_color_ex->cnames = NULL;

  if ( !strncmp(str,HGRAD_TOKEN,strlen(HGRAD_TOKEN)) ) {
    new_color_ex->ctype = HORZ_GRADIENT;
    cname_list          = str+strlen(HGRAD_TOKEN);
  } 
    else
  if ( !strncmp(str,VGRAD_TOKEN,strlen(VGRAD_TOKEN)) ) {
    new_color_ex->ctype = VERT_GRADIENT;
    cname_list          = str+strlen(VGRAD_TOKEN);
  } 
    else
  if ( !strncmp(str,DGRAD_TOKEN,strlen(DGRAD_TOKEN)) ) {
    new_color_ex->ctype = DIAG_GRADIENT;
    cname_list          = str+strlen(DGRAD_TOKEN);
  }
  
  if (new_color_ex->ctype==PLAIN_COLOR) {
    new_color_ex->ccount    = 1;
    new_color_ex->cnames    = calloc(1,sizeof *new_color_ex->cnames);
    new_color_ex->cnames[0] = strdup(str);
  } else {
    /* new_color_ex->ctype== xxxx_GRADIENT */
    
    int   comma   = 0;
    int   pos     = 0;
    int   cindex  = 0;
    char *cname;
    
    while (cname_list[pos]!='\0') {
      if ( cname_list[pos]==',') {
	comma++;
      }
      pos++;
    }
        
    if (comma>0) {
      new_color_ex->ccount = comma+1;
      new_color_ex->cnames = calloc(new_color_ex->ccount,sizeof *new_color_ex->cnames);

      cname = strsep(&cname_list,",");
      
      while (cname) {
	if (strlen(cname)==0) {
	  cname="White";
	}
	new_color_ex->cnames[cindex++] = strdup(cname);
        cname = strsep(&cname_list,",");
      }
    } else {
      
      /* str has no comma. Maybe there's only one color in which
       * case the second must be set to White by default.
       */

      new_color_ex->ccount = 2;
      new_color_ex->cnames = calloc(2,sizeof *new_color_ex->cnames);
      
      cname=cname_list;
      if (strlen(cname)==0) {
	cname="White";
      }

      new_color_ex->cnames[0] = strdup(cname);
      new_color_ex->cnames[1] = strdup("White");
    } 
  }
  
  return new_color_ex;
}

/* Free the memory used by the specified *t_color_ex.
 */
void free_color_ex(t_color_ex *color_ex) {
  if (!color_ex) {
    return;
  }
  
  for (int i=0;i<color_ex->ccount;i++) {
    free(color_ex->cnames[i]);
  }
  
  free(color_ex->cnames);
  free(color_ex);
}

/* Prepends the specified path(s) to the Imlib font
 * paths list.
 */
void add_font_path_to_imlib_path(t_path_list *path_list) {
  if ( (!path_list) || (path_list->count==0) ) {
    return;
  }

  int            in_count;
  char         **paths   = imlib_list_font_path(&in_count);
  t_path_list  *in_list = new_path_list();
  wordexp_t      exp_path;

  /* there's no other way to prepends the imlib2 font 
   * path list  than to re-create the list ...
   */
   
  for (int i=0;i<in_count;i++) {
    add_path(in_list,paths[i],NULL,-1);
  }
  
  clear_imlib_font_path();
  
  for (int i=0;i<path_list->count;i++) {
    wordexp(path_list->pathnames[i],&exp_path,0);
    imlib_add_path_to_font_path(exp_path.we_wordv[0]);
    wordfree(&exp_path);
  }
  
  for (int i=0;i<in_count;i++) {
    imlib_add_path_to_font_path(in_list->pathnames[i]);
  }
  
  free_path_list(in_list);
}

/* Returns the list of fonts installed on the system which
 * are supported by Imlib2 library. 
 * 
 * Attention, it is up to the caller to free the returned 
 * list.
 */
char** get_imlib_font_list(int *count) {
  return imlib_list_fonts(count);
}

/* Prints on standard output the imlib font path.
 * 
 * The string prefix is printed in front of each path.
 */
void print_imlib_font_path(char *prefix) {
  int     count;
  char  **pathlist=imlib_list_font_path(&count);

  for (int i=0;i<count;i++) {
    fprintf(stdout,"%s%s\n",prefix,pathlist[i]);	
  }
}

/* Returns the root window.
 */
Window get_root_window() {
  return root_window;
}

/* Returns the main window.
 */
Window get_main_window() {
  return main_window;
}

/* Returns the icon window attached to the main window.
 */
Window get_icon_window() {
  return icon_window;
}



/* (re)creates the pixmap for the tooltip window 
 * according to the data stored in variable
 * tooltip_data.
 */
static void create_tooltip_pixmap() {
  int         text_width, text_height;
  int         text_x, text_y;
  Imlib_Image image;
  
  if (tooltip_data.pixmap) {
    XFreePixmap(display,tooltip_data.pixmap);
    tooltip_data.pixmap=0;
  }
    
  imlib_context_set_font(tooltip_data.font);
  
  imlib_get_text_size(tooltip_data.text,&text_width,&text_height);

  tooltip_data.width  = text_width+2*(tooltip_data.border+tooltip_data.text_padding);
  tooltip_data.height = text_height+2*(tooltip_data.border+tooltip_data.text_padding);

  text_x=(tooltip_data.width-text_width)/2;
  text_y=(tooltip_data.height-text_height)/2;
  
  tooltip_data.pixmap = XCreatePixmap(display,
				      root_window,
				      tooltip_data.width,
				      tooltip_data.height,
				      get_depth());
  image = imlib_create_image(tooltip_data.width,tooltip_data.height);
  imlib_context_set_image(image);

  imlib_fill_rectangle(0,0,
                       tooltip_data.width,tooltip_data.height,
		       tooltip_data.bg_color);
		       
  imlib_context_set_color(
    tooltip_data.fg_color._red,
    tooltip_data.fg_color._green,
    tooltip_data.fg_color._blue,
    255);
    
  if (tooltip_data.border>0) {
    for (int b=0;b<tooltip_data.border;b++) {
      imlib_image_draw_rectangle(b,b,
                                 tooltip_data.width-(2*b),
				 tooltip_data.height-(2*b));
    }
  }
    
  imlib_text_draw(text_x,text_y,tooltip_data.text);
  
  imlib_context_set_drawable(tooltip_data.pixmap);
  imlib_render_image_on_drawable(0,0);
  
  imlib_free_image();
  imlib_free_font();
}

/* Create the tooltip window for the running application with :
 * 
 *   'text' as tooltip-text.
 *   'bg_color' as background color.
 *   'fg_color' has foreground color.
 *   'font' as font. 
 *   'border' as window border width.
 *   'padding' as tooltip text padding size.
 * 
 * fallback_fonts must be the list (colon separated) of font(s) to
 * try when the font 'fontname' does not exist. Each font in this
 * list must be to the format: <name>/<size>
 */
void create_tooltip_window(char  *text, 
			   char  *bg_color, 
			   char  *fg_color, 
			   char  *fontname,
			   int    border,
			   int    text_padding,
			   char  *fallback_fonts) {
  XSetWindowAttributes xsw_attr;

  if (tooltip_data.text) {
    free(tooltip_data.text);
  }
  
  if (tooltip_data.bg_color) {
    free_color_ex(tooltip_data.bg_color);
  }

  tooltip_data.enabled      = true;
  tooltip_data.text         = strdup(text);
  tooltip_data.fg_color     = get_color(fg_color);
  tooltip_data.bg_color     = new_color_ex(bg_color);
  tooltip_data.font         = imlib_load_font(fontname);
  tooltip_data.border       = border;
  tooltip_data.text_padding = text_padding;
  tooltip_data.visible      = false;
  tooltip_data.xpos         = -1;
  tooltip_data.ypos         = -1;
  tooltip_data.width        = 0;
  tooltip_data.height       = 0;
  tooltip_data.pixmap       = 0;

  if (!tooltip_data.font) {
    fprintf(stderr,
      _NLS_("Failed to load font %s..."),fontname);
    
    char *flist;
    char *cfont;
    char *stok_saveptr=NULL;
    
    if (fallback_fonts) {
      flist=strdup(fallback_fonts);
    } else {
      flist=strdup("DejaVuSans/10:LiberationSans-Regular/10");
    }
    
    cfont=strtok_r(flist,":",&stok_saveptr);
    while (cfont) {
      tooltip_data.font=imlib_load_font(cfont);
      
      if (tooltip_data.font) {
	fprintf(stderr,_NLS_("...fall back to default font '%s'\n"),cfont);
	break;
      }
      cfont=strtok_r(NULL,":",&stok_saveptr);
    }
    
    free(flist);
    
    if (!tooltip_data.font) {
      fprintf(stderr,_NLS_("disable tooltip support.\n"));
      tooltip_data.enabled=false;
      return;
    }
  }
  
  tooltip_window=XCreateSimpleWindow(display,
    root_window,
    0,0,
    1,1,
    0,
    0,
    0);
    
  xsw_attr.event_mask=0;
  xsw_attr.override_redirect=True;
  
  XChangeWindowAttributes(display,
    tooltip_window,
    CWEventMask|CWOverrideRedirect,
    &xsw_attr);
    
  create_tooltip_pixmap();
}

/* Returns the tooltip window, if any.
 */
Window get_tooltip_window() {
  return tooltip_window;
}

/* Returns true if the tooltip is enabled, false otherwise.
 */ 
bool is_tooltip_enabled() {
  return tooltip_data.enabled;
}

/* Returns true if the tooltip window is visible, false otherwise.
 */ 
bool is_tooltip_visible() {
  return tooltip_data.visible;
}

/* Sets to 'text' the text of the tooltip window.
 */
void set_tooltip_text(char *text) {
  
  if (!tooltip_data.enabled) {
    return;
  }
  
  free(tooltip_data.text);
  tooltip_data.text = strdup(text);
  
  create_tooltip_pixmap();
  
  if (tooltip_data.visible) {
    hide_tooltip();
    show_tooltip(tooltip_data.xpos,tooltip_data.ypos);
  }
}

/* Gets the text of the tooltip window, or NULL if there is
 * no toolitp window.
 */
char *get_tooltip_text() {
  return tooltip_data.text;
}

/* Shows the tooltip window at the specified position (x,y), 
 * or at the position pointed by the mouse pointer when x=-1
 * and y=-1.
 */
void show_tooltip(int x, int y) {
  Window pointed_root, pointed_child;
  int    xpos_in_root, ypos_in_root;
  int    xpos_in_child, ypos_in_child;
  unsigned int mask;
  int screen_w=DisplayWidth(display,get_screen());
  int screen_h=DisplayHeight(display,get_screen());
  
  
  if ( (x==-1) || (y==-1)) {
    XQueryPointer(display,
                  root_window,
		  &pointed_root,
		  &pointed_child,
		  &xpos_in_root,&ypos_in_root,
		  &xpos_in_child,&ypos_in_child,
		  &mask);
		  
    x=xpos_in_root + 16;
    y=ypos_in_root + 16;
  }
  
  if ( x+tooltip_data.width > screen_w ) {
    x=screen_w - tooltip_data.width - 16;
  }
  
  if ( y+tooltip_data.height > screen_h ) {
    y=screen_h - tooltip_data.height - 16;
  }
  
  XMoveResizeWindow(display,
                    tooltip_window,
		    x,y,
		    tooltip_data.width,
		    tooltip_data.height);
		    
  XMapRaised(display,tooltip_window);
  
  XCopyArea(display,
    tooltip_data.pixmap,
    tooltip_window,
    default_gc,
    0,0,
    tooltip_data.width,tooltip_data.height,
    0,0);

  tooltip_data.visible = true;
  tooltip_data.xpos    = x;
  tooltip_data.ypos    = y;
}

/* Hides the tooltip window, when visible.
 */
void hide_tooltip() {
  if (!tooltip_data.visible) {
    return;
  }
  
  XUnmapWindow(display,tooltip_window);
  tooltip_data.visible=false;
}

/* Loads an svg image and returns it as an Imlib_Image
 * instance. Returns NULL on error.
 */
static Imlib_Image load_svg_image(char *filepath) {
  RsvgHandle        *svg_handle = NULL;
  RsvgDimensionData  svg_dim;
  cairo_surface_t   *surface;
  cairo_t           *cairo;
  int                stride;
  unsigned char    *svg_data = NULL;
  GError            *cairo_error = NULL;
  Imlib_Image       image;
  
  svg_handle=rsvg_handle_new_from_file(filepath,&cairo_error);
  
  if (!svg_handle) {
    fprintf(stderr,
      _NLS_("Failed to load %s\n"),filepath);
      
    fprintf(stderr,
      _NLS_("  Error %d: %s\n"),
      cairo_error->code,cairo_error->message);
    return NULL;
  }
  
  /*invariant: the svg image has been loaded successfully. */
  
  rsvg_handle_get_dimensions(svg_handle,&svg_dim);
  
  stride   = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,svg_dim.width);
  svg_data = calloc(stride*svg_dim.height,sizeof(char));

  surface  = cairo_image_surface_create_for_data(
    svg_data,
    CAIRO_FORMAT_ARGB32,
    svg_dim.width,
    svg_dim.height,
    stride);
    
  cairo = cairo_create(surface);
  
  cairo_translate(cairo,0.0,0.0);
  rsvg_handle_render_cairo(svg_handle,cairo);

  image=imlib_create_image_using_copied_data(
    svg_dim.width,
    svg_dim.height,
    (DATA32 *)svg_data);

  free(svg_data);

  cairo_surface_destroy(surface);    
  cairo_destroy(cairo);

  rsvg_handle_close(svg_handle,&cairo_error);    
  g_object_unref(svg_handle);  
  
  return image;
}

/* Loads an image and returns it as an Imlib_Image instance 
 * on success, NULL otherwise in which case error_return is 
 * the code of encountered error.
 */
Imlib_Image load_image(char *filepath, Imlib_Load_Error *error_return) {
  Imlib_Image       image=NULL;
  Imlib_Load_Error imlib_error=IMLIB_LOAD_ERROR_NONE;
  
  if (filepath) {
    /* (try to) load the image with imlib...
     */
    image=imlib_load_image_with_error_return(filepath,error_return);

    /* If the image loading failed, this might be because it 
     * is to format svg which is not supported by imlib...
     */
    if (!image) {
      image=load_svg_image(filepath);
      
      if (!image) {
	imlib_error=IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT;
      }
    }
  } else {
    imlib_error=IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST;
  }
  
  if (error_return) {
    *error_return=imlib_error;
  }
  return image;
}

/* Composes an image from an input image and the specified
 * brightness/contrast/gamma values.
 * 
 * The output image will be in grayscale mode when 
 * 'grayscale' is true, in the same color mode as the 
 * input image otherwise.
 */
Imlib_Image compose_image(Imlib_Image source, 
                           bool         grayscale,
                           float       brightness,
                           float       contrast,
                           float       gamma) {
  int                    width,height;
  Imlib_Image           image;
  Imlib_Color_Modifier cmodifier  = imlib_create_color_modifier();
  
  imlib_context_set_image(source);
  
  width  = imlib_image_get_width();
  height = imlib_image_get_height();
  
  /* Create an image to the same size of the source, selects
   * it, clears it and enables its alpha channel...
   */
  image = imlib_create_image(width,height);

  imlib_context_set_image(image);
  imlib_image_clear();
  imlib_image_set_has_alpha(1);
  
  /* Set the color modifier of the current context, and
   * updates its brightness/contrast/gamma according to
   * specified values..
   */

  imlib_context_set_color_modifier(cmodifier);

  imlib_modify_color_modifier_brightness(brightness);
  imlib_modify_color_modifier_contrast(contrast);
  imlib_modify_color_modifier_gamma(gamma);

  /* Blend the source onto the destination, i.e. the 
   * image to return.
   * 
   * Note that valgrind report issues of kind 'Use of 
   * uninitialised value of size 8' when 'source' is an
   * XPM image  with transparency. There is no such problem
   * with non transparent XPM image nor with PNG/JPG images.
   */

  imlib_blend_image_onto_image(source,1,0,0,width,height,0,0,width,height);
  
  if (grayscale) {
    DATA32 *data = imlib_image_get_data();
    int    len   = width*height;
    unsigned int color;
    int alpha,red,green,blue;
    int grey;
	
    for (int i=0;i<len;i++) {
      color = data[i];
      alpha = (color >> 24) & 0xff;
      red   = ((color >> 16) & 0xff);
      green = ((color >>  8) & 0xff);
      blue  = (color & 0xff);
      
      /* grayscale conversion using BT709-6 coefficients 
       * (https://www.itu.int/rec/R-REC-BT.709)
       */
      
      grey  = (int)(0.2126*red + 0.7152*green + 0.0722*blue);
	  
      data[i] = (alpha << 24) + 
                (grey << 16 ) +
		(grey << 8 )  + 
		grey;
    }
    imlib_image_put_back_data(data);
  }

  /* Copies alpha channel of the source image to ensures
   * that alpha channel of generated image is the same as
   * the source image. Without this, the alpha channel of
   * the generated image could be different because of the
   * brightness/contrast/gamma value applied previously.
   */
  imlib_image_copy_alpha_to_image(source,0,0);
  
  imlib_free_color_modifier();
  
  return image;
}

/* Creates an icon with the specified size and on which the
 * message "Failed to load icon" is printed using the X11 
 * font font_name.
 * 
 * Note that if the font font_name cannot be loaded, 
 * create_fail_icon() fall backs to the font "fixed",
 * and, if this font is not available, a red cross is
 * drawn in the icon area.
 */
Imlib_Image create_fail_icon(int width, int height, char *font_name) {
  char *text[]={ "Failed", "To load",  "Icon" };
    
  int  tcount=sizeof(text)/sizeof(char *);
  
  Display      *display = get_display();
  Window       mwin     = get_main_window();
  int          depth    = get_depth();
  GC           cgc      = get_gc();
  Pixmap       pixmap   = XCreatePixmap(display,mwin,width,height,depth);
  XFontStruct *font;
  int          ypos;
  int          xpos;
  int          hline;
  int          wtext;
  Imlib_Image icon;
  
  font = XLoadQueryFont(display,font_name);
  if (!font) {
    font = XLoadQueryFont(display,"fixed");
  }
  
  XSetForeground(display,cgc,get_color("white").pixel);
  XFillRectangle(display,pixmap,cgc,0,0,width,height);
  
  XSetForeground(display,cgc,get_color("red").pixel);
  XDrawRectangle(display,pixmap,cgc,0,0,width-1,height-1);
  
  if (font) {
    XSetFont(display,cgc,font->fid);
    hline = font->max_bounds.ascent+font->max_bounds.descent;
    ypos  = ( (height-(tcount*hline))/2 )+font->max_bounds.ascent;
  
    for (int t=0;t<tcount;t++) {
      wtext = XTextWidth(font,text[t],strlen(text[t]));
      xpos  = (width-wtext)/2;
      XDrawString(display,pixmap,cgc,xpos,ypos,text[t],strlen(text[t]));
      ypos +=hline;
    }
    XFreeFont(display,font);
  } else {
      XDrawLine(display,pixmap,cgc,0,0,width-1,height-1);
      XDrawLine(display,pixmap,cgc,0,height-1,width-1,0);
      
    for (int i=1;i<2;i++) {
      XDrawLine(display,pixmap,cgc,i,0,width-1,height-i-1);
      XDrawLine(display,pixmap,cgc,0,i,width-i-1,height-1);
      
      XDrawLine(display,pixmap,cgc,i,height-1,width-1,i);
      XDrawLine(display,pixmap,cgc,0,height-i-1,width-i-1,0);
    }
  }
  
  imlib_context_set_drawable(pixmap);  
  icon=imlib_create_image_from_drawable(0,0,0,width,height,0);
  
  return icon;
}

/* Returns the interpolation between the colors c1 and c2 at
 * t (in interval [0,1]) as a t_color.
 * 
 * This function is used by draw_hv_gradient() and draw_diagonal_gradient()
 * which use Imlib2 to draw the gradients. Therefore, only the fields 
 * _red, _green, _blue of the returned t_color are significant, all others
 * fields (ie: red,green,blue, and pixel) are set to 0.
 */
static t_color lerp_rgb(t_color c1, t_color c2, float t) {
  t_color ic = { .pixel=0, .red=0,  .green=0,  .blue=0, ._red=0, ._green=0, ._blue=0 } ;
 
  ic._red   = (1 - t) * c1._red   + t * c2._red;
  ic._green = (1 - t) * c1._green + t * c2._green;
  ic._blue  = (1 - t) * c1._blue  + t * c2._blue;
  
  return ic;
}

/* fills the rectangle R(x,y,width,height) using Imlib2 and with :
 *   * an horizontal gradient of the given count colors when dir=HORZ_GRADIENT
 *   * a vertical gradient of the given count colors when dir=VERT_GRADIENT
 */		
static void draw_hv_gradient(int x, int y, int width, int height, int count, char *colors[], int dir ) {
  t_color c1;
  t_color c2;
  t_color cc;
  float   t;

  int     avg_bsz;
  int     bsize;
  int     delta;
  int     cx=x;
  int     cy=y;
  int     x2=x+width-1;
  int     y2=y+height-1;
  
  if (dir==HORZ_GRADIENT) {
    avg_bsz = width/(count-1);
    delta   = width%(count-1);
  } else {
    avg_bsz = height/(count-1);
    delta   = height%(count-1);
  }

  c1=get_color(colors[0]);
  
  for (int c=1;c<count;c++) {
    c2    = get_color(colors[c]);
    bsize = avg_bsz;
    
    if (delta>0) {      
      if (c==count-2) {
	bsize +=delta;
      } else {
        bsize++;
        delta--;
      }
    }
        
    for (int i=1;i<=bsize;i++) {
      t  = (float)i/(float)bsize;    
      cc = lerp_rgb(c1,c2,t);

      imlib_context_set_color(cc._red,cc._green,cc._blue,255);
      
      if (dir==HORZ_GRADIENT) {
        imlib_image_draw_line(cx,y,cx,y2,0);
	cx++;
      } else {
	imlib_image_draw_line(x,cy,x2,cy,0);
	cy++;
      }
    }
    c1 = c2;
  }
}


/* fills the rectangle R(x,y,width,height) with a diagonal gradient of 
 * the given count colors using Imlib2.
 */		
static void draw_diagonal_gradient(int x, int y, int width, int height, int count, char *colors[]) {
  t_color c1;
  t_color c2;
  t_color cc;
  float   t;

  int dlen    = width+height;
  int avg_bsz = dlen/(count-1);
  int delta   = dlen%(count-1);
  int bsize;
  int cx      = x;
  int xclip,yclip,wclip,hclip;

  imlib_context_get_cliprect(&xclip,&yclip,&wclip,&hclip);
  imlib_context_set_cliprect(x,y,width,height);

  c1 = get_color(colors[0]);
  
  for (int c=1;c<count;c++) {
    c2    = get_color(colors[c]);
    bsize = avg_bsz;

    if (delta>0) {      
      if (c==count-2) {
	bsize +=delta;
      } else {
        bsize++;
        delta--;
      }
    }
    
    for (int i=1;i<=bsize;i++) {
      t  = (float)i/(float)bsize;
      cc = lerp_rgb(c1,c2,t);
      
      imlib_context_set_color(cc._red,cc._green,cc._blue,255);
      imlib_image_draw_line(cx,y,x,cx,0);

      cx++;
    }
    c1=c2;
  }
  imlib_context_set_cliprect(xclip,yclip,wclip,hclip);  
}


/* Fill the rectangle starting at P(x,y) and of dimension width x height
 * with the specified *t_color_ex using Imlib2.
 */
void imlib_fill_rectangle(int x,int y, int width, int height, t_color_ex *color_ex) {
  t_color bg_color;

  if (!color_ex) {
    return;
  }
  
  switch(color_ex->ctype) {
    case PLAIN_COLOR :
      bg_color=get_color(color_ex->cnames[0]);
      imlib_context_set_color(bg_color._red,bg_color._green,bg_color._blue,255);
      imlib_image_fill_rectangle(x,y,width,height);
    break;
    
    case HORZ_GRADIENT:
    case VERT_GRADIENT:
      draw_hv_gradient(x,y,width,height,color_ex->ccount,color_ex->cnames,color_ex->ctype);
    break;
    
    case DIAG_GRADIENT:
      draw_diagonal_gradient(x,y,width,height,color_ex->ccount,color_ex->cnames);
    break;
  }
  
}

