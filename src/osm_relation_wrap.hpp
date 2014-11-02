#ifndef OSM_RELATION_WRAP_HPP
#define OSM_RELATION_WRAP_HPP

// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

// osmium
#include <osmium/osm/relation.hpp>
namespace osmium {
    class OSMEntity;
}

// node-osmium
#include "node_osmium.hpp"
#include "osm_entity_wrap.hpp"
#include "osm_object_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    class OSMRelationWrap : public OSMObjectWrap {

        static v8::Handle<v8::Value> get_type(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
            return symbol_relation;
        }

        static v8::Handle<v8::Value> get_members_count(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> members(const v8::Arguments& args);

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        static const osmium::Relation& wrapped(const v8::Local<v8::Object>& object) {
            return static_cast<const osmium::Relation&>(unwrap<OSMEntityWrap>(object));
        }

        OSMRelationWrap(const osmium::OSMEntity& entity) :
            OSMObjectWrap(entity) {
        }

    private:

        ~OSMRelationWrap() {
        }

    }; // class OSMRelationWrap

} // namespace node_osmium

#endif // OSM_RELATION_WRAP_HPP
