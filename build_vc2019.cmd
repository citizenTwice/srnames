@echo off
cl SRNAMES.CPP /EHsc /Ox /GR- /DLOG_CHANGES_BY_DEFAULT /D_WITH_REGEX /Ox /FeBIN\W\SRNAMES.EXE
if ERRORLEVEL 1 EXIT /b
cl SRNAMES.CPP /EHsc /Ox /GR- /DLOG_CHANGES_BY_DEFAULT /D_WITH_REGEX /DUNICODE /D_UNICODE /Ox /FeBIN\W\SRNAMESU.EXE
if ERRORLEVEL 1 EXIT /b

REM UNIT TESTS
cl SRNAMES.CPP  /EHsc /DUNIT_TEST /D_WITH_REGEX /Ox /FeBIN\W\TEST_SRNAMES.EXE
if ERRORLEVEL 1 EXIT /b
cl SRNAMES.CPP  /EHsc /DUNIT_TEST /D_WITH_REGEX /DUNICODE /D_UNICODE /Ox /FeBIN\W\TEST_SRNAMESU.EXE
if ERRORLEVEL 1 EXIT /b

