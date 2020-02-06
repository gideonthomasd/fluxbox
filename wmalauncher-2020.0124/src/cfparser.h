/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#ifndef CFPARSER_H_INCLUDED
#define CFPARSER_H_INCLUDED

#include <stdbool.h>

          /*** 
           *    Typedefs and Structs      *
                                        ***/
/* 
 * A key-value pair. 
 * 
 * Note:
 *   The field 'loaded' is internally used by function
 *   parse_config_file() and must always be initially set 
 *   to false.
 */
typedef struct {
  const char *key;
  char       **value;
  bool         loaded;
} t_key_value;



          /*** 
           *    Functions prototypes      *
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
 *   int sz_keyvals=sizeof(keyvals)/sizeof(t_key_value);
 * 
 *   parse_config_file("/path/to/config/file",keyvals,sz_keyvals);
 */
bool parse_config_file(const char  *filename, 
                        t_key_value kvlist[], 
			int          sz_kvlist,
			bool         warn_redefines);

#endif
