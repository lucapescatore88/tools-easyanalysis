export TOOLSSYS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TOOLSSYS/roofit/dic

echo
echo "Setup tools-easyanalysis"
echo
echo "TOOLS  $TOOLSSYS"
echo "LCGDIR $LCGDIR"
echo
#printenv
echo
