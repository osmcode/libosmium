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
TESTDATA_DIR := ../../../osm-testdata

.PHONY: all clean overview clean-overview multipolygon clean-multipolygon testcases clean-testcases xml clean-xml

all: overview multipolygon testcases xml

test: clean all

# --------------

testdata-overview: testdata-overview.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO) $(LIB_OGR)

overview: testdata-overview
	rm -f testdata-overview.db
	./testdata-overview $(TESTDATA_DIR)/grid/data/all.osm

clean-overview:
	rm -f testdata-overview testdata-overview.db

# --------------

testdata-multipolygon: testdata-multipolygon.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_OGR)

multipolygon.db: testdata-multipolygon
	rm -f multipolygon.db multipolygon-tests.json
	./testdata-multipolygon $(TESTDATA_DIR)/grid/data/all.osm >multipolygon.log 2>&1

compare-areas.log: $(TESTDATA_DIR)/grid/data/tests.json multipolygon-tests.json
	$(TESTDATA_DIR)/bin/compare-areas.rb $^

multipolygon: multipolygon.db compare-areas.log

clean-multipolygon:
	rm -f testdata-multipolygon multipolygon.db multipolygon.log multipolygon-tests.json compare-areas.log

# --------------

tests-include.hpp: testcases/*.hpp
	(for file in testcases/*.hpp; do echo "#include \"$${file}\""; done) >tests-include.hpp

tests-run.hpp: testcases/*.hpp
	(for id in $(patsubst testcases/test-%.hpp,%,$(wildcard testcases/*.hpp)); do echo "std::cerr << \"Running test $${id}...\\\n\"; testcase_$${id}::RunTest t$${id}; t$${id}(directory);"; done) >tests-run.hpp

testdata-testcases: testdata-testcases.cpp testcases/*.hpp check_basics_handler.hpp check_wkt_handler.hpp tests-include.hpp tests-run.hpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO) $(LIB_OGR)

testcases: testdata-testcases
	./testdata-testcases $(TESTDATA_DIR)/grid/data

clean-testcases:
	rm -f testdata-testcases tests-include.hpp tests-run.hpp

# --------------

testdata-xml: testdata-xml.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO) -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework

xml: testdata-xml
	TESTDIR=$(TESTDATA_DIR)/xml/data ./testdata-xml

clean-xml:
	rm -f testdata-xml

# --------------

clean: clean-overview clean-multipolygon clean-testcases clean-xml
	rm -f *.o core

