# Osmium Library

http://osmcode.org/libosmium

A fast and flexible C++ library for working with OpenStreetMap data.

NOTE: This is a beta version of the next-generation Osmium. For production
use, see the Osmium version at https://github.com/joto/osmium .

There are a few applications that use the Osmium library in the examples
directory. See the [osmium-contrib](http://github.com/osmcode/osmium-contrib)
repository for more example code.

[![Build Status](https://secure.travis-ci.org/osmcode/libosmium.png)](http://travis-ci.org/osmcode/libosmium)

Libosmium is developed on Linux, but also works on OSX and Windows (with some
limitations).

## Prerequisites

Because Osmium uses many C++11 features you need a modern compiler and standard
C++ library. Osmium needs at least GCC 4.7.3 or clang (LLVM) 3.2. (Some parts may
work with older versions.)

Different parts of Osmium (and the applications built on top of it) need
different libraries. You DO NOT NEED to install all of them, just install those
you need for the programs you need.

    boost-iterator, boost-regex
        http://www.boost.org/
        Debian/Ubuntu: libboost-dev
        openSUSE: boost-devel

    boost-program-options (for parsing command line options in some examples)
        http://www.boost.org/doc/libs/1_54_0/doc/html/program_options.html
        Debian/Ubuntu: libboost-program-options-dev

    Google protocol buffers (for PBF support)
        http://code.google.com/p/protobuf/ (at least version 2.3.0 needed)
        Debian/Ubuntu: libprotobuf-dev protobuf-compiler
        openSUSE: protobuf-devel
        Also see http://wiki.openstreetmap.org/wiki/PBF_Format

    OSMPBF (for PBF support)
        https://github.com/scrosby/OSM-binary
        Debian/Ubuntu: libosmpbf-dev
        (The package in Ubuntu 14.04 and older is too old, install from source
        in these cases.)

    Expat (for parsing XML files)
        http://expat.sourceforge.net/
        Debian/Ubuntu: libexpat1-dev
        openSUSE: libexpat-devel

    zlib (for PBF and for gzip support when reading/writing XML)
        http://www.zlib.net/
        Debian/Ubuntu: zlib1g-dev
        openSUSE: zlib-devel

    bz2lib (for bzip2 support when reading/writing XML)
        http://www.bzip.org/
        Debian/Ubuntu: libbz2-dev

    Google sparsehash
        http://code.google.com/p/google-sparsehash/
        Debian/Ubuntu: libsparsehash-dev
        openSUSE: sparsehash

    GDAL (for OGR support)
        http://gdal.org/
        Debian/Ubuntu: libgdal1-dev
        openSUSE: libgdal-devel

    GEOS (for GEOS support)
        http://trac.osgeo.org/geos/
        Debian/Ubuntu: libgeos++-dev
        openSUSE: libgeos-devel

    libproj (for projection support)
        http://trac.osgeo.org/proj/
        Debian/Ubuntu: libproj-dev

    Doxygen (to build API documentation) and tools
        http://www.stack.nl/~dimitri/doxygen/
        Debian/Ubuntu: doxygen graphviz xmlstarlet

You need to either install the packages for your distribution or install those
libraries from source. Most libraries should be available in all distributions.


## Directories

* include: C/C++ include files. All of Osmium is in those header files which
  are needed for building Osmium applications.

* examples: Osmium example applications.

* test: Tests (see below).

* doc: This is where the documentation will be build, call "make doc" to build.


## Building

Osmium is a header-only library, so there is nothing to build for the
library itself.

Call "make doc" in the top-level directory to build the Osmium C++ docs.

Call "make install" to install the include files and documentation.

Call "make clean" to clean up.

To build the examples go to the "examples" directory and type "make".

For Mac users: If you have clang 3.2 or newer, use the system compiler.
If not you have to build the compiler yourself. See the instructions
on http://clang.llvm.org/ .

Preliminary support for cmake is provided. You can use this instead of "make":

    mkdir build
    cd build
    cmake ..
    make


## Testing

There are a few unit tests using the Catch unit test framework in the "test"
directory. Many more tests are needed, any help appreciated.

For [Catch](https://github.com/philsquared/Catch/) only one header file is
needed which is included (`test/include/catch.hpp`).

Run "make test" from the main directory or go to the "test" directory and type

    ./run_tests.sh
    
to compile and run the tests. You can run a single test by calling 

    ./run_test.sh TESTFILE

for instance:

    ./run_test.sh t/basic/test_node.cpp

In addition there are some test based on the OSM Test Data Repository at
http://osmcode.org/osm-testdata/ . Go into the test/osm-testdata directory
and type 'make' to run them all.


## Switching from the old Osmium

See `README-changes-from-old-osmium`.


## License

The Osmium Library is available under the Boost Software License. See
LICENSE.txt.


## Authors

The Osmium Library was mainly written and is maintained by Jochen Topf
(jochen@topf.org).

Other authors:
* Peter Körner (github@mazdermind.de) (PBF writer, ...)

