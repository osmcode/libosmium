#ifndef OSM_NODE_WRAP_HPP
#define OSM_NODE_WRAP_HPP

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

// osmium
#include <osmium/osm/node.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>

#include "osm_object_wrap.hpp"

using namespace v8;

namespace node_osmium {

    class OSMNodeWrap : public OSMObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> wkb(const Arguments& args);
        static Handle<Value> wkt(const Arguments& args);
        static Handle<Value> get_lon(Local<String> property,const AccessorInfo& info);
        static Handle<Value> get_lat(Local<String> property,const AccessorInfo& info);

        static osmium::Node& wrapped(Local<Object> object) {
            return static_cast<osmium::Node&>(OSMObjectWrap::wrapped(object));
        }

        OSMNodeWrap(const input_iterator&);

        osmium::Node& object() {
            return static_cast<osmium::Node&>(*get());
        }

    private:

        ~OSMNodeWrap();

    }; // class OSMNodeWrap

} // namespace node_osmium

#endif // OSM_NODE_WRAP_HPP
