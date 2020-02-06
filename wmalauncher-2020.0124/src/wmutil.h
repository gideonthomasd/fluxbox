/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#ifndef WMUTIL_H_INCLUDED
#define WMUTIL_H_INCLUDED

#include <stdbool.h>
#include <Imlib2.h>

#include "fileutil.h"

          /*** 
           *    Defines      *
                           ***/

/* Identifiers of supported color types. See t_color_ex.
 */
#define PLAIN_COLOR   0x0
#define HORZ_GRADIENT 0x1
#define VERT_GRADIENT 0x2
#define DIAG_GRADIENT 0x4

          /*** 
           *    Typedefs and Structs      *
                                        ***/
 
/* An RGB color.
 * 
 * pixel is the pixel value of the color as defined
 * by X11 XColor type.
 * 
 * The red, green and blue values are always in the
 * range 0..65535 (inclusive).
 * 
 * The _red, _green and _blue values are always in the
 * range 0..255 (inclusive).
 */
typedef struct {
  unsigned long  pixel;
  unsigned short red, green, blue;
  int             _red, _green, _blue;
} t_color;


/* Represents a color or a color gradient.
 * 
 * ctype can be one of the following :
 *   PLAIN_COLOR
 *   HORZ_GRADIENT
 *   VERT_GRADIENT
 *   DIAG_GRADIENT
 * 
 * ccount is the number of color names stored in array
 * cnames[]. Always set to 1 when type==PLAIN_COLOR, >=2
 * otherwise.
 */
typedef struct {
    int   ctype;
    int   ccount;
    char **cnames;
} t_color_ex;

          /*** 
           *    Function Prototypes      *
                                       ***/

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
bool open_connection(char *display_name, char *font_search_paths);

/* Close the connection to the X server, and cleanup 
 * resources used by imlib2 library.
 * 
 * This is the counterpart of open_connection(), and this
 * should be the last function called when the program 
 * is about to terminate.
 */
void close_connection();

/* Creates the main and icon windows for the running 
 * application, then maps these windows.
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
			  bool support_del_msg);


/* Returns the connection number for the display get_display().
 */
int get_connection();

/* Returns the current connection to the X server as a 
 * *Display.
 */			     
Display *get_display();

/* Returns the default screen.
 */
int get_screen();

/* Returns the depth of the root window for the default screen.
 */
int get_depth();

/* Returns the default GC in use.
 */
GC get_gc();

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
t_color get_color(char *colorstr);

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
t_color_ex *new_color_ex(char *str);

/* Free the memory used by the specified *t_color_ex.
 */
void free_color_ex(t_color_ex *color_ex);
 
/* Prepends the specified path(s) to the Imlib font
 * paths list.
 */
void add_font_path_to_imlib_path(t_path_list *path_list) ;

/* Returns the list of fonts installed on the system which
 * are supported by Imlib2 library. 
 * 
 * Attention, it is up to the caller to free the returned 
 * list.
 */
char **get_imlib_font_list(int *count);

/* Prints on standard output the imlib font path.
 * 
 * The string prefix is printed in front of each path.
 */
void print_imlib_font_path(char *prefix);


/* Returns the root window.
 */
Window get_root_window();

/* Returns the main window.
 */
Window get_main_window();

/* Returns the icon window attached to the main window.
 */
Window get_icon_window();


/* Create the tooltip window for the running application 
 * with :
 * 
 *   'text' as tooltip-text.
 *   'bg_color' as background color.
 *   'fg_color' has foreground color.
 *   'font' as font. 
 *   'border' as window border width.
 *   'padding' as tooltip text padding size.
 * 
 * Note that 'font' must be to the format: "<name>/<size>"
 * where 'name' is the name of a true type font accessible
 * from the font path, and size, the size of the font.
 * 
 * fallback_fonts must be the list (colon separated) of font(s) to
 * try when the font 'fontname' does not exist. Each font in this
 * list must be to the format: <name>/<size>
 */
void create_tooltip_window(char *text, 
			   char *bg_color, 
			   char *fg_color, 
			   char *fontname,
			   int   border,
			   int   text_padding,
			   char *fallback_fonts);

/* Returns the tooltip window, if any.
 */
Window get_tooltip_window();

/* Returns true if the tooltip is enabled, false otherwise.
 */ 
bool is_tooltip_enabled();

/* Returns true if the tooltip window is visible, false otherwise.
 */ 
bool is_tooltip_visible();

/* Sets to 'text' the text of the tooltip window.
 */
void set_tooltip_text(char *text);

/* Gets the text of the tooltip window, or NULL if there is
 * no toolitp window.
 */
char *get_tooltip_text();

/* Shows the tooltip window at the specified position (x,y), 
 * or at the position pointed by the mouse pointer when x=-1
 * and y=-1.
 */
void show_tooltip(int x, int y);

/* Hides the tooltip window, when visible.
 */
void hide_tooltip();


/* Loads an image and returns it as an Imlib_Image instance 
 * on success, NULL otherwise in which case error_return is 
 * the code of encountered error.
 */
Imlib_Image load_image(char *filepath, Imlib_Load_Error *error_return);

/* Composes an image from an input image and the specified
 * brightness/contrast/gamma values.
 * 
 * The output image will be in grayscale mode when 
 * 'grayscale' is true, in the same color mode as the 
 * input image otherwise.
 */
Imlib_Image compose_image(Imlib_Image source, 
			   bool         grayscale,
                           float        brightness,
			   float        contrast,
			   float        gamma);


/* Creates an icon with the specified size and on which the
 * message "Failed to load icon" is printed using the X11 
 * font font_name.
 * 
 * Note that if the font font_name cannot be loaded, 
 * create_fail_icon() fall backs to the font "fixed",
 * and, if this font is not available, a red cross is
 * drawn in the icon area.
 */
Imlib_Image create_fail_icon(int width, int height, char *font_name);

/* Fill the rectangle starting at P(x,y) and of dimension width x height
 * with the specified *t_color_ex using Imlib2.
 */
void imlib_fill_rectangle(int x,int y, int width, int height, t_color_ex *color_ex);

#endif
