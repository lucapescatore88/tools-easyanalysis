#!/bin/bash

shopt -s expand_aliases

# LIBS
export CVMFS=/cvmfs/lhcb.cern.ch
export LCGSYS=$CVMFS/lib/lcg
export ARCH=x86_64-slc6-gcc49-opt

if [ ! -d $LCGSYS ]; then
    echo
    echo "Cannot access $LCGSYS"
    echo
    return
fi

if [ ! -n "${LD_INCLUDE_PATH+x}" ]; then
    export LD_INCLUDE_PATH
fi
if [ ! -n "${LD_LIBRARY_PATH+x}" ]; then
    export LD_LIBRARY_PATH
fi
if [ ! -n "${ROOT_INCLUDE_PATH+x}" ]; then
    export ROOT_INCLUDE_PATH
fi
if [ ! -n "${PYTHONPATH+x}" ]; then
    export PYTHONPATH
fi

if [ ! -n "${TOOLSSYS+x}" ]; then
    export TOOLSSYS="$( cd . "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

    export LD_LIBRARY_PATH=$TOOLSSYS/roofit/dic:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$TOOLSSYS/build/tools:$TOOLSSYS/build/roofit:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$TOOLSSYS/lib:$LD_LIBRARY_PATH

    export LD_INCLUDE_PATH=${TOOLSSYS}:$TOOLSSYS/tools:$TOOLSSYS/roofit:$LD_INCLUDE_PATH

    export ROOT_INCLUDE_PATH=${TOOLSSYS}:$TOOLSSYS/tools:$TOOLSSYS/roofit:$ROOT_INCLUDE_PATH

    export PYTHONPATH=$TOOLSSYS/python:$PYTHONPATH

    echo
    echo "Setup tools-easyanalysis"
    echo
    echo "Configuring TOOLSSYS to $TOOLSSYS"
    echo

    source $TOOLSSYS/scripts/setup.sh cmake
    source $TOOLSSYS/scripts/setup.sh gcc
    source $TOOLSSYS/scripts/setup.sh python
    source $TOOLSSYS/scripts/setup.sh gsl
    source $TOOLSSYS/scripts/setup.sh root

    source $TOOLSSYS/scripts/setup.sh env
fi

# CASES
case "$1" in

    env)
        echo
        echo "Configuring PATH              to $PATH"
        echo "Configuring LD_LIBRARY_PATH   to $LD_LIBRARY_PATH"
        echo "Configuring LD_INCLUDE_PATH   to $LD_INCLUDE_PATH"
        echo "Configuring ROOT_INCLUDE_PATH to $ROOT_INCLUDE_PATH"
        echo "Configuring PYTHONPATH        to $PYTHONPATH"
        echo

        ;;

    cmake)
        SYS=$CVMFS/lib/contrib/CMake
        VER=3.7.2
        VER=$VER/Linux-x86_64
        if [ `echo "$PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export CMAKESYS=$SYS/$VER
                export PATH=$CMAKESYS/bin:$PATH

                echo "Configuring CMAKE      from $CMAKESYS"
            else
                echo
                echo "CMAKE $SYS/$VER not available"
                echo
            fi
        fi

        ;;

    gcc)
        SYS=$LCGSYS/releases/gcc
        VER=4.9.3
        VER=$VER/x86_64-slc6
        if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -f $SYS/$VER/setup.sh ]; then
                export GCCSYS=$SYS/$VER
                source $GCCSYS/setup.sh $LCGSYS/external

                echo "Configuring GCC        from $GCCSYS"
            else
                echo
                echo "GCC $SYS/$VER not available"
                echo
            fi
        fi

        ;;

    python)
        SYS=$LCGSYS/releases/Python
        VER=2.7.13-597a5
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export PYTHONSYS=$SYS/$VER
                export PATH=$PYTHONSYS/bin:$PATH
                export LD_LIBRARY_PATH=$PYTHONSYS/lib:$LD_LIBRARY_PATH
                #export PYTHONPATH=$PYTHONSYS/lib/python2.7:$PYTHONPATH
		export PYTHONSTARTUP=$HOME/.pythonstartup.py

                echo "Configuring PYTHON     from $PYTHONSYS"

                source $TOOLSSYS/scripts/setup.sh pyanalysis
                source $TOOLSSYS/scripts/setup.sh pytools
            else
                echo
                echo "PYTHON $SYS/$VER not available"
                echo
            fi
        fi

        ;;

    pyanalysis)
        SYS=$LCGSYS/releases/pyanalysis
        VER=2.0-32412
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export PYANALYSISSYS=$SYS/$VER
                export PATH=$PYANALYSISSYS/bin:$PATH
                export LD_LIBRARY_PATH=$PYANALYSISSYS/lib/python2.7/site-packages:$LD_LIBRARY_PATH
                export PYTHONPATH=$PYANALYSISSYS/lib/python2.7/site-packages:$PYTHONPATH

                echo "Configuring PYANALYSIS from $PYANALYSISSYS"
            else
                echo
                echo "PYANALYSIS $SYS/$VER not available"
                echo
            fi
        fi

        ;;

    pytools)
        SYS=$LCGSYS/releases/pytools
        VER=2.0-93db0
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export PYTOOLSSYS=$SYS/$VER
                export PATH=$PYTOOLSSYS/bin:$PATH
                export LD_LIBRARY_PATH=$PYTOOLSSYS/lib/python2.7/site-packages:$LD_LIBRARY_PATH
                export PYTHONPATH=$PYTOOLSSYS/lib/python2.7/site-packages:$PYTHONPATH

                echo "Configuring PYTOOLS    from $PYTOOLSSYS"
            else
                echo
                echo "PYTOOLS $SYS/$VER not available"
                echo
            fi
        fi

        ;;

    gsl)
        SYS=$LCGSYS/releases/GSL
        VER=2.1-36ee5
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export GSLSYS=$SYS/$VER
                export LD_LIBRARY_PATH=$GSLSYS/lib:$LD_LIBRARY_PATH
                export LD_INCLUDE_PATH=$GSLSYS/include:$LD_INCLUDE_PATH

                echo "Configuring GSL        from $GSLSYS"
            else
                echo
                echo "GSL $SYS/$VER not available"
                echo
            fi
        fi

        ;;

    root)
        SYS=$LCGSYS/releases/ROOT
        VER=6.08.06-c8fb4
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export ROOTSYS=$SYS/$VER
                export PATH=$ROOTSYS/bin:${PATH}
                export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
                export LD_INCLUDE_PATH=$ROOTSYS/include:$LD_INCLUDE_PATH
                export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH

                echo "Configuring ROOT       from $ROOTSYS"
            else
                echo
                echo "ROOT $SYS/$VER not available"
                echo
            fi
        fi

        ;;

esac
