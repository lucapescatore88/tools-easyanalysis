#!/bin/bash

shopt -s expand_aliases

if [ ! -n "${TOOLSSYS+x}" ]; then
    export TOOLSSYS="$( cd . "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

if [ ! -n "${LD_LIBRARY_PATH+x}" ]; then
    export LD_LIBRARY_PATH
fi
if [ ! -n "${LD_INCLUDE_PATH+x}" ]; then
    export LD_INCLUDE_PATH
fi
if [ ! -n "${PYTHONPATH+x}" ]; then
    export PYTHONPATH
fi

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
SYSPYTOOLS=$LCGDIR/releases/pytools
SYSPYANALYSIS=$LCGDIR/releases/pyanalysis
if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ]; then
    VER=2.7.10-8dd46
    export PYTHONSYS=$SYS/$VER/$ARCH
    if [ -d $PYTHONSYS ]; then
        export PATH=$PYTHONSYS/bin:$PATH
        export LD_LIBRARY_PATH=$PYTHONSYS/lib:$LD_LIBRARY_PATH
        echo "Configuring PYTHON from $PYTHONSYS"
    else
        echo
        echo "PYTHON $PYTHONSYS not available"
        echo
    fi
fi
if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYSPYTOOLS"` == 0 ]; then
    VER=2.0-93db0
    export PYTOOLSSYS=$SYSPYTOOLS/$VER/$ARCH
    if [ -d $PYTOOLSSYS ]; then
        export PATH=$PYTOOLSSYS/bin:$PATH
        export PYTHONPATH=$PYTOOLSSYS/lib/python2.7/site-packages/:$PYTHONPATH
        echo "Configuring PYTHON TOOLS from $PYTOOLSSYS"
    else
        echo
        echo "PYTHON TOOLS $PYTOOLSSYS not available"
        echo
    fi
fi
if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYSPYANALYSIS"` == 0 ]; then
    VER=2.0-32412
    export PYANALYSISSYS=$SYSPYANALYSIS/$VER/$ARCH
    if [ -d $PYANALYSISSYS ]; then	
        export PYTHONPATH=$PYANALYSISSYS/lib/python2.7/site-packages/:$PYTHONPATH
        echo "Configuring PYTHON ANALYSIS from $PYANALYSISSYS"
    else
        echo
        echo "PYTHON ANALYSIS $PYANALYSISSYS not available"
        echo
    fi
fi
export PYTHONPATH=$TOOLSSYS/python:$PYTHONPATH

# GSL
SYS=$LCGDIR/releases/GSL
if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ]; then
    VER=2.1-36ee5
    VER=$VER/$ARCH
    if [ -d $SYS/$VER ]; then
        export GSLSYS=$SYS/$VER
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

echo
echo "Configuring PATH               to $PATH"
echo "Configuring LD_LIBRARY_PATH    to $LD_LIBRARY_PATH"
echo "Configuring LD_INCLUDE_PATH    to $LD_INCLUDE_PATH"
echo "Configuring PYTHONPATH         to $PYTHONPATH"
echo
