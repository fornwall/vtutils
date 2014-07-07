#!/bin/sh

set -e -u

./prepare.sh

rm -Rf build
mkdir build
cd build
../configure --prefix=$HOME/lib/prefix
make install
make dist
