
Osmium
======

A fast and flexible C++ library for working with OpenStreetMap data.

NOTE: This is an alpha version of the next-generation Osmium. For production
use, see the Osmium version at https://github.com/joto/osmium .

There are a few applications that use Osmium in the examples directory.


PREREQUISITES
=============

Because Osmium uses many C++11 features you need a modern compiler and standard
C++ library. Osmium needs at least GCC 4.8 or clang (LLVM) 3.2. (Some parts may
work with GCC 4.7 or older clang versions.)

Different parts of Osmium (and the applications built on top of it) need
different libraries. You DO NOT NEED to install all of them, just install those
you need for the programs you need.

boost (several libraries)
    http://www.boost.org/
    Debian/Ubuntu: libboost-dev
    openSUSE: boost-devel

boost-program-options (for parsing command line options)
    http://www.boost.org/doc/libs/1_54_0/doc/html/program_options.html
    Debian/Ubuntu: libboost-program-options-dev

zlib (for PBF support)
    http://www.zlib.net/
    Debian/Ubuntu: zlib1g-dev
    openSUSE: zlib-devel

GDAL (for OGR support)
    http://gdal.org/
    Debian/Ubuntu: libgdal1-dev
    openSUSE: libgdal-devel

Expat (for parsing XML files)
    http://expat.sourceforge.net/
    Debian/Ubuntu: libexpat1-dev
    openSUSE: libexpat-devel

libxml2 (for writing XML files)
    http://xmlsoft.org/
    Debian/Ubuntu: libxml2-dev

GEOS (for assembling multipolygons etc.)
    http://trac.osgeo.org/geos/
    Debian/Ubuntu: libgeos++-dev
    openSUSE: libgeos-devel

Google sparsehash
    http://code.google.com/p/google-sparsehash/
    Debian/Ubuntu: libsparsehash-dev
    openSUSE: sparsehash

Google protocol buffers (for PBF support)
    http://code.google.com/p/protobuf/ (at least version 2.3.0 needed)
    Debian/Ubuntu: libprotobuf-dev protobuf-compiler
    openSUSE: protobuf-devel
    Also see http://wiki.openstreetmap.org/wiki/PBF_Format

Doxygen (to build API documentation)
    http://www.stack.nl/~dimitri/doxygen/
    Debian/Ubuntu: doxygen

libboost-test (for tests)
    http://www.boost.org/doc/libs/1_54_0/libs/test/doc/html/index.html
    Debian/Ubuntu: libboost-test-dev

OSMPBF (for PBF support)
    https://github.com/scrosby/OSM-binary
    Debian/Ubuntu: libosmpbf-dev

You need to either install the packages for your distribution or install those
libraries from source. Most libraries should be available in all distributions.


FILES
=====

Doxyfile - Needed for building the Osmium C++ docs, call "make doc" to build.
include  - C/C++ include files. All of Osmium is in those header files which
           are needed for building Osmium applications.
examples - Osmium example applications.
test     - Tests (see below).


BUILDING
========

Osmium is a header-only library, so there is nothing to build for the
library itself.

Call "make doc" in the top-level directory to build the Osmium C++ docs.

Call "make install" to install the include files and documentation.

Call "make clean" to clean up.

To build the examples go to the "examples" directory and type "make".


TESTING
=======

There are a few tests using the Boost Unit Test Framework in the "test"
directory. Many more tests are needed, any help appreciated.

Run "make test" from the main directory or go to the "test" directory and type
  ./run_tests.sh
to compile and run the tests. You can run a single test by calling 
  ./run_test.sh TESTFILE"
for instance:
  ./run_test.sh t/osm/test_node.cpp


LICENSE
=======

Osmium is available under the Boost Software License. See LICENSE.txt.


AUTHORS
=======

Osmium was mainly written and is maintained by Jochen Topf <jochen@topf.org>.

Other authors:
* Peter Körner <github@mazdermind.de> (XML and PBF writer, ...)

