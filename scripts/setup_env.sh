#!/bin/bash

shopt -s expand_aliases

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
fi


