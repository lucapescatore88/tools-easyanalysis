#!/bin/bash

OPT="$1"

if [ "$OPT" == "p" ]; then
    cd $TOOLSSYS
    echo
    echo "PURGE"
    rm -f *~ */*~ */*/*~
    rm -f *.root *.pdf *.png *.eps *.C
    echo
    exit
fi

if [ "$OPT" == "ca" ]; then
    cd $TOOLSSYS/build
    echo
    echo "CLEANALL"
    rm -rf *
    echo
    exit
fi

export MAKEFLAGS=-j8

if [ ! -d $TOOLSSYS/build ]; then
    mkdir -p $TOOLSSYS/build
fi

cd $TOOLSSYS/build

if [ "$OPT" != "m" ]; then
    echo
    echo "CMAKE"
    cmake ..
fi

echo
echo "MAKE"
make

echo
