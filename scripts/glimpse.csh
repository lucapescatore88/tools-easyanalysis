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
        grep  -ie "$1" $DIR/*.*p* | sed s:"$TOOLSSYS/":: | sed s/":"/" "/ | awk '{ printf("%-40s %s \n", $1, substr($0, index($0, $2))) }' | grep -a --colour=auto -ie "$1"
    else
        grep  -ie "$1" $DIR/*.*p* | sed s:"$TOOLSSYS/":: | sed s/":"/" "/ | awk '{ printf("%-40s %s \n", $1, substr($0, index($0, $2))) }' | grep -a --colour=auto -ie "$1" | grep -a --colour=auto -e "$2"
    endif
    echo "********************************************************************************"
    echo
end
