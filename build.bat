@echo off
REM Build with MinGW gcc on Windows
gcc -Wall -Wextra -std=c99 -pedantic -o pizza.exe main.c utils.c fileio.c auth.c menu.c customer.c order.c stats.c
if errorlevel 1 (echo Build FAILED) else (echo Build OK: pizza.exe)
pause
