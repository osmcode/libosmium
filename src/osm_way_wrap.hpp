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

namespace node_osmium {

    class OSMWayWrap : public OSMObjectWrap {

        static v8::Handle<v8::Value> wkb(const v8::Arguments& args);
        static v8::Handle<v8::Value> wkt(const v8::Arguments& args);
        static v8::Handle<v8::Value> nodes(const v8::Arguments& args);

        static v8::Persistent<v8::FunctionTemplate> constructor;

    public:

        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        static osmium::Way& wrapped(v8::Local<v8::Object> object) {
            return static_cast<osmium::Way&>(OSMObjectWrap::wrapped(object));
        }

        OSMWayWrap(const input_iterator&);

        static v8::Local<v8::Object> create(const input_iterator& it) {
            v8::HandleScope scope;
            v8::Handle<v8::Value> ext = v8::External::New(new OSMWayWrap(it));
            return scope.Close(OSMWayWrap::constructor->GetFunction()->NewInstance(1, &ext));
        }

        osmium::Way& object() {
            return static_cast<osmium::Way&>(*get());
        }

    private:

        ~OSMWayWrap();

    }; // class OSMWayWrap

} // namespace node_osmium

#endif // OSM_WAY_WRAP_HPP
