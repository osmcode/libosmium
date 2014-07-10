#!/bin/sh
#
#  get_options.sh --cflags|--libs FILE...
#

set -e

if [ "x$1" = "x--cflags" ]; then
    macro=OSMIUM_COMPILE_WITH_CFLAGS_
    shift
elif [ "x$1" = "x--libs" ]; then
    macro=OSMIUM_LINK_WITH_LIBS_
    shift
else
    echo "Usage: $0 --cflags|--libs FILE..."
    exit 1
fi

(for input in $*; do
    cpp -xc++ -std=c++11 -dD -E -Iinclude -I../include $input 2>/dev/null | grep $macro | cut -d' ' -f3-
done) | while read line
do
    case "$line" in
    \`*)
        eval expand=$line
        echo $expand
        ;;
    *)
        echo $line
        ;;
    esac
done | sort -u | xargs echo

