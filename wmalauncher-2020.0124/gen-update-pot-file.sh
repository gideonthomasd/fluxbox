#!/bin/sh

# script to generate/update .pot file for wmalauncher project.

PROJECTNAME="wmalauncher"

VERSION="2018.0921"

PROJECT_MAIL="slacker6896@gmail.com"

KEYWORD="_NLS_"

LANGUAGE="C"

INPUT_ENCODING="UTF-8"


SOURCES=( "src/cfparser.c" \
          "src/cmdparser.c" \
	  "src/settings.c" \
	  "src/wmalauncher.c" \
	  "src/wmalhelp.c" \
	  "src/wmutil.c" )
	  
DESTDIR="po"

if [ ! -e ${DESTDIR}/${PROJECTNAME}.pot ] ; then

  echo -n " generating ${DESTDIR}/${PROJECTNAME}.pot..."

  if [ ! -d ${DESTDIR} ] ; then
    mkdir -p ${DESTDIR}
  fi
  
  xgettext --default-domain="${PROJECTNAME}" \
	   --package-name="${PROJECTNAME}" \
	   --package-version="${VERSION}" \
	   --msgid-bugs-address="${PROJECT_MAIL}" \
           --keyword="${KEYWORD}" \
           --language="${LANGUAGE}" \
	   --from-code="${INPUT_ENCODING}" \
	   --add-comment \
	   --sort-output \
	   --force-po \
	   --output=${DESTDIR}/${PROJECTNAME}.pot \
	   ${SOURCES[@]} 
  echo "Done."
else
  echo -n " updating ${DESTDIR}/${PROJECTNAME}.pot..."
  
  xgettext --default-domain="${PROJECTNAME}" \
	   --package-name="${PROJECTNAME}" \
	   --package-version="${VERSION}" \
	   --msgid-bugs-address="${PROJECT_MAIL}" \
           --keyword="${KEYWORD}" \
           --language="${LANGUAGE}" \
	   --from-code="${INPUT_ENCODING}" \
	   --add-comment \
	   --sort-output \
	   --join-existing \
	   --output=${DESTDIR}/${PROJECTNAME}.pot \
	   ${SOURCES[@]} 
  echo "Done."
fi
