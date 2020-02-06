/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <config.h>

#include "nls.h"
#include "cmdparser.h"
#include "command.h"
#include "settings.h"
#include "wmalhelp.h"
#include "fileutil.h"
#include "wmutil.h"

          /*** 
           *    Functions    *
			   ***/
			   
/* Parses the option --config <FILE> [--prefix <PREFIX>] which 
 * starts at cindex in argv.
 * 
 * Returns true on success in which case cindex point the next 
 * argument to handle, false otherwise in which case exit_code
 * is the exit-code that must be returned by main().
 */
static bool parse_arg_config(int *cindex, int argc, char *argv[], int *exit_code) {
  int cidx=*cindex+1;
  
  if ( (cidx==argc) || (get_option_id(argv[cidx],true)!=NULL_OPTION) ) {
    fprintf(stderr,
      _NLS_("%s: Argument 'FILE' is missing.\n"),
      argv[*cindex]);
      
    *exit_code=1;
    return false;
  }
    
  char *cfgfile = resolve_pathname(argv[cidx++]);
  char *prefix  = NULL;
   
  /*
   * Handle optional argument -p|--prefix, when present
   */      
  if (get_option_id(argv[cidx],1)==PREFIX) {

    if ( (++cidx==argc) || (get_option_id(argv[cidx],true)!=NULL_OPTION) ) {
      free(cfgfile);
      
      fprintf(stderr,
        _NLS_("%s: Argument 'PREFIX' is missing.\n"),
        argv[cidx-1]);
	
      *exit_code=1;
      return false;
    }
    prefix=argv[cidx++];
  }
    
  /* Load the (default) configuration (missing values are
   * checked when prefix==NULL)...
   */
  if ( !load_config(cfgfile,NULL, (prefix==NULL)) ) {
    free(cfgfile);
    *exit_code=1;
    return false;
  }
        
  /* Updates the configuration using the settings with the given 
   * prefix, if any.
   */
  if ( (prefix) && (!load_config(cfgfile,prefix,true)) ) {
    free(cfgfile);
    *exit_code=1;    
    return false;
  }
  
  free(cfgfile);

  *cindex=cidx;
  return true;
}

/* Parses the option --command <CMD> which starts at cindex in 
 * argv[] and ends at option with identifier next_option_id, or at 
 * argc when there is no option with the given identifier.
 * 
 * Returns true on success in which case cindex points the next
 * argument to handle, false otherwise in which case exit_code
 * is the exit-code that must be returned by main().
 */
static bool parse_arg_command(int         *cindex,
                              int           argc,
                              char         *argv[], 
                              t_option_id  next_option_id,
			      int          *exit_code) {
  t_command *command = new_command();
  int         carg;
  
  for (carg=*cindex+1;carg<argc;carg++) {
    /* breaks the loop if the current argument is the
     * one which must follow the arguments after option
     * --command, i.e with identifier 'next_option_id'.
     */
    if (get_option_id(argv[carg],true)==next_option_id) {
      break;
    }

    add_argument(command,strdup(argv[carg]));
  }   
    
  if (command->argc==0) {
    free_command(command);

    fprintf(stderr,
      _NLS_("%s: Expected 'CMD' is missing.\n"),
      argv[*cindex]);
      
    *cindex    = carg;
    *exit_code = 1;

    return false;
  }
    
  set_config_field(COMMAND,command);

  *cindex=carg;
  return true;
}

/* Parses the optional arguments starting at cindex in argv.
 * 
 * Returns true on success in which case cindex point the next
 * argument to handle, false otherwise in which case exit_code
 * is the exit-code that must ber returned by main().
 */
static bool parse_args_optional(int *cindex, int argc, char *argv[], int *exit_code) {
  int           cidx        = *cindex;
  t_option_id  option_id   = NULL_OPTION;
  char         *value       = NULL;
  char         *missing_arg = NULL;
  char         *carg        = NULL;
  char          temp[256]    = {0};
  
  while (cidx<argc) {
    carg      = argv[cidx];
    option_id = get_option_id(argv[cidx++],true);
    value     = NULL;
          
    /* Exit with an error in case of unrecognized option */
    
    if (option_id==NULL_OPTION) {
      fprintf(stderr,
        _NLS_("Unrecognized option '%s'.\n"),
	carg);
	
      *cindex=cidx-1;
      *exit_code=1;
      return false;
    }
    
    /* 1. Handles the options :
     *  * for which no value has to be passed on the command 
     *    line (--icon-grayscale, ...)
     * 
     *  * for which an *optional value* can be passed on the 
     *    command line : --double-click...
     * 
     * 2. Exit with an error message in case of unexpected arguments.
     */

    switch (option_id) {
      case RUN_IN_TERM:      
      case ICON_GRAYSCALE:
      case HOVER_ICON_GRAYSCALE:
      case BROKEN_WM:
      case EXIT_ON_RCLICK:
        set_config_field(option_id,"true");
        continue;
      break;
      
      case DOUBLE_CLICK :
        /* If there's something after the option DOUBLE_CLICK which
	 * is not a recognized option, this must be the value of the
	 * double-click timing delay to use. Otherwise, the default
	 * double-click timing delay is used.
	 */
        if ( (cidx<argc) && (get_option_id(argv[cidx],true)==NULL_OPTION) ) {
	  value=argv[cidx++];
	} else {
	  snprintf(temp,256,"%d",DEFAULT_DOUBLE_CLICK_DELAY);
	  value=temp;
	}
	set_config_field(DOUBLE_CLICK,value);
	
	continue;
      break;
      
      case HELP:
      case FONTS_INFO:
      case COMMAND:
      case ICON:
      case CONFIG:
      case PREFIX:
      case DESKTOP_FILE:
        fprintf(stderr,
	  _NLS_("Unexpected argument '%s'\n"),
	  carg);
	  
        *cindex=cidx-1;
        *exit_code=1;
        return false;
      break;
		
      default: /* required to avoid bunch of warning messages at compile time */
      break;
    }
	
      /* invariant: 
       *   argv[cidx] *must be* the value attached to the current
       *   option.
       */
	
    if (cidx<argc) {
      value=argv[cidx++];
    }

    /* Notify user he missed something when no value is attached
     * to the current arg, or if the given value is a valid option.
     */

    if ( (!value) || (get_option_id(value,true)!=NULL_OPTION) ) {
      switch(option_id) {
	case DISPLAY : missing_arg=_NLS_("<NAME>") ; break;
	
	case PREF_TERMS: missing_arg=_NLS_("<TERM>:[<TERM>...]") ; break;
	
        case ICON_SEARCH_PATH         :
	case DESKTOP_FILE_SEARCH_PATH :
	case FONT_SEARCH_PATH         : missing_arg=_NLS_("<PATH>[:<PATH>...]") ; break;

        case WINDOW_SIZE       :
        case ICON_PADDING      : 
	case BORDER_SIZE       :
        case TTIP_BORDER_SIZE  :
        case TTIP_TEXT_PADDING : missing_arg=_NLS_("<SIZE>") ; break;
  
        case TTIP_TEXT         : missing_arg=_NLS_("<TEXT>") ; break; 

        case BACKGROUND_COLOR       :
        case HOVER_BACKGROUND_COLOR :
        case BORDER_COLOR           :
        case HOVER_BORDER_COLOR     : 
        case TTIP_BG_COLOR          : missing_arg=_NLS_("<COLOR|GRADIENT>"); break; 
	
        case TTIP_FG_COLOR          : missing_arg=_NLS_("<COLOR>"); break; 
    
        case TTIP_FONT         : missing_arg=_NLS_("<FONT/SIZE>"); break;
    
        
	case TTIP_SHOW_DELAY   :
        case TTIP_HIDE_DELAY   : missing_arg=_NLS_("<DELAY>"); break;
    
        case ICON_BRIGHTNESS        :
        case HOVER_ICON_BRIGHTNESS  :
        case ICON_CONTRAST          :
        case HOVER_ICON_CONTRAST    : 
        case ICON_GAMMA             :
        case HOVER_ICON_GAMMA       : missing_arg=_NLS_("<VALUE>"); break;
	
	case FRAME: missing_arg=_NLS_("<HICOLOR>/<HOVER_HICOLOR>:<LOCOLOR>/<HOVER_LOCOLOR>[:<SIZE>]") ; break;

        default: /* required to avoid bunch of warning messages at compile time */
        break;
      }
      
      fprintf(stderr,
        _NLS_("%s: Argument '%s' is missing.\n"),
	carg,
	missing_arg);
	
      *cindex=cidx-1;
      *exit_code=1;
      return false;
    }
      
    set_config_field(option_id,value);
  }
  
  *cindex=cidx;
  return true;
}

/* Parses the given command line (as an arrray of strings) and updates
 * the application configuration accordingly.
 * 
 * Returns true if the application can continue, false otherwise
 * in which case exit_code is the exit code that must be returned
 * by main().
 */
bool parse_command_line(int argc, char *argv[], int *exit_code) {
 
  char       opt_name1[32];
  
  t_config    *config = get_config();
  t_option_id  option_id;
  int           index=1;
  char         *extra_font_path=NULL;
  
  if (argc==1) {
    print_help_summary();
    return false;
  }
  
  /* invariant: 
   *   argc > 1
   *   option_id can be HELP|FONTS_INFO|COMMAND|CONFIG|DESKTOP_FILE
   */
  option_id=get_option_id(argv[index],true);

  switch(option_id) {
  
    case HELP:
      print_help();
      *exit_code=0;
      return false;
    break;
    
    case FONTS_INFO:      
      if (++index<argc) {
	extra_font_path=argv[index];
      }
      print_fonts_info(extra_font_path);
      *exit_code=0;
      return false;
    break;
   
    case CONFIG:
      if (!parse_arg_config(&index,argc,argv,exit_code)) {
	return false;
      }
    break;

    case COMMAND:
      if (!parse_arg_command(&index,argc,argv,ICON,exit_code) ) {
	return false;
      }
      
        /*
	 * The next argument must be -i|--icon
	 */
	 
      if ( (index==argc) || (get_option_id(argv[index],true)!=ICON) ) {
        fprintf(stderr,
	  _NLS_("Argument '%s' is missing.\n"),
	  get_option_name(ICON,BOTH_NAMES,true,opt_name1,32));
        *exit_code=1;
        return false;
      }
      
      if ( (++index==argc) || (get_option_id(argv[index],true)!=NULL_OPTION) ) {
        fprintf(stderr,
	  _NLS_("%s: Argument 'NAME' is missing.\n"),
	  argv[index-1]);
	  
        *exit_code=1;
        return false;
      }
      set_config_field(ICON,argv[index++]);
    break;
    
    case DESKTOP_FILE:
      if ( (++index==argc) || (get_option_id(argv[index],true)!=NULL_OPTION) ) {
        fprintf(stderr,
	  _NLS_("%s: Argument 'FILE' is missing.\n"),
	  argv[index-1]);
	  
        *exit_code=1;
        return false;
      }
      set_config_field(DESKTOP_FILE,argv[index++]);
    break;
    
    default:
      if (option_id==NULL_OPTION) {
        fprintf(stderr,
	  _NLS_("Unrecognized option '%s'.\n"),
	  argv[index]);
	  
        *exit_code=1;
      } else {
        fprintf(stderr,
	  _NLS_("Unexpected option '%s'.\n"),
	  argv[index]);
	  
        *exit_code=1;
      }
      return false;
    break;
  }
  
  if ( !parse_args_optional(&index,argc,argv,exit_code) )  {
    return false;
  }
  
  if (config->desktop_file) {
    bool  df_loaded   = false;
    char *pathname    = NULL;
    char *stok_savptr = NULL;
    char *dfnames     = strdup(config->desktop_file);
    char *cname       = strtok_r(dfnames,":",&stok_savptr);
    int   ncount      = 0;
    
    while ((cname) && (!df_loaded)) {
      ncount++;
      
      if ( is_absolute_path(cname) ) {
	pathname=strdup(cname);
      } else {
        if (ends_with_extension(cname,DESKTOP_FILE_EXT)) {
          pathname=search_file(config->desktop_file_search_path,cname);
        } else {
	  pathname=search_file_ext(config->desktop_file_search_path,
	                           cname,
				   DESKTOP_FILE_EXT);
	}
      }
      
      #ifdef DEBUG
        fprintf(stdout,"Loading desktop-file %s\n",pathname);
      #endif
      
      df_loaded=load_desktop_file(pathname);
      free(pathname);

      cname=strtok_r(NULL,":",&stok_savptr);
    }

    free(dfnames);

    if (!df_loaded) {
      if (ncount==1) {
        fprintf(stderr,
	  _NLS_("Error, .desktop file '%s' does not exist.\n"),
	  config->desktop_file);	
      } else {
	fprintf(stderr,
	  _NLS_("Error, No .desktop file match any of the specified (path)names: %s\n"),
	  config->desktop_file);
      }
      return false;
    }
  }
  
  
  if (config->font_search_path) {
    add_font_path_to_imlib_path(config->font_search_path);
  }
  
  if (config->run_in_term) {   
    char *stok_savptr  = NULL;
    char *cterm        = strtok_r(config->pref_terms,":",&stok_savptr);
    char *first_term   = cterm;
    
    while ( cterm ) {
      if ( !access(cterm,F_OK|X_OK) ) {
	  break;
      }
      cterm=strtok_r(NULL,":",&stok_savptr);
    }
    
    /* Selects the 1st terminal in pref_terms when none are installed
     * on the system or when the current user has no execution permissions
     * for none of them. This should never happens unless pref_terms
     * contains only garbage.
     */
    if (!cterm) {
      cterm=first_term;
    }
    
     /* prefix the command to run with '<cterm> -e' ... */
    add_argument_at(config->command,strdup(cterm),0);
    add_argument_at(config->command,strdup("-e"),1);
  }
  
  return true;
}
