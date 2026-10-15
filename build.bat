@echo off
setlocal

if not exist bin mkdir bin

rem Optional instrumentation:  build.bat profile
set "PROF_CL="
set "PROF_GCC="
if /I "%~1"=="profile" (
    set "PROF_CL=/DMICROSHELL_PROFILE /I..\microprof\include"
    set "PROF_GCC=-DMICROSHELL_PROFILE -I../microprof/include"
)

where cl >nul 2>nul
if errorlevel 1 goto try_clang

echo Building bin\microshell.exe with cl...
cl /nologo /DBASE_IMPLEMENTATION %PROF_CL% /Zi /Od /Iinclude /W4 /Fo:bin\ /Fe:bin\microshell.exe src\main.c
if errorlevel 1 goto build_failed
echo Done.
exit /b 0

:try_clang
where clang >nul 2>nul
if errorlevel 1 goto try_gcc

echo Building bin\microshell.exe with clang...
clang -DBASE_IMPLEMENTATION -g -O0 -Iinclude %PROF_GCC% -Wall -Wextra -Werror src\main.c -o bin\microshell.exe
if errorlevel 1 goto build_failed
echo Done.
exit /b 0

:try_gcc
where gcc >nul 2>nul
if errorlevel 1 goto no_compiler

echo Building bin\microshell.exe with gcc...
gcc -DBASE_IMPLEMENTATION -g -O0 -Iinclude %PROF_GCC% -Wall -Wextra -Werror src\main.c -o bin\microshell.exe
if errorlevel 1 goto build_failed
echo Done.
exit /b 0

:no_compiler
echo No C compiler found. Install cl, clang, or gcc and make sure it is on PATH.
exit /b 1

:build_failed
echo Build failed.
exit /b 1
