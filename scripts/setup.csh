#!/bin/tcsh

set SWITCH = "$1"
set PKG = `echo $SWITCH | awk '{ print toupper( $0 ) }'`

setenv LCG /cvmfs/sft.cern.ch/lcg

if ( ! ($?LD_INCLUDE_PATH) )   setenv LD_INCLUDE_PATH
if ( ! ($?LD_LIBRARY_PATH) )   setenv LD_LIBRARY_PATH
if ( ! ($?ROOT_INCLUDE_PATH) ) setenv ROOT_INCLUDE_PATH
if ( ! ($?PYTHONPATH) )        setenv PYTHONPATH
if ( ! ($?GLIMPSEPATH) )       setenv GLIMPSEPATH

if ( ! ($?TOOLSSYS) ) then
    set EXE = "$0"
    set EXE = `echo $EXE | sed s:"-"::`
    set EXE = `basename $EXE`

    setenv TOOLSSYS `readlink -f "$EXE"`
    setenv TOOLSSYS `echo $TOOLSSYS | sed s:"/$EXE"::`

    setenv LD_LIBRARY_PATH $TOOLSSYS/roofit/dic:$LD_LIBRARY_PATH
    setenv LD_LIBRARY_PATH $TOOLSSYS/build:$LD_LIBRARY_PATH
    setenv LD_LIBRARY_PATH $TOOLSSYS/lib:$LD_LIBRARY_PATH

    setenv LD_INCLUDE_PATH ${TOOLSSYS}:$TOOLSSYS/tools:$TOOLSSYS/roofit:$LD_INCLUDE_PATH

    setenv ROOT_INCLUDE_PATH ${TOOLSSYS}:$TOOLSSYS/tools:$TOOLSSYS/roofit:$ROOT_INCLUDE_PATH

    setenv PYTHONPATH $TOOLSSYS/python:$PYTHONPATH

    setenv GLIMPSEPATH "$GLIMPSEPATH $TOOLSSYS/tools $TOOLSSYS/roofit"

    alias git_setup "source $TOOLSSYS/scripts/setup.csh"
    alias git_glimpse "$TOOLSSYS/scripts/glimpse.csh"

    alias cMake "$TOOLSSYS/scripts/make.csh"

    echo
    echo "Setup tools-easyanalysis"
    echo
    echo "Configuring TOOLSSYS to $TOOLSSYS"
    echo

    if ( "$SWITCH" == "old" ) then
        source $TOOLSSYS/scripts/setup.csh old
        source $TOOLSSYS/scripts/setup.csh cmake
        source $TOOLSSYS/scripts/setup.csh gcc
        source $TOOLSSYS/scripts/setup.csh python
        source $TOOLSSYS/scripts/setup.csh pyanalysis
        source $TOOLSSYS/scripts/setup.csh pytools
        source $TOOLSSYS/scripts/setup.csh gsl
        source $TOOLSSYS/scripts/setup.csh root
    else
        source $TOOLSSYS/scripts/setup.csh arch
        source $TOOLSSYS/scripts/setup.csh lcg
    endif

    source $TOOLSSYS/scripts/setup.csh env
    exit
endif

# CASES
switch ( "$SWITCH" )

    case env:
        echo
        echo "Configuring PATH              to $PATH"
        echo "Configuring LD_LIBRARY_PATH   to $LD_LIBRARY_PATH"
        echo "Configuring LD_INCLUDE_PATH   to $LD_INCLUDE_PATH"
        echo "Configuring ROOT_INCLUDE_PATH to $ROOT_INCLUDE_PATH"
        echo "Configuring PYTHONPATH        to $PYTHONPATH"
        echo

        breaksw

    case arch:
        setenv ARCH `uname`
        if ( $ARCH == "Linux" ) then
            if ( `cat /etc/redhat-release | grep -ie "Scientific" | grep -ie "release 6" | wc -l` == 1 ) setenv ARCH x86_64-slc6-gcc62-opt
            if ( `cat /etc/redhat-release | grep -ie "CentOS" | grep -ie "release 7" | wc -l` == 1 ) setenv ARCH x86_64-centos7-gcc62-opt
        endif
        if ( "$2" != "" ) setenv ARCH $2

        printf "Configuring %-10s to   %-1s \n" $PKG $ARCH

        breaksw

    case lcg:
        set SYS = $LCG/views
        set VER = LCG_92
        if ( "$2" != "" ) set VER = LCG_$2
        setenv LCGSYS $LCG
        setenv LCGVER $VER
        #if ( `echo "$ARCH" | grep -ci "slc6"` == 1 ) then
        #    echo
        #    echo "Please use centos (e.g. lxplus7.cern.ch)"
        #    echo
        #    setenv ARCH ""
        #    exit
        #endif
        set PKG = "LCG"
        set VER = $VER/$ARCH
        if ( `echo "$PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -f $SYS/$VER/setup.csh ) then
                #source $SYS/$VER/setup.csh
                set DUMMY = $TOOLSSYS/scripts/setup_lcg.csh
                if ( -f $DUMMY ) rm -rf $DUMMY
                touch $DUMMY
                echo "set thisfile=$SYS/$VER/setup.csh" >> $DUMMY
                echo "set thisdir=$SYS/$VER" >> $DUMMY
                tail -n+13 $SYS/$VER/setup.csh >> $DUMMY
                source $DUMMY
                rm -rf $DUMMY

                if ( `echo "$ARCH" | grep -ci "centos"` == 1 ) then
                    setenv CC gcc
                    setenv CXX g++
                endif

                setenv PYTHONUSERBASE $TOOLSSYS/python/local
                #setenv PYTHONPATH $PYTHONUSERBASE/lib/python2.7/site-packages:$PYTHONPATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$LCGVER

                source $TOOLSSYS/scripts/setup.csh lcgenv
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$LCGVER
                echo
            endif
        endif

        breaksw

    case lcgenv:
        set SYS = $LCGSYS/releases/lcgenv
        set VER = latest
        if ( `echo "$PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -f $SYS/$VER/lcgenv ) then
                setenv LCGENVSYS $SYS/$VER
                setenv LCGENV_PATH $LCGSYS/releases
                setenv PATH ${LCGENVSYS}:$PATH

                alias lcgpkg "source $TOOLSSYS/scripts/setup.csh lcgpkg"

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            endif
        endif

        breaksw

    case lcgpkg:
        if ( "$2" == "" ) then
            echo
            lcgenv -p $LCGVER $ARCH
            echo
        else
            set PKG = `echo $2 | awk '{ print toupper( $0 ) }'`
            set DUMMY = $TOOLSSYS/scripts/setup_lcg.csh
            if ( -f $DUMMY ) rm -rf $DUMMY
            lcgenv -p $LCGVER $ARCH $2 > $DUMMY
            set SRC = `grep -e "setenv PATH" $DUMMY | head -n 1 | awk '{ print $3 }' | sed s:'"'::g | sed s:"/$ARCH":" ": | awk '{ print $1 }'`
            if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SRC"` == 0 ) then
                printf "Configuring %-10s from %-1s \n" $PKG $SRC
                source $DUMMY
            endif
            rm -rf $DUMMY
        endif

        breaksw

    case old:
        setenv CVMFS /cvmfs/lhcb.cern.ch
        setenv LCGSYS $CVMFS/lib/lcg
        source $TOOLSSYS/scripts/setup.csh arch
        if ( `echo "$ARCH" | grep -ci "slc6"` == 0 ) then
            echo
            echo "Wrong platform, please use x86_64-slc6"
            echo
            setenv ARCH ""
            exit
        endif
        source $TOOLSSYS/scripts/setup.csh arch x86_64-slc6-gcc49-opt

        printf "Configuring %-10s from %-1s \n" "LCG" $LCGSYS

        breaksw

    case cmake:
        set SYS = $CVMFS/lib/contrib/CMake
        set VER = 3.7.2
        set VER = $VER/Linux-x86_64
        if ( `echo "$PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -d $SYS/$VER ) then
                setenv CMAKESYS $SYS/$VER
                setenv PATH $CMAKESYS/bin:$PATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            endif
        endif

        breaksw

    case gcc:
        set SYS = $LCGSYS/releases/gcc
        set VER = 4.9.3
        set VER = $VER/x86_64-slc6
        if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -f $SYS/$VER/setup.csh ) then
                setenv GCCSYS $SYS/$VER
                #source $GCCSYS/setup.csh $LCGSYS/external
                set gcc_config_version = $VER
                set LCGPLAT = x86_64-slc6
                set LCG_lib_name = lib64
                set LCG_arch = x86_64

                set LCG_contdir = $LCGSYS/releases
                #set LCG_gcc_home = ${LCG_contdir}/gcc/${gcc_config_version}/${LCGPLAT}
                set LCG_gcc_home = ${LCG_contdir}/gcc/${gcc_config_version}

                setenv PATH ${LCG_gcc_home}/bin:${PATH}
                setenv COMPILER_PATH ${LCG_gcc_home}/lib/gcc/${LCG_arch}-unknown-linux-gnu/${gcc_config_version}

                if ($?LD_LIBRARY_PATH) then
                    setenv LD_LIBRARY_PATH ${LCG_gcc_home}/${LCG_lib_name}:${LD_LIBRARY_PATH}
                else
                    setenv LD_LIBRARY_PATH ${LCG_gcc_home}/${LCG_lib_name}
                endif

                setenv FC `which gfortran`
                setenv CXX `which g++`
                setenv CC `which gcc`

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            endif
        endif

        breaksw

    case python:
        set SYS = $LCGSYS/releases/Python
        set VER = 2.7.13-597a5
        set VER = $VER/$ARCH
        if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -d $SYS/$VER ) then
                setenv PYTHONSYS $SYS/$VER
                setenv PATH $PYTHONSYS/bin:$PATH
                setenv LD_LIBRARY_PATH $PYTHONSYS/lib:$LD_LIBRARY_PATH
                #setenv PYTHONPATH $PYTHONSYS/lib/python2.7:$PYTHONPATH
                setenv PYTHONSTARTUP $HOME/.pythonstartup.py

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            endif
        endif

        breaksw

    case pyanalysis:
        set SYS = $LCGSYS/releases/pyanalysis
        set VER = 2.0-32412
        set VER = $VER/$ARCH
        if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -d $SYS/$VER ) then
                setenv PYANALYSISSYS $SYS/$VER
                setenv PATH $PYANALYSISSYS/bin:$PATH
                setenv LD_LIBRARY_PATH $PYANALYSISSYS/lib/python2.7/site-packages:$LD_LIBRARY_PATH
                setenv PYTHONPATH $PYANALYSISSYS/lib/python2.7/site-packages:$PYTHONPATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            endif
        endif

        breaksw

    case pytools:
        set SYS = $LCGSYS/releases/pytools
        set VER = 2.0-93db0
        set VER = $VER/$ARCH
        if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -d $SYS/$VER ) then
                setenv PYTOOLSSYS $SYS/$VER
                setenv PATH $PYTOOLSSYS/bin:$PATH
                setenv LD_LIBRARY_PATH $PYTOOLSSYS/lib/python2.7/site-packages:$LD_LIBRARY_PATH
                setenv PYTHONPATH $PYTOOLSSYS/lib/python2.7/site-packages:$PYTHONPATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            endif
        endif

        breaksw

    case gsl:
        set SYS = $LCGSYS/releases/GSL
        set VER = 2.1-36ee5
        set VER = $VER/$ARCH
        if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -d $SYS/$VER ) then
                setenv GSLSYS $SYS/$VER
                setenv PATH $GSLSYS/bin:$PATH
                setenv LD_LIBRARY_PATH $GSLSYS/lib:$LD_LIBRARY_PATH
                setenv LD_INCLUDE_PATH $GSLSYS/include:$LD_INCLUDE_PATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            endif
        endif

        breaksw

    case root:
        set SYS = $LCGSYS/releases/ROOT
        set VER = 6.08.06-c8fb4
        set VER = $VER/$ARCH
        if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS/$VER"` == 0 ) then
            if ( -d $SYS/$VER ) then
                setenv ROOTSYS $SYS/$VER
                setenv PATH $ROOTSYS/bin:${PATH}
                setenv LD_LIBRARY_PATH $ROOTSYS/lib:$LD_LIBRARY_PATH
                setenv LD_INCLUDE_PATH $ROOTSYS/include:$LD_INCLUDE_PATH
                setenv PYTHONPATH $ROOTSYS/lib:$PYTHONPATH

                printf "Configuring %-10s from %-1s \n" $PKG $SYS/$VER
            else
                echo
                printf "%-10s not availalbe at %-1s \n" $PKG $SYS/$VER
                echo
            endif
        endif

        breaksw

endsw
