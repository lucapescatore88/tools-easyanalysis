#!/bin/bash

shopt -s expand_aliases

SWITCH="$1"
if [ "$SWITCH" == "" ]; then
    SWITCH="lcg"
fi
PKG=`echo $SWITCH | awk '{ print toupper( $0 ) }'`

export LCG=/cvmfs/sft.cern.ch/lcg

export CVMFS=/cvmfs/lhcb.cern.ch
export LCGSYS=$CVMFS/lib/lcg

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
    export TOOLSSYS="$( command cd . "$( dirname "${BASH_SOURCE[0]}" )" && command pwd )"

    export LD_LIBRARY_PATH=$TOOLSSYS/roofit/dic:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$TOOLSSYS/build:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$TOOLSSYS/lib:$LD_LIBRARY_PATH

    export LD_INCLUDE_PATH=${TOOLSSYS}:$TOOLSSYS/tools:$TOOLSSYS/roofit:$LD_INCLUDE_PATH

    export ROOT_INCLUDE_PATH=${TOOLSSYS}:$TOOLSSYS/tools:$TOOLSSYS/roofit:$ROOT_INCLUDE_PATH

    export PYTHONPATH=$TOOLSSYS/python:$PYTHONPATH

    echo
    echo "Setup tools-easyanalysis"
    echo
    echo "Configuring TOOLSSYS to $TOOLSSYS"
    echo

    source $TOOLSSYS/scripts/setup.sh arch
    if [ $ARCH == "Darwin" ]; then
        source $TOOLSSYS/scripts/setup.sh env
        return
    fi
    source $TOOLSSYS/scripts/setup.sh cmake
    source $TOOLSSYS/scripts/setup.sh gcc
    source $TOOLSSYS/scripts/setup.sh python
    source $TOOLSSYS/scripts/setup.sh pyanalysis
    source $TOOLSSYS/scripts/setup.sh pytools
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
        if [ $ARCH == "Darwin" ]; then
            echo "Configuring DYLD_LIBRARY_PATH to $DYLD_LIBRARY_PATH"
        fi
        echo "Configuring ROOT_INCLUDE_PATH to $ROOT_INCLUDE_PATH"
        echo "Configuring PYTHONPATH        to $PYTHONPATH"
        echo

        ;;

    arch)
        export ARCH=`uname`
        if [ $ARCH == "Linux" ]; then
            if [ `cat /etc/redhat-release | grep -ie "Scientific" | grep -ie "release 6" | wc -l` == 1 ]; then
                export ARCH=x86_64-slc6-gcc49-opt
                #export ARCH=x86_64-slc6-gcc62-opt
            fi
            if [ `cat /etc/redhat-release | grep -ie "CentOS" | grep -ie "release 7" | wc -l` == 1 ]; then
                export ARCH=x86_64-centos7-gcc62-opt
            fi
        fi
        if [ "$2" != "" ]; then
            export ARCH=$2
        fi

        printf "Configuring %-10s to   %-1s \n" $PKG $ARCH

        ;;

    lcg)
        SYS=/cvmfs/sft.cern.ch/lcg/views
        VER=LCG_92
        if [ "$2" != "" ]; then
            VER=LCG_$2
        fi
        export LCGSYS=$LCG
        export LCGVER=$VER
        source $REPOSYS/scripts/setup.sh arch
        if [ `echo "$ARCH" | grep -ci "slc6"` == 1 ]; then
            echo
            echo "Please use lxplus7.cern.ch"
            echo
            export ARCH=""
            return
        fi
        source $REPOSYS/scripts/setup.sh batch
        if [ $ARCH == "Darwin" ]; then
            return
        fi
        PKG="LCG"
        VER=$VER/$ARCH
        if [ `echo "$PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -f $SYS/$VER/setup.sh ]; then
                source $SYS/$VER/setup.sh

                if [ `echo "$ARCH" | grep -ci "centos"` == 1 ]; then
                    export CC=gcc
                    export CXX=g++
                fi

                export PYTHONUSERBASE=$REPOSYS/python/local
                #export PYTHONPATH=$PYTHONUSERBASE/lib/python2.7/site-packages:$PYTHONPATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$LCGVER

                source $REPOSYS/scripts/setup.sh lcgenv
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$LCGVER
                echo
            fi
        fi

        ;;

    lcgenv)
        SYS=$LCGSYS/releases/lcgenv
        VER=latest
        if [ `echo "$PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -f $SYS/$VER/lcgenv ]; then
                export LCGENVSYS=$SYS/$VER
                export LCGENV_PATH=$LCGSYS/releases
                export PATH=${LCGENVSYS}:$PATH

                alias lcgpkg="source $REPOSYS/scripts/setup.sh lcgpkg"

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            fi
        fi

        ;;

    lcgpkg)
        if [ "$2" == "" ]; then
            echo
            lcgenv -p $LCGVER $ARCH
            echo
        else
            PKG=`echo $2 | awk '{ print toupper( $0 ) }'`
            DUMMY=$REPOSYS/scripts/setup_lcg.sh
            if [ -f $DUMMY ]; then
                rm -rf $DUMMY
            fi
            lcgenv -p $LCGVER $ARCH $2 > $DUMMY
            SRC=`grep -e "export PATH" $DUMMY | head -n 1 | awk '{ print $2 }' | sed s:'PATH='::g | sed s:'"'::g | sed s:"/$ARCH":" ": | awk '{ print $1 }'`
            if [ `echo "$LD_LIBRARY_PATH" | grep -ci "$SRC"` == 0 ]; then
                printf "Configuring %-10s from %-1s \n" $PKG $SRC
                source $DUMMY
            fi
            rm -rf $DUMMY
        fi

        ;;

    cmake)
        SYS=$CVMFS/lib/contrib/CMake
        VER=3.7.2
        VER=$VER/Linux-x86_64
        if [ `echo "$PATH" | grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export CMAKESYS=$SYS/$VER
                export PATH=$CMAKESYS/bin:$PATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
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

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            fi
        fi

        ;;

    python)
        SYS=$LCGSYS/releases/Python
        VER=2.7.13-597a5
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | command grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export PYTHONSYS=$SYS/$VER
                export PATH=$PYTHONSYS/bin:$PATH
                export LD_LIBRARY_PATH=$PYTHONSYS/lib:$LD_LIBRARY_PATH
                #export PYTHONPATH=$PYTHONSYS/lib/python2.7:$PYTHONPATH
                export PYTHONSTARTUP=$HOME/.pythonstartup.py

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            fi
        fi

        ;;

    pyanalysis)
        SYS=$LCGSYS/releases/pyanalysis
        VER=2.0-32412
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | command grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export PYANALYSISSYS=$SYS/$VER
                export PATH=$PYANALYSISSYS/bin:$PATH
                export LD_LIBRARY_PATH=$PYANALYSISSYS/lib/python2.7/site-packages:$LD_LIBRARY_PATH
                export PYTHONPATH=$PYANALYSISSYS/lib/python2.7/site-packages:$PYTHONPATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            fi
        fi

        ;;

    pytools)
        SYS=$LCGSYS/releases/pytools
        VER=2.0-93db0
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | command grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export PYTOOLSSYS=$SYS/$VER
                export PATH=$PYTOOLSSYS/bin:$PATH
                export LD_LIBRARY_PATH=$PYTOOLSSYS/lib/python2.7/site-packages:$LD_LIBRARY_PATH
                export PYTHONPATH=$PYTOOLSSYS/lib/python2.7/site-packages:$PYTHONPATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            fi
        fi

        ;;

    gsl)
        SYS=$LCGSYS/releases/GSL
        VER=2.1-36ee5
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | command grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export GSLSYS=$SYS/$VER
                export PATH=$GSLSYS/bin:$PATH
                export LD_LIBRARY_PATH=$GSLSYS/lib:$LD_LIBRARY_PATH
                export LD_INCLUDE_PATH=$GSLSYS/include:$LD_INCLUDE_PATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            fi
        fi

        ;;

    root)
        SYS=$LCGSYS/releases/ROOT
        VER=6.08.06-c8fb4
        VER=$VER/$ARCH
        if [ `echo "$LD_LIBRARY_PATH" | command grep -ci "$SYS/$VER"` == 0 ]; then
            if [ -d $SYS/$VER ]; then
                export ROOTSYS=$SYS/$VER
                export PATH=$ROOTSYS/bin:${PATH}
                export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
                export LD_INCLUDE_PATH=$ROOTSYS/include:$LD_INCLUDE_PATH
                export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            fi
        fi

        ;;

esac
