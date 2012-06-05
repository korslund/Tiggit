#!/bin/bash

if [ "$1" == "clean" ]; then
    make clean
    rm -rf CMakeFiles/ CMakeCache.txt cmake_install.cmake Makefile
    exit
fi

test -f Makefile || test -f CMakeLists.txt && cmake . || exit

make || exit

mkdir -p output

PROGS=*_test

for a in $PROGS; do
    if [ -f "output/$a.out" ]; then
        echo "Running $a:"
        ./$a | diff output/$a.out -
    else
        echo "Creating $a.out"
        ./$a > "output/$a.out"
        git add "output/$a.out"
    fi
done
