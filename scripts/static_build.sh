#!/bin/bash

set -u
if [[ ${TMP_DEPS_DIR:-false} == false ]]; then
    TMP_DEPS_DIR=/tmp/osrm-build
    rm -rf ${TMP_DEPS_DIR}
    mkdir -p ${TMP_DEPS_DIR}
fi
if [[ ${NODE_MODULE_ROOT:-false} == false ]]; then
    NODE_MODULE_ROOT=`pwd`
fi
git clone --depth=1 https://github.com/mapnik/mapnik-packaging.git  ${TMP_DEPS_DIR}/mapnik-packaging
cd ${TMP_DEPS_DIR}/mapnik-packaging
export CXX11=false
source build.sh
build_osrm
cd ${NODE_MODULE_ROOT}
if [[ ${PACKAGE_COMMAND_LINE_TOOLS:-false} != false ]]; then
    ls ${NODE_MODULE_ROOT}
    ls ${NODE_MODULE_ROOT}/lib/binding/
    mkdir -p "${NODE_MODULE_ROOT}/lib/binding/"
    cp `which lua` "${NODE_MODULE_ROOT}/lib/binding/"
    cp `which osrm-extract` "${NODE_MODULE_ROOT}/lib/binding/"
    cp `which osrm-prepare` "${NODE_MODULE_ROOT}/lib/binding/"
    cp `which osrm-datastore` "${NODE_MODULE_ROOT}/lib/binding/"
else
    echo "not packaging command line tools"
fi
