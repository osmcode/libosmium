#!/bin/sh
#
#  get_options.sh --cflags|--libs FILE...
#

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
    cpp -xc++ -dD -E -Iinclude -I../include $input 2>/dev/null | grep $macro | cut -d' ' -f3-
done) | sort -u | xargs echo

