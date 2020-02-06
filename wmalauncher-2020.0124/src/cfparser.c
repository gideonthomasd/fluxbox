/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */


/* Required to build on FreeBSD. For more about this, see
 * section 'COMPATIBILITY' in FreeBSD 'man getline'.
 */
#define _WITH_GETLINE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "cfparser.h"

          /*** 
           *    Functions    *
			   ***/

/* Parses the configuration file 'filename' and updates
 * the values in key-value pair table 'kvlist' accordingly.
 * 
 * Returns true if the file has been parsed, false if an
 * error occured while opening the file. In this case,
 * errno (see errno.h) is set to indicate the error.
 * 
 * Each entry in the file 'filename' must be to the
 * format :
 *   key = value
 * 
 * Notes:
 *   + commented lines (i.e which start with a '#', preceded or
 *     not by spaces) are ignored.
 * 
 *   + spaces at both side of loaded values are automatically
 *     removed.
 * 
 *   + When there are more than one occurence of a given key, 
 *     only the first value is loaded. The other values are 
 *     ignored, and, the function outputs warnings to STDERR
 *     unless when argument warn_redefines is false.
 * 
 * Usage sample:
 *   char *key1value=NULL;
 *   char *key2value=NULL;
 * 
 *   t_key_value keyvals[] = {
 *     { "key1", &key1value, false},
 *     { "key2", &key2value, false}
 *   };
 *   size_t sz_keyvals=sizeof(keyvals)/sizeof(t_key_value);
 * 
 *   parse_config_file("/path/to/config/file",keyvals,sz_keyvals);
 */
bool parse_config_file(const char   *filename, 
                        t_key_value  kvlist[], 
			int           sz_kvlist,
			bool          warn_redefines) {
  FILE    *file;
  char    *cline=NULL;
  ssize_t nread;
  char    *pstr, *cstr, *cend;
  size_t  len=0, klen, clen;
  int     lindex=0;
  
  file=fopen(filename,"r");
  
  if (!file) {
    return false;
  }
  
  while ( (nread=getline(&cline,&len,file))!=-1) {
    lindex++;
    
    if ( *cline=='\n' ) continue;

    /* get rid of new line at the end of current 
     * line, if any.
     */
    if (cline[nread-1]=='\n') {
      cline[nread-1]='\0'; 
    }
    
    pstr=cline;
    
    /* skip blanks... */
    while ( isspace(*pstr) ) {
      pstr++;
    }

    /* The 1st non blank character is a '#'. The line 
     * is a comment, skip it.
     */
    if (*pstr=='#') {
      continue ;
    }
    
    for (int i=0;i<sz_kvlist;i++) {
      
      /* should never be encountered, but who knows ? */
      if (!kvlist[i].key) {
	continue;
      }
      
      cstr=pstr;
      klen=strlen(kvlist[i].key);
      
      if (strncmp(kvlist[i].key,cstr,klen)) {
	/* cstr does not match the current key, pass to 
	 * the next key...
	 */
	continue;
      }

      /* Invariant:
       *   The string cstr seems to match the current
       *   key. However, this can simply because key
       *   is a substring of pstr (ex. key=command and
       *   pstr=commander). In fact, cstr matches the 
       *   current key, if and only if the character
       *   at cstr+klen is a a blank (ie. ' ' or '\t')
       *   or the delimiter '='.
       */
      cstr +=klen;
      
      if ( (!isspace(*cstr)) && (*cstr != '=') ) {
	/* cstr does not match the current key, pass to
	 * the next key 
	 */
	continue;
      }
      
      /* Invariant:
       *   cstr *match* the current key, and the current
       *   character (ie. *cstr) is a space or a '='...
       */
	  
	/* skip spaces, if any ... */
      while ( isspace(*cstr) ) {
        cstr++;
      }
	    
      /* if cstr points the delimiter '=', loads the
       * value of kvlist[i] (unless another value has
       * already been loaded) and breaks the for loop. 
       * 
       * Otherwise, ignore the current key, and
       * pass to the next..
       */
      if (*cstr == '=') {
        cstr++;
	    
	/* trim trailing spaces, if any */
	    
	clen=strlen(cstr);
	cend=cstr+clen-1;
	    
	while ( isspace(*cend) ) {
	  cend--;
	}
	*(cend + 1)='\0';
	    
	/* trim leading spaces, if any  */
	  
	while ( isspace(*cstr) ) {
	  cstr++;
	}
	  
	if ( ! kvlist[i].loaded ) {
	  free(*kvlist[i].value);
	  *kvlist[i].value = strdup(cstr);
	  kvlist[i].loaded = true;
	} 
	  else 
	if (warn_redefines) {
	  fprintf(stderr,
_NLS_("Warning, entry ignored at line #%d in file %s. \
The key '%s' is already assigned.\n\n"),lindex,filename,kvlist[i].key);
	}
	break;
      }
    }
  }

  fclose(file);
  
  if (cline) {
    free(cline);
  }
  
  return true;
}
