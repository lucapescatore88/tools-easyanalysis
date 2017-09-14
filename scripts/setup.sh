#!/bin/bash

shopt -s expand_aliases

if [ ! -n "${REPOSYS+x}" ]; then
    export REPOSYS="$( cd . "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

if [ ! -n "${TOOLSSYS+x}" ]; then
    export TOOLSSYS=$REPOSYS
fi
#if [ ! -n "${CPLUS_INCLUDE_PATH+x}" ]; then
#    export CPLUS_INCLUDE_PATH
#fi
if [ ! -n "${LD_LIBRARY_PATH+x}" ]; then
    export LD_LIBRARY_PATH
fi
if [ ! -n "${LD_INCLUDE_PATH+x}" ]; then
    export LD_INCLUDE_PATH
fi
if [ ! -n "${PYTHONPATH+x}" ]; then
    export PYTHONPATH
fi

#export CPLUS_INCLUDE_PATH=$TOOLSSYS:$TOOLSSYS/src:$TOOLSSYS/roofit:$TOOLSSYS/roofit/src:$CPLUS_INCLUDE_PATH
export LD_LIBRARY_PATH=$TOOLSSYS/roofit/dic:$LD_LIBRARY_PATH

echo
echo "Setup tools-easyanalysis"
echo
echo "Configuring TOOLSSYS to $TOOLSSYS"
echo

# LIBS
LCGDIR=/cvmfs/lhcb.cern.ch/lib/lcg
ARCH=x86_64-slc6-gcc49-opt

# GCC
SYS=$LCGDIR/releases/gcc
if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ]; then
    VER=4.9.3
    VER=$VER/x86_64-slc6
    if [ -f $SYS/$VER/setup.sh ]; then
	export GCCSYS=$SYS/$VER
	source $GCCSYS/setup.sh $LCGDIR/external

	echo "Configuring GCC    from $GCCSYS"
    else
	echo
	echo "GCC $GCCSYS not available"
	echo
    fi
fi

# PYTHON
SYS=$LCGDIR/releases/Python
if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ]; then
    VER=2.7.6-31787
    VER=$VER/$ARCH
    if [ -d $SYS/$VER ]; then
	export PYTHONSYS=$SYS/$VER
	export LD_LIBRARY_PATH=$PYTHONSYS/lib:$LD_LIBRARY_PATH
	export PYTHONPATH=$TOOLSSYS/python:$PYTHONPATH
	export PYTHONPATH=/usr/lib64/python2.6/site-packages:$PYTHONPATH
	export PYTHONPATH=/usr/lib/python2.6/site-packages:$PYTHONPATH

	echo "Configuring PYTHON from $PYTHONSYS"
    else
	echo
	echo "PYTHON $PYTHONSYS not available"
	echo
    fi
fi

# GSL
SYS=$LCGDIR/releases/GSL
if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ]; then
    VER=2.1-36ee5
    VER=$VER/$ARCH
    if [ -d $SYS/$VER ]; then
	export GSLSYS=$SYS/$VER
	#export CPLUS_INCLUDE_PATH=$GSLSYS/include:$CPLUS_INCLUDE_PATH
        export LD_LIBRARY_PATH=$GSLSYS/lib:$LD_LIBRARY_PATH
        export LD_INCLUDE_PATH=$GSLSYS/include:$LD_INCLUDE_PATH

	echo "Configuring GSL    from $GSLSYS"
    else
	echo
	echo "GSL $GSLSYS not available"
	echo
    fi
fi

# ROOT
SYS=$LCGDIR/releases/ROOT
if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ]; then
    VER=6.08.06-c8fb4
    VER=$VER/$ARCH
    if [ -d $SYS/$VER ]; then
	export ROOTSYS=$SYS/$VER
        export PATH=$ROOTSYS/bin:${PATH}
        export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
        export LD_INCLUDE_PATH=$ROOTSYS/include:$LD_INCLUDE_PATH
        export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH

	echo "Configuring ROOT   from $ROOTSYS"
    else
	echo
	echo "ROOT $ROOTSYS not available"
	echo
    fi
fi

#echo
#echo "Configuring CPLUS_INCLUDE_PATH to $CPLUS_INCLUDE_PATH"
echo
echo "Configuring PATH               to $PATH"
echo "Configuring LD_LIBRARY_PATH    to $LD_LIBRARY_PATH"
echo "Configuring LD_INCLUDE_PATH    to $LD_INCLUDE_PATH"
echo "Configuring PYTHONPATH         to $PYTHONPATH"
echo
