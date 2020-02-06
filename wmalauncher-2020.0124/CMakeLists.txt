# CMake script for wmalauncher 
#
# Copyright 2018-2020 Sébastien Ballet. All rights reserved.
# 
# Use of this source code is governed by the BSD 3-clause license
# that can be found in the LICENSE file. 
#
# IMPORTANT NOTES:
# ---------------
#
# 1. The variable SYSRES_ROOTDIR defines the directory hierarchy from
#    which resources like icons, fonts and .desktop file can be found.
#
#    It is set to "/usr/share" by default, and used when defining the 
#    default value of variables below :
#
#      DEFAULT_FONT_SEARCH_PATH
#      DEFAULT_ICON_SEARCH_PATH
#      MAIN_ROOT_ICON_SEARCH_PATH
#      DEFAULT_DESKTOP_FILE_SEARCH_PATH
#
# 2. The variable SYSBIN_DIR defines the directory from which binaries
#    can be found.
#
#    It is set to "/usr/bin" by default and used when defining the 
#    default value of variables below :
#
#      DEFAULT_PREFERRED_TERMINALS
#
# 1. The variable DEFAULT_FONT_SEARCH_PATH defines the "default fonts
#    search path list" used by wmalauncher. By default, it includes 
#    the following paths (in the given order) :
#
#      ~/.fonts
#      ${SYSRES_ROOTDIR}/fonts/TTF
#      ${SYSRES_ROOTDIR}/fonts/truetype
#      ${SYSRES_ROOTDIR}/truetype/dejavu
#      ${SYSRES_ROOTDIR}/truetype/liberation
#      ${SYSRES_ROOTDIR}/fonts/dejavu
#      ${SYSRES_ROOTDIR}/fonts/liberation
#    
# 2. The variable DEFAULT_ICON_SEARCH_PATH defines the "default icons
#    search path list" used by wmalauncher. By default, it includes the
#    following paths (in the given order) :
#
#      ~/.icons
#      ${SYSRES_ROOTDIR}/icons/Adwaita/scalable/
#      ${SYSRES_ROOTDIR}/icons/Adwaita/48x48/
#      ${SYSRES_ROOTDIR}/icons/hicolor/scalable/
#      ${SYSRES_ROOTDIR}/icons/hicolor/48x48/
#      ${SYSRES_ROOTDIR}/pixmaps/
#      ${SYSRES_ROOTDIR}/icons
#    
# 3. The variable MAIN_ROOT_ICON_SEARCH_PATH defines the "root path that
#    wmalauncher used to resolve relative paths passed in argument of
#    option --icon-search-path". By default, it is set to /usr/share/icons/
#
# 4. The variable DEFAULT_PREFERRED_ICON_TYPES defines the precedence
#    order of icon types (colon separated list). This list is used to
#    select the (best) icon when the icon name specified in arguments 
#    (or defined in a .desktop  file) is a simple name without an extension.
#
#    By default, it is set to: svgz:svg:png:xpm
#
# 5. The variable DEFAULT_DESKTOP_FILE_SEARCH_PATH defines the "default
#    desktop files search paths list" used by wmalauncher. By default, 
#    it includes the following paths (in the given order) :
#
#      ~/.local/share/applications/
#      ${SYSRES_ROOTDIR}/applications/
#
# 6. The variable DEFAULT_FALLBACK_FONTS defines the fallback fonts as a
#    colon separated list of font to the format : <fontname>/<fontsize>
#
#    By default, it is set to:
#
#      DejaVuSans/11:LiberationSans-Regular/11:luxisr/11
#
# 7. The variable DEFAULT_PREFERRED_TERMINALS defines the default list
#    of terminals, in preference order, to use when a command/application
#    must be run in a terminal window.
#
#    By default, it is set to :
#      ${SYSBIN_DIR}/xterm:${SYSBIN_DIR}/rxvt:${SYSBIN_DIR}/xfce4-terminal:${SYSBIN_DIR}/konsole
#
# 8. To build wmalauncher without National Language Support enabled,
#    run cmake as below :
#
#     $ cmake -DENABLE_NLS=OFF ..
#
# 9. To test the translations without to have to install wmalauncher 
#    system wide, it is required to build and install wmalauncher 
#    like as below :
#
#     $ mkdir _build
#     $ cd _build
#     $ cmake -DCMAKE_INSTALL_PREFIX=/usr -DMAKE_INSTALL_DESTDIR=./TEMP ..
#     $ make install DESTDIR=./TEMP
#
#    this configures wmalauncher so that it will search for translation
#    files in the right directory hierarchy (ie ./TEMP).
# 
# 10. Building wmalauncher on FreeBSD.
#
#    On FreeBSD, resources like fonts, icons, .desktop file are
#    accessible from the directory hierarchy /usr/local/share, 
#    and, 3rd party binaries are stored in /usr/local/bin. Therefore,
#    the easiest way to build wmalauncher on FreeBSD is to run cmake
#    like as below :
#
#    $ mkdir _build
#    $ cd _build
#    $ LDFLAGS="-L/usr/local/lib" \
#      cmake -DSYSRES_ROOTDIR=/usr/local/share \
#            -DSYSBIN_DIR=/usr/local/bin ..
#
#    The LDFLAGS is required otherwise the build fails with an error
#    '/usr/bin/ld: cannot find -lxxxxxx', where 'xxxxxx' can be 
#    Imlib2, cairo, or rsvg-2, depending on the order in which the 
#    variables IMLIB2_LIBRARIES, LIBRSVG_LIBRARIES, and CAIRO_LIBRARIES 
#    are passed to target_link_libraries().
#
#    This issue is not specific to FreeBSD and could be encountered
#    on any system on which these libraries are installed under 
#    /usr/local/lib. As stated here(*), it is possible to fix that 
#    by passing xxxxx_LDFLAGS instead of xxxxxx_LIBRARIES. However, 
#    this does not seems to be a good idea in the long run. Instead, 
#    it is better (and less intrusive) to pass -L/usr/local/lib to
#    cmake.
#
#     (*) https://cmake.org/pipermail/cmake/2011-May/044376.html
#
cmake_minimum_required(VERSION 2.8)
project(wmalauncher)

# Defines wmalauncher version number and copyright.
#
set(WMAL_VERSION "2020.0124")
set(WMAL_COPYRIGHT "(c) 2018-2020, Sébastien Ballet <slacker6896@gmail.com>")

# Sets the variable SYSRES_ROOTDIR to its default value, when required.
# 
if ( "${SYSRES_ROOTDIR}" STREQUAL "" )
  set(SYSRES_ROOTDIR "/usr/share")
endif ()

# Sets the variable SYSBIN_DIR to its default value, when required.
# 
if ( "${SYSBIN_DIR}" STREQUAL "" )
  set(SYSBIN_DIR "/usr/bin")
endif ()


# Sets the variables below to their respective default value, when
# required :
#   DEFAULT_FONT_SEARCH_PATH     DEFAULT_ICON_SEARCH_PATH
#   MAIN_ROOT_ICON_SEARCH_PATH   DEFAULT_DESKTOP_FILE_SEARCH_PATH
#   DEFAULT_FALLBACK_FONTS       DEFAULT_PREFERRED_TERMINALS
#
if ( "${DEFAULT_FONT_SEARCH_PATH}" STREQUAL "" )
 set(DEFAULT_FONT_SEARCH_PATH 
 "~/.fonts:\
${SYSRES_ROOTDIR}/fonts/TTF:\
${SYSRES_ROOTDIR}/fonts/truetype:\
${SYSRES_ROOTDIR}/fonts/truetype/dejavu:\
${SYSRES_ROOTDIR}/fonts/truetype/liberation:\
${SYSRES_ROOTDIR}/fonts/dejavu:\
${SYSRES_ROOTDIR}/fonts/liberation" )

endif ()

if ( "${DEFAULT_ICON_SEARCH_PATH}" STREQUAL "" )
  set(DEFAULT_ICON_SEARCH_PATH 
  "~/.icons:\
${SYSRES_ROOTDIR}/icons/Adwaita/scalable:\
${SYSRES_ROOTDIR}/icons/Adwaita/48x48:\
${SYSRES_ROOTDIR}/icons/hicolor/scalable:\
${SYSRES_ROOTDIR}/icons/hicolor/48x48:\
${SYSRES_ROOTDIR}/pixmaps:\
${SYSRES_ROOTDIR}/icons"  )
endif ()

if ( "${MAIN_ROOT_ICON_SEARCH_PATH}" STREQUAL "" )
  set ( MAIN_ROOT_ICON_SEARCH_PATH "${SYSRES_ROOTDIR}/icons" )
endif ()

if ( "${DEFAULT_PREFERRED_ICON_TYPES}" STREQUAL "" )
  set ( DEFAULT_PREFERRED_ICON_TYPES "svgz:svg:png:xpm" )
endif ()

if ( "${DEFAULT_DESKTOP_FILE_SEARCH_PATH}" STREQUAL "" )
  set ( DEFAULT_DESKTOP_FILE_SEARCH_PATH 
"~/.local/share/applications:\
${SYSRES_ROOTDIR}/applications" )
endif ()

if ( "${DEFAULT_PREFERRED_TERMINALS}" STREQUAL "") 
  set (DEFAULT_PREFERRED_TERMINALS 
"${SYSBIN_DIR}/xterm:\
${SYSBIN_DIR}/rxvt:\
${SYSBIN_DIR}/xfce4-terminal:\
${SYSBIN_DIR}/konsole")
endif ()

if ( "${DEFAULT_FALLBACK_FONTS}" STREQUAL "" )
  set (DEFAULT_FALLBACK_FONTS "DejaVuSans/11:LiberationSans-Regular/11:luxisr/11")
endif ()

# Set default build type if none is defined.
#
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Adds the flag DEBUG to CMAKE_C_FLAGS_{DEBUG,RelWithDebInfo} when build
# type is set to 'Debug' or 'RelWithDebInfo'. 
#
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
 set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG") 
endif()

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
 set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DDEBUG") 
endif()

# Ensure that flags -Wall -pedantic and -Wextra are enabled regardless
# the build type.
#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra")

# Includes required cmake modules.
#
include(GNUInstallDirs)
include(FindGettext)
include(FindIntl)
include(FindPkgConfig)

# Ensures That packages X11, imlib2, librsvg, and cairo are present
# on the system.
#
find_package(X11 REQUIRED)
pkg_check_modules(IMLIB2 REQUIRED imlib2)
pkg_check_modules(LIBRSVG REQUIRED librsvg-2.0)
pkg_check_modules(CAIRO REQUIRED cairo)

# Adds the option ENABLE_NLS for "Nation Language Support" and
# enable it, if possible.
#
if (Intl_FOUND AND GETTEXT_FOUND) 
  option(ENABLE_NLS "Enable National Language Support" ON)
endif ()

# Defines the package name, and where can be found: the sources, translation
# files, man pages, and samples configuration files
#
set(PACKAGE "${PROJECT_NAME}") 
set(SOURCES src)
set(PO_SOURCES po)
set(MAN_SOURCES man)
set(CONF_SOURCES conf)

if (NOT "${MAKE_INSTALL_DESTDIR}" STREQUAL "")

  # If MAKE_INSTALL_DESTDIR is not an absolute path, 
  # resolves it relatively to CMAKE_BINARY_DIR...
  #
  if (NOT IS_ABSOLUTE ${MAKE_INSTALL_DESTDIR})
    get_filename_component(MAKE_INSTALL_DESTDIR
                           ${MAKE_INSTALL_DESTDIR}
			   ABSOLUTE
			   REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
  endif ()
endif ()

# Generate config.h starting from config.h.in and value
# of the variables below, then adds -DHAVE_CONFIG_H to
# the compilation of source files :
#
#  WMAL_VERSION                      WMAL_COPYRIGHT
#  DEFAULT_FONT_SEARCH_PATH          DEFAULT_ICON_SEARCH_PATH
#  MAIN_ROOT_ICON_SEARCH_PATH        DEFAULT_ICON_SEARCH_PATH,
#  MAIN_ROOT_ICON_SEARCH_PATH        WMAL_PREFERRED_ICON_TYPES
#  DEFAULT_DESKTOP_FILE_SEARCH_PATH  DEFAULT_PREFERRED_TERMINALS
#  DEFAULT_FALLBACK_FONTS            MAKE_INSTALL_DESTDIR
#  CMAKE_INSTALL_FULL_DATADIR
#
configure_file(config.h.in config.h)
add_definitions(-DHAVE_CONFIG_H)


# Generate :
#   man/wmalauncher.1 starting from man/wmalauncher.1.in
#   man/fr/wmalauncher.1 starting from man/fr/wmalauncher.1.in
#
#   conf/lighting.conf from lighting.conf.in  
#   conf/multi-effects.conf from multi-effects.conf.in
#   conf/old-school.conf from old-school.conf.in
#   conf/frame-onthefly.conf from frame-onthefly.conf.in
#   conf/wmaker-style.conf from wmaker-style.conf.in
#
configure_file(${MAN_SOURCES}/wmalauncher.1.in man/wmalauncher.1)
configure_file(${MAN_SOURCES}/fr/wmalauncher.1.in man/fr/wmalauncher.1)

configure_file(${CONF_SOURCES}/lighting.conf.in conf/lighting.conf)
configure_file(${CONF_SOURCES}/multi-effects.conf.in conf/multi-effects.conf)
configure_file(${CONF_SOURCES}/old-school.conf.in conf/old-school.conf)
configure_file(${CONF_SOURCES}/frame-onthefly.conf.in conf/frame-onthefly.conf)
configure_file(${CONF_SOURCES}/wmaker-style.conf.in conf/wmaker-style.conf)


# Add the source, the output directories and the directory
# in which libintl.h is stored as include directories. 
#
# The output directory (ie pointed by CMAKE_CURRENT_BINARY_DIR)
# is defined as an include directory, so that sources may includes 
# the header file config.h as #include <config.h>
#
include_directories(${SOURCES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Intl_INCLUDE_DIRS})

# Add directories in which the headers of required libraries
# cairo & librsvg are stored.
# 
include_directories(${LIBRSVG_INCLUDE_DIRS})
include_directories(${CAIRO_INCLUDE_DIRS})

# Configure National Language Support and generates 
# translation files when required.
#
if (ENABLE_NLS)
  add_definitions(-DENABLE_NLS)

  # Defines the supported languages. Only french is supported at
  # the moment. To support other languages, it is required to :
  #   1. references the new languages in variable LANGS. For instance, 
  #      to  add support for german, spanish and italian set variable
  #      LANGS must be set as below :
  #
  #        set (LANGS fr de es it)
  #
  #   2. generate the .po files for the new supported languages
  #      by running :
  #
  #        * the command msginit manually, as in example below for
  #          german language :
  #
  #            msginit --input=po/wmalauncher.pot \
  #                    --no-translator \
  #                    --locale=de \
  #                    --output=po/de.po
  #
  #       * the script init-update-po-files.sh, as in example below
  #         for german, spanish and italian languages :
  #
  #            init-update-po-files.sh --init de es it
  #
  #   3. edit the generated .po files to provide the right 
  #      translations.
  #
  set (LANGS fr)
  
  configure_file (${PO_SOURCES}/${PACKAGE}.pot
                  ${PROJECT_BINARY_DIR}/${PACKAGE}.pot
		  COPYONLY)
		  
  foreach (CLANG ${LANGS})
    configure_file (${PO_SOURCES}/${CLANG}.po
                    ${PROJECT_BINARY_DIR}/${CLANG}.po
		    COPYONLY)
  endforeach()

  set(PACKAGE_LOCALE_DIR ${CMAKE_INSTALL_FULL_DATADIR}/locale)
  
  gettext_process_pot_file(${PROJECT_BINARY_DIR}/${PACKAGE}.pot
			   ALL
			   INSTALL_DESTINATION
			   ${PACKAGE_LOCALE_DIR}
			   LANGUAGES ${LANGS})
endif ()
	
# Defines sources for wmalauncher executable.
#
add_executable(wmalauncher 
               ${SOURCES}/nls.h
               ${SOURCES}/nls.c
               ${SOURCES}/fileutil.h
	       ${SOURCES}/fileutil.c
               ${SOURCES}/command.h
               ${SOURCES}/command.c
	       ${SOURCES}/cfparser.h
	       ${SOURCES}/cfparser.c
	       ${SOURCES}/settings.h
	       ${SOURCES}/settings.c
               ${SOURCES}/wmutil.h 
	       ${SOURCES}/wmutil.c 
	       ${SOURCES}/wmalhelp.h
	       ${SOURCES}/wmalhelp.c
	       ${SOURCES}/cmdparser.h
	       ${SOURCES}/cmdparser.c
	       ${SOURCES}/wmalauncher.c)

# Ensure that C Standard gnu99 is used when building the
# target.
#
set_property (TARGET wmalauncher PROPERTY C_STANDARD 99)

# Defines the libraries to link with.
#
# Note that Intl_LIBRARIES is required (only) on systems which
# don't use the GNU libc (ex. FreeBSD). Without it, the compilation
# fails. 
#
target_link_libraries(wmalauncher 
                      ${Intl_LIBRARIES}
                      ${X11_LIBRARIES} 
		      ${IMLIB2_LIBRARIES}
		      ${LIBRSVG_LIBRARIES}
		      ${CAIRO_LIBRARIES})

# Installs the binary 'wmalauncher', the manual pages, the README
# and sample configuration files .
#
# The binary is installed into the 'bin' folder, relatively 
# to the directory specified by ${CMAKE_INSTALL_PREFIX}
#
install(TARGETS wmalauncher DESTINATION bin)

install(FILES ${CMAKE_BINARY_DIR}/man/wmalauncher.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)
install(FILES ${CMAKE_BINARY_DIR}/man/fr/wmalauncher.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/fr/man1)

install(FILES AUTHORS DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}-${WMAL_VERSION})
install(FILES ChangeLog DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}-${WMAL_VERSION})
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}-${WMAL_VERSION})
install(FILES README DESTINATION ${CMAKE_INSTALL_PREFIX}/doc/${PROJECT_NAME}-${WMAL_VERSION})

install(FILES ${CMAKE_BINARY_DIR}/conf/lighting.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
install(FILES ${CMAKE_BINARY_DIR}/conf/multi-effects.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
install(FILES ${CMAKE_BINARY_DIR}/conf/old-school.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
install(FILES ${CMAKE_BINARY_DIR}/conf/frame-onthefly.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
install(FILES ${CMAKE_BINARY_DIR}/conf/wmaker-style.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})


message(STATUS "------------------------------------------------------------------------")
message(STATUS "${PROJECT_NAME} project settings :")
message(STATUS "  WMAL_VERSION                     = ${WMAL_VERSION}")
message(STATUS "  SYSRES_ROOTDIR                   = ${SYSRES_ROOTDIR}")
message(STATUS "  SYSBIN_DIR                       = ${SYSBIN_DIR}")
message(STATUS "  DEFAULT_FONT_SEARCH_PATH         = ${DEFAULT_FONT_SEARCH_PATH}")
message(STATUS "  DEFAULT_ICON_SEARCH_PATH         = ${DEFAULT_ICON_SEARCH_PATH}")
message(STATUS "  MAIN_ROOT_ICON_SEARCH_PATH       = ${MAIN_ROOT_ICON_SEARCH_PATH}")
message(STATUS "  DEFAULT_PREFERRED_ICON_TYPES     = ${DEFAULT_PREFERRED_ICON_TYPES}")
message(STATUS "  DEFAULT_DESKTOP_FILE_SEARCH_PATH = ${DEFAULT_DESKTOP_FILE_SEARCH_PATH}")
message(STATUS "  DEFAULT_PREFERRED_TERMINALS      = ${DEFAULT_PREFERRED_TERMINALS}")
message(STATUS "  DEFAULT_FALLBACK_FONTS           = ${DEFAULT_FALLBACK_FONTS}")
message(STATUS "  CMAKE_BUILD_TYPE                 = ${CMAKE_BUILD_TYPE}")

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  message(STATUS "    CMAKE_C_FLAGS_DEBUG            = ${CMAKE_C_FLAGS_DEBUG}")
endif()

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  message(STATUS "    CMAKE_C_FLAGS_RELWITHDEBINFO   = ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
endif()

message(STATUS "  CMAKE_C_FLAGS                    = ${CMAKE_C_FLAGS}")
message(STATUS "  CMAKE_INSTALL_PREFIX             = ${CMAKE_INSTALL_PREFIX}")

if (NOT "${MAKE_INSTALL_DESTDIR}" STREQUAL "")
  message(STATUS "  MAKE_INSTALL_DESTDIR             = ${MAKE_INSTALL_DESTDIR}")
endif()
message(STATUS "------------------------------------------------------------------------")
