#!/bin/sh

make clean;
make install;
/Applications/Pd-0.48-1-i386.app/Contents/Resources/bin/pd -open "/Users/larralde/Desktop/pd-gbend-testing.pd" &