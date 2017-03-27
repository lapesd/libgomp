#! /bin/bash

if [ $# -eq 0 ]; 
then
    echo "Usage: ./libgomp-run.sh testname [testargs]"
    exit
fi

LIBGOMP_PATH=$HOME/dev/libgomp/src/libgomp-srr/libgomp

if [ "x$LIBGOMP_PATH" == "x" ];
then
    echo "Fatal: LIBGOMP_PATH must be defined in libgomp-run.sh."
    exit
fi

LIBGOMP_INSTALL_PATH=$LIBGOMP_PATH/build/.libs

RUN_GDB=0

if [ "$1" == "-d" ];
then
    RUN_GDB=1
    shift
fi

(uname -a | grep Darwin) > /dev/null 2> /dev/null
DARWIN_SYS=$?

if [ $DARWIN_SYS -eq 0 ]; 
then
    export DYLD_FORCE_FLAT_NAMESPACE=1
    export DYLD_INSERT_LIBRARIES="$LIBGOMP_INSTALL_PATH/libgomp.dylib"
    export DYLD_LIBRARY_PATH="$LIBGOMP_INSTALL_PATH/libgomp.dylib:$DYLD_LIBRARY_PATH"
    echo "set environment DYLD_LIBRARY_PATH=$LIBGOMP_INSTALL_PATH:$DYLD_LIBRARY_PATH" > $LIBGOMP_PATH/gdbinit
else
    export LD_PRELOAD="$LIBGOMP_INSTALL_PATH/libgomp.so"
    echo "set environment LD_LIBRARY_PATH=$LIBGOMP_INSTALL_PATH:$LD_LIBRARY_PATH" > $LIBGOMP_PATH/gdbinit
fi

testname=$1
shift

# Run the test  
if [ $RUN_GDB -eq 1 ]
then
    exec gdb -x $LIBGOMP_PATH/gdbinit --args $testname "$@"
else
    exec $testname "$@"
fi
