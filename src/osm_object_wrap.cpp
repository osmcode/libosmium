
#include "osm_object_wrap.hpp"

namespace node_osmium {

    v8::Handle<v8::Value> OSMObjectWrap::tags(const v8::Arguments& args) {
        v8::HandleScope scope;

        osmium::OSMObject& object = *(node::ObjectWrap::Unwrap<OSMObjectWrap>(args.This())->m_it);

        if (args.Length() == 0) {
            v8::Local<v8::Object> tags = v8::Object::New();
            for (auto& tag : object.tags()) {
                tags->Set(v8::String::New(tag.key()), v8::String::New(tag.value()));
            }
            return scope.Close(tags);
        } else if (args.Length() == 1) {
            const char* value = object.tags().get_value_by_key(*v8::String::Utf8Value(args[0]));
            if (value) {
                return scope.Close(v8::String::New(value));
            } else {
                return v8::Undefined();
            }
        }
        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_id(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).id()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_version(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).version()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_changeset(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).changeset()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_visible(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Boolean::New(wrapped(info.This()).visible()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_timestamp(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).timestamp()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_uid(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).uid()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_user(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::String::New(wrapped(info.This()).user()));
    }

} // namespace node_osmium
