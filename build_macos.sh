#!/bin/sh
mkdir -p bin/mac >/dev/null 2>&1
clang++ -D_WITH_REGEX -DLOG_CHANGES_BY_DEFAULT -std=gnu++11 -Wall -Wno-deprecated-declarations srnames.cpp -o bin/mac/srnames 2>&1
if [[ "$?" != "0" ]] ; then  exit ; fi
clang++ -D_WITH_REGEX -DUNIT_TEST -DLOG_CHANGES_BY_DEFAULT -std=gnu++11 -Wall -Wno-deprecated-declarations srnames.cpp -o bin/mac/test_srnames 2>&1
if [[ "$?" != "0" ]] ; then  exit ; fi
ls -l $( find bin/mac -type f)