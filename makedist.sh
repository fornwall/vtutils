#!/bin/sh

./prepare.sh

mkdir build
cd build
../configure
make dist
