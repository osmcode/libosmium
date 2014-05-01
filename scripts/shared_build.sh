#!/bin/bash

UNAME=$(uname -s);
if [ ${UNAME} = 'Darwin' ]; then
    brew install boost protobuf osm-pbf expat google-sparsehash
else
    # install packages
    sudo apt-add-repository --yes ppa:mapnik/boost # boost 1.49 (in future 1.55)
    sudo apt-get -qq update
    sudo apt-get -y install git build-essential libboost-dev zlib1g-dev protobuf-compiler libprotobuf-lite7 libprotobuf-dev libexpat1-dev libsparsehash-dev

    # install osmpbf
    git clone --depth=1 https://github.com/scrosby/OSM-binary.git
    cd OSM-binary/src
    make
    sudo make install
    cd ../../
fi