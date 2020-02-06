/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */
 
#ifndef CMDPARSER_H_INCLUDED
#define CMDPARSER_H_INCLUDED

#include <stdbool.h>

          /*** 
           *    Functions    *
			   ***/

/* Parses the given command line (as an arrray of strings) and updates
 * the application configuration accordingly.
 * 
 * Returns true if the application can continue, false otherwise
 * in which case exit_code is the exit code that must be returned
 * by main().
 */
bool parse_command_line(int argc, char *argv[], int *exit_code) ;

#endif
