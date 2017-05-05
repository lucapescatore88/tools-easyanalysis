LCGDIR=/cvmfs/lhcb.cern.ch/lib/lcg
GSLDIR=$LCGDIR/external/GSL/1.10/x86_64-slc6-gcc48-opt

export TOOLSSYS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export LD_INCLUDE_PATH=$GSLDIR/include:$LD_INCLUDE_PATH
export LD_LIBRARY_PATH=$GSLDIR/lib:$LD_LIBRARY_PATH

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TOOLSSYS/roofit/dic

echo
echo "Setup tools-easyanalysis"
echo
echo "TOOLS  $TOOLSSYS"
echo "LCGDIR $LCGDIR"
echo
printenv
echo
