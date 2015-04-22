#!/bin/sh
#
#  travis_before_install.sh
#

if [ "$TRAVIS_OS_NAME" = "linux" ]; then

    # install dependencies
    sudo apt-get install --yes make libsparsehash-dev libgdal-dev spatialite-bin

elif [ "$TRAVIS_OS_NAME" = "osx" ]; then

    brew install protobuf osm-pbf google-sparsehash || true

    # workaround for gdal homebrew problem
    brew remove gdal
    brew install gdal

fi

cd ..
git clone --quiet --depth 1 https://github.com/osmcode/osm-testdata.git
git clone --quiet --depth 1 https://github.com/scrosby/OSM-binary.git
cd OSM-binary/src
make

