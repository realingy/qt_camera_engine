@echo off
set src_dir=%~dp0

set /p build=<%src_dir%build_no.txt
set /a build_new=%build%+1

echo %build_new% > %src_dir%build_no.txt

echo #ifndef VERSION_H > %src_dir%version.h
echo #define VERSION_H >> %src_dir%version.h
echo. >> %src_dir%version.h
rem echo #define MAJOR_NUMBER 1 >> %src_dir%version.h
rem echo #define MIDDLE_NUMBER 0 >> %src_dir%version.h
rem echo #define MINOR_NUMBER 0 >> %src_dir%version.h
echo #define BUILD_NUMBER %build_new% >> "%src_dir%"version.h
echo. >> %src_dir%version.h
echo #endif //VERSION_H >> %src_dir%version.h

