/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "command.h"

          /*** 
           *    Functions    *
			   ***/

/* Creates and returns a new *t_command. 
 */
t_command *new_command() {
  t_command *cmd=malloc(sizeof(t_command));
  
  cmd->argc = 0;
  cmd->argv = NULL;

  return cmd;
}

/* Free the memory used by the specified *t_command.
 */
void free_command(t_command *cmd) {
  if (!cmd) {
    return;
  }
  
  for (int i=0;i<cmd->argc;i++) {
    free(cmd->argv[i]);
  }
  free(cmd->argv);
  free(cmd);
}

/* Adds the arguments 'arg' at the end of the list of argument 
 * of the specified *t_command.
 * 
 * Attention, 'arg' *must never* be freed by the caller.
 */
void add_argument(t_command *cmd, char *arg) {
  char **new_argv;

  /* Reminder: 
   *   cmd->argv must be an array of argc+1 strings 
   *   with cmd->argv[cmd->argc]=NULL
   */
  cmd->argc++;
  new_argv=calloc(cmd->argc+1,sizeof *cmd->argv);

  if (cmd->argc > 1) {
    memcpy(new_argv,cmd->argv,(cmd->argc-1)*(sizeof *cmd->argv));
    free(cmd->argv);
  }

  new_argv[cmd->argc-1] = arg;
  new_argv[cmd->argc]   = NULL;
  cmd->argv             = new_argv;  
}

/* Adds the arguments 'arg' at 'pos' into the list of argument 
 * of the specified *t_command, or at the end when pos is 0 < 
 * or > cmd->argc.
 * 
 * Attention, 'arg' *must never* be freed by the caller.
 */
void add_argument_at(t_command *cmd, char *arg, int pos) {

  if ((pos<0) || (pos>=cmd->argc)) {
    add_argument(cmd,arg);
    return;
  }
  
  char **new_argv;

  /* Reminder: 
   *   cmd->argv must be an array of argc+1 strings 
   *   with cmd->argv[cmd->argc]=NULL
   * 
   * Invariant:
   *   At this point, the argument list (cmd->argv) contains
   *   *at least* one element (cmd->argc>=1), and the insertion
   *   point is in range [0..cmd->argc-1]
   */
  cmd->argc++;
  new_argv=calloc(cmd->argc+1,sizeof *cmd->argv);
  
  int lsize=pos;
  int rsize=cmd->argc-pos;
    
  if (lsize>0) {
    memcpy(new_argv,cmd->argv,lsize*(sizeof *cmd->argv));
  }
    
  if (rsize>0) {
    memcpy(new_argv+pos+1,cmd->argv+pos,rsize*(sizeof *cmd->argv));
  }
  
  free(cmd->argv);
  
  new_argv[pos] = arg;
  cmd->argv     = new_argv;
}

/* Returns the string representation of the specified *t_command.
 * 
 * It is up to the caller to free the memory used by the returned
 * string.
 */
char *command_to_str(t_command *cmd) {
  if (!cmd) {
    return NULL;
  }
  
  if (cmd->argc==0) {
    return calloc(1,sizeof(char));
  }

  char *strcmd;
  int   len=0; 
  
  for (int i=0;i<cmd->argc;i++) {
    len=len+strlen(cmd->argv[i]);

      /* space between argv[i-1] & argv[i] */
    if (i>0) len++; 
    
      /* the argument includes spaces. It must be placed in quotes */
    if ( strstr(cmd->argv[i]," ") || strstr(cmd->argv[i],"\t") ) len=len+2;
  }
  
  /* +1 for null terminate byte */
  strcmd=calloc(len+1,sizeof(char));
  
  for (int i=0;i<cmd->argc;i++) {
      /* space between argv[i-1] & argv[i] */
    if (i>0) strcat(strcmd," "); 
    
    if ( strstr(cmd->argv[i]," ") || strstr(cmd->argv[i],"\t") ) {
      strcat(strcmd,"\"");
      strcat(strcmd,cmd->argv[i]);
      strcat(strcmd,"\"");
    } else {
      strcat(strcmd,cmd->argv[i]);
    }
  }
  
  return strcmd;  
}


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
t_command *strcmd_to_command(char *strcmd) {
  if (!strcmd) {
    return NULL;
  }
  
  t_command *newcmd = new_command();
  char      *cptr   = strcmd;
  char      *token;
  int        tpos;
  int        tlen;
  
  while (*cptr) {
    
    /* skip blanks */
    while (isblank(*cptr)) {
      cptr++;
    }
    
    /* A token (seems to) start(s) at cptr */
    tpos = (cptr-strcmd);
    tlen = 0;
    
    if (*cptr=='"') {
      /* Found a quoted token starting at tpos. The argument 
       * start at tpos+1 and ends just before the next quotation
       * mark, if any, or just before the null terminate byte 
       * otherwise.
       */
      tpos++;
      cptr++;

      /* move forward until the next quotation mark, if any... 
       * 
       * NOTE: the loop below does not handle the case where
       * the current argument includes quotes like in example
       * below: 
       *   "an argument with "quoted" text"
       * 
       * In this case, the argument above will be splitted in 3 :
       *   (1) an argument with, (2) quoted", (3) text".
       */
      while ( (*cptr) && (*cptr!='"') ) {
	cptr++;
	tlen++;
      }
      
      /* extract and add the quoted argument unless it is empty.
       */
      if (tlen>0) {
        token=calloc(tlen+1,sizeof(char));
        strncpy(token,strcmd+tpos,tlen);

	add_argument(newcmd,token);
      }
      
      /* pass to the next character after the quotation mark, if any. */
      if (*cptr) {
        cptr++;
      }
    } 
      else 
    if (*cptr) {
      /* Found an argument starting at tpos and ending at the character
       * just before the next blank, or just before the null terminate
       * byte.
       */
      while ( (*cptr) && (!isblank(*cptr)) ) {
        tlen++;
        cptr++;
      }

      /* extract and add the argument, unless it is empty. */
      if (tlen>0) {
        token=calloc(tlen+1,sizeof(char));
        strncpy(token,strcmd+tpos,tlen);

	add_argument(newcmd,token);
      }
    }
  }
  
  return newcmd;
}

/* Executed when the child process started by exec_command()
 * to launch a command in the background has ended (SIGCHLD). 
 * 
 * When it is run, this handler execute the command below to 
 * prevent the child process started by exec_command() to become
 * a zombie [1] :
 * 
 *   ret=waitpid(-1,&status,WNOHANG);
 * 
 * [1] ===== from 'wait/waitpid' manual page ====
 *   A child that terminates, but has not  been  waited for
 *   becomes a "zombie". 
 * 
 *   The kernel maintains a minimal set of information about
 *   the zombie process (PID, termination status, resource 
 *   usage  information) in order to allow the parent to later
 *   perform a wait to obtain information about the child.
 */
void exec_handler(int signal) {
  pid_t ret;
  int status;
  
  if (signal==SIGCHLD) {
    ret=waitpid(-1,&status,WNOHANG);
  }
}

/* Executes the external command represented by cmd in 
 * the background.
 * 
 * On success, returns true and the pid of the started
 * command in cmdpid, returns false otherwise.
 */
bool exec_command(t_command *cmd, int *cmdpid) {
  pid_t cpid;

  if (!cmd) {
    return false;
  }
  
  /* Installs a signal handler to be able to wait for the 
   * child process which is about to be started.
   */
  signal(SIGCHLD,exec_handler);

  cpid=fork();
  
  if (cpid==-1) {
    /* Fork failed.  
     */
    return false;
  }

  if (cpid==0) {
    /*
     * This is run in the child process with pid *cpid*
     * 
     * Note that execvp() only returns if an error has
     * occured in which case the return value is -1 and
     * errno is set to indicate the error.
     */
    if (execvp(*cmd->argv,cmd->argv) == -1) {
      return false;
    }
  }
  
   /* invariant: running in parent process (cpid != 0 && -1)
    */
  *cmdpid=cpid;
  return true;
}
