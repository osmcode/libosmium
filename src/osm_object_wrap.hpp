#ifndef OSM_OBJECT_WRAP_HPP
#define OSM_OBJECT_WRAP_HPP

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

// osmium
#include <osmium/io/reader.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/osm/object.hpp>

namespace node_osmium {

    typedef osmium::io::InputIterator<osmium::io::Reader, osmium::OSMObject> input_iterator;

    class OSMObjectWrap : public node::ObjectWrap {

        input_iterator m_it;

    protected:

        typedef v8::Handle<v8::Value> accessor_type(v8::Local<v8::String> property, const v8::AccessorInfo& info);

        static void set_accessor(v8::Persistent<v8::FunctionTemplate> t, const char* name, accessor_type getter, v8::PropertyAttribute attributes) {
            t->InstanceTemplate()->SetAccessor(v8::String::NewSymbol(name), getter, nullptr, v8::Handle<v8::Value>(), v8::DEFAULT, attributes);
        }

    public:

        static v8::Handle<v8::Value> tags(const v8::Arguments& args);
        static v8::Handle<v8::Value> get_id(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_version(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_changeset(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_visible(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_timestamp(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_uid(v8::Local<v8::String> property, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> get_user(v8::Local<v8::String> property, const v8::AccessorInfo& info);

        static osmium::OSMObject& wrapped(v8::Local<v8::Object> object) {
            return *(node::ObjectWrap::Unwrap<OSMObjectWrap>(object)->get());
        }

        OSMObjectWrap(const input_iterator& it) :
            m_it(it) {
        }

        input_iterator& get() {
            return m_it;
        }

    }; // class OSMObjectWrap

} // namespace node_osmium

#endif // OSM_OBJECT_WRAP_HPP
