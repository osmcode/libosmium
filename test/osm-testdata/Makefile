#------------------------------------------------------------------------------
#
#  You can set several environment variables before running make if you don't
#  like the defaults:
#
#  CXX                - Your C++ compiler.
#  CPLUS_INCLUDE_PATH - Include file search path.
#  CXXFLAGS           - Extra compiler flags.
#  LDFLAGS            - Extra linker flags.
#  
#------------------------------------------------------------------------------

CXXFLAGS += -O3 -g
CXXFLAGS += -std=c++11 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += -I../../include

OS:=$(shell uname -s)
ifeq ($(OS),Darwin)
	CXXFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++
endif

CXXFLAGS_OGR      := $(shell gdal-config --cflags)
CXXFLAGS_WARNINGS := -Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wold-style-cast

LIB_EXPAT  := -lexpat
LIB_PBF    := -pthread -lz -lprotobuf-lite -losmpbf
LIB_GZIP   := -lz
LIB_BZIP2  := -lbz2

LIB_IO     := $(LIB_EXPAT) $(LIB_PBF) $(LIB_GZIP) $(LIB_BZIP2)

LIB_OGR    := $(shell gdal-config --libs)

TESTDATA_DIR := ../../../osm-testdata/grid/data

PROGRAMS := testdata-overview runtests

.PHONY: all clean

all: testdata-overview test

testdata-overview: testdata-overview.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO) $(LIB_OGR)

overview: testdata-overview
	rm -f testdata-overview.db
	./testdata-overview $(TESTDATA_DIR)/all.osm

tests-include.hpp: testcases/*.hpp
	(for file in testcases/*.hpp; do echo "#include \"$${file}\""; done) >tests-include.hpp

tests-run.hpp: testcases/*.hpp
	(for id in $(patsubst testcases/test-%.hpp,%,$(wildcard testcases/*.hpp)); do echo "std::cerr << \"Running test $${id}...\\\n\"; testcase_$${id}::RunTest t$${id}; t$${id}(directory);"; done) >tests-run.hpp

runtests: runtests.cpp testcases/*.hpp check_wkt_handler.hpp count_objects_handler.hpp tests-include.hpp tests-run.hpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO) $(LIB_OGR)

test: runtests
	./runtests $(TESTDATA_DIR)

clean:
	rm -f *.o core $(PROGRAMS) testdata-overview.db tests-include.hpp tests-run.hpp

