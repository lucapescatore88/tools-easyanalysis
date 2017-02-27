#source SetupProject.sh root 

export TOOLSSYS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Variable TOOLSSYS set: "$TOOLSSYS

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TOOLSSYS/roofit/dic


