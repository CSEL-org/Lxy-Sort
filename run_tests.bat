@echo off
REM run_tests.bat - Windows one-click build + run all tests (pure ASCII)
REM   run_tests.bat          (build + correctness/stability tests + minimal example)
REM   run_tests.bat bench    (also run the performance benchmark vs std::sort)
setlocal
set CXX=g++
set FLAGS=-O2 -std=c++17 -fopenmp

echo ===== Build =====
%CXX% %FLAGS% -Wall -o stress.exe  stress.cpp  >nul 2>&1
%CXX% %FLAGS% -Wall -o stress2.exe stress2.cpp >nul 2>&1
%CXX% %FLAGS% -Wall -o example.exe example.cpp >nul 2>&1

echo.
echo ===== Correctness + Stability =====
stress.exe
stress2.exe

echo.
echo ===== Minimal usage example =====
example.exe

if /I "%1"=="bench" (
    echo.
    echo ===== Performance benchmark =====
    %CXX% %FLAGS% -Wall -o lxy-test.exe lxy-test.cpp >nul 2>&1
    lxy-test.exe
)
echo.
echo ===== All done =====
endlocal
