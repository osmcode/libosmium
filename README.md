# node-osmium

Node.js bindings to [libosmium](https://github.com/osmcode/libosmium).

[![Build Status](https://secure.travis-ci.org/osmcode/node-osmium.png)](http://travis-ci.org/osmcode/node-osmium)

# Depends

 - Compiler that supports `-std=c++11` (>= clang++ 3.2 || >= g++ 4.8)
 - Node.js v0.10.x
 - Boost >= 1.49 with development headers
 - OSM-Binary
 - Protocol buffers
 - zlib

Set depedencies up on Ubuntu Precise (12.04) like:

    sudo apt-add-repository --yes ppa:chris-lea/node.js
    sudo apt-add-repository --yes ppa:mapnik/boost
    sudo apt-add-repository --yes ppa:ubuntu-toolchain-r/test
    sudo apt-get -y update
    sudo apt-get -y install git gcc-4.7 g++-4.7 build-essential nodejs libboost-dev zlib1g-dev protobuf-compiler libprotobuf-lite7 libprotobuf-dev libexpat1-dev libsparsehash-dev
    export CC=gcc-4.7
    export CXX=g++-4.7
    git clone https://github.com/scrosby/OSM-binary.git
    cd OSM-binary/src
    make && sudo make install

Set depedencies up on OS X like:

    git clone https://github.com/mapnik/mapnik-packaging.git
    cd mapnik-packaging/osx
    # open the settings and make `export CXX11=true`
    export CXX11=true
    source MacOSX.sh
    ./scripts/download_deps.sh
    ./scripts/build_google_sparsetable.sh
    ./scripts/build_icu.sh
    ./scripts/build_boost.sh
    ./scripts/build_protobuf.sh
    ./scripts/build_node.sh
    ./scripts/build_osm-pbf.sh
    # NOTE: in the same terminal then run the build commands
    # Or from a different terminal re-run `source MacOSX.sh`

# Building

To build the bindings:

    git clone https://github.com/osmcode/libosmium.git
    git clone https://github.com/osmcode/node-osmium.git
    cd node-osmium
    npm install

# Testing

Run the tests like:

    npm install mocha
    make test

# Troubleshooting

If you hit a test error like the below it means you need to run `make test` instead of just `npm test` so that the test data is downloaded:

    1) osmium should be able to create an osmium.Reader:
         TypeError: Open failed

If you hit an error like the below it means you need a more recent compiler that implements the C++11 language standard

    cc1plus: error: unrecognized command line option ‘-std=c++11’

This error indicates you need the boost development headers installed:

    ../../include/osmium/osm/location.hpp:40:31: fatal error: boost/operators.hpp: No such file or directory

An error like this indicates that your compiler is too old and does not support all needed c++11 features

    ../../include/osmium/io/header.hpp:55:51: sorry, unimplemented: non-static data member initializers
    ../../include/osmium/io/header.hpp:55:51: error: ISO C++ forbids in-class initialization of non-const static member ‘m_has_multiple_object_versions’

And error like this indicates that you need to do `export CXXFLAGS=-fPIC` and then recompile `libosmpbf`:

    /usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/4.7/../../../../lib/libosmpbf.a(fileformat.pb.o): relocation R_X86_64_32 against `.rodata.str1.1' can not be used when making a shared object; recompile with -fPIC
