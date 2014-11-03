#ifndef OSM_AREA_WRAP_HPP
#define OSM_AREA_WRAP_HPP

// v8/node
#include "include_v8.hpp"

// osmium
#include <osmium/osm/area.hpp>
namespace osmium {
    class OSMEntity;
}

// node-osmium
#include "node_osmium.hpp"
#include "osm_entity_wrap.hpp"
#include "osm_object_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    class OSMAreaWrap : public OSMObjectWrap {

        static v8::Handle<v8::Value> get_type(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
            return symbol_area;
        }

        static v8::Handle<v8::Value> wkb(const v8::Arguments& args);
        static v8::Handle<v8::Value> wkt(const v8::Arguments& args);

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        static const osmium::Area& wrapped(const v8::Local<v8::Object>& object) {
            return static_cast<const osmium::Area&>(unwrap<OSMEntityWrap>(object));
        }

        OSMAreaWrap(const osmium::OSMEntity& entity) :
            OSMObjectWrap(entity) {
        }

    private:

        ~OSMAreaWrap() = default;

    }; // class OSMAreaWrap

} // namespace node_osmium

#endif // OSM_AREA_WRAP_HPP
