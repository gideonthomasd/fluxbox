/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "nls.h"
#include "settings.h"
#include "wmalhelp.h"
#include "config.h"
#include "wmutil.h"


/* Prints help summary.
 */
void print_help_summary() {
  char opt_name1[32];
  char opt_name2[32];

  fprintf(stdout,_NLS_("Usage: wmalauncher %s\n"),
          get_option_name(HELP,LONG_NAME,true,opt_name1,32));
	  
  fprintf(stdout,_NLS_("       wmalauncher %s [<PATH>[:<PATH>...]]\n"),
          get_option_name(FONTS_INFO,LONG_NAME,true,opt_name1,32));

  fprintf(stdout,_NLS_("       wmalauncher %s <FILE> [%s <PREFIX>] [OPTION]...\n"),
          get_option_name(CONFIG,LONG_NAME,true,opt_name1,32),
	  get_option_name(PREFIX,LONG_NAME,true,opt_name2,32));
	  
  fprintf(stdout,_NLS_("       wmalauncher %s <CMD> %s <ICON> [OPTION]...\n"),
          get_option_name(COMMAND,LONG_NAME,true,opt_name1,32),
	  get_option_name(ICON,LONG_NAME,true,opt_name2,32));

  fprintf(stdout,_NLS_("       wmalauncher %s <FILE>[:<FILE>...] [OPTION]...\n"),
          get_option_name(DESKTOP_FILE,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout, "\n");
  
  fprintf(stdout,_NLS_("[OPTION] can be :\n"));

  fprintf(stdout,"  [%s] ",get_option_name(RUN_IN_TERM,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <TERM>[:<TERM>...]]\n"),get_option_name(PREF_TERMS,LONG_NAME,true,opt_name1,32));

  fprintf(stdout,_NLS_("  [%s <SIZE>] "),get_option_name(WINDOW_SIZE,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <SIZE>] "),get_option_name(ICON_PADDING,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <SIZE>]\n"),get_option_name(BORDER_SIZE,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <COLOR|GRADIENT>] "),get_option_name(BACKGROUND_COLOR,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <COLOR|GRADIENT>]\n"),get_option_name(HOVER_BACKGROUND_COLOR,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <COLOR|GRADIENT>] "),get_option_name(BORDER_COLOR,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <COLOR|GRADIENT>]\n"),get_option_name(HOVER_BORDER_COLOR,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <PATH>[:<PATH>...]]\n"),get_option_name(ICON_SEARCH_PATH,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("  [%s <PATH>[:<PATH>...]]\n"),get_option_name(DESKTOP_FILE_SEARCH_PATH,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("  [%s <PATH>[:<PATH>...]]\n"),get_option_name(FONT_SEARCH_PATH,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <TEXT>] "),get_option_name(TTIP_TEXT,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <FONT/SIZE>]\n"),get_option_name(TTIP_FONT,LONG_NAME,true,opt_name1,32));  
  
  fprintf(stdout,_NLS_("  [%s <COLOR|GRADIENT>] "),get_option_name(TTIP_BG_COLOR,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <COLOR>]\n"),get_option_name(TTIP_FG_COLOR,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <SIZE>] "),get_option_name(TTIP_BORDER_SIZE,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <SIZE>]\n"),get_option_name(TTIP_TEXT_PADDING,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <DELAY>] "),get_option_name(TTIP_SHOW_DELAY,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <DELAY>]\n"),get_option_name(TTIP_HIDE_DELAY,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,"  [%s] ",get_option_name(ICON_GRAYSCALE,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,"[%s]\n",get_option_name(HOVER_ICON_GRAYSCALE,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <HICOLOR>/<HOVER_HICOLOR>:<LOCOLOR>/<HOVER_LOCOLOR>[:SIZE]]\n"),
          get_option_name(FRAME,LONG_NAME,true,opt_name1,32));
	  
  fprintf(stdout,_NLS_("  [%s <VALUE>] "),get_option_name(ICON_BRIGHTNESS,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <VALUE>]\n"),get_option_name(HOVER_ICON_BRIGHTNESS,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <VALUE>] "),get_option_name(ICON_CONTRAST,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <VALUE>]\n"),get_option_name(HOVER_ICON_CONTRAST,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,_NLS_("  [%s <VALUE>] "),get_option_name(ICON_GAMMA,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <VALUE>]\n"),get_option_name(HOVER_ICON_GAMMA,LONG_NAME,true,opt_name1,32));
    
  fprintf(stdout,_NLS_("  [%s [<DELAY>]] "),get_option_name(DOUBLE_CLICK,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,_NLS_("[%s <NAME>] "),get_option_name(DISPLAY,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,"[%s] ",get_option_name(BROKEN_WM,LONG_NAME,true,opt_name1,32));
  fprintf(stdout,"[%s] ",get_option_name(EXIT_ON_RCLICK,LONG_NAME,true,opt_name1,32));
  
  fprintf(stdout,"\n");

}

/* Prints help page.
 */
void print_help() {
  
  char opt_name1[32];
  char opt_name2[32];
  char opt_name3[32];

  print_help_summary();

  fprintf(stdout, "\n");
  
  fprintf(stdout, 
_NLS_("%s\n\
  Prints this help page.\n\n"),
  get_option_name(HELP,BOTH_NAMES,true,opt_name1,32));
  
  fprintf(stdout,
_NLS_("%s [<PATH>[:<PATH>...]]\n\
  Prints information on (default) supported fonts and on fonts accessible\n\
  from the given path(s), if any.\n\n"),
  get_option_name(FONTS_INFO,BOTH_NAMES,true,opt_name1,32));
  
  fprintf(stdout,
_NLS_("%s <FILE> [%s <PREFIX>]\n\
  Loads the configuration from the file <FILE>, then updates the settings\n\
  for which there are entries with the prefix <PREFIX>, if any.\n\n"),
  get_option_name(CONFIG,BOTH_NAMES,true,opt_name1,32),
  get_option_name(PREFIX,BOTH_NAMES,true,opt_name2,32));
  
  
  fprintf(stdout,
_NLS_("%s <CMD> %s <ICON>\n\
  Sets the command to execute to <CMD> and the icon to display to <ICON>.\n\n"),
  get_option_name(COMMAND,BOTH_NAMES,true,opt_name1,32),
  get_option_name(ICON,BOTH_NAMES,true,opt_name2,32));
  
  
  fprintf(stdout,
_NLS_("%s <FILE>[:<FILE>...]\n\
  Loads the 1st .desktop file that matches any of the specified (path)names,\n\
  then setups the command to execute, the icon to display and the tooltip text\n\
  accordingly.\n\n"),
  get_option_name(DESKTOP_FILE,BOTH_NAMES,true,opt_name1,32));
  
  
  fprintf(stdout,
_NLS_("%s\n\
  Activates execution of the command/application in a terminal window (see\n\
  option %s).\n\n"),
  get_option_name(RUN_IN_TERM,BOTH_NAMES,true,opt_name1,32),
  get_option_name(PREF_TERMS,LONG_NAME,true,opt_name2,32));

  fprintf(stdout,
_NLS_("%s <TERM>[:<TERM>...]\n\
  Sets the list of preferred terminal emulators. The 1st available terminal\n\
  in this list is used to run the command/application when (1) the option\n\
  %s is passed in argument, (2) the desktop file passed to\n\
  option %s includes an entry 'Terminal=true'.\n\
\n\
  By default, the list of preferred terminal emulators is set to :\n\n\
    %s.\n\n"),
  get_option_name(PREF_TERMS,BOTH_NAMES,true,opt_name1,32),
  get_option_name(RUN_IN_TERM,LONG_NAME,true,opt_name2,32),
  get_option_name(DESKTOP_FILE,LONG_NAME,true,opt_name3,32),
  WMAL_PREFERRED_TERMINALS);
  
  fprintf(stdout,
_NLS_("%s <SIZE>\n\
  Sets the window size to <SIZE> x <SIZE>. Default to %d.\n\n"),
  get_option_name(WINDOW_SIZE,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_WIN_SIZE);


  fprintf(stdout,
_NLS_("%s <SIZE>\n\
  Sets the padding size around the icon to <SIZE>. Default to %d.\n\n"),
  get_option_name(ICON_PADDING,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_ICON_PADDING);
  
  fprintf(stdout,
_NLS_("%s <SIZE>\n\
  Sets the window border size to <SIZE>. Default to %d.\n\n"),
  get_option_name(BORDER_SIZE,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_BORDER_SIZE);
  
  fprintf(stdout,
_NLS_("%s <COLOR|GRADIENT>\n\
  Sets the background color to the plain color <COLOR> or to the color gradient\n\
  <GRADIENT>. Default to the plain color %s.\n\n"),
  get_option_name(BACKGROUND_COLOR,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_BACKGROUND_COLOR);
  
  fprintf(stdout,
_NLS_("%s <COLOR|GRADIENT>\n\
  Sets the background color on mouse hovers to the plain color <COLOR> or to the\n\
  color gradient <GRADIENT>. Default to the plain color %s.\n\n"),
  get_option_name(HOVER_BACKGROUND_COLOR,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_HOVER_BACKGROUND_COLOR);
  
  fprintf(stdout,
_NLS_("%s <COLOR|GRADIENT>\n\
  Sets the window border color to the plain color <COLOR> or to the color gradient\n\
  <GRADIENT>. Default to the plain color %s.\n\n"),
  get_option_name(BORDER_COLOR,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_BORDER_COLOR);
  
  fprintf(stdout,
_NLS_("%s <COLOR|GRADIENT>\n\
  Sets the window border color on mouse hovers to the plain color <COLOR> or to the\n\
  color gradient <GRADIENT>. Default to the plain color %s.\n\n"),
  get_option_name(HOVER_BORDER_COLOR,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_HOVER_BORDER_COLOR);
  
  fprintf(stdout,
_NLS_("%s <PATH>[:<PATH>...]\n\
  Prepends the given path(s) to the icons search path.\n\n"),
  get_option_name(ICON_SEARCH_PATH,BOTH_NAMES,true,opt_name1,32));
  
  fprintf(stdout,
_NLS_("%s <PATH>[:<PATH>...]\n\
  Prepends the given path(s) to the \"desktop files\" search path.\n\n"),
  get_option_name(DESKTOP_FILE_SEARCH_PATH,BOTH_NAMES,true,opt_name1,32));

  fprintf(stdout,
_NLS_("%s <PATH>[:<PATH>...]\n\
  Prepends the given path(s) to the fonts search path.\n\n"),
  get_option_name(FONT_SEARCH_PATH,BOTH_NAMES,true,opt_name1,32));
  
  fprintf(stdout,
_NLS_("%s <TEXT>\n\
  Sets the tooltip text to <TEXT>.\n\n"),
  get_option_name(TTIP_TEXT,BOTH_NAMES,true,opt_name1,32));
  
  fprintf(stdout,
_NLS_("%s <FONT/SIZE>\n\
  Sets the tooltip font to <FONT> with a size of <SIZE>. Default to %s.\n\n"),
  get_option_name(TTIP_FONT,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_TOOLTIP_FONT);
  
  fprintf(stdout,
_NLS_("%s <COLOR|GRADIENT>\n\
  Sets the tooltip background color to the plain color <COLOR> or to the color\n\
  gradient <GRADIENT>. Default to the plain color %s.\n\n"),
  get_option_name(TTIP_BG_COLOR,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_TOOLTIP_BACKGROUND_COLOR);
  
  fprintf(stdout,
_NLS_("%s <COLOR>\n\
  Sets the tooltip foreground color to <COLOR>. Default to %s.\n\n"),
  get_option_name(TTIP_FG_COLOR,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_TOOLTIP_FOREGROUND_COLOR);
  
  fprintf(stdout,
_NLS_("%s <SIZE>\n\
  Sets the tooltip border size to <SIZE>. Default to %d.\n\n"),
  get_option_name(TTIP_BORDER_SIZE,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_TOOLTIP_BORDER_SIZE);
  
  fprintf(stdout,
_NLS_("%s <SIZE>\n\
  Sets the padding size around the tooltip text to <SIZE>. Default to %d.\n\n"),
  get_option_name(TTIP_TEXT_PADDING,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_TOOLTIP_TEXT_PADDING);
  
  fprintf(stdout,
_NLS_("%s <DELAY>\n\
  Sets the tooltip show timing delay to <DELAY> ms. Default to %d ms.\n\n"),
  get_option_name(TTIP_SHOW_DELAY,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_TOOLTIP_SHOW_DELAY);

  fprintf(stdout,
_NLS_("%s <DELAY>\n\
  Sets the tooltip hide timing delay to <DELAY> ms. Default to %d ms.\n\n"),
  get_option_name(TTIP_HIDE_DELAY,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_TOOLTIP_HIDE_DELAY);

  fprintf(stdout,
_NLS_("%s\n\
  Displays the icon in grayscale mode.\n\n"),
  get_option_name(ICON_GRAYSCALE,BOTH_NAMES,true,opt_name1,32));

  fprintf(stdout,
_NLS_("%s\n\
  Displays the icon in grayscale mode on mouse hovers.\n\n"),
  get_option_name(HOVER_ICON_GRAYSCALE,BOTH_NAMES,true,opt_name1,32));

  fprintf(stdout,
_NLS_("%s <HICOLOR>/<HOVER_HICOLOR>:<LOCOLOR>/<HOVER_LOCOLOR>[:SIZE]\n\
  Draws a frame with a thickness of <SIZE> pixels around the icon. The top and left\n\
  borders are painted using color <HICOLOR>, while the bottom and right borders are\n\
  painted using color <LOCOLOR>. On mouse hovers, the colors <HOVER_HICOLOR> and\n\
  <HOVER_LOCOLOR> are used instead. When the left mouse button is pressed, the \n\
  HI/LO colors are swapped, unless when option %s is used.\
  \n\n\
  Default to: %s/%s:%s/%s:%d\n\n"),
  get_option_name(FRAME,BOTH_NAMES,true,opt_name1,32),
  get_option_name(DOUBLE_CLICK,BOTH_NAMES,true,opt_name2,32),
  DEFAULT_FRAME_HICOLOR,
  DEFAULT_FRAME_HOVER_HICOLOR,
  DEFAULT_FRAME_LOCOLOR,
  DEFAULT_FRAME_HOVER_LOCOLOR,
  DEFAULT_FRAME_THICKNESS);

  fprintf(stdout,
_NLS_("%s <VALUE>\n\
  Sets the icon brightness to <VALUE>. Default to %f.\n\n"),
  get_option_name(ICON_BRIGHTNESS,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_ICON_BRIGHTNESS);

  fprintf(stdout,
_NLS_("%s <VALUE>\n\
  Sets the icon brightness on mouse hovers to <VALUE>. Default to %f.\n\n"),
  get_option_name(HOVER_ICON_BRIGHTNESS,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_HOVER_ICON_BRIGHTNESS);
  
  fprintf(stdout,
_NLS_("%s <VALUE>\n\
  Sets the icon contrast to <VALUE>. Default to %f.\n\n"),
  get_option_name(ICON_CONTRAST,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_ICON_CONTRAST);

  fprintf(stdout,
_NLS_("%s <VALUE>\n\
  Sets the icon contrast on mouse hovers to <VALUE>. Default to %f.\n\n"),
  get_option_name(HOVER_ICON_CONTRAST,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_HOVER_ICON_CONTRAST);
  

  fprintf(stdout,
_NLS_("%s <VALUE>\n\
  Sets the icon gamma to <VALUE>. Default to %f.\n\n"),
  get_option_name(ICON_GAMMA,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_ICON_GAMMA);

  fprintf(stdout,
_NLS_("%s <VALUE>\n\
  Sets the icon gamma on mouse hovers to <VALUE>. Default to %f.\n\n"),
  get_option_name(HOVER_ICON_GAMMA,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_HOVER_ICON_GAMMA);
  

  fprintf(stdout,
_NLS_("%s [<DELAY>]\n\
  Enables option \"double-click to run the command\" and sets the double-click\n\
  timing delay to <DELAY> (Default to %d ms), if any.\n\n"),
  get_option_name(DOUBLE_CLICK,BOTH_NAMES,true,opt_name1,32),
  DEFAULT_DOUBLE_CLICK_DELAY);

  fprintf(stdout,
_NLS_("%s <NAME>\n\
  Sets the display to use to <NAME>.\n\n"),
  get_option_name(DISPLAY,BOTH_NAMES,true,opt_name1,32));

  fprintf(stdout,
_NLS_("%s\n\
  Enables fix for \"broken\" window managers.\n\n"),
  get_option_name(BROKEN_WM,BOTH_NAMES,true,opt_name1,32));

  fprintf(stdout,
_NLS_("%s\n\
  Terminates this wmalauncher instance on right click.\n\n"),
  get_option_name(EXIT_ON_RCLICK,BOTH_NAMES,true,opt_name1,32));

  fprintf(stdout,_NLS_("For more detailed informations, read the man page :\n\n\
\t$ man wmalauncher\n\n"));

}

/* Prints informations about supported fonts.
 */
void print_fonts_info(char *extra_font_paths) {
  char        **fonts;
  int           fcount;
  int           clen;
  int           csize=-1;
  
  if (! open_connection(NULL,WMAL_FONT_SEARCH_PATH) ) {
    fprintf(stderr,
      _NLS_("Failed to open display '%s'\n"),XDisplayName(NULL));
  }

  if (extra_font_paths) {
    char         *stok_saveptr;
    
    t_path_list *path_list = new_path_list();
    char         *cpath     = strtok_r(extra_font_paths,":",&stok_saveptr);
    int           pos=0;
    
    while (cpath) {
      add_path(path_list,cpath,NULL,pos++);
      cpath=strtok_r(NULL,":",&stok_saveptr);
    }
    
    add_font_path_to_imlib_path(path_list);
    free_path_list(path_list);
  }

  fprintf(stdout,_NLS_("Font paths :\n"));
  print_imlib_font_path("    ");
  fprintf(stdout,"\n");

  fonts=imlib_list_fonts(&fcount);

  fprintf(stdout,_NLS_("%d fonts found :\n\n"),fcount);
  
  /* Prints the fonts on two columns. For this
   * it is first needed to find the maximum
   * column width...
   */

  for (int i=0;i<fcount;i++) {
    clen=strlen(fonts[i]);
    if ( clen > csize ) {
      csize=clen;
    }
  }
  
  for (int i=0;i<fcount;i++) {
    fprintf(stdout,"  %-*s",csize,fonts[i]);
    if ( (i+1)<fcount ) {
      fprintf(stdout,"    %-*s",csize,fonts[i+1]);
    }
    fprintf(stdout,"\n");
    free(fonts[i]);
  }
  fprintf(stdout,"\n");

  free(fonts);

  close_connection(NULL);
}
