# node-osmium

Fast and flexible Javascript library for working with OpenStreetMap data.

Provides bindings to the [libosmium](https://github.com/osmcode/libosmium) C++ library.

[![Build Status](https://secure.travis-ci.org/osmcode/node-osmium.png)](http://travis-ci.org/osmcode/node-osmium)

## Depends

 - Node.js v0.10.x
 - libosmium (https://github.com/osmcode/libosmium)
 - Mocha (for tests)

## Installing

By default, binaries are provided and no external dependencies or compile is needed.

Just do:

    npm install osmium

We currently provide binaries for 64 bit OS X and 64 bit Linux. Running `npm install` on other
platforms will fall back to a source compile (see `Developing` below for build details).

## Usage

### Get the bounds of an `.osm` file

```js
var osmium = require('osmium');
var reader = new osmium.Reader("test/data/winthrop.osm");
var bounds = reader.header().bounds[0];
console.log(bounds.left(), bounds.bottom(), bounds.right(), bounds.top());
```

Result:
```
-120.2024 48.4636 -120.1569 48.4869
```

### Parse an OSM file and create a node handler callback to count total nodes

```js
var osmium = require('osmium');
var reader = new osmium.Reader("test/data/winthrop.osm");
var handler = new osmium.Handler();
var nodes = 0;
handler.on('node', function(node) {
    ++nodes;
});
osmium.apply(reader, handler);
console.log(nodes);
```

Result:
```
1525
```

## Demos

There are some demo applications in the 'demo' directory. See the README.md
there.

## Developing

If you wish to develop on `node-osmium` you can check out the code and then build like:

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
    sudo apt-get -y install git gcc-4.8 g++-4.8 build-essential nodejs libboost-dev zlib1g-dev protobuf-compiler libprotobuf-lite7 libprotobuf-dev libexpat1-dev libsparsehash-dev
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

