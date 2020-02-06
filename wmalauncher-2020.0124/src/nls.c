/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#include <stdarg.h>

#include "nls.h"

          /*** 
           *    Functions    *
			   ***/

/* Sets the i18n environment, if required.
 * 
 * any software which requires National Language Support
 * must call this function at startup.
 * 
 * Attention, this function does nothing unless ENABLE_NLS
 * is defined.
 * 
 * when count==0, setup_i18n_env(), sets LC_ALL to "", otherwise,
 * only the categories which follows in arguments are modified. Each
 * of these arguments can be one of the following :
 *   LC_ADDRESS                        LC_COLLATE
 *   LC_CTYPE                          LC_IDENTIFICATION
 *   LC_MEASUREMENT                    LC_MESSAGES
 *   LC_MONETARY                       LC_NAME
 *   LC_NUMERIC                        LC_PAPER
 *   LC_TELEPHONE                      LC_TIME
 */
void setup_i18n_env(char *domainname, char *dirname, int count, ...) {
#ifdef ENABLE_NLS

  va_list args;
  int     category;
  
  if (count==0) {
    setlocale(LC_ALL,"");
  } else {
    va_start(args,count);
    for (int i=0;i<count;i++) {
      category=va_arg(args,int);
      setlocale(category,"");
    }
    va_end(args);
  }

  bindtextdomain(domainname,dirname);
  textdomain(domainname);
#endif
}
