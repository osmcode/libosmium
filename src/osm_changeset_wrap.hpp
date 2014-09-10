#ifndef OSM_CHANGESET_WRAP_HPP
#define OSM_CHANGESET_WRAP_HPP

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/osm/changeset.hpp>

// node-osmium
#include "osm_entity_wrap.hpp"

namespace node_osmium {

    class OSMChangesetWrap : public OSMEntityWrap {

        static v8::Persistent<v8::FunctionTemplate> constructor;

        static v8::Handle<v8::Value> get_id(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_uid(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_user(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_num_changes(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_created_at(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_closed_at(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_open(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_closed(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> tags(const v8::Arguments& args);

    public:

        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Local<v8::Object> create(const osmium::OSMEntity& entity);

        static const osmium::Changeset& wrapped(v8::Local<v8::Object> object) {
            return static_cast<const osmium::Changeset&>(OSMEntityWrap::wrapped(object));
        }

        OSMChangesetWrap(const osmium::OSMEntity& entity) :
            OSMEntityWrap(entity) {
        }

    }; // class OSMChangesetWrap

} // namespace node_osmium

#endif // OSM_CHANGESET_WRAP_HPP
