#!/bin/sh

# script to init/update .po files for wmalauncher project.

PROJECTNAME="wmalauncher"
PO_DIR="po"


print_help() {
  echo "Usage: init-update-po-files --init <lang> [<lang> ...]"
  echo "       init-update-po-files --update"
  echo ""
  echo " -i|--init <lang> [<lang>...]"
  echo "    for each specified language <lang> initializes the corresponding"
  echo "    file ${PO_DIR}/<lang>.po using ${PO_DIR}/${PROJECTNAME}.pot as"
  echo "    source"
  echo ""
  echo " -u|--update"
  echo "    Updates the .po files found in ./${PO_DIR} according to the changes"
  echo "    found in ${PO_DIR}/${PROJECTNAME}.pot, if any."
}

CHOICE="$1"

if [ -z "$CHOICE" ] ; then
  print_help
  exit 0
fi

case "$CHOICE" in
  -i|--init)
    shift 1
    if [ -z "$1" ]  ; then
      echo "$CHOICE: Expected <lang> [<lang> ...] is missing."
      exit 1
    fi
    
    if [ ! -e "${PO_DIR}/${PROJECTNAME}.pot" ] ; then
      echo "$CHOICE: required pot file ${PO_DIR}/${PROJECTNAME}.pot is missing."
      exit 1
    fi
    
    while [ ! -z "$1" ] ; do
      if [ -e "${PO_DIR}/$1.po" ] ; then
        echo "$CHOICE: Skipping language $1. ${PO_DIR}/$1.po already exist."
      else
        echo -n "init ${PO_DIR}/$1.po ..."
	
	# pass --no-translator to msginit to avoid to have
	# to enter/confirm an email.
	#
	if ! msginit --input=${PO_DIR}/${PROJECTNAME}.pot \
	        --locale=$1 \
		--no-translator \
		--output=${PO_DIR}/$1.po ; then
	  echo "Failure while initializing ${PO_DIR}/$1.po. Exiting."
	  exit 1
	fi
		
      fi
      shift 1
    done
  ;;
  
  -u|--update)
    PO_FILE_LIST=$(find ${PO_DIR} -type f -iname "*.po" -printf "%f\n")
    
    for POFILE in $PO_FILE_LIST ; do
      echo -n "Updating ${PO_DIR}/$POFILE ..."
      if ! msgmerge ${PO_DIR}/${POFILE} \
               ${PO_DIR}/${PROJECTNAME}.pot \
	       --output=${PO_DIR}/${POFILE} ; then
        echo "Failure while updating ${PO_DIR}/$POFILE. Exiting"
	exit 1
      fi
    done
  ;;
  
  *)
  print_help
  echo ""
  echo "Unknown option $1."
  ;;
esac
