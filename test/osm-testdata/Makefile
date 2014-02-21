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

# place where the test data directory has been cloned
# https://github.com/osmcode/osm-testdata
TESTDATA_DIR := ../../../osm-testdata/grid/data

.PHONY: all clean overview clean-overview multipolygon clean-multipolygon testcases clean-testcases

all: overview multipolygon testcases

# --------------

testdata-overview: testdata-overview.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO) $(LIB_OGR)

overview: testdata-overview
	rm -f testdata-overview.db
	./testdata-overview $(TESTDATA_DIR)/all.osm

clean-overview:
	rm -f testdata-overview testdata-overview.db

# --------------

testdata-multipolygon: testdata-multipolygon.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO) $(LIB_OGR)

multipolygon: testdata-multipolygon
	rm -f multipolygon.db multipolygon-is.wkt
	./testdata-multipolygon $(TESTDATA_DIR)/all.osm
	diff -u multipolygon-should.wkt multipolygon-is.wkt

clean-multipolygon:
	rm -f testdata-multipolygon multipolygon.db multipolygon-is.wkt

# --------------

tests-include.hpp: testcases/*.hpp
	(for file in testcases/*.hpp; do echo "#include \"$${file}\""; done) >tests-include.hpp

tests-run.hpp: testcases/*.hpp
	(for id in $(patsubst testcases/test-%.hpp,%,$(wildcard testcases/*.hpp)); do echo "std::cerr << \"Running test $${id}...\\\n\"; testcase_$${id}::RunTest t$${id}; t$${id}(directory);"; done) >tests-run.hpp

testdata-testcases: testdata-testcases.cpp testcases/*.hpp check_basics_handler.hpp check_wkt_handler.hpp tests-include.hpp tests-run.hpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO) $(LIB_OGR)

testcases: testdata-testcases
	./testdata-testcases $(TESTDATA_DIR)

clean-testcases:
	rm -f testdata-testcases tests-include.hpp tests-run.hpp

# --------------

clean: clean-overview clean-multipolygon clean-testcases
	rm -f *.o core

