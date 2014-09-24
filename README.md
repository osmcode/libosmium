# node-osmium

Fast and flexible Javascript library for working with OpenStreetMap data.

Provides bindings to the [libosmium](https://github.com/osmcode/libosmium) C++
library.

[![Build Status](https://secure.travis-ci.org/osmcode/node-osmium.png)](http://travis-ci.org/osmcode/node-osmium)

## Depends

 - Node.js v0.10.x
 - libosmium (https://github.com/osmcode/libosmium)
 - Mocha (for tests)

## Installing

By default, binaries are provided and no external dependencies or compile is
needed.

Just do:

    npm install osmium

We currently provide binaries for 64 bit OS X and 64 bit Linux. Running `npm
install` on other platforms will fall back to a source compile (see
`Developing` below for build details).


## Usage

See [the tutorial](doc/tutorial.md) for an introduction. There are some demo
applications in the 'demo' directory. See the [README.md](demo/README.md)
there. You can also have a look at the tests in the `test` directory.


## Developing

If you wish to develop on `node-osmium` you can check out the code and then
build like:

    git clone https://github.com/osmcode/node-osmium.git
    cd node-osmium
    make

## Testing

    npm install mocha
    make test

### Source build dependencies

 - Compiler that supports `-std=c++11` (>= clang++ 3.2 || >= g++ 4.8)
 - Boost >= 1.49 with development headers
 - OSM-Binary
 - Protocol buffers
 - zlib

See also the dependency information for the Osmium library.

Set dependencies up on Ubuntu Precise (12.04) like:

    sudo apt-add-repository --yes ppa:chris-lea/node.js
    sudo apt-add-repository --yes ppa:mapnik/boost
    sudo apt-add-repository --yes ppa:ubuntu-toolchain-r/test
    sudo apt-get -y update
    sudo apt-get -y install git gcc-4.8 g++-4.8 build-essential nodejs
    sudo apt-get -y install libboost-dev zlib1g-dev protobuf-compiler
    sudo apt-get -y install libprotobuf-lite7 libprotobuf-dev libexpat1-dev
    sudo apt-get -y install libsparsehash-dev
    export CC=gcc-4.8
    export CXX=g++-4.8
    git clone https://github.com/scrosby/OSM-binary.git
    cd OSM-binary/src
    make && sudo make install

Set dependencies up on OS X like:

    git clone https://github.com/mapnik/mapnik-packaging.git
    cd mapnik-packaging/osx
    export CXX11=true
    source MacOSX.sh
    ./scripts/build_bzip2.sh
    ./scripts/build_expat.sh
    ./scripts/build_google_sparsetable.sh
    ./scripts/build_boost.sh --with-test --with-program_options
    ./scripts/build_protobuf.sh
    ./scripts/build_osm-pbf.sh
    # NOTE: in the same terminal then run the build commands
    # Or from a different terminal re-run `source MacOSX.sh`

## License

node-osmium is available under the Boost Software License. See LICENSE.txt for
details.

## Contact

Please open bug reports on https://github.com/osmcode/node-osmium/issues. You
can ask questions on the
[OSM developer mailing list](https://lists.openstreetmap.org/listinfo/dev)
or on [OFTC net IRC channel #osm-dev](https://wiki.openstreetmap.org/wiki/Irc).

## Authors

 - Dane Springmeyer (dane@mapbox.com)
 - Jochen Topf (jochen@topf.org)

