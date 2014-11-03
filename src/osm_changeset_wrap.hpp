#ifndef OSM_CHANGESET_WRAP_HPP
#define OSM_CHANGESET_WRAP_HPP

// v8/node
#include "include_v8.hpp"

// osmium
#include <osmium/osm/changeset.hpp>
namespace osmium {
    class OSMEntity;
}

// node-osmium
#include "node_osmium.hpp"
#include "osm_entity_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    class OSMChangesetWrap : public OSMEntityWrap {

        static v8::Handle<v8::Value> get_type(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
            return symbol_changeset;
        }

        static v8::Handle<v8::Value> get_id(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_uid(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_user(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_num_changes(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_created_at(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_closed_at(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_open(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_closed(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_bounds(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> tags(const v8::Arguments& args);

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        static const osmium::Changeset& wrapped(const v8::Local<v8::Object>& object) {
            return static_cast<const osmium::Changeset&>(unwrap<OSMEntityWrap>(object));
        }

        OSMChangesetWrap(const osmium::OSMEntity& entity) :
            OSMEntityWrap(entity) {
        }

    private:

        ~OSMChangesetWrap() = default;

    }; // class OSMChangesetWrap

} // namespace node_osmium

#endif // OSM_CHANGESET_WRAP_HPP
