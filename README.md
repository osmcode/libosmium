# node-osmium

Fast and flexible Javascript library for working with OpenStreetMap data.

Provides a bindings to the [libosmium](https://github.com/osmcode/libosmium) C++ library.

[![Build Status](https://secure.travis-ci.org/osmcode/node-osmium.png)](http://travis-ci.org/osmcode/node-osmium)

# Depends

 - Node.js v0.10.x

# Installing

By default, binaries are provided and no external depedencies and no compile is needed.

Just do:

    npm install osmium

We currently provide binaries for 64 bit OS X and 64 bit Linux. Running `npm install` on other
platforms will fall back to a source compile (see `Developing` below for build details).

# Usage

## Get the bounds of an `.osm` file

```js
var osmium = require('osmium');
var file = new osmium.File("test/data/winthrop.osm");
var reader = new osmium.Reader(file);
console.log(reader.header())
{ generator: 'CGImap 0.2.0',
  bounds: [ -120.2024, 48.4636, -120.1569, 48.4869 ] }
```

## Parse a `.pbf` file and create a node handler callback to count total nodes

```js
var osmium = require('osmium');
var file = new osmium.File("test/data/winthrop.osm");
var reader = new osmium.Reader(file);
var handler = new osmium.Handler();
var nodes = 0;
handler.on('node',function(node) {
    ++nodes;
});
reader.apply(handler);
console.log(nodes);
1525
```

# Developing

If you wish to develop on `node-osmium` you can check out the code and then build like:

    git clone https://github.com/osmcode/node-osmium.git
    cd node-osmium
    make
    make test

## Source build dependencies

 - Compiler that supports `-std=c++11` (>= clang++ 3.2 || >= g++ 4.8)
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
