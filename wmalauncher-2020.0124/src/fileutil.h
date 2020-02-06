/* Copyright 2018 Sébastien Ballet. All rights reserved.
 * 
 * Use of this source code is governed by the BSD 3-clause license
 * that can be found in the LICENSE file. 
 */

#ifndef FILEUTIL_H_INCLUDED
#define FILEUTIL_H_INCLUDED

#include <stdbool.h>
#include <dirent.h>

          /*** 
           *    type(defs) and structs      *
                                          ***/

/* Represents a list of path.
 */
typedef struct {
  char **pathnames;
  int    count;
} t_path_list;

          /*** 
           *    Functions prototypes      *
                                        ***/

/* 
 * Creates and returns a *t_path_list.
 */
t_path_list *new_path_list();

/* Free the memory used by the specified *t_path_list.
 */
void free_path_list(t_path_list *path_list);

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
void add_path(t_path_list *path_list, char *path, char *wdir, int pos);

/* Parses the colon separated list of path given by the string 'str' 
 * and returns a *t_pathlist.
 * 
 * if any of the paths in 'str' is relative and 'wdir' is not NULL, 
 * str_to_path_list assumes that path is relative to 'wdir'.
 */
t_path_list *str_to_path_list(char *str, char *wdir);

/* Perform word expansion (like a posix-shell) on the specified
 * pathname and returns the resulting pathname.
 * 
 * It is up to the caller to free the memory used by the returned
 * pathname.
 */
char *resolve_pathname(char *pname) ;

/* Returns true if the specified pathname is absolute, false
 * otherwise.
 */
bool is_absolute_path(char *pathname);

/* Returns the basename of the specified pathname, i.e. the 
 * name with any leading directory component removed, or NULL
 * when pathname is NULL.
 * 
 * It is up to the caller to free the memory used by the 
 * returned basename.
 */
char *get_file_basename(char *pathname);

/* Returns the directory name of the specified pathname.
 * 
 * It is up to the caller to free the memory used by the 
 * returned dirname.
 */
char *get_file_dirname(char *pathname);

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
char *get_file_extension(char *pathname);

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
bool ends_with_extension(char *pathname, char *ext);

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
bool is_filename_equals_to(char *filename, char *basename, char *ext);

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
bool find_files(char *rootdir, char *filename, int max, t_path_list **filelist);

/* Returns, the full path of the first file with name 'filename' 
 * accessible from any of the paths given by 'pathlist', NULL 
 * otherwise.
 *
 * It is up to the caller to free the memory used by the returned
 * string.
 */
char *search_file(t_path_list *pathlist, char *filename);

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
char *search_file_ext(t_path_list *pathlist, char *basename, char *ext);

#endif
