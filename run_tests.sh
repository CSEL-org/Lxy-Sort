#!/usr/bin/env bash
# run_tests.sh - one-click build + run all tests (pure ASCII)
#   ./run_tests.sh [bench]
#   no arg: build + correctness/stability tests + minimal example
#   bench  : also run the performance benchmark (vs std::sort)
set -e
CXX=${CXX:-g++}
FLAGS="-O2 -std=c++17 -fopenmp"

echo "===== Build ====="
$CXX $FLAGS -Wall -o stress.exe    stress.cpp    2>&1 | grep -iE 'warning|error' | grep -viE 'NOMINMAX|redefined|os_defines' || true
$CXX $FLAGS -Wall -o stress2.exe   stress2.cpp   2>&1 | grep -iE 'warning|error' | grep -viE 'NOMINMAX|redefined|os_defines' || true
$CXX $FLAGS -Wall -o example.exe   example.cpp   2>&1 | grep -iE 'warning|error' | grep -viE 'NOMINMAX|redefined|os_defines' || true

echo ""
echo "===== Correctness + Stability (stress) ====="
./stress.exe
./stress2.exe

echo ""
echo "===== Minimal usage example ====="
./example.exe

if [ "$1" == "bench" ]; then
    echo ""
    echo "===== Performance benchmark (vs std::sort) ====="
    $CXX $FLAGS -Wall -o lxy-test.exe lxy-test.cpp 2>&1 | grep -iE 'warning|error' | grep -viE 'NOMINMAX|redefined|os_defines' || true
    ./lxy-test.exe
fi
echo ""
echo "===== All done ====="
