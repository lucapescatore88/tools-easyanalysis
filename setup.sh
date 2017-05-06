export TOOLSSYS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TOOLSSYS/roofit/dic

echo
echo "Setup tools-easyanalysis"
echo
echo "TOOLS  $TOOLSSYS"
echo
if [ "$1" = "print" ]; then
    printenv
fi
echo
