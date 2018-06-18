#!/bin/sh

# this creates a jl directory for publication with deken :

JLDIR=../jl
# JLDIR=/Users/larralde/Documents/Pd/externals/jl

rm -rf $JLDIR;

make clean;
make install;
make clean;

cp ../abstractions/* $JLDIR;
cp ../helpfiles/* $JLDIR;

mkdir $JLDIR"/src";
cp -r ./* $JLDIR"/src";

# or
# modify the makefile to install binaries in ~/Documents/Pd/externals, switch JLDIR above
# and comment the lines that copy the sources

# then you can run some test patch after build :
# /Applications/Pd-0.48-1-i386.app/Contents/Resources/bin/pd -open "/Users/larralde/Desktop/pd-gbend-testing.pd" &