#!/bin/tcsh

set OPT = "$1"

if ( "$OPT" == "p" ) then
    cd $TOOLSSYS
    echo
    echo "PURGE"
    rm -f *~ */*~ */*/*~
    rm -f *.root *.pdf *.png *.eps *.C
    echo
    exit
endif

if ( "$OPT" == "ca" ) then
    cd $TOOLSSYS/build
    echo
    echo "CLEANALL"
    rm -rf *
    echo
    exit
endif

setenv MAKEFLAGS -j8

if ( ! -d $TOOLSSYS/build ) then
    mkdir -p $TOOLSSYS/build
endif

cd $TOOLSSYS/build

if ( "$OPT" != "m" ) then
    echo
    echo "CMAKE"
    cmake ..
endif

echo
echo "MAKE"
make

echo
