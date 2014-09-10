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

namespace node_osmium {

    class OSMNodeWrap : public OSMObjectWrap {

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Handle<v8::Value> wkb(const v8::Arguments& args);
        static v8::Handle<v8::Value> wkt(const v8::Arguments& args);
        static v8::Handle<v8::Value> get_lon(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_lat(v8::Local<v8::String> property, const v8::AccessorInfo& info);

        static osmium::Node& wrapped(v8::Local<v8::Object> object) {
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
