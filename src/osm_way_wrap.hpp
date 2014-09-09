#ifndef OSM_WAY_WRAP_HPP
#define OSM_WAY_WRAP_HPP

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/osm/way.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>

#include "osm_object_wrap.hpp"

using namespace v8;

namespace node_osmium {

    class OSMWayWrap : public OSMObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> wkb(const Arguments& args);
        static Handle<Value> wkt(const Arguments& args);
        static Handle<Value> nodes(const Arguments& args);

        static osmium::Way& wrapped(Local<Object> object) {
            return static_cast<osmium::Way&>(OSMObjectWrap::wrapped(object));
        }

        OSMWayWrap(const input_iterator&);

        osmium::Way& object() {
            return static_cast<osmium::Way&>(*get());
        }

    private:

        ~OSMWayWrap();

    }; // class OSMWayWrap

} // namespace node_osmium

#endif // OSM_WAY_WRAP_HPP
