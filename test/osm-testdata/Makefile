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
CXXFLAGS_WARNINGS += -Werror

# remove optional version from compiler name in CXX
COMPILER := $(shell echo $(CXX) | sed -e's/-.*//')

ifeq ($(COMPILER),g++)
    # remove warnings that create false positives on g++
    CXXFLAGS_WARNINGS += -Wno-return-type -Wno-array-bounds
endif

ifeq ($(COMPILER),clang++)
    CXXFLAGS_WARNINGS += -Wdocumentation -Wunused-exception-parameter -Wmissing-declarations
    CXXFLAGS_WARNINGS += -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-unused-macros -Wno-exit-time-destructors -Wno-global-constructors -Wno-padded -Wno-switch-enum -Wno-missing-prototypes -Wno-weak-vtables
    CXXFLAGS_WARNINGS += -Wno-cast-align -Wno-float-equal
endif

# suppress warnings from Catch unit test macros
CXXFLAGS_WARNINGS += -Wno-unused-variable

LIB_EXPAT  := -lexpat
LIB_PBF    := -pthread -lz -lprotobuf-lite -losmpbf
LIB_GZIP   := -lz
LIB_BZIP2  := -lbz2

LIB_IO     := $(LIB_EXPAT) $(LIB_PBF) $(LIB_GZIP) $(LIB_BZIP2)

# -pthread is needed for g++-4.8
LIB_OGR    := $(shell gdal-config --libs) -pthread

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

TESTCASES := $(wildcard testcases/test-*.cpp)
TESTCASES_O := $(patsubst %.cpp,%.o,$(TESTCASES))

testcases/%.o: testcases/%.cpp
	$(CXX) -Iinclude -I../include $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ -c $<

testdata-testcases.o: testdata-testcases.cpp
	$(CXX) -Iinclude -I../include $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ -c $<

testdata-testcases: testdata-testcases.o $(TESTCASES_O)
	$(CXX) -o $@ $< $(TESTCASES_O) $(LDFLAGS) $(LIB_IO) $(LIB_OGR)

testcases: testdata-testcases
	TESTCASES_DIR=$(TESTDATA_DIR)/grid/data ./testdata-testcases

clean-testcases:
	rm -f testdata-testcases testcases/*.o

# --------------

testdata-xml: testdata-xml.cpp
	$(CXX) $(CXXFLAGS) -I../include $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_IO)

xml: testdata-xml
	TESTDIR=$(TESTDATA_DIR)/xml/data ./testdata-xml

clean-xml:
	rm -f testdata-xml

# --------------

clean: clean-overview clean-multipolygon clean-testcases clean-xml
	rm -f *.o core

