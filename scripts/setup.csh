#!/bin/tcsh

set EXE = `echo $0 | sed s:"-"::`
if ( ! ($?REPOSYS) ) setenv REPOSYS `readlink -f "$EXE"`
set EXE = `echo $EXE | sed s:"."::`
if ( ! ($?REPOSYS) ) setenv REPOSYS `echo $REPOSYS | sed s:"$EXE"::`

if ( ! ($?TOOLSSYS) ) setenv TOOLSSYS $REPOSYS
#if ( ! ($?CPLUS_INCLUDE_PATH) ) setenv CPLUS_INCLUDE_PATH
if ( ! ($?LD_LIBRARY_PATH) ) setenv LD_LIBRARY_PATH
if ( ! ($?LD_INCLUDE_PATH) ) setenv LD_INCLUDE_PATH
if ( ! ($?PYTHONPATH) ) setenv PYTHONPATH

#setenv CPLUS_INCLUDE_PATH $TOOLSSYS:$TOOLSSYS/src:$TOOLSSYS/roofit:$TOOLSSYS/roofit/src:$CPLUS_INCLUDE_PATH
setenv LD_LIBRARY_PATH $TOOLSSYS/roofit/dic:$LD_LIBRARY_PATH

echo
echo "Setup tools-easyanalysis"
echo
echo "Configuring TOOLSSYS to $TOOLSSYS"
echo

# LIBS
set LCGDIR = /cvmfs/lhcb.cern.ch/lib/lcg
set ARCH = x86_64-slc6-gcc49-opt

# GCC
set SYS = $LCGDIR/releases/gcc
if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ) then
    set VER = 4.9.3
    set VER = $VER/x86_64-slc6
    if ( -f $SYS/$VER/setup.csh ) then
	setenv GCCSYS $SYS/$VER
	#source $GCCSYS/setup.csh $LCGDIR/external
	set gcc_config_version = $VER
	set LCGPLAT = x86_64-slc6
	set LCG_lib_name = lib64
	set LCG_arch = x86_64

	set LCG_contdir = $LCGDIR/releases
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

        echo "Configuring GCC    from $GCCSYS"
    else
	echo
	echo "GCC $GCCSYS not available"
	echo
    endif
endif

# PYTHON
set SYS = $LCGDIR/releases/Python
if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ) then
    set VER = 2.7.6-31787
    set VER = $VER/$ARCH
    if ( -d $SYS/$VER ) then
	setenv PYTHONSYS $SYS/$VER
	setenv LD_LIBRARY_PATH $PYTHONSYS/lib:$LD_LIBRARY_PATH
        setenv PYTHONPATH $TOOLSSYS/python:$PYTHONPATH
	setenv PYTHONPATH /usr/lib64/python2.6/site-packages:$PYTHONPATH
	setenv PYTHONPATH /usr/lib/python2.6/site-packages:$PYTHONPATH

	echo "Configuring PYTHON from $PYTHONSYS"
    else
	echo
	echo "PYTHON $PYTHONSYS not available"
	echo
    endif
endif

# GSL
set SYS = $LCGDIR/releases/GSL
if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ) then
    set VER = 2.1-36ee5
    set VER = $VER/$ARCH
    if ( -d $SYS/$VER ) then
	setenv GSLSYS $SYS/$VER
	#setenv CPLUS_INCLUDE_PATH $GSLSYS/include:$CPLUS_INCLUDE_PATH
        setenv LD_LIBRARY_PATH $GSLSYS/lib:$LD_LIBRARY_PATH
        setenv LD_INCLUDE_PATH $GSLSYS/include:$LD_INCLUDE_PATH

        echo "Configuring GSL    from $GSLSYS"
    else
	echo
	echo "GSL $GSLSYS not available"
	echo
   endif
endif

# ROOT
set SYS = $LCGDIR/releases/ROOT
if ( `echo "$LD_LIBRARY_PATH" | grep -ci "$SYS"` == 0 ) then
    set VER = 6.08.06-c8fb4
    set VER = $VER/$ARCH
    if ( -d $SYS/$VER ) then
	setenv ROOTSYS $SYS/$VER
	setenv PATH $ROOTSYS/bin:${PATH}
	setenv LD_LIBRARY_PATH $ROOTSYS/lib:$LD_LIBRARY_PATH
	setenv LD_INCLUDE_PATH $ROOTSYS/include:$LD_INCLUDE_PATH
	setenv PYTHONPATH $ROOTSYS/lib:$PYTHONPATH

        echo "Configuring ROOT   from $ROOTSYS"
    else
	echo
	echo "ROOT $ROOTSYS not available"
	echo
    endif
endif

#echo
#echo "Configuring CPLUS_INCLUDE_PATH to $CPLUS_INCLUDE_PATH"
echo
echo "Configuring PATH            to $PATH"
echo "Configuring LD_LIBRARY_PATH to $LD_LIBRARY_PATH"
echo "Configuring LD_INCLUDE_PATH to $LD_INCLUDE_PATH"
echo "Configuring PYTHONPATH      to $PYTHONPATH"
echo
