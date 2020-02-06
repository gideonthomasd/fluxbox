/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include <stdbool.h>

#include "command.h"
#include "fileutil.h"


          /*** 
           *    type(defs) and structs      *
                                          ***/

/* 
 * The identifiers of wmalauncher options.
 * 
 * As its name suggests it, the identifier NULL_OPTION represents
 * no option.
 */
typedef enum { 
  NULL_OPTION,
  
  HELP,
  FONTS_INFO,
  CONFIG,
  PREFIX,
  COMMAND,
  ICON,
  DESKTOP_FILE,

  RUN_IN_TERM,
  PREF_TERMS,
  
  WINDOW_SIZE,
  ICON_PADDING,
  BORDER_SIZE,

  BACKGROUND_COLOR,
  HOVER_BACKGROUND_COLOR,

  BORDER_COLOR,
  HOVER_BORDER_COLOR,

  ICON_SEARCH_PATH,
  DESKTOP_FILE_SEARCH_PATH,
  FONT_SEARCH_PATH,

  TTIP_TEXT,
  TTIP_FONT,
  TTIP_BG_COLOR,
  TTIP_FG_COLOR,
  TTIP_BORDER_SIZE,
  TTIP_TEXT_PADDING,
  TTIP_SHOW_DELAY,
  TTIP_HIDE_DELAY,

  ICON_GRAYSCALE,
  HOVER_ICON_GRAYSCALE,

  FRAME,

  ICON_BRIGHTNESS,
  HOVER_ICON_BRIGHTNESS,
  ICON_CONTRAST,
  HOVER_ICON_CONTRAST,
  ICON_GAMMA,
  HOVER_ICON_GAMMA,
  
  DOUBLE_CLICK,
  DISPLAY,
  BROKEN_WM,
  EXIT_ON_RCLICK,
} t_option_id;

/* Represents an application option.
 * 
 * name      : long option name, without the double-dash (ie. --)
 * shortname : short option name, without the single-dash (ie. -)
 * id        : the option identifier.
 */
typedef struct {
    const char  *name;
    const char  *shortname;
    t_option_id id;
} t_option;

/* Represents a frame.
 */
typedef struct {
  char *hicolor;
  char *locolor;
  
  char *hover_hicolor;
  char *hover_locolor;
  
  int   thickness;
} t_frame;

/* Represents the application's configuration.
 */
typedef struct {
  
  t_command *command;
  
  char  *icon_name;

  /* colon separated list of (path)names. Only the 1st .desktop 
   * file (accessible from desktop_file_search_path) that matches
   * any of (path)names in that list is used.
   */
  char *desktop_file; 

  bool  run_in_term;
  char *pref_terms;

  int winsize;
  int icon_size;
  int icon_padding;
  int border_size;

  char *background_color;
  char *hover_background_color;

  char *border_color;
  char *hover_border_color;

  t_path_list *icon_search_path; 
  t_path_list *desktop_file_search_path;
  t_path_list *font_search_path;

  char *tooltip_text;
  char *tooltip_font;
  char *tooltip_bg_color;
  char *tooltip_fg_color;
  int   tooltip_border_size;
  int   tooltip_text_padding;
  int   tooltip_show_delay;
  int   tooltip_hide_delay;

  bool icon_grayscale;
  bool hover_icon_grayscale;

  t_frame *frame;

  float icon_brightness;
  float hover_icon_brightness;

  float icon_contrast;
  float hover_icon_contrast;

  float icon_gamma;
  float hover_icon_gamma;
  
  /* when single_click is true (default), double_click_delay is 
   * not used.
   */  
  bool single_click;
  unsigned int double_click_delay; 
  
  char *display;
  
  bool broken_wm;
  bool exit_on_rclick;
} t_config ;

/* Enumeration of the formats supported by function
 * get_option_name().
 */
typedef enum { 
  SHORT_NAME, 
  LONG_NAME, 
  BOTH_NAMES } t_option_name_format;


          /*** 
           *    Functions prototypes      *
                                        ***/

/* Creates and returns the application configuration
 * with the default settings.
 */
t_config *create_config();

/* Deletes the application configuration and frees
 * the memory it uses.
 */
void delete_config();

/* Returns the current application configuration.
 */
t_config *get_config();

/* Returns the identifier of the option which matches
 * the specified name, NULL_OPTION otherwise.
 * 
 * inc_dashes must be true when get_option_id() must
 * ensure that 'name' starts with (1 or 2) dashes.
 */
t_option_id get_option_id(char *name, bool inc_dashes);

/* copies into the buffer 'buffer' of 'size' bytes the short 
 * and/or long name of the given option id, then returns the 
 * buffer 'buffer'. If the given id is invalid, the buffer 
 * 'buffer' is emptied. 
 * Note that get_option_name write at most 'size' bytes (including
 * the terminate null byte) in the buffer .
 * 
 * when inc_dashes is true, the returned short/long name will
 * starts with one/two dashes.
 * 
 * when format==SHORT_NAME, the returned string is to format : 
 *   [-]shortname.
 * 
 * when format=LONG_NAME, the returned string is to format :
 *   [--]longname.
 * 
 * when format=BOTH_NAMES, the returned string is to format :
 *   [-]shortname|[--]longname.
 */
char *get_option_name(t_option_id            option_id, 
		       t_option_name_format  format, 
		       bool                   inc_dashes,
		       char                  *buffer,
		       size_t                 size);

/* Returns true if option_id is one of the 'count' t_option_id 
 * which follow in arguments, false otherwise.
 */
bool is_option_in(t_option_id option_id, int count, ...);


/* Assigns the value 'value' to the field of the current
 * application configuration which corresponds to the
 * option with identifier option_id.
 */
void set_config_field(t_option_id option_id, void *value);

/* Loads the settings, with the given prefix when not NULL, found 
 * in the specified file and updates the current application
 * configuration accordingly. 
 * 
 * Returns true if the settings have been successfully loaded, 
 * false otherwise, i.e when :
 *   * the file does not exist
 *   * the file contain invalid values
 *   * there are missing value (unless chk_missing is false)
 * 
 * The file 'filename' must be a plain text with entries
 * to the format :
 * 
 *   [prefix.]key = value
 * 
 * The purpose of the prefix is to allow storage of 
 * multiple configuration that can be loaded by different
 * instances of the application from the same source.
 */
bool load_config(char *filename, char *prefix, bool chk_missing);

/* Loads the application desktop file 'filename' and updates accordingly
 * the following fields of the current application configuration:  
 * 'command', 'icon', 'tooltip_text', 'run_in_term'
 * 
 * Returns true on success, false otherwise.
 */
bool load_desktop_file(char *filename);


/* Prints the current configuration on standard output.
 */
void print_config();

#endif
