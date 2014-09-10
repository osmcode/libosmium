#ifndef OSM_ENTITY_WRAP_HPP
#define OSM_ENTITY_WRAP_HPP

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/osm/entity.hpp>

namespace node_osmium {

    class OSMEntityWrap : public node::ObjectWrap {

        const osmium::OSMEntity* m_entity;

    protected:

        typedef v8::Handle<v8::Value> accessor_type(v8::Local<v8::String> property, const v8::AccessorInfo& info);

        static void set_accessor(v8::Persistent<v8::FunctionTemplate> t, const char* name, accessor_type getter, v8::PropertyAttribute attributes) {
            t->InstanceTemplate()->SetAccessor(v8::String::NewSymbol(name), getter, nullptr, v8::Handle<v8::Value>(), v8::DEFAULT, attributes);
        }

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        static const osmium::OSMEntity& wrapped(v8::Local<v8::Object> object) {
            return static_cast<const osmium::OSMEntity&>(*(node::ObjectWrap::Unwrap<OSMEntityWrap>(object)->get()));
        }

        OSMEntityWrap(const osmium::OSMEntity& entity) :
            m_entity(&entity) {
        }

        const osmium::OSMEntity* get() {
            return m_entity;
        }

    }; // class OSMObjectWrap

} // namespace node_osmium

#endif // OSM_ENTITY_WRAP_HPP
