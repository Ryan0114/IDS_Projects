@echo off
g++ *.cpp -I "C:\Program Files\eigen-5.0.0" -O2 -o main.exe
if ERRORLEVEL 1 (
        echo Compilation failed.
        pause
        exit /b
)
main.exe
pause
