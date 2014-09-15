#ifndef OSM_WAY_WRAP_HPP
#define OSM_WAY_WRAP_HPP

// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

// osmium
#include <osmium/osm/way.hpp>
namespace osmium {
    class OSMEntity;
}

// node-osmium
#include "osm_entity_wrap.hpp"
#include "osm_object_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    class OSMWayWrap : public OSMObjectWrap {

        static v8::Handle<v8::Value> get_nodes_count(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> nodes(const v8::Arguments& args);
        static v8::Handle<v8::Value> wkb(const v8::Arguments& args);
        static v8::Handle<v8::Value> wkt(const v8::Arguments& args);

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        static const osmium::Way& wrapped(const v8::Local<v8::Object>& object) {
            return static_cast<const osmium::Way&>(unwrap<OSMEntityWrap>(object));
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
