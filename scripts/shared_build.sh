#!/bin/bash

UNAME=$(uname -s);
if [ ${UNAME} = 'Darwin' ]; then
    brew install boost cmake protobuf libstxxl lua luabind osm-pbf
else
    # install packages
    sudo apt-add-repository --yes ppa:mapnik/boost # boost 1.49 (in future 1.55)
    sudo apt-get -qq update
    sudo apt-get install -y libboost-filesystem-dev libboost-program-options-dev libboost-iostreams-dev libboost-regex-dev libboost-system-dev libboost-thread-dev
    sudo apt-get install -y build-essential git cmake libprotoc-dev libprotobuf7 protobuf-compiler libprotobuf-dev libbz2-dev libstxxl-dev libstxxl-doc libstxxl1 libxml2-dev libzip-dev lua5.1 liblua5.1-0-dev

    # install luabind
    git clone --depth=1 https://github.com/DennisOSRM/luabind.git
    cd luabind
    mkdir -p build
    cd build
    cmake ../ -DCMAKE_BUILD_TYPE=Release
    make
    sudo make install
    cd ../../

    # install osmpbf
    git clone --depth=1 https://github.com/scrosby/OSM-binary.git
    cd OSM-binary/src
    make
    sudo make install
    cd ../../

fi

# install OSRM
git clone --depth=1 https://github.com/DennisOSRM/Project-OSRM.git Project-OSRM -b develop
cd Project-OSRM
mkdir -p build 
cd build
cmake ../ -DWITH_TOOLS=1
make
sudo make install
cd ../../
pkg-config libosrm --cflags
pkg-config libosrm --libs
pkg-config libosrm --libs --static
cat /usr/local/lib/pkgconfig/*
