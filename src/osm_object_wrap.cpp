
#include "osm_object_wrap.hpp"

namespace node_osmium {

    Handle<Value> OSMObjectWrap::tags(const Arguments& args) {
        HandleScope scope;

        osmium::OSMObject& object = *(node::ObjectWrap::Unwrap<OSMObjectWrap>(args.This())->m_it);

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

    Handle<Value> OSMObjectWrap::get_id(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(Number::New(wrapped(info.This()).id()));
    }

    Handle<Value> OSMObjectWrap::get_version(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(Number::New(wrapped(info.This()).version()));
    }

    Handle<Value> OSMObjectWrap::get_changeset(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(Number::New(wrapped(info.This()).changeset()));
    }

    Handle<Value> OSMObjectWrap::get_visible(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(Boolean::New(wrapped(info.This()).visible()));
    }

    Handle<Value> OSMObjectWrap::get_timestamp(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(Number::New(wrapped(info.This()).timestamp()));
    }

    Handle<Value> OSMObjectWrap::get_uid(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(Number::New(wrapped(info.This()).uid()));
    }

    Handle<Value> OSMObjectWrap::get_user(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(String::New(wrapped(info.This()).user()));
    }

} // namespace node_osmium
