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

using namespace v8;

#define SET_ACCESSOR(t, name, getter,attributes)                                         \
    t->InstanceTemplate()->SetAccessor(String::NewSymbol(name),getter,NULL,Handle<Value>(),v8::DEFAULT,attributes); \
 

namespace node_osmium {

    typedef osmium::io::InputIterator<osmium::io::Reader, osmium::OSMObject> input_iterator;

    class OSMObjectWrap : public node::ObjectWrap {

        input_iterator m_it;

    public:

        static Handle<Value> tags(const Arguments& args);
        static Handle<Value> get_id(Local<String> property,const AccessorInfo& info);
        static Handle<Value> get_version(Local<String> property,const AccessorInfo& info);
        static Handle<Value> get_changeset(Local<String> property,const AccessorInfo& info);
        static Handle<Value> get_visible(Local<String> property,const AccessorInfo& info);
        static Handle<Value> get_timestamp(Local<String> property,const AccessorInfo& info);
        static Handle<Value> get_uid(Local<String> property,const AccessorInfo& info);
        static Handle<Value> get_user(Local<String> property,const AccessorInfo& info);

        static osmium::OSMObject& wrapped(Local<Object> object) {
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
