#!/bin/sh
# Default build
mkdir -p bin/unix >/dev/null 2>&1
g++ -D_WITH_REGEX -DLOG_CHANGES_BY_DEFAULT -O3 -std=gnu++11 -Wall -Wno-deprecated-declarations srnames.cpp -o bin/unix/srnames 2>&1
if [[ "$?" != "0" ]] ; then  exit ; fi
# Static build
# g++ -static -D_WITH_REGEX -DLOG_CHANGES_BY_DEFAULT -O3 -std=gnu++11 -Wall -Wno-deprecated-declarations srnames.cpp -o bin/unix/srnames_static
# if [[ "$?" != "0" ]] ; then  exit ; fi
# Unit test
g++ -D_WITH_REGEX -DUNIT_TEST -DLOG_CHANGES_BY_DEFAULT -std=gnu++11 -Wall -Wno-deprecated-declarations srnames.cpp -o bin/unix/test_srnames 2>&1
if [[ "$?" != "0" ]] ; then  exit ; fi
ls -l $( find bin/unix -type f )
