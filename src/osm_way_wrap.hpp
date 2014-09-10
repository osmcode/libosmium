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
#include <osmium/io/reader.hpp>

#include "osm_object_wrap.hpp"

namespace node_osmium {

    class OSMWayWrap : public OSMObjectWrap {

        static v8::Persistent<v8::FunctionTemplate> constructor;

        static v8::Handle<v8::Value> get_nodes_count(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info);

        static v8::Handle<v8::Value> nodes(const v8::Arguments& args);
        static v8::Handle<v8::Value> wkb(const v8::Arguments& args);
        static v8::Handle<v8::Value> wkt(const v8::Arguments& args);

    public:

        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Local<v8::Object> create(const osmium::OSMEntity& entity);

        static const osmium::Way& wrapped(v8::Local<v8::Object> object) {
            return static_cast<const osmium::Way&>(OSMObjectWrap::wrapped(object));
        }

        OSMWayWrap(const osmium::OSMEntity& entity) :
            OSMObjectWrap(entity) {
        }

    private:

        ~OSMWayWrap() {
        }

    }; // class OSMWayWrap

} // namespace node_osmium

#endif // OSM_WAY_WRAP_HPP
