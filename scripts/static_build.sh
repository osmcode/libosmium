#!/bin/bash

set -u
if [[ ${TMP_DEPS_DIR:-false} == false ]]; then
    TMP_DEPS_DIR=/tmp/osrm-build
    rm -rf ${TMP_DEPS_DIR}
    mkdir -p ${TMP_DEPS_DIR}
fi
if [[ ${NODE_MODULE_ROOT:-false} == false ]]; then
    NODE_MODULE_ROOT=$(pwd)
fi
git clone --depth=1 https://github.com/mapnik/mapnik-packaging.git ${TMP_DEPS_DIR}/mapnik-packaging
cd ${TMP_DEPS_DIR}/mapnik-packaging
git describe
cat build.sh
export CXX11=true
source build.sh
build_osmium
cd ${NODE_MODULE_ROOT}