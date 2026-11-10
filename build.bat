@echo off
REM Build with MinGW gcc on Windows. Run this from the project folder.
gcc -Wall -Wextra -std=c99 -pedantic -I. -o pizza.exe main.c utils.c fileio.c auth.c menu.c customer.c order.c stats.c
if errorlevel 1 goto fail
echo Build OK: pizza.exe

gcc -Wall -Wextra -std=c99 -pedantic -I. -o test_utils.exe tests\test_utils.c utils.c
if errorlevel 1 goto fail
gcc -Wall -Wextra -std=c99 -pedantic -I. -o test_auth.exe tests\test_auth.c auth.c utils.c customer.c fileio.c
if errorlevel 1 goto fail
echo Tests compiled: test_utils.exe and test_auth.exe
echo Run them from this folder: test_utils.exe  then  test_auth.exe
goto end

:fail
echo Build FAILED

:end
pause
