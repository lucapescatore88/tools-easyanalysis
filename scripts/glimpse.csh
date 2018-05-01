#!/bin/tcsh

if ( $# < 1 ) then
    echo
    echo "Usage: $0 STRING"
    echo
    exit
endif

echo
echo "********************************************************************************"
echo "Searching in ..."
echo
echo $GLIMPSEPATH
echo "********************************************************************************"
echo

foreach DIR ( $GLIMPSEPATH )
    echo "********************************************************************************"
    echo $DIR
    echo
    if ( "$2" == "" ) then
        grep -ie "$1" $DIR/*.*p* | grep -ve ".pyc" | sed s/":"/"  :  "/ | sed s:"$TOOLSSYS/"::
    else
        grep -ie "$1" $DIR/*.*p* | grep -ve ".pyc" | sed s/":"/"  :  "/ | sed s:"$TOOLSSYS/":: | grep -e "$2"
    endif
    echo "********************************************************************************"
    echo
end
