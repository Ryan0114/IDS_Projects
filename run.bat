@echo off
g++ *.cpp -o main.exe
if ERRORLEVEL 1 (
        echo Compilation failed.
        pause
        exit /b
)
main.exe
pause
