
#include "osm_object_wrap.hpp"

namespace node_osmium {

    Handle<Value> OSMObjectWrap::tags(const Arguments& args) {
        HandleScope scope;

        osmium::Object& object = *(node::ObjectWrap::Unwrap<OSMObjectWrap>(args.This())->m_it);

        if (args.Length() == 0) {
            Local<Object> tags = Object::New();
            for (auto& tag : object.tags()) {
                tags->Set(String::New(tag.key()), String::New(tag.value()));
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
