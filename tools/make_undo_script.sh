#!/bin/sh
if [[ "$1" == "" ]] ; then
  echo "specify a changes log file/path"
  exit
fi
TEMP_FILE=tmp$$.tmp
SCRIPT_FILE=UNDO_SCRIPT_$$.sh
cat $1 | egrep -v '^#' | sort -n -r > $TEMP_FILE
if [[ "$?" != "0" ]] ; then 
  echo error sorting file $1
  exit
fi
cat $TEMP_FILE | awk -F $'\t' 'BEGIN { } {print "mv " $3 " " $2 ; }' > $SCRIPT_FILE
if [[ "$?" != "0" ]] ; then 
  echo error creating script $SCRIPT_FILE
  exit
fi
echo $SCRIPT_FILE created
