#!/bin/tcsh

# LIBS
setenv CVMFS /cvmfs/lhcb.cern.ch
setenv LCGSYS $CVMFS/lib/lcg
setenv ARCH x86_64-slc6-gcc49-opt

if ( ! -d $LCGSYS ) then
    echo
    echo "Cannot access $LCGSYS"
    echo
    exit
endif

if ( ! ($?LD_INCLUDE_PATH) ) setenv LD_INCLUDE_PATH
if ( ! ($?LD_LIBRARY_PATH) ) setenv LD_LIBRARY_PATH
if ( ! ($?ROOT_INCLUDE_PATH) ) setenv ROOT_INCLUDE_PATH
if ( ! ($?PYTHONPATH) )      setenv PYTHONPATH

if ( ! ($?TOOLSSYS) ) then
    set EXE = "$0"
    set EXE = `echo $EXE | sed s:"-"::`
    set EXE = `basename $EXE`

    setenv TOOLSSYS `readlink -f "$EXE"`
    setenv TOOLSSYS `echo $TOOLSSYS | sed s:"/$EXE"::`

    setenv LD_LIBRARY_PATH $TOOLSSYS/roofit/dic:$LD_LIBRARY_PATH
    setenv LD_LIBRARY_PATH $TOOLSSYS/build/tools:$TOOLSSYS/build/roofit:$LD_LIBRARY_PATH
    setenv LD_LIBRARY_PATH $TOOLSSYS/lib:$LD_LIBRARY_PATH

    setenv LD_INCLUDE_PATH ${TOOLSSYS}:$TOOLSSYS/tools:$TOOLSSYS/roofit:$LD_INCLUDE_PATH

    setenv ROOT_INCLUDE_PATH ${TOOLSSYS}:$TOOLSSYS/tools:$TOOLSSYS/roofit:$ROOT_INCLUDE_PATH

    setenv PYTHONPATH $TOOLSSYS/python:$PYTHONPATH

    echo
    echo "Setup tools-easyanalysis"
    echo
    echo "Configuring TOOLSSYS to $TOOLSSYS"
    echo

    source $TOOLSSYS/scripts/setup.csh cmake
    source $TOOLSSYS/scripts/setup.csh gcc
    source $TOOLSSYS/scripts/setup.csh python
    source $TOOLSSYS/scripts/setup.csh gsl
    source $TOOLSSYS/scripts/setup.csh root

    source $TOOLSSYS/scripts/setup.csh env
endif

# CASES
switch ( "$1" )

    case env:
        echo
        echo "Configuring PATH              to $PATH"
        echo "Configuring LD_LIBRARY_PATH   to $LD_LIBRARY_PATH"
        echo "Configuring LD_INCLUDE_PATH   to $LD_INCLUDE_PATH"
        echo "Configuring ROOT_INCLUDE_PATH to $ROOT_INCLUDE_PATH"
        echo "Configuring PYTHONPATH        to $PYTHONPATH"
        echo

        breaksw

    case cmake:
        set SYS = $CVMFS/lib/contrib/CMake
        set VER = 3.7.2
        set VER = $VER/Linux-x86_64
        if ( `echo "$PATH" | grep -ci "$SYS/$VER"` == 0 ) then
	    if ( -d $SYS/$VER ) then
		setenv CMAKESYS $SYS/$VER
		setenv PATH $CMAKESYS/bin:$PATH

		echo "Configuring CMAKE      from $CMAKESYS"
	    else
		echo
		echo "CMAKE $SYS/$VER not available"
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

                echo "Configuring GCC        from $GCCSYS"
            else
                echo
                echo "GCC $SYS/$VER not available"
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

                echo "Configuring PYTHON     from $PYTHONSYS"

                source $TOOLSSYS/scripts/setup.csh pyanalysis
                source $TOOLSSYS/scripts/setup.csh pytools
            else
                echo
                echo "PYTHON $SYS/$VER not available"
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

                echo "Configuring PYANALYSIS from $PYANALYSISSYS"
            else
                echo
                echo "PYANALYSIS $SYS/$VER not available"
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

                echo "Configuring PYTOOLS    from $PYTOOLSSYS"
            else
                echo
                echo "PYTOOLS $SYS/$VER not available"
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
                setenv LD_LIBRARY_PATH $GSLSYS/lib:$LD_LIBRARY_PATH
                setenv LD_INCLUDE_PATH $GSLSYS/include:$LD_INCLUDE_PATH

                echo "Configuring GSL        from $GSLSYS"
            else
                echo
                echo "GSL $SYS/$VER not available"
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

                echo "Configuring ROOT       from $ROOTSYS"
            else
                echo
                echo "ROOT $ROOTSYS not available"
                echo
            endif
        endif

        breaksw

endsw
