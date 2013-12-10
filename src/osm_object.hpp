#ifndef OSM_OBJECT_HPP
#define OSM_OBJECT_HPP

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

using namespace v8;

namespace node_osmium {

    typedef osmium::io::InputIterator<osmium::io::Reader, osmium::Object> input_iterator;

    class OSMObject : public node::ObjectWrap {

    protected:

        input_iterator m_it;

    public:

        static Handle<Value> tags(const Arguments& args);

        OSMObject(const input_iterator& it) :
            m_it(it) {
        }

    }; // class OSMObject

    Handle<Value> OSMObject::tags(const Arguments& args) {
        HandleScope scope;

        osmium::Object& object = *(node::ObjectWrap::Unwrap<OSMObject>(args.This())->m_it);

        if (args.Length() == 0) {
            Local<Object> tags = Object::New();
            for (auto& tag : object.tags()) {
                tags->Set(String::NewSymbol(tag.key()), String::New(tag.value()));
            }
            return scope.Close(tags);
        } else if (args.Length() == 1) {
            const char* value = object.tags().get_value_by_key(*String::Utf8Value(args[0]));
            if (value) {
                return scope.Close(String::New(value));
            } else {
                return Undefined();
            }
        }
        return Undefined();
    }

} // namespace node_osmium

#endif // OSM_OBJECT_HPP
