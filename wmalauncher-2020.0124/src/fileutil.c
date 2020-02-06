/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <wordexp.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#include "fileutil.h"

          /*** 
           *    Functions    *
			   ***/


/*
 * Creates and returns a *t_path_list.
 */
t_path_list *new_path_list() {
  t_path_list *path_list=malloc(sizeof(t_path_list));
  
  path_list->pathnames = NULL;
  path_list->count     = 0;
  
  return path_list;
}
 
/* Free the memory used by the specified *t_path_list.
 */
void free_path_list(t_path_list *path_list) {

  if (!path_list) {
    return;
  }
  
  for (int i=0;i<path_list->count;i++) {
    free(path_list->pathnames[i]);
  }
  free(path_list->pathnames);
  free(path_list);
}

 
/* Adds the specified path at 'pos' in the given list of path, or
 * at the end when pos is 0 < or >= path_list->count.
 * 
 * if 'path' is relative and 'wdir' is not NULL, add_path()
 * assumes that 'path' is relative to 'wdir' and add the path
 * 'wdir'/'path' into the specified list of paths.
 * 
 * a word expansion (like a posix-shell) is automatically done 
 * on 'path' and 'wdir' (if not NULL) to resolves tilde and 
 * references to environment variables.
 * 
 * Attention, it is up to the caller to free the memory used
 * by 'path' and 'wdir', when required.
 */
void add_path(t_path_list *path_list, char *path, char *wdir, int pos) {
  char **new_pathnames=NULL;
  char *new_path=NULL;
  
  wordexp_t exp_path;
  wordexp_t exp_wdir;
  int len, plen, wlen;
  
  if ( (!path) || (strlen(path)==0) ) {
    return;
  }

  wordexp(path,&exp_path,0);
  plen=strlen(exp_path.we_wordv[0]);
  
  if ((exp_path.we_wordv[0][0] != '/') && (wdir)) {
    /* path is relative and wdir is not NULL. The path
     * to add is P(wdir/path)
     */
    
    wordexp(wdir,&exp_wdir,0);
    wlen=strlen(exp_wdir.we_wordv[0]);
    
    if (exp_wdir.we_wordv[0][wlen-1]=='/') {
       /* wdir ends with a /. Therefore, new_path=wdir+path */
      len=wlen+plen;
      new_path=calloc(len+1,sizeof(char));
      
      strcat(new_path,exp_wdir.we_wordv[0]);
    } else {
       /* new_path=wdir+/+path */
      len=wlen+1+plen;
      new_path=calloc(len+1,sizeof(char));
      
      strcat(new_path,exp_wdir.we_wordv[0]);
      strcat(new_path,"/");
    }
    strcat(new_path,exp_path.we_wordv[0]);
    
    wordfree(&exp_wdir);
  } else {
    new_path=strdup(exp_path.we_wordv[0]);
  }
    
  new_pathnames=calloc(path_list->count+1,sizeof *path_list->pathnames);

  if ((pos < 0) || (pos > path_list->count)) {
    pos = path_list->count;
  }

  if (path_list->count > 0) {

    int lsize = pos;
    int rsize = path_list->count - pos;
    
    if (lsize > 0) {
      memcpy(new_pathnames,
	     path_list->pathnames,
	     lsize*(sizeof *path_list->pathnames));
    }

    if (rsize > 0) {
      memcpy(new_pathnames+pos+1,
	     path_list->pathnames+pos,
	     rsize*(sizeof *path_list->pathnames));
    }
    free(path_list->pathnames);
  }

  new_pathnames[pos]   = new_path;
  path_list->pathnames = new_pathnames;

  path_list->count++;
  
  wordfree(&exp_path);
}

/* Parses the colon separated list of path given by the string 'str' 
 * and returns a *t_pathlist.
 * 
 * if any of the paths in 'str' is relative and 'wdir' is not NULL, 
 * str_to_path_list assumes that path is relative to 'wdir'.
 */
t_path_list *str_to_path_list(char *str, char *wdir) {
  if (!str) {
    return NULL;
  }
  
  t_path_list *path_list = new_path_list();

  /* duplicates the specified string to prevent strtok to modify it */  
  char *wstr         = strdup(str); 
  char *stok_saveptr = NULL;
  char *cpath        = strtok_r(wstr,":",&stok_saveptr);
  int   pos          = 0;

  while (cpath) {
    add_path(path_list,cpath,wdir,pos++);
    cpath=strtok_r(NULL,":",&stok_saveptr);
  }
  
  free(wstr);
  return path_list;
}


/* Perform word expansion (like a posix-shell) on the specified
 * pathname and returns the resulting pathname.
 * 
 * It is up to the caller to free the memory used by the returned
 * pathname.
 */
char *resolve_pathname(char *pname) {
  wordexp_t  exp_temp;
  char     *exp_pname;
  
  wordexp(pname,&exp_temp,0);
  
  exp_pname=strdup(exp_temp.we_wordv[0]);
  
  wordfree(&exp_temp);

  return exp_pname;
}

/* Returns true if the specified pathname is absolute, false
 * otherwise.
 */
bool is_absolute_path(char *pathname) {
  if (!pathname) {
    return false;
  }
  
  return ( pathname[0]=='/' ) ;
}

/* Returns the basename of the specified pathname, i.e. the 
 * name with any leading directory component removed, or NULL
 * when pathname is NULL.
 * 
 * It is up to the caller to free the memory used by the 
 * returned basename.
 */
char *get_file_basename(char *pathname) {
  int  start, end, len;
  char *basename=NULL;
  
  if (!pathname) {
    return NULL;
  }

  end=strlen(pathname)-1;
  
   /* ignore useless trailing /, if any */
  while ( (end>=0) && ( *(pathname+end)=='/') ) {
    end--;
  }

  if ( end==-1 ) {
    /* there's only / in pathname => return an empty string */
    basename=calloc(1,sizeof(char));
  } else {
    start=end;
    
    while ( (start>0) && ( *(pathname+start-1)!='/') ) {
      start--;
    }
  
    len=end-start+1;

    basename=calloc(len+1,sizeof(char));
    strncpy(basename,pathname+start,len);
  }
  return basename;
}

/* Returns the directory name of the specified pathname if any,
 * NULL otherwise.
 * 
 * It is up to the caller to free the memory used by the 
 * returned string.
 */
char *get_file_dirname(char *pathname) {
  int pos,len;
  char *dirname;
  
  if (!pathname) {
    return NULL;
  }
  
  pos=strlen(pathname)-1;
  
  /* ignore trailing /, if any. This is to handle case
   * where pathname=/path/to/something///////// 
   */
  while ( (pos>=0) && (*(pathname+pos)=='/') ) {
    pos--;
  }
  
  /* invariant. when pos>0 => pathname[pos] != '/' */

     /* search for the 1st / starting at pos, if any 
      */
  while ( (pos>=0) && (*(pathname+pos)!='/') ) {
    pos--;
  }
  
  /* invariant. when pos>0 => pathname[pos] == '/' */

  
  /* continue to go back while current char is a / */
  
  while ( (pos>=0) && ( *(pathname+pos)=='/') ) {
    pos--;
  }
  
  len=pos+1; /* the returned dirname must include the trailing '/' */

  if ( len==0 ) {
    dirname=calloc(2,sizeof(char));

    if (pathname[0]=='/') {
      dirname[0]='/';
    } else {
      dirname[0]='.';
    }
  } else {
    dirname=calloc(len+1,sizeof(char));
    strncpy(dirname,pathname,len);
  }
  
  return dirname;
}

/* Returns the (last) extension found in the given 'pathname',
 * or an empty string if no extension were found. Returns NULL
 * if pathname is NULL.
 * 
 * The extension is returned as a pointer on the 1st character
 * in 'pathname' which follows the last '.' found in the string.
 * 
 * When 'pathname' has no extension, the returned pointer is
 * on the NULL terminate byte of 'pathname'.
 * 
 * Examples:
 *   pathname=/usr/share/pixmaps/firefox.png
 *                                       ^
 *                                       returned pointer
 * 
 *   pathname=/path/to/file/without/extension
 *                                           ^ 
 *                                           returned pointer
 */
char *get_file_extension(char *pathname) {
  int pos,len;

  if (!pathname) {
    return NULL;
  }
  
  len=strlen(pathname);
  pos=len-1;
  
  while ( (pos>=0) && (*(pathname+pos)!='.') ) {
    pos--;
  }
  
  if (pos==-1) {
    /* The pathname has no extension. Return a pointer to 
     * the \0 of pathname. 
     */
    return pathname+len;
  }
  
  return pathname+pos+1;
}

/* Returns true if the specified pathname ends with a given
 * extension.
 * 
 * ext can be a simple extension (ex: jpg), or a colon separated
 * list of extension (ex: svg:png:jpg), in which case this function
 * returns true if the given pathname ends with any of the given
 * extensions.
 * 
 * Attention, the extension(s) must not start with a dot (i.e. '.')
 */
bool ends_with_extension(char *pathname, char *ext) {
  
  if ( (!pathname) || (!ext) ) {
    return false;
  }

  int   len=strlen(pathname);
  char *end=pathname+len-1;
  char *pos=end;
  
  bool  ext_found=false;
  
  /* Search the last '.' in pathname, if any ...
   */
  while ((pos>pathname) && (*pos!='.')) {
    pos--;
  }
  
  if ( (*pos=='.') && (end>pos) ) {
    
    char *wext         = strdup(ext);
    char *stok_saveptr = NULL;
    char *cext         = strtok_r(wext,":",&stok_saveptr);
    
    while (cext) {
      if ( !strcmp(pos+1,cext) ) {
	ext_found=true;
	break;
      }
      cext=strtok_r(NULL,":",&stok_saveptr);
    }
    
    free(wext);
  }
  
  return ext_found;
}

/* Return true if filename is equals to basename+"."+ext, false
 * otherwise.
 * 
 * Attention, filename must not start with any directory component.
 * 
 * ext can be a simple extension (ex: jpg), or a colon separated
 * list of extension (ex: svg:png:jpg), in which case this function
 * returns true if the given filename is equals to the given basename
 * followed by any of the given extensions.
 * 
 * Attention, the extension(s) must not start with a dot (i.e. '.')
 */
bool is_filename_equals_to(char *filename, char *basename, char *ext) {
    
  if ((!filename) || (!basename) || (!ext)) {
    return false;
  }
  
  int  blen      = strlen(basename);
  bool ext_found = false;
  
  if (!strncmp(filename,basename,blen)) {
    /* 'filename' starts with 'basename' */
    
    if (filename[blen]=='.') {

      char *wext         = strdup(ext);
      char *stok_saveptr = NULL;
      char *cext         = strtok_r(wext,":",&stok_saveptr);
      
      while (cext) {
	if (!strcmp(filename+blen+1,cext)) {
	  ext_found=true;
	  break;
	}
        cext=strtok_r(NULL,":",&stok_saveptr);
      }

      free(wext);
    }
  }
  
  return ext_found;
}

/* Searches for the regular file(s) with name 'filename' in the 
 * directory hierarchy 'rootdir'. When max>0, stop searching after 
 * 'max' matching files.
 *
 * On success, returns true and store the pathname(s) of matching 
 * file(s) in filelist, otherwise, returns false.
 * 
 * *filelist is automatically allocated by find_files(), if required.
 * 
 * Note that directory symlinks  found in 'rootdir' are ignored by 
 * find_files(). That is, find_files() works the same way as the 
 * command 'find' by default (ie. when used without the option -L).
 */
bool find_files(char *rootdir, char *filename, int max, t_path_list **filelist) {

  if ( !rootdir || !filename) {
    return false;
  }

  if (!*filelist) {
    *filelist=new_path_list();
  }

  int             rdlen=strlen(rootdir);
  int             fcount;
  struct dirent **flist=NULL;
  struct dirent  *centry;
  int             left, right, middle, cmpres;
  bool             match;
  char            cpath[PATH_MAX]={0};
  struct stat     st_buf;

  fcount = scandir(rootdir,&flist,NULL,alphasort);
  left   = 0;
  right  = fcount-1;
  match  = false;

  while ( !match && left<=right ) {
    middle = (left+right)/2;
    centry = flist[middle];
    cmpres = strcmp(centry->d_name,filename);

    if (cmpres<0) {
      left = middle+1;
    } 
      else 
    if (cmpres>0) {
      right = middle-1;
    } else {
      /* centry->d_name match filename. Before accepting this file, 
       * it is required to ensure it is a regular file...
       */

      if ( rootdir[rdlen-1]=='/' ) {
        snprintf(cpath,sizeof(cpath),"%s%s",rootdir,centry->d_name);
      } else {
        snprintf(cpath,sizeof(cpath),"%s/%s",rootdir,centry->d_name);
      }

      if ( stat(cpath,&st_buf)==-1 ) {
        /* An error occured while getting file status (cpath can be 
	 * a broken symlink for instance). Prints a warning on standard
	 * error, and break the loop... 
         */
        fprintf(stderr,"stat(%s) failed with error %d\n",cpath,errno);
        break;
      } 
        else
      if ( (st_buf.st_mode & S_IFMT)==S_IFREG ) {
	/* centry->d_name is a regular file => add the current
	 * path to the list of matching files...
	 */
        add_path(*filelist,cpath,NULL,-1);
        match=true;
      } else {
        /* centry->d_name match filename, however it is a directory not
	 * a regular file. Therefore, the loop must be break since no 
	 * regular file named 'filename' can be found in the current
	 * directory (ie. rootdir).
	 */
        break;
      }
    }
  }

  /* continue to search for filename in directories found in the
   * current directory (ie. rootdir)...  
   */

  for (int i=0;i<fcount;i++) {
    if ( (max>0) && ((*filelist)->count==max) ) {
      break;
    }
    centry=flist[i];
    
    /* do not handle . and .. to prevent infinite loop ...
     */
    if ( !strcmp(centry->d_name,".") || !strcmp(centry->d_name,"..") ) {
      continue;
    }

    if ( rootdir[rdlen-1]=='/' ) {
      snprintf(cpath,sizeof(cpath),"%s%s",rootdir,centry->d_name);
    } else {
      snprintf(cpath,sizeof(cpath),"%s/%s",rootdir,centry->d_name);
    }
    
    /* If the current path (cpath) is a directory, searches for
     * files in that directory.
     * 
     * Note that lstat() is used instead of stat() to prevent
     * following of any symlink to directory which could lead
     * to infinite loop (which can be prevented by maintaining
     * a list of traversed directories, but, this is, currently,
     * not in the scope of this function).
     */
    if ( (lstat(cpath,&st_buf)==0) && ((st_buf.st_mode & S_IFMT)==S_IFDIR) ) {
      find_files(cpath,filename,max,filelist);
    }
  }

  for (int i=0;i<fcount;i++) {
    free(flist[i]);
  }
  free(flist);

  return (*filelist)->count > 0;
}

/* Returns, the full path of the first file with name 'filename' 
 * accessible from any of the paths given by 'pathlist', NULL 
 * otherwise.
 *
 * It is up to the caller to free the memory used by the returned
 * string.
 */
char *search_file(t_path_list *pathlist, char *filename) {
  if ( (!pathlist) || (!filename) ) {
    return NULL;
  }

  char         *pathname=NULL;
  t_path_list *filepaths=NULL;

  for (int i=0;i<pathlist->count;i++) {
    /* Note that filepaths is automatically allocated by
     * find_files(), *when required*.
     */
    if ( find_files(pathlist->pathnames[i],filename,1,&filepaths) ) {
      pathname=calloc(strlen(filepaths->pathnames[0])+1,sizeof(char));
      strcat(pathname,filepaths->pathnames[0]);
      break;
    }
  }

  free_path_list(filepaths);

  return pathname;
}

/* Returns the full path of the 1st file found in any of the paths
 * given by "pathlist" whose name match basename+ext, NULL otherwise.
 *
 * ext can be a simple extension (ex: jpg), or a colon separated 
 * list of extension (ex: svg:png:jpg) in which case search_file_ext() 
 * will return the first file whose name match basename+ext[i], 
 * if any.
 * 
 * Attention, the extension(s) must not start with a dot (i.e. '.')
 *
 * It is up to the caller to free the memory used by the returned 
 * string.
 */
char *search_file_ext(t_path_list *pathlist, char *basename, char *ext) {

  if ( (!pathlist) || (!basename) || (!ext) ) {
    return NULL;
  }

  char        *wext;
  char        *cext;
  t_path_list *filepaths          = NULL;
  char        *stok_saveptr       = NULL;
  char        *pathname           = NULL;
  char         filename[PATH_MAX] = {0};
  
  for (int i=0;i<pathlist->count && !pathname;i++) {
    wext = strdup(ext);
    cext = strtok_r(wext,":",&stok_saveptr);

    while (cext) {
      snprintf(filename,sizeof(filename),"%s.%s",basename,cext);

      /* Note that filepaths is automatically allocated by
       * find_files(), *when required*.
       */
      if ( find_files(pathlist->pathnames[i],filename,1,&filepaths) ) {
        pathname=calloc(strlen(filepaths->pathnames[0])+1,sizeof(char));
        strcat(pathname,filepaths->pathnames[0]);
        break;
      }
      cext=strtok_r(NULL,":",&stok_saveptr);
    }
    free(wext);
  }
  
  free_path_list(filepaths);

  return pathname;
}
