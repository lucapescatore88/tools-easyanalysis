#source SetupProject.sh root 

export TOOLSSYS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Variable TOOLSYS set: "$TOOLSSYS

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/afs/cern.ch/user/p/pluca/repositories/tools-easyanalysis/roofit/dic


