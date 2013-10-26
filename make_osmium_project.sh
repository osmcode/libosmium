#!/bin/sh
#
#  make_osmium_project.sh PROJECTNAME
#
#  Creates a subdirectory named as the project in the current directory and
#  adds a skeleton of files needed to begin a new project using the Osmium
#  library.
#

if [ "x$1" = "x" ]; then
    echo "Usage: $0 PROJECTNAME"
    exit 1
fi

if [ -e "$1" ]; then
    echo "Directory '$1' exists"
    exit 1
fi

mkdir $1
cd $1

sed -e "s/__PROJECTNAME__/$1/g" >Makefile <<'__EOF__'
CXXFLAGS += -O3
#CXXFLAGS += -g
CXXFLAGS += -std=c++11 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

OS:=$(shell uname -s)
ifeq ($(OS),Darwin)
	CXXFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++
endif

CXXFLAGS_WARNINGS := -Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wold-style-cast

LIB_EXPAT := -lexpat
LIB_PBF   := -pthread -lz -lprotobuf-lite -losmpbf

PROGRAMS := __PROJECTNAME__

.PHONY: all clean

all: $(PROGRAMS)

__PROJECTNAME__: main.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

clean:
	rm -f *.o core $(PROGRAMS)

__EOF__

cat >main.cpp <<'__EOF__'

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>

class MyHandler : public osmium::handler::Handler<MyHandler> {

public:

    MyHandler() :
        osmium::handler::Handler<MyHandler>() {
    }

    void node(const osmium::Node& node) {
    }

    void way(const osmium::Way& way) {
    }

    void relation(const osmium::Relation& relation) {
    }

}; // class MyHandler

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    MyHandler handler;
    osmium::io::Reader reader(argv[1]);
    osmium::io::Header header = reader.open();
    reader.apply(handler);
}

__EOF__

