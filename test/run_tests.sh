#!/bin/sh
#
#  Compile and run unit tests
#
#  ./run_tests.sh [-v]               -- compiles and runs all tests
#  ./run_tests.sh [-v] SOME_FILE.CPP -- compiles and runs only one test
#
#  -v  -- Run tests under valgrind
#  

set -e

if [ -z "$CXX" ]; then
    CXX="c++"
fi

if [ -z "$CXXFLAGS_WARNINGS" ]; then
    CXXFLAGS_WARNINGS="-Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long"
fi

if [ -z "$CXXFLAGS" ]; then
    CXXFLAGS="-g -std=c++11"
fi

COMPILE="$CXX -I../include -I. $CXXFLAGS $CXXFLAGS_WARNINGS -o tests"

if [ "x$1" = "x-v" ]; then
    VALGRIND="valgrind --leak-check=full --show-reachable=yes"
    shift
else
    VALGRIND=""
fi

BOLD="[1m"
NORM="[0m"

test_file () {
    FILES="test_main.cpp test_utils.cpp $1"

    eval OPTS_CFLAGS=`../get_options.sh --cflags $FILES`
    eval OPTS_LIBS=`../get_options.sh --libs $FILES`
    PARAMS="$OPTS_CFLAGS $OPTS_LIBS -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework"

    echo "Checking $BOLD$1$NORM..."
    echo $COMPILE $FILES $PARAMS
    $COMPILE $FILES $PARAMS
    $VALGRIND ./tests
}

if [ "x$1" = "x" ]; then
    for FILE in t/*/test_*.cpp; do
        test_file $FILE
    done
else
    test_file $1
fi

