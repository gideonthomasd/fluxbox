/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

#include <stdbool.h>

          /*** 
           *    type(defs) and structs      *
                                          ***/

/* Represents a command to execute.
 * 
 * argv is an array of argc+1 strings, with
 * argv[argc]=NULL.
 * 
 * This is designed to easily execute external programs
 * using the functions execv/execvp/execvpe as below:
 * 
 *   execvp(cmd.argv[0],cmd.argv);
 */
typedef struct {
  int    argc;
  char **argv;
} t_command;

          /*** 
           *    Functions prototypes      *
                                        ***/


/* Creates and returns a new *t_command. 
 */
t_command *new_command();

/* Free the memory used by the specified *t_command.
 */
void free_command(t_command *cmd);

/* Adds the arguments 'arg' at the end of the list of argument 
 * of the specified *t_command.
 * 
 * Attention, 'arg' *must never* be freed by the caller.
 */
void add_argument(t_command *cmd, char *arg);

/* Adds the arguments 'arg' at 'pos' into the list of argument 
 * of the specified *t_command, or at the end when pos is 0 < 
 * or >=cmd->argc.
 * 
 * Attention, 'arg' *must never* be freed by the caller.
 */
void add_argument_at(t_command *cmd, char *arg, int pos);

/* Returns the string representation of the specified *t_command.
 * 
 * It is up to the caller to free the memory used by the returned
 * string.
 */
char *command_to_str(t_command *cmd);

/* Parses the command given by the string 'strcmd' and 
 * returns a *t_command.
 * 
 * To prevent splitting of arguments containing spaces, it
 * is required to put them in double quotes.
 * 
 * For instance, if strcmd='usr/bin/foo -arg1 a string with spaces -arg2', 
 * the argument 'a string with spaces' will be splitted in 4 parts. To
 * avoid that, this argument must be in double quotes :
 * 
 *   strcmd='usr/bin/foo -arg1 "a string with spaces" -arg2'
 */
t_command *strcmd_to_command(char *strcmd);


/* Executes the external command represented by cmd in 
 * the background.
 * 
 * On success, returns true and the pid of the started
 * command in cmdpid, returns false otherwise.
 */
bool exec_command(t_command *cmd, int *cmdpid);

#endif
