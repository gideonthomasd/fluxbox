/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <ctype.h>
#include <errno.h>

#include <config.h>

#include "nls.h"
#include "cfparser.h"
#include "settings.h"

          /*** 
           *    Variables      *
                             ***/

/* The application configuration.
 */
static t_config *config = NULL;

/* The options supported by the application.
 */
static t_option option_list[] = { 
  { "help"                     , "h", HELP },
  { "fonts-info"               , "f", FONTS_INFO },    
  { "config"                   , "C", CONFIG }, 
  { "prefix"                   , "p", PREFIX },
  { "command"                  , "c", COMMAND },
  { "icon"                     , "i", ICON },
  { "desktop-file"             , "d", DESKTOP_FILE },
  
  { "run-in-terminal"          , "T", RUN_IN_TERM },
  { "preferred-terminals"      , "P", PREF_TERMS },
    
  { "window-size"              , "w", WINDOW_SIZE },
  { "icon-padding"             , "g", ICON_PADDING},
  { "border-size"              , "z", BORDER_SIZE },

  { "background-color"         , "k", BACKGROUND_COLOR },
  { "hover-background-color"   , "K", HOVER_BACKGROUND_COLOR },   

  { "border-color"             , "b", BORDER_COLOR },
  { "hover-border-color"       , "B", HOVER_BORDER_COLOR },

  { "icon-search-path"         , "a", ICON_SEARCH_PATH },
  { "desktop-file-search-path" , "A", DESKTOP_FILE_SEARCH_PATH},
  { "font-search-path"         , "n", FONT_SEARCH_PATH},

  { "tooltip-text"             , "t", TTIP_TEXT },
  { "tooltip-font"             , "F", TTIP_FONT },
  { "tooltip-background-color" , "o", TTIP_BG_COLOR },
  { "tooltip-foreground-color" , "O", TTIP_FG_COLOR },    
  { "tooltip-border-size"      , "Z", TTIP_BORDER_SIZE },
  { "tooltip-text-padding"     , "G", TTIP_TEXT_PADDING },
  { "tooltip-show-delay"       , "l", TTIP_SHOW_DELAY},
  { "tooltip-hide-delay"       , "L", TTIP_HIDE_DELAY},

  { "icon-grayscale"           , "y", ICON_GRAYSCALE},
  { "hover-icon-grayscale"     , "Y", HOVER_ICON_GRAYSCALE},

  { "frame"                    , "E", FRAME },

  { "icon-brightness"          , "r", ICON_BRIGHTNESS },
  { "hover-icon-brightness"    , "R", HOVER_ICON_BRIGHTNESS },
  { "icon-contrast"            , "s", ICON_CONTRAST },
  { "hover-icon-contrast"      , "S", HOVER_ICON_CONTRAST },
  { "icon-gamma"               , "m", ICON_GAMMA },    
  { "hover-icon-gamma"         , "M", HOVER_ICON_GAMMA },

  { "double-click"             , "2", DOUBLE_CLICK },
  { "display"                  , "D", DISPLAY },
  { "broken-wm"                , "W", BROKEN_WM },
  { "exit-on-right-click"      , "x", EXIT_ON_RCLICK }
} ;

static int option_count = sizeof(option_list)/sizeof(t_option);

          /*** 
           *    Functions    *
			   ***/

/* Creates and returns a new *t_frame
 */
static t_frame *new_frame(char *hicolor, 
                           char *hover_hicolor, 
                           char *locolor, 
                           char *hover_locolor,
                           int  thickness) {
  t_frame *frame=malloc(sizeof(t_frame));
  
  if (!hicolor)       hicolor=DEFAULT_FRAME_HICOLOR;
  if (!hover_hicolor) hover_hicolor=DEFAULT_FRAME_HOVER_HICOLOR;

  if (!locolor)       locolor=DEFAULT_FRAME_LOCOLOR;
  if (!hover_locolor) hover_locolor=DEFAULT_FRAME_HOVER_LOCOLOR;
  
  frame->hicolor       = strdup(hicolor);
  frame->locolor       = strdup(locolor);
  frame->hover_hicolor = strdup(hover_hicolor);
  frame->hover_locolor = strdup(hover_locolor);
  frame->thickness     = thickness;
  
  return frame;
}

/* Parses frame_str and returns a new *t_frame. 
 * 
 * frame_str must be to the format :
 *   hicolor/hover_hicolor:locolor/hover_locolor[:size]
 * 
 * when hicolor and/or hover_hicolor is not set, default to 'White'
 * when locolor and/or hover_locolor is not set, default to 'Black'
 * when size is not set, Default to 1.
 */
static t_frame *parse_frame_str(char *frame_str, char *warn_prefix) {
  if (!frame_str) {
    return new_frame(NULL,NULL,NULL,NULL,1);
  }
  
  char *hi_field=NULL, *lo_field=NULL, *ts_field=NULL;
  char *hicolor, *hvr_hicolor;
  char *locolor, *hvr_locolor;
  int  thickness=1;
  
  hi_field = strsep(&frame_str,":");
  lo_field = strsep(&frame_str,":");
  ts_field = strsep(&frame_str,":");
  
  if (hi_field) { 
    hicolor     = strsep(&hi_field,"/");
    hvr_hicolor = strsep(&hi_field,"/");

    if ( (!hicolor) || (strlen(hicolor)==0) ) {
      fprintf(stderr,
        _NLS_("[%s] Warning, <HICOLOR> is missing. Default to '%s'.\n"),
	warn_prefix,
	DEFAULT_FRAME_HICOLOR);
	
      hicolor=DEFAULT_FRAME_HICOLOR;
    }

    if ( (!hvr_hicolor) || (strlen(hvr_hicolor)==0) ){
      fprintf(stderr,
        _NLS_("[%s] Warning, <HOVER_HICOLOR> is missing. Default to '%s'.\n"),
	warn_prefix,
	DEFAULT_FRAME_HOVER_HICOLOR);
	
      hvr_hicolor=DEFAULT_FRAME_HOVER_HICOLOR;
    }
  } else {
    fprintf(stderr,
      _NLS_("[%s] Warning, <HICOLOR/HOVER_HICOLOR> is missing. Default to '%s/%s'.\n"),
      warn_prefix,
      DEFAULT_FRAME_HICOLOR,
      DEFAULT_FRAME_HOVER_HICOLOR);
      
    hicolor     = DEFAULT_FRAME_HICOLOR;
    hvr_hicolor = DEFAULT_FRAME_HOVER_HICOLOR;
  }

  if (lo_field) { 
    locolor     = strsep(&lo_field,"/");
    hvr_locolor = strsep(&lo_field,"/");

    if ( (!locolor) || (strlen(locolor)==0) ) {
      fprintf(stderr,
        _NLS_("[%s] Warning, <LOCOLOR> is missing. Default to '%s'.\n"),
	warn_prefix,
	DEFAULT_FRAME_LOCOLOR);
	
      locolor=DEFAULT_FRAME_LOCOLOR;
    }

    if ( (!hvr_locolor) || (strlen(hvr_locolor)==0) ) {
      fprintf(stderr,
        _NLS_("[%s] Warning, <HOVER_LOCOLOR> is missing. Default to '%s'.\n"),
        warn_prefix,
	DEFAULT_FRAME_HOVER_LOCOLOR);
	
      hvr_locolor=DEFAULT_FRAME_HOVER_LOCOLOR;
    }
  } else {
    fprintf(stderr,
      _NLS_("[%s] Warning, <LOCOLOR/HOVER_LOCOLOR> is missing. Default to '%s/%s'.\n"),
      warn_prefix,
      DEFAULT_FRAME_LOCOLOR,
      DEFAULT_FRAME_HOVER_LOCOLOR);
      
    locolor     = DEFAULT_FRAME_LOCOLOR;
    hvr_locolor = DEFAULT_FRAME_HOVER_LOCOLOR;
  }
  
  if (ts_field) {
    char *endptr;
    
    thickness=(int)strtol(ts_field,&endptr,0);
    if ((*endptr !='\0') || (thickness<=0)) {
      fprintf(stderr,
        _NLS_("[%s] Warning, invalid thickness '%s'. Must be >0. Reset to %d.\n"),
	warn_prefix,
	ts_field,
	DEFAULT_FRAME_THICKNESS);
      thickness=DEFAULT_FRAME_THICKNESS;
    }
  }

  return new_frame(hicolor,hvr_hicolor,locolor,hvr_locolor,thickness);
}

/* Free the memory used by the given *t_frame.
 */
static void free_frame(t_frame *frame) {
  if (!frame) {
    return;
  }
  
  free(frame->hicolor);
  free(frame->hover_hicolor);

  free(frame->locolor);
  free(frame->hover_locolor);

  free(frame);
}

/* Recomputes and updates the icon size according to the current 
 * configuration of the window size, the border size, the icon padding
 * size, and, if a frame is defined, its thickness.
 * 
 * option_id must be :
 *   BORDER_SIZE when update_icon_size() is called following change
 *   of the border size.
 * 
 *   ICON_PADDING when update_icon_size() is called following change
 *   of the icon padding.
 * 
 *   FRAME when update_icon_size() is called following change of
 *   the frame settings.
 * 
 *   NULL_OPTION when update_icon_size() is called following multiple
 *   changes (ex: 'border size' and 'icon padding')
 * 
 * when the icon size computed with the current configuration is less
 * than MIN_ICON_SIZE, a warning message is printed, and  :
 *   + The border is adjusted when option_id=BORDER_SIZE
 *   + The padding is adjusted when option_id=ICON_PADDING
 *   + The frame thickness is adjusted when option_id=FRAME
 *   + The border, the padding, and the frame thickness (if enabled)
 *     are reset to their default value, when option_id=NULL_OPTION
 * 
 * warn_prefix is the string that must be printed in front of
 * warning message, when required.
 */
static void update_icon_size(t_option_id option_id, char *warn_prefix) {
  int winsize   = config->winsize;
  int border    = config->border_size; 
  int padding   = config->icon_padding ;
  int thickness = (config->frame) ? config->frame->thickness : 0;
  int icon_size = winsize - (2* (border+padding+thickness) );
  
  if (icon_size >= MIN_ICON_SIZE) {
    config->icon_size = icon_size;
  } else  {
    if (is_option_in(option_id,3,BORDER_SIZE,ICON_PADDING,FRAME) ) {
    
      switch(option_id) {
        case BORDER_SIZE  :
	  config->border_size=(winsize - MIN_ICON_SIZE - 2*(thickness+padding)) / 2;

	  fprintf(stderr,
	    _NLS_("[%s] Warning, border size %d is to high. Set to %d\n"),
	    warn_prefix,border,config->border_size);
	break;
	
	case ICON_PADDING : 
	  config->icon_padding=(winsize - MIN_ICON_SIZE - 2*(thickness+border)) / 2;

	  fprintf(stderr,
	    _NLS_("[%s] Warning, icon padding %d is to high. Set to %d\n"),
	    warn_prefix,padding,config->icon_padding);
	break;
	
        case FRAME    :
	  if (config->frame) {
	    config->frame->thickness=(winsize - MIN_ICON_SIZE - 2*(border+padding)) / 2;
	    
	    fprintf(stderr,
	    _NLS_("[%s] Warning, frame thickness %d is to high. Set to %d\n"),
	    warn_prefix,thickness,config->frame->thickness);
	  }
	break;
	
	default : /* required to avoid bunch of warning messages at compile time */
	break;
      }
    } else {	
      config->border_size  = DEFAULT_BORDER_SIZE;
      config->icon_padding = DEFAULT_ICON_PADDING;
    
      if (config->frame) {
        config->frame->thickness = DEFAULT_FRAME_THICKNESS;
	
        fprintf(stderr,
          _NLS_("[%s] Warning, reset border size, icon padding and frame thickness \
to their default value.\n"),
	  warn_prefix);
      } else {
        fprintf(stderr,
          _NLS_("[%s] Warning, reset border size and icon padding to their default\
 value.\n"),
	  warn_prefix);
      }
    }
    
    border    = config->border_size; 
    padding   = config->icon_padding ;
    thickness = (config->frame) ? config->frame->thickness : 0;

    config->icon_size = winsize - ( 2 * (border+padding+thickness) );
  }
}
 
/* Creates and returns the application configuration
 * with the default settings.
 */
t_config *create_config() {
  if (config) {
    return config;
  }

  /* The WMAL_xxx and DEFAULT_xxxx "constants" are defined in config.h.in
   */
  
  config=malloc(sizeof(t_config));

  config->command                = NULL;
  config->icon_name              = NULL;
  config->desktop_file           = NULL;

  config->run_in_term            = false;
  config->pref_terms             = strdup(WMAL_PREFERRED_TERMINALS);

  config->winsize                = DEFAULT_WIN_SIZE;
  config->icon_padding           = DEFAULT_ICON_PADDING;
  config->border_size            = DEFAULT_BORDER_SIZE;

  config->background_color       = strdup(DEFAULT_BACKGROUND_COLOR);
  config->hover_background_color = strdup(DEFAULT_HOVER_BACKGROUND_COLOR);

  config->border_color           = strdup(DEFAULT_BORDER_COLOR);
  config->hover_border_color     = strdup(DEFAULT_HOVER_BORDER_COLOR);
    
  config->icon_search_path         = str_to_path_list(WMAL_ICON_SEARCH_PATH,WMAL_MAIN_ROOT_ICON_SEARCH_PATH);
  config->desktop_file_search_path = str_to_path_list(WMAL_DESKTOP_FILE_SEARCH_PATH,NULL);
  config->font_search_path         = str_to_path_list(WMAL_FONT_SEARCH_PATH,NULL);
  
  config->tooltip_text           = NULL;
  config->tooltip_font           = strdup(DEFAULT_TOOLTIP_FONT);
  config->tooltip_bg_color       = strdup(DEFAULT_TOOLTIP_BACKGROUND_COLOR);
  config->tooltip_fg_color       = strdup(DEFAULT_TOOLTIP_FOREGROUND_COLOR);
  config->tooltip_border_size    = DEFAULT_TOOLTIP_BORDER_SIZE;
  config->tooltip_text_padding   = DEFAULT_TOOLTIP_TEXT_PADDING;
  config->tooltip_show_delay     = DEFAULT_TOOLTIP_SHOW_DELAY;
  config->tooltip_hide_delay     = DEFAULT_TOOLTIP_HIDE_DELAY;

  config->icon_grayscale         = false;
  config->hover_icon_grayscale   = false;

  config->frame                  = NULL;

  config->icon_brightness        = DEFAULT_ICON_BRIGHTNESS;
  config->hover_icon_brightness  = DEFAULT_HOVER_ICON_BRIGHTNESS;

  config->icon_contrast          = DEFAULT_ICON_CONTRAST;
  config->hover_icon_contrast    = DEFAULT_HOVER_ICON_CONTRAST;

  config->icon_gamma             = DEFAULT_ICON_GAMMA;
  config->hover_icon_gamma       = DEFAULT_HOVER_ICON_GAMMA;
    
  /* when single_click is true (default), double_click_delay is 
   * not used.
   */  
  config->single_click           = true;  
  config->double_click_delay     = 0; 

  config->display                = NULL;
  config->broken_wm              = false;
  config->exit_on_rclick	 = false;

  update_icon_size(NULL_OPTION,"create_config");
    
  return config;
}

/* Deletes the application configuration and frees
 * the memory it uses.
 */
void delete_config() {
  if (!config) {
    return;
  }
  
  free_command(config->command);
  free(config->icon_name);
  free(config->desktop_file);

  free(config->pref_terms);

  free(config->background_color);
  free(config->hover_background_color);

  free(config->border_color);
  free(config->hover_border_color);

  free_path_list(config->icon_search_path);
  free_path_list(config->desktop_file_search_path);
  free_path_list(config->font_search_path);

  free(config->tooltip_text);
  free(config->tooltip_font);
  free(config->tooltip_bg_color);
  free(config->tooltip_fg_color);

  free_frame(config->frame);
  

  free(config->display); 
  
  free(config);
  config=NULL;
}

/* Returns the current application configuration.
 */
t_config *get_config() {
  if (!config) {
    create_config();
  }
  return config;
}

/* Returns the identifier of the option which matches
 * the specified name, NULL_OPTION otherwise.
 * 
 * inc_dashes must be true when get_option_id() must
 * ensure that 'name' starts with (1 or 2) dashes.
 */
t_option_id get_option_id(char *name, bool inc_dashes) {
  if (!name) {
    return NULL_OPTION;
  }

  char *pname = name;
  int dcount  = 0;
  int mlong;
  int mshrt;

  /* 
   * skip the dashes found in front of the specified 
   * option, if any.
   */
  while (pname[0]=='-') {
    dcount++;
    pname++;
  }
  
  for (int i=0;i<option_count;i++) {
    mlong=strcmp(pname,option_list[i].name);
    mshrt=strcmp(pname,option_list[i].shortname);
      
    /* when inc_dashes is true, there must be :
     *   2 '-' before name when it matches option_list[i].name
     *   1 '-' before name when it matches option_list[i].shortname
     */
    if (inc_dashes) {
      if ( ((dcount==2) && (!mlong)) || ((dcount==1) && (!mshrt)) ) {
        return option_list[i].id;
      }
    }
      else 
    if ( (!mlong) || (!mshrt) ) {
      return option_list[i].id;
    }
  }
  
  return NULL_OPTION;  
}

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
		       size_t                 size) {
  bool         found      = false;
  const char *mlong      = NULL;
  const char *mshrt      = NULL;
  char       *one_dash   = (inc_dashes) ? "-"  : "";
  char       *two_dashes = (inc_dashes) ? "--" : "";
  
  memset(buffer,0,size);
  
  for (int i=0;i<option_count;i++) {
    if (option_list[i].id==option_id) {
      found = true;
      mlong = option_list[i].name;
      mshrt = option_list[i].shortname;
      break;
    }
  }
  
  if (!found) {
    return buffer;
  }
  
  switch(format) {
    case SHORT_NAME: 
      snprintf(buffer,size,"%s%s",one_dash,mshrt);
    break;
    
    case LONG_NAME:
      snprintf(buffer,size,"%s%s",two_dashes,mlong);
    break;
    
    case BOTH_NAMES:
      snprintf(buffer,size,"%s%s|%s%s",one_dash,mshrt,two_dashes,mlong);
    break;
  }
  
  return buffer;
}


/* Returns true if option_id is one of the 'count' t_option_id 
 * which follow in arguments, false otherwise.
 */
bool is_option_in(t_option_id option_id, int count, ...) {
  va_list args;
  t_option_id cid;
  
  va_start(args,count);
  
  for (int i=0;i<count;i++) {
    cid=va_arg(args,t_option_id);
    
    if (option_id==cid) {
      return true;
    }
  }
  va_end(args);
  
  return false;
}

/* Returns true if s1 matches (*) one of the count 
 * strings which follow in arguments, false otherwise.
 * 
 * (*) ignoring case of character when nocase is true
 */
bool is_string_in(char *s1, bool nocase, int count, ...) {
  if (!s1) {
    return false;
  }

  va_list  args;
  char    *cstr;
  bool     match=false;
    
  va_start(args,count);

  for (int i=0;i<count && !match;i++) {
    cstr=va_arg(args,char*);
    
    if (cstr) {
      match=nocase ? !strcasecmp(s1,cstr) : !strcmp(s1,cstr);
    }
  }
  
  va_end(args);

  return match;
}

/* Returns the boolean value represented by the string 'str'.
 * 
 * 'str' can be one of the following (case insensitive) :
 * 0, false, off, no
 * 1, true, on, yes
 * 
 * when the value of 'str' is not recognized and warn_prefix is
 * not NULL, a warning message with the given prefix is printed 
 * on the standard output and str_to_bool returns _default.
 */
bool str_to_bool(char *str, char *warn_prefix, bool _default) {
  if (!str) {
    return _default;
  }
  
  if ( is_string_in(str,true,4,"1","true","on","yes") ) {
    return true;
  }
  
  if ( (!is_string_in(str,true,4,"0","false","off","no")) && (warn_prefix) ) {
    char *default_str= (_default) ? "true" : "false";
    
    fprintf(stderr,
      _NLS_("[%s] Warning, unrecognized value '%s'. Default to '%s'.\n"),
      warn_prefix,
      str,
      default_str);
  }
  
  return _default;
}

/* Assigns the value 'value' to the field of the current
 * application configuration which corresponds to the
 * option with identifier option_id.
 */
void set_config_field(t_option_id option_id, void *value) {
  char *strval=(char *)value;
  char  opt_name1[32];
  char *temp;
  char *endptr;
  char *stok_saveptr;
  int   pos;
  int   len;
  bool  flag;
  
  if (!config) {
    fprintf(stderr,
      _NLS_("set_config_field(): Internal Error (config==NULL).\n"));
    return;
  }
   
  switch(option_id) {
    
    case NULL_OPTION :
      fprintf(stderr,
        _NLS_("set_config_field(): Internal Error. option_id==NULL_OPTION.\n"));
      return;
    break;

    /* The following identifiers are ignored.
     */
    case HELP:
    case FONTS_INFO:
    case CONFIG:
    case PREFIX:
    break;
    
    case COMMAND:
      free_command(config->command);
      config->command=(t_command*)value;
    break;
    
    case ICON:
      free(config->icon_name);
      config->icon_name = (strval) ? strdup(strval) : NULL;
    break;
    
    case DESKTOP_FILE:
      free(config->desktop_file);
      config->desktop_file = (strval) ? strdup(strval) : NULL;
    break;


    case RUN_IN_TERM:
      config->run_in_term=str_to_bool(strval,
                       get_option_name(option_id,LONG_NAME,false,opt_name1,32),
		       false);
    break;

    case PREF_TERMS:
      free(config->pref_terms);
      config->pref_terms = (strval) ? strdup(strval) : strdup(WMAL_PREFERRED_TERMINALS);
    break;

    case WINDOW_SIZE:
      config->winsize=strtol(strval,&endptr,0);

      if ((*endptr != '\0') 
          || (config->winsize < MIN_WIN_SIZE) 
	  || (config->winsize > MAX_WIN_SIZE) ) {
	fprintf(stderr,
	        _NLS_("[%s] Warning, invalid size '%s'. \
Must be an integer in range [%d..%d]. Default to: %d.\n"),
		get_option_name(WINDOW_SIZE,LONG_NAME,false,opt_name1,32),
		strval,
		MIN_WIN_SIZE,
		MAX_WIN_SIZE,
		DEFAULT_WIN_SIZE);
	config->winsize=DEFAULT_WIN_SIZE;
      }
      update_icon_size(NULL_OPTION,
        get_option_name(WINDOW_SIZE,LONG_NAME,false,opt_name1,32));
    break;

    case ICON_PADDING:
      config->icon_padding=strtol(strval,&endptr,0);

      if ((*endptr != '\0') || (config->icon_padding < 0) ) {
	fprintf(stderr,
	_NLS_("[%s] Warning, invalid size '%s'. Must be an integer >=1. Default to: %d.\n"),
	get_option_name(ICON_PADDING,LONG_NAME,false,opt_name1,32),
	strval,
	DEFAULT_ICON_PADDING);
	config->icon_padding=DEFAULT_ICON_PADDING;
      }
      update_icon_size(ICON_PADDING,
        get_option_name(ICON_PADDING,LONG_NAME,false,opt_name1,32));
    break;

    case BORDER_SIZE:
      config->border_size=strtol(strval,&endptr,0);

      if ((*endptr != '\0') || (config->border_size < 0) ) {
	fprintf(stderr,
	_NLS_("[%s] Warning. invalid size '%s'. Must be an integer >=0. Default to: %d.\n"),
	get_option_name(BORDER_SIZE,LONG_NAME,false,opt_name1,32),
        strval,
	DEFAULT_BORDER_SIZE);
	
	config->border_size=DEFAULT_BORDER_SIZE;
      }
      update_icon_size(BORDER_SIZE,
        get_option_name(BORDER_SIZE,LONG_NAME,false,opt_name1,32));
    break;

    case BACKGROUND_COLOR:
      free(config->background_color);
      config->background_color = (strval) ? strdup(strval) : NULL;
    break;

    case HOVER_BACKGROUND_COLOR:
      free(config->hover_background_color);
      config->hover_background_color = (strval) ? strdup(strval) : NULL;
    break;
    
    case BORDER_COLOR:
      free(config->border_color);
      config->border_color = (strval) ? strdup(strval) : NULL;
    break;

    case HOVER_BORDER_COLOR:
      free(config->hover_border_color);
      config->hover_border_color = (strval) ? strdup(strval) : NULL;
    break;
    
    
    case ICON_SEARCH_PATH:
      temp=strtok_r(strval,":",&stok_saveptr);
      pos=0;
      while (temp) {
	add_path(config->icon_search_path,temp,WMAL_MAIN_ROOT_ICON_SEARCH_PATH,pos++);
	temp=strtok_r(NULL,":",&stok_saveptr);
      }
    break;
    
    case DESKTOP_FILE_SEARCH_PATH:
      temp=strtok_r(strval,":",&stok_saveptr);
      pos=0;
      while (temp) {
	add_path(config->desktop_file_search_path,temp,NULL,pos++);
	temp=strtok_r(NULL,":",&stok_saveptr);
      }
    break;

    case FONT_SEARCH_PATH:
      temp=strtok_r(strval,":",&stok_saveptr);
      pos=0;
      while (temp) {
	add_path(config->font_search_path,temp,NULL,pos++);
	temp=strtok_r(NULL,":",&stok_saveptr);
      }
    break;
    
    case TTIP_TEXT:
      free(config->tooltip_text);
      config->tooltip_text = (strval) ? strdup(strval) : NULL;
    break;

    case TTIP_FONT:
      free(config->tooltip_font);
      config->tooltip_font=NULL;
      
      
      if (strval) {
        if ( !strstr(strval,"/") )  {
	  
	  len=strlen(strval)+3;
	  temp=calloc(len+1,sizeof(char));
	  strcat(temp,strval);
	  strcat(temp,"/10");

	  config->tooltip_font=strdup(temp);	  
	  free(temp);
	  	  
	  fprintf(stderr,
	    _NLS_("[%s] Warning, missing size in font spec. '%s'. Default to: %s\n"),
	    get_option_name(TTIP_FONT,LONG_NAME,false,opt_name1,32),
	    strval,
	    config->tooltip_font);
	} else {
	  config->tooltip_font=strdup(strval);
	}
      }
      
    break;

    case TTIP_BG_COLOR:
      free(config->tooltip_bg_color);
      config->tooltip_bg_color = (strval) ? strdup(strval) : NULL;
    break;

    case TTIP_FG_COLOR:
      free(config->tooltip_fg_color);
      config->tooltip_fg_color = (strval) ? strdup(strval) : NULL;
    break;

    case TTIP_BORDER_SIZE:
      config->tooltip_border_size=strtol(strval,&endptr,0);

      if ((*endptr != '\0') || (config->tooltip_border_size < 0) ) {
	fprintf(stderr,
	  _NLS_("[%s] Warning. invalid size '%s'. Must be an integer >=0. Default to: %d.\n"),
	  get_option_name(TTIP_BORDER_SIZE,LONG_NAME,false,opt_name1,32),
	  strval,
	  DEFAULT_TOOLTIP_BORDER_SIZE);
	  
	config->tooltip_border_size=DEFAULT_TOOLTIP_BORDER_SIZE;
      }
    break;

    case TTIP_TEXT_PADDING:
      config->tooltip_text_padding=strtol(strval,&endptr,0);

      if ((*endptr != '\0') || (config->tooltip_text_padding < 0) ) {
	fprintf(stderr,
	  _NLS_("[%s] Warning. invalid size '%s'. Must be an integer >=0. Default to: %d.\n"),
	  get_option_name(TTIP_TEXT_PADDING,LONG_NAME,false,opt_name1,32),
	  strval,
	  DEFAULT_TOOLTIP_TEXT_PADDING);
	  
	config->tooltip_text_padding=DEFAULT_TOOLTIP_TEXT_PADDING;
      }
    break;

    case TTIP_SHOW_DELAY:
      config->tooltip_show_delay=(int)strtol(strval,&endptr,0);
      
      if ((*endptr != '\0') 
          || (config->tooltip_show_delay < MIN_TOOLTIP_SHOW_DELAY) 
	  || (config->tooltip_show_delay > MAX_TOOLTIP_SHOW_DELAY) ) {
	fprintf(stderr,
	  _NLS_("[%s] Warning, invalid delay '%s'. Must be an integer in \
range [%d..%d]. Default to: %d.\n"),
          get_option_name(TTIP_SHOW_DELAY,LONG_NAME,false,opt_name1,32),
	  strval,
	  MIN_TOOLTIP_SHOW_DELAY,
	  MAX_TOOLTIP_SHOW_DELAY,
	  DEFAULT_TOOLTIP_SHOW_DELAY);
	  
	config->tooltip_show_delay=DEFAULT_TOOLTIP_SHOW_DELAY;
      }
    break;
    
    case TTIP_HIDE_DELAY:
      config->tooltip_hide_delay=(int)strtol(strval,&endptr,0);
      
      if ((*endptr != '\0') 
          || (config->tooltip_hide_delay < MIN_TOOLTIP_HIDE_DELAY) 
	  || (config->tooltip_hide_delay > MAX_TOOLTIP_HIDE_DELAY) ){
	fprintf(stderr,
	  _NLS_("[%s] Warning, invalid delay '%s'. Must be an integer in \
range [%d..%d]. Default to: %d.\n"),
        get_option_name(TTIP_HIDE_DELAY,LONG_NAME,false,opt_name1,32),
	strval,
	MIN_TOOLTIP_HIDE_DELAY,
	MAX_TOOLTIP_HIDE_DELAY,
	DEFAULT_TOOLTIP_HIDE_DELAY);
	
	config->tooltip_hide_delay=DEFAULT_TOOLTIP_HIDE_DELAY;
      }
    break;
    
    case ICON_GRAYSCALE :
    case HOVER_ICON_GRAYSCALE :
      flag=str_to_bool(strval,
                       get_option_name(option_id,LONG_NAME,false,opt_name1,32),
		       false);
      
      if (option_id==ICON_GRAYSCALE) {
	config->icon_grayscale=flag;
      } else {
	config->hover_icon_grayscale=flag;
      }
    break;

    case FRAME:
      if (config->frame) {
	free_frame(config->frame);
	config->frame=NULL;
      }
	
      config->frame=parse_frame_str(
        strval,
	get_option_name(FRAME,LONG_NAME,false,opt_name1,32));
	
      update_icon_size(FRAME,
        get_option_name(FRAME,LONG_NAME,false,opt_name1,32));
    break;
    
    case ICON_BRIGHTNESS:
      config->icon_brightness=(float)strtof(strval,&endptr);
      
      if (*endptr != '\0') {
        fprintf(stderr,
	_NLS_("[%s] Warning, invalid value '%s'. Must be a float. Default to: %f.\n"),
	get_option_name(ICON_BRIGHTNESS,LONG_NAME,false,opt_name1,32),
	strval,
	DEFAULT_ICON_BRIGHTNESS);

	config->icon_brightness=DEFAULT_ICON_BRIGHTNESS;
      }
    break;

    case HOVER_ICON_BRIGHTNESS:
      config->hover_icon_brightness=(float)strtof(strval,&endptr);

      if (*endptr != '\0') {
        fprintf(stderr,
	_NLS_("[%s] Warning, invalid value '%s'. Must be a float. Default to: %f.\n"),
	get_option_name(HOVER_ICON_BRIGHTNESS,LONG_NAME,false,opt_name1,32),
	strval,
	DEFAULT_HOVER_ICON_BRIGHTNESS);

	config->hover_icon_brightness=DEFAULT_HOVER_ICON_BRIGHTNESS;
      }
    break;
    
    case ICON_CONTRAST:
      config->icon_contrast=(float)strtof(strval,&endptr);
      
      if (*endptr != '\0') {
        fprintf(stderr,
	_NLS_("[%s] Warning, invalid value '%s'. Must be a float. Default to: %f.\n"),
	get_option_name(ICON_CONTRAST,LONG_NAME,false,opt_name1,32),
	strval,
	DEFAULT_ICON_CONTRAST);

	config->icon_contrast=DEFAULT_ICON_CONTRAST;
      }
    break;

    case HOVER_ICON_CONTRAST:
      config->hover_icon_contrast=(float)strtof(strval,&endptr);

      if (*endptr != '\0') {
        fprintf(stderr,
	_NLS_("[%s] Warning, invalid value '%s'. Must be a float. Default to: %f.\n"),
	get_option_name(HOVER_ICON_CONTRAST,LONG_NAME,false,opt_name1,32),
	strval,
	DEFAULT_HOVER_ICON_CONTRAST);

	config->hover_icon_contrast=DEFAULT_HOVER_ICON_CONTRAST;
      }
    break;

    case ICON_GAMMA:
      config->icon_gamma=(float)strtof(strval,&endptr);
      
      if ( (*endptr != '\0') || (config->icon_gamma<MIN_ICON_GAMMA) ){
        fprintf(stderr,
	_NLS_("[%s] Warning, invalid value '%s'. Must be a float >= %f. Default to: %f.\n"),
	get_option_name(ICON_GAMMA,LONG_NAME,false,opt_name1,32),
	strval,
	MIN_ICON_GAMMA,
	DEFAULT_ICON_GAMMA);

	config->icon_gamma=DEFAULT_ICON_GAMMA;
      }
    break;

    case HOVER_ICON_GAMMA:
      config->hover_icon_gamma=(float)strtof(strval,&endptr);
      
      if ( (*endptr != '\0') || (config->hover_icon_gamma < MIN_HOVER_ICON_GAMMA) ){
        fprintf(stderr,
	_NLS_("[%s] Warning, invalid value '%s'. Must be a float >= %f. Default to: %f.\n"),
	get_option_name(HOVER_ICON_GAMMA,LONG_NAME,false,opt_name1,32),
	strval,
	MIN_HOVER_ICON_GAMMA,
	DEFAULT_HOVER_ICON_GAMMA);

	config->hover_icon_gamma=DEFAULT_HOVER_ICON_GAMMA;
      }
    break;
    
    case DOUBLE_CLICK:
      config->single_click       = false;
      config->double_click_delay = (int)strtol(strval,&endptr,0);
      
      if ((*endptr != '\0') 
           || (config->double_click_delay < MIN_DOUBLE_CLICK_DELAY) 
	   || (config->double_click_delay > MAX_DOUBLE_CLICK_DELAY) ) {
	fprintf(stderr,
	        _NLS_("[%s] Warning, invalid timing delay '%s'. Must be an integer\
in range [%d..%d]. Default to: %d.\n"), 
                get_option_name(DOUBLE_CLICK,LONG_NAME,false,opt_name1,32),
                strval,
		MIN_DOUBLE_CLICK_DELAY,
		MAX_DOUBLE_CLICK_DELAY,
		DEFAULT_DOUBLE_CLICK_DELAY);
	config->double_click_delay = DEFAULT_DOUBLE_CLICK_DELAY;
      }
    break;
        
                
    case DISPLAY:
      free(config->display);
      config->display = (strval) ? strdup(strval) : NULL;
    break;
    
    case BROKEN_WM:
      config->broken_wm=str_to_bool(strval,
                       get_option_name(option_id,LONG_NAME,false,opt_name1,32),
		       false);
    break;

    case EXIT_ON_RCLICK:
      config->exit_on_rclick=str_to_bool(strval,
                       get_option_name(option_id,LONG_NAME,false,opt_name1,32),
		       false);
    break;
  }
}

/* creates and returns a "key" from the specified 
 * prefix (optional) and name. The returned key is
 * to the format :
 * 
 * [prefix.]name
 * 
 * The returned key should be freed when it is no
 * longer needed.
 */
static char *create_key(char *prefix, char *name) {
  char *dest=NULL;
  
  if (prefix) {
    int slen=strlen(prefix)+1+strlen(name); /* +1 for the dot */
    dest=calloc(slen+1,sizeof(char)); /* +1 for the null terminate byte */
    sprintf(dest,"%s.%s",prefix,name);
  } else {
    dest=strdup(name);
  }
  return dest;
}

/* Loads the settings, with the given prefix when not NULL, found 
 * in the specified file and update the current application
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
bool load_config(char *filename, char *prefix, bool chk_missing) {
  
  char *command=NULL;
  char *icon_name=NULL;
  char *desktop_file=NULL;
  
  char *run_in_term=NULL;
  char *pref_terms=NULL;
    
  char *winsize=NULL;
  char *icon_padding=NULL;
  char *border_size=NULL;

  char *background_color=NULL;
  char *hover_background_color=NULL;

  char *border_color=NULL;
  char *hover_border_color=NULL;

  char *icon_search_path=NULL;
  char *desktop_file_search_path=NULL;
  char *font_search_path=NULL;

  char *tooltip_text=NULL;
  char *tooltip_font=NULL;
  char *tooltip_bg_color=NULL;
  char *tooltip_fg_color=NULL;
  char *tooltip_border_size=NULL;
  char *tooltip_text_padding=NULL;  
  char *tooltip_show_delay=NULL;
  char *tooltip_hide_delay=NULL;

  char *icon_grayscale=NULL;
  char *hover_icon_grayscale=NULL;

  char *frame=NULL;

  char *icon_brightness=NULL;
  char *hover_icon_brightness=NULL;
  
  char *icon_contrast=NULL;
  char *hover_icon_contrast=NULL;
  
  char *icon_gamma=NULL;
  char *hover_icon_gamma=NULL;
  
  char *double_click_delay=NULL;

  char *exit_on_right_click=NULL;
  
  char opt_name[32];
  
  t_key_value loaded_config[]={

    { create_key(prefix,get_option_name(COMMAND,LONG_NAME,false,opt_name,32)), &command, false },
    { create_key(prefix,get_option_name(ICON,LONG_NAME,false,opt_name,32)), &icon_name, false },
    { create_key(prefix,get_option_name(DESKTOP_FILE,LONG_NAME,false,opt_name,32)), &desktop_file, false },

    { create_key(prefix,get_option_name(RUN_IN_TERM,LONG_NAME,false,opt_name,32)), &run_in_term, false },
    { create_key(prefix,get_option_name(PREF_TERMS,LONG_NAME,false,opt_name,32)), &pref_terms, false },

    { create_key(prefix,get_option_name(WINDOW_SIZE,LONG_NAME,false,opt_name,32)), &winsize, false },
    { create_key(prefix,get_option_name(ICON_PADDING,LONG_NAME,false,opt_name,32)), &icon_padding, false },
    { create_key(prefix,get_option_name(BORDER_SIZE,LONG_NAME,false,opt_name,32)), &border_size, false },

    { create_key(prefix,get_option_name(BACKGROUND_COLOR,LONG_NAME,false,opt_name,32)), &background_color, false },
    { create_key(prefix,get_option_name(HOVER_BACKGROUND_COLOR,LONG_NAME,false,opt_name,32)), &hover_background_color, false },

    { create_key(prefix,get_option_name(BORDER_COLOR,LONG_NAME,false,opt_name,32)), &border_color, false },
    { create_key(prefix,get_option_name(HOVER_BORDER_COLOR,LONG_NAME,false,opt_name,32)), &hover_border_color, false },

    { create_key(prefix,get_option_name(ICON_SEARCH_PATH,LONG_NAME,false,opt_name,32)), &icon_search_path, false },
    { create_key(prefix,get_option_name(DESKTOP_FILE_SEARCH_PATH,LONG_NAME,false,opt_name,32)), &desktop_file_search_path, false},
    { create_key(prefix,get_option_name(FONT_SEARCH_PATH,LONG_NAME,false,opt_name,32)), &font_search_path, false},

    { create_key(prefix,get_option_name(TTIP_TEXT,LONG_NAME,false,opt_name,32)), &tooltip_text, false },
    { create_key(prefix,get_option_name(TTIP_FONT,LONG_NAME,false,opt_name,32)), &tooltip_font, false },
    { create_key(prefix,get_option_name(TTIP_BG_COLOR,LONG_NAME,false,opt_name,32)), &tooltip_bg_color, false },
    { create_key(prefix,get_option_name(TTIP_FG_COLOR,LONG_NAME,false,opt_name,32)), &tooltip_fg_color, false },
    { create_key(prefix,get_option_name(TTIP_BORDER_SIZE,LONG_NAME,false,opt_name,32)), &tooltip_border_size, false },
    { create_key(prefix,get_option_name(TTIP_TEXT_PADDING,LONG_NAME,false,opt_name,32)), &tooltip_text_padding, false },
    { create_key(prefix,get_option_name(TTIP_SHOW_DELAY,LONG_NAME,false,opt_name,32)), &tooltip_show_delay, false },
    { create_key(prefix,get_option_name(TTIP_HIDE_DELAY,LONG_NAME,false,opt_name,32)), &tooltip_hide_delay, false },

    { create_key(prefix,get_option_name(ICON_GRAYSCALE,LONG_NAME,false,opt_name,32)), &icon_grayscale, false },
    { create_key(prefix,get_option_name(HOVER_ICON_GRAYSCALE,LONG_NAME,false,opt_name,32)), &hover_icon_grayscale, false },

    { create_key(prefix,get_option_name(FRAME,LONG_NAME,false,opt_name,32)), &frame, false },

    { create_key(prefix,get_option_name(ICON_BRIGHTNESS,LONG_NAME,false,opt_name,32)), &icon_brightness, false },
    { create_key(prefix,get_option_name(HOVER_ICON_BRIGHTNESS,LONG_NAME,false,opt_name,32)), &hover_icon_brightness, false },
    { create_key(prefix,get_option_name(ICON_CONTRAST,LONG_NAME,false,opt_name,32)), &icon_contrast, false },
    { create_key(prefix,get_option_name(HOVER_ICON_CONTRAST,LONG_NAME,false,opt_name,32)), &hover_icon_contrast, false },
    { create_key(prefix,get_option_name(ICON_GAMMA,LONG_NAME,false,opt_name,32)), &icon_gamma, false },
    { create_key(prefix,get_option_name(HOVER_ICON_GAMMA,LONG_NAME,false,opt_name,32)), &hover_icon_gamma, false },

    { create_key(prefix,get_option_name(DOUBLE_CLICK,LONG_NAME,false,opt_name,32)), &double_click_delay, false },
    { create_key(prefix,get_option_name(EXIT_ON_RCLICK,LONG_NAME,false,opt_name,32)), &exit_on_right_click, false }
  };

  int sz_loaded_config=sizeof(loaded_config)/sizeof(t_key_value);
  
  t_command *cmd;
  
  char *key, *value, *optname;
  int   id;
  bool  load_ok = true;
  
  if ( !access(filename,F_OK|R_OK) ) {

    /* parse_config_file() is called with warn_redefines to true to
     * ensure that it outputs warning to STDERR when the configuration
     * file contains multiple definitions for the same key.
     */
    load_ok=parse_config_file(filename,loaded_config,sz_loaded_config,true);

    for (int i=0;i<sz_loaded_config && load_ok;i++) {
      key=(char *)loaded_config[i].key;
      value=*loaded_config[i].value;
    
      optname=key;
    
      if (prefix) {
      
         /* invariant: 
          * key starts with string 'prefix.' Therefore, the
	  * option starts at key+strlen(prefix)+1.
	  */

        optname+=strlen(prefix)+1;
      } 

      id=get_option_id(optname,false);

      if (id != NULL_OPTION) {
        if (!value) {
	  switch(id) {
	  
	    /* when chk_missing, it is required to check if the 
	     * options 'command' & 'icon' are set, unless when the
	     * option 'desktop-file' is set...
	     */
	  
	    case COMMAND:
	    case ICON:
	      if ( (chk_missing) && (!desktop_file) ) {
	        fprintf(stderr,
		_NLS_("Error, Expected '%s=<value>' is missing in configuration file %s\n"),
		key,
		filename);
		
	        load_ok=false;
	      }
	    break;
	  
	    default: /* required to avoid bunch of warning message at compile time.*/
	    break ;
	  }	
        } else {
	  switch(id) {
	    case COMMAND :
	      cmd=strcmd_to_command(value);
	      set_config_field(id,cmd);
	    break;
	  
	    default: 
	      set_config_field(id,value);
	    break;
	  }
        }
      }    
    }
  } else {
    load_ok = false;
    fprintf(stderr,
      _NLS_("Failed to load configuration file '%s'. Error #%d\n"),
      filename,
      errno);
  }
  
  /* loaded_config is now staled and must be freed.
   * 
   * Note that in the context of load_config(), the
   * value of members 'key' (of type const char*)
   * is not a string literal but a string allocated 
   * by create_key(), and thus needs to be freed.
   */
  for (int i=0;i<sz_loaded_config;i++) {
    free((void *)loaded_config[i].key);
    free(*loaded_config[i].value);
  }
  
  return load_ok;
}

/* Returns the generic key of the locale entry with name
 * 'name' used in application desktop files according to
 * the current locale, or NULL when LC_MESSAGES is set
 * to C.
 * 
 * The returned key has the following syntax :
 *   <name>[<LANG>]
 * 
 * It is up to the caller to free the memory used by
 * the returned key.
 * 
 * Note that function will returns always NULL if
 * the program has not been made portable to all
 * locale by calling setlocale(LC_ALL,"") on startup.
 */
static char *create_desktop_file_key(char *name) {
  char *key         = NULL;
  char *lc_messages = NULL; 
  char *lang        = NULL;
  char *stok_saveptr;
  int  len;

  lc_messages=setlocale(LC_MESSAGES,NULL);
  
  if ((!lc_messages) || (lc_messages[0]=='C')) {
    return NULL;
  }
  
  /* lc_messages format is: lang_COUNTRY.ENCODING@MODIFIER.
   * 
   * application desktop files might includes severals locale
   * comment key for the "same" language (ex. Comment[en],
   * Comment[en_US], ...). However, for simplicity purposes,
   * get_locale_comment_key() get rid of country, encoding and
   * modifier informations.
   */
  lang=strdup(strtok_r(lc_messages,"_",&stok_saveptr));
    
  len=strlen(name)+strlen(lang)+2; /* +2 for the brackets [] */
  key=calloc(len+1,sizeof(char)); /* +1 for the null terminate byte */
  strcat(key,name);
  strcat(key,"[");
  strcat(key,lang);
  strcat(key,"]");
  
  free(lang);
  
  return key;
}

/* Replaces the substring of 'nchar' characters starting at 'pos' in
 * 'source' by the string 'str'. when in_quotes is true, 'str' is placed
 * in double quotes.
 */
static char *replace_substring(char *source, int pos, int nchar, char *str, bool in_quotes) {
  
  char *dest;
  int   slen=strlen(source);
  
  /* llen : number of chars at the left of the insertion point.
   * rlen : number of chars at the right of the insertion point, minus the nchar to remove.
   * tlen : len (without null terminate byte) of destination string
   */
  int   llen=pos;
  int   rlen=slen-(pos+nchar);
  int   tlen=llen+strlen(str)+rlen;
  
  if (in_quotes) {
    /* +3=+2 for the quotes and +1 for the null terminate byte */
    dest=calloc(tlen+3,sizeof(char));
    strncpy(dest,source,llen);
    strcat(dest,"\"");
    strcat(dest,str);
    strcat(dest,"\"");
  } else {
    dest=calloc(tlen+1,sizeof(char));
  
    strncpy(dest,source,llen);
    strcat(dest,str);
  }
  
  if (rlen>0) {
    strcat(dest,source+pos+nchar);
  }
  return dest;
}

/* Loads the application desktop file 'filename' and updates accordingly
 * the following fields of the current application configuration:  
 * 'command', 'icon', 'tooltip_text', 'run_in_term'
 * 
 * Returns true on success, false otherwise.
 */
bool load_desktop_file(char *filename) {

  t_command *cmd;

  char *exec=NULL;
  char *icon=NULL;
  char *name=NULL;
  char *generic_name=NULL;
  char *comment=NULL;
  char *run_in_term=NULL;

  char *locale_name=NULL;
  char *locale_generic_name=NULL;
  char *locale_comment=NULL;
  int  len;
  int  pos;
  
  bool load_ok=true;
    
  /* In case of keys Name, GenericName and Comment, the function
   * create_desktop_file_key() returns the keys Name[<LANG>],
   * GenericName[<LANG>], and Comment[<LANG>], where LANG is equals
   * to the field lang of the current locale, unless when LC_MESSAGES
   * is set to C in which case these keys are set to NULL.
   */ 
  t_key_value desktop_entry[]={
    { strdup("Exec"), &exec, false },
    { strdup("Icon"), &icon, false },
    { strdup("Terminal"), &run_in_term, false },
    { strdup("Name"), &name, false },
    { strdup("GenericName"), &generic_name, false },
    { strdup("Comment"), &comment, false },
    { create_desktop_file_key("Name"),&locale_name, false },
    { create_desktop_file_key("GenericName"),&locale_generic_name, false },
    { create_desktop_file_key("Comment"),&locale_comment, false }
  };

  int sz_desktop_entry=sizeof(desktop_entry)/sizeof(t_key_value);

  if ( (filename) && (!access(filename,F_OK|R_OK)) ) {

    /* parse_config_file() is called with warn_redefines set to 
     * false to prevent it to output warnings to STDERR if the
     * desktop file contains multiple definition for the same
     * key.
     */
    load_ok=parse_config_file(filename,desktop_entry,sz_desktop_entry,false);
    
    if (load_ok) {
       char *caption = NULL;
  
      /* The value of exec key in application desktop file
       * might includes special field codes which starts 
       * with a % followed by an alpha character. These
       * field codes must be removed (to prevent them to
       * be wrongly passed to the application), with exception
       * to the code %c which must be superseded (otherwise the 
       * application will not start) by the value of variable 
       * 'locale_name' if not NULL, by the value of variable 'name' 
       * if not NULL, otherwise by the string '__'.
       */
      len = strlen(exec);
      pos = 0;
      
      if (locale_name) {
	caption=locale_name;
      }
        else
      if (name) {
	caption=name;
      }

      while (pos<len) {

        if ( (exec[pos]=='%') && (pos+1<len) && (isalpha(exec[pos+1])) ) {
	  if (exec[pos+1]=='c') {
	    
	    /* The code %c must be superseded by value of 'caption' (in 
	     * quotes to prevent cases where caption includes spaces) if
	     * not NULL, by '__' otherwise.
	     */
	    if (caption) {
	      char *new_exec = replace_substring(exec,pos,2,caption,true);
	      free(exec);
	      
	      exec = new_exec;
	      len  = strlen(exec);
	      pos += strlen(caption);
	    } else {
	      exec[pos]='_';
	      exec[pos+1]='_';
	    } 
	  } else {
            exec[pos]=' ';
            exec[pos+1]=' ';
	  }
        }
        pos++;
      }
      
      cmd=strcmd_to_command(exec);

      set_config_field(COMMAND,cmd);  
      set_config_field(ICON,icon);
      
      /* set run_in_term to the loaded value unless it is already set.
       * 
       * This is required to handle the case where option --run-in-terminal
       * is passed in argument (after --desktop-file or --config)
       */
      if (!config->run_in_term) {
        set_config_field(RUN_IN_TERM,run_in_term);
      }
      
      /* update tooltip-text to the loaded value unless it is already set.
       * 
       * This is required to handle the case where option --tooltip-text
       * is passed in argument (after --desktop-file or --config)
       */
      if (!config->tooltip_text) {
        if (locale_comment) {
          set_config_field(TTIP_TEXT,locale_comment);
        } 
          else 
        if (comment) {
          set_config_field(TTIP_TEXT,comment);
        }
          else
        if (locale_generic_name) {
          set_config_field(TTIP_TEXT,locale_generic_name);
        } 
          else
        if (generic_name) {
          set_config_field(TTIP_TEXT,generic_name);
        }
          else
        if (locale_name) {
          set_config_field(TTIP_TEXT,locale_name);
        } 
          else 
        if (name) {
          set_config_field(TTIP_TEXT,name);
        }
      }
    }
  } else {
    load_ok=false;
  }
  
  /* desktop_entryis now staled must be freed.
   * 
   * Note that in the context of load_desktop_file(), the
   * value of members 'key' (of type const char*)
   * is not a string literal but an allocated string, 
   * and thus needs to be freed.
   */
     
  for (int i=0;i<sz_desktop_entry;i++) {  
    free((void *)desktop_entry[i].key);
    free(*desktop_entry[i].value);
  }
  
  return load_ok;
}

/* Prints the current configuration on standard output.
 */
void print_config() {
  
  if (!config) {
    fprintf(stdout,
      _NLS_("print_config() : config is not initialized.\n"));
    return;
  }

  char *cmdstr=command_to_str(config->command);
  
  fprintf(stdout,"command                      : %s\n",cmdstr);  
  fprintf(stdout,"icon                         : %s\n",config->icon_name);
  fprintf(stdout,"desktop-file                 : %s\n",config->desktop_file);
  fprintf(stdout,"\n");

  fprintf(stdout,"run-in-terminal              : %d\n",config->run_in_term);
  fprintf(stdout,"preferred-terminals          : %s\n",config->pref_terms);
  fprintf(stdout,"\n");

  fprintf(stdout,"window-size                  : %d\n",config->winsize);
  fprintf(stdout,"icon-size                    : %d\n",config->icon_size);  
  fprintf(stdout,"icon-padding                 : %d\n",config->icon_padding);
  fprintf(stdout,"border-size                  : %d\n",config->border_size);
  fprintf(stdout,"\n");

  fprintf(stdout,"background-color             : %s\n",config->background_color);
  fprintf(stdout,"hover-background-color       : %s\n",config->hover_background_color);
  fprintf(stdout,"\n");

  fprintf(stdout,"border-color                 : %s\n",config->border_color);
  fprintf(stdout,"hover-border-color           : %s\n",config->hover_border_color);
  fprintf(stdout,"\n");

  fprintf(stdout,"icon-search-path : \n");  
  for (int i=0;i<config->icon_search_path->count;i++) {
    fprintf(stdout,"  %s\n",config->icon_search_path->pathnames[i]);
  }
  fprintf(stdout,"\n");

  fprintf(stdout,"desktop-file-search-path : \n");  
  for (int i=0;i<config->desktop_file_search_path->count;i++) {
    fprintf(stdout,"  %s\n",config->desktop_file_search_path->pathnames[i]);
  }
  fprintf(stdout,"\n");

  fprintf(stdout,"font-search-path : \n");  
  for (int i=0;i<config->font_search_path->count;i++) {
    fprintf(stdout,"  %s\n",config->font_search_path->pathnames[i]);
  }
  fprintf(stdout,"\n");

  fprintf(stdout,"tooltip-text                 : %s\n",config->tooltip_text);
  fprintf(stdout,"tooltip-font                 : %s\n",config->tooltip_font);
  fprintf(stdout,"tooltip-background-color     : %s\n",config->tooltip_bg_color);
  fprintf(stdout,"tooltip-foreground-color     : %s\n",config->tooltip_fg_color);
  fprintf(stdout,"tooltip-border-size          : %d\n",config->tooltip_border_size);
  fprintf(stdout,"tooltip-text-padding         : %d\n",config->tooltip_text_padding);
  fprintf(stdout,"tooltip-show_delay           : %d\n",config->tooltip_show_delay);
  fprintf(stdout,"tooltip-hide_delay           : %d\n",config->tooltip_hide_delay);
  fprintf(stdout,"\n");

  fprintf(stdout,"icon-grayscale               : %d\n",config->icon_grayscale);
  fprintf(stdout,"hover-icon-grayscale         : %d\n",config->hover_icon_grayscale);
  fprintf(stdout,"\n");

  if (config->frame) {
    fprintf(stdout,"frame                        : %s/%s:%s/%s:%d\n",
      config->frame->hicolor,
      config->frame->hover_hicolor,
      config->frame->locolor,
      config->frame->hover_locolor,
      config->frame->thickness);
    
  } else {    
    fprintf(stdout,"frame                        : disabled\n");
  }
  fprintf(stdout,"\n");

  fprintf(stdout,"icon-brightness              : %f\n",config->icon_brightness);
  fprintf(stdout,"hover-icon-brightness        : %f\n",config->hover_icon_brightness);
  fprintf(stdout,"\n");
  
  fprintf(stdout,"icon-contrast                : %f\n",config->icon_contrast);
  fprintf(stdout,"hover-icon-contrast          : %f\n",config->hover_icon_contrast);
  fprintf(stdout,"\n");
  
  fprintf(stdout,"icon-gamma                   : %f\n",config->icon_gamma);
  fprintf(stdout,"hover-icon-gamma             : %f\n",config->hover_icon_gamma);
  fprintf(stdout,"\n");

  if (config->single_click) {
    fprintf(stdout,"double-click                 : disabled\n");
  } else {
    fprintf(stdout,"double-click                 : enabled (%dms)\n",config->double_click_delay);
  }
  
  fprintf(stdout,"display                      : %s\n",config->display);
  fprintf(stdout,"broken-wm                    : %d\n",config->broken_wm);
  fprintf(stdout,"exit-on-right-click          : %d\n",config->exit_on_rclick);
  fprintf(stdout,"\n");
  
  free(cmdstr);
}
