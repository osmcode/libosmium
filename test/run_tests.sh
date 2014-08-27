#!/bin/sh
#
#  Compile and run unit tests
#
#  ./run_tests.sh [-v] [-o]              -- compiles and runs all tests
#  ./run_tests.sh [-v] [-o] FILE.CPP ... -- compiles and runs only given tests
#
#  -v  -- Run tests under valgrind
#  -o  -- Show standard output also if tests passed
#  

set -e

if [ -z "$CXX" ]; then
    CXX="c++"
fi

if [ -z "$CXXFLAGS_WARNINGS" ]; then
    CXXFLAGS_WARNINGS="-Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wold-style-cast"
fi

if [ "$CXX" = "g++" ]; then
    # remove warnings that create false positives on g++
    CXXFLAGS_WARNINGS="$CXXFLAGS_WARNINGS -Wno-return-type -Wno-array-bounds"
fi

if [ "$CXX" = "clang++" ]; then
    CXXFLAGS_WARNINGS="$CXXFLAGS_WARNINGS -Wdocumentation -Wunused-exception-parameter -Wmissing-declarations"
    CXXFLAGS_WARNINGS="$CXXFLAGS_WARNINGS -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-unused-macros -Wno-exit-time-destructors -Wno-global-constructors -Wno-padded -Wno-switch-enum -Wno-missing-prototypes -Wno-weak-vtables"
    CXXFLAGS_WARNINGS="$CXXFLAGS_WARNINGS -Wno-cast-align -Wno-float-equal"
fi

CXXFLAGS="$CXXFLAGS -g -std=c++11"

if [ `uname -s` = 'Darwin' ]; then
    CXXFLAGS="${CXXFLAGS} -stdlib=libc++"
fi

COMPILE="$CXX -I../include -Iinclude $CXXFLAGS -Werror $CXXFLAGS_WARNINGS -o tests $LDFLAGS"

if [ "x$1" = "x-v" ]; then
    VALGRIND="valgrind --leak-check=full --show-reachable=yes --suppressions=valgrind.supp --error-exitcode=1"
    shift
else
    VALGRIND=""
fi

if [ "x$1" = "x-o" ]; then
    ALWAYS_SHOW_OUTPUT="1"
    shift
else
    ALWAYS_SHOW_OUTPUT="0"
fi

BOLD="\033[1m"
NORM="\033[0m"
GREEN="\033[1;32m"
DARKRED="\033[31m"
RED="\033[1;31m"

TESTS_COMPILE_ERROR=0
TESTS_FAILED=0
TESTS_OK=0

test_file () {
    FILES="test_main.o $1"
    OPTS_CFLAGS=`../get_options.sh --cflags $1`
    OPTS_LIBS=`../get_options.sh --libs $1`
    cmdline="$COMPILE $FILES $OPTS_CFLAGS $OPTS_LIBS $LDFLAGS"
    msg=`echo "Checking ${BOLD}test/$1$NORM ................................................" | cut -c1-70`
    echo -n "$msg "
    if ! output=$($cmdline 2>&1); then
        echo "$DARKRED[COMPILE ERROR]$NORM"
        TESTS_COMPILE_ERROR=$(($TESTS_COMPILE_ERROR+1))
        echo "=========================="
        echo $cmdline
        echo "--------------------------"
        echo "$output"
        echo "=========================="
        return
    else
        if [ $ALWAYS_SHOW_OUTPUT = 1 ]; then
            echo "\n=========================="
            echo $cmdline
            echo "--------------------------"
            echo "$output"
            echo "=========================="
        fi
    fi

    if ! output=$($VALGRIND ./tests 2>&1 ); then
        echo "$RED[TEST FAILED]$NORM"
        TESTS_FAILED=$(($TESTS_FAILED+1))
        echo "=========================="
        echo "$output"
        echo "=========================="
        return
    else
        echo "$GREEN[SUCCESS]$NORM"
        TESTS_OK=$((TESTS_OK+1))
        if [ $ALWAYS_SHOW_OUTPUT = 1 ]; then
            echo "=========================="
            echo "$output"
            echo "=========================="
        fi
    fi
}

setup() {
    if [ \( ! -e test_main.o \) -o \( test_main.cpp -nt test_main.o \) ]; then
        echo "Compiling test runner"
        $CXX -I../include -Iinclude $CXXFLAGS -c test_main.cpp
    fi
}

my_path=`dirname $0`
cd $my_path
setup
if [ "x$1" = "x" ]; then
    FILES=t/*/test_*.cpp
else
    FILES=$*
fi

for FILE in $FILES; do
    test_file $FILE
done

if [ $(($TESTS_COMPILE_ERROR + $TESTS_FAILED)) = 0 ]; then
    echo "all tests succeeded"
    exit 0
else
    echo "some tests failed"
    echo "$TESTS_OK ok, $TESTS_COMPILE_ERROR compile error, $TESTS_FAILED fail"
    exit 1
fi

