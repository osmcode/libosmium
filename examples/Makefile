#------------------------------------------------------------------------------
#
#  Makefile for Osmium examples
#
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

CXXFLAGS += -O3
#CXXFLAGS += -g
CXXFLAGS += -std=c++11 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += -I../include -I/usr/include/libshp

OS:=$(shell uname -s)
ifeq ($(OS),Darwin)
  # atomic_bool workaround is a libc++ bug in current lib: http://stackoverflow.com/questions/14136128/clang-doesnt-know-stdatomic-bool-but-xcode-does
	CXXFLAGS += -stdlib=libc++ -D"atomic_bool=atomic<bool>"
	LDFLAGS += -stdlib=libc++
endif

# remove this if you do not want debugging to be compiled in
CXXFLAGS += -DOSMIUM_WITH_DEBUG

CXXFLAGS_GEOS     := $(shell geos-config --cflags)
CXXFLAGS_LIBXML2  := $(shell xml2-config --cflags)
CXXFLAGS_OGR      := $(shell gdal-config --cflags)
CXXFLAGS_WARNINGS := -Wall -Wextra -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long

LIB_EXPAT  := -lexpat
LIB_PBF    := -lz -lpthread -lprotobuf-lite -losmpbf
LIB_GD     := -lgd -lz -lm
LIB_GEOS   := $(shell geos-config --libs)
LIB_OGR    := $(shell gdal-config --libs)
LIB_SHAPE  := -lshp $(LIB_GEOS)
LIB_XML2   := $(shell xml2-config --libs)

PROGRAMS := \
    osmium_convert \
    osmium_debug \
    osmium_read \
    osmium_toogr
#    osmium_find_bbox \
#    osmium_mpdump \
#    osmium_progress \
#    osmium_range_from_history \
#    osmium_relation_members \
#    osmium_sizeof \
#    osmium_serdebug \
#    osmium_serdump \
#    osmium_serget \
#    osmium_store_and_debug \
#    osmium_time \
#    osmium_toogr2 \
#    osmium_to_postgis \
#    osmium_toshape \
#    nodedensity

.PHONY: all clean

all: $(PROGRAMS)

osmium_convert: osmium_convert.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_LIBXML2) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_XML2)

osmium_debug: osmium_debug.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_read: osmium_read.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_find_bbox: osmium_find_bbox.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_mpdump: osmium_mpdump.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_GEOS)

osmium_progress: osmium_progress.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_range_from_history: osmium_range_from_history.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_LIBXML2) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_XML2)

osmium_relation_members: osmium_relation_members.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_sizeof: osmium_sizeof.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_serdebug: osmium_serdebug.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS)

osmium_serdump: osmium_serdump.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_serget: osmium_serget.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS)

osmium_store_and_debug: osmium_store_and_debug.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_time: osmium_time.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_toogr: osmium_toogr.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_OGR)

osmium_toogr2: osmium_toogr2.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_OGR) $(LIB_GEOS)

osmium_to_postgis: osmium_to_postgis.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_OGR)

osmium_toshape: osmium_toshape.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_SHAPE)

nodedensity: nodedensity.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_GD)

clean:
	rm -f *.o core $(PROGRAMS)

