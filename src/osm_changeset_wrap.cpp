
#include "osm_changeset_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    extern v8::Persistent<v8::Object> module;

    v8::Persistent<v8::FunctionTemplate> OSMChangesetWrap::constructor;

    void OSMChangesetWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMChangesetWrap::New));
        constructor->Inherit(OSMEntityWrap::constructor);
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Changeset"));
        auto attributes = static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
        set_accessor(constructor, "id", get_id, attributes);
        set_accessor(constructor, "uid", get_uid, attributes);
        set_accessor(constructor, "user", get_user, attributes);
        set_accessor(constructor, "num_changes", get_num_changes, attributes);
        set_accessor(constructor, "created_at_seconds_since_epoch", get_created_at, attributes);
        set_accessor(constructor, "closed_at_seconds_since_epoch", get_closed_at, attributes);
        set_accessor(constructor, "open", get_open, attributes);
        set_accessor(constructor, "closed", get_closed, attributes);
        set_accessor(constructor, "bounds", get_bounds, attributes);
        node::SetPrototypeMethod(constructor, "tags", tags);
        target->Set(v8::String::NewSymbol("Changeset"), constructor->GetFunction());
    }

    v8::Handle<v8::Value> OSMChangesetWrap::New(const v8::Arguments& args) {
        if (args.Length() == 1 && args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            static_cast<OSMChangesetWrap*>(ext->Value())->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.OSMChangeset cannot be created in Javascript")));
        }
    }

    v8::Handle<v8::Value> OSMChangesetWrap::tags(const v8::Arguments& args) {
        v8::HandleScope scope;

        switch (args.Length()) {
            case 0:
                {
                    v8::Local<v8::Object> tags = v8::Object::New();
                    for (const auto& tag : wrapped(args.This()).tags()) {
                        tags->Set(v8::String::New(tag.key()), v8::String::New(tag.value()));
                    }
                    return scope.Close(tags);
                }
            case 1:
                {
                    if (!args[0]->IsString()) {
                        return ThrowException(v8::Exception::TypeError(v8::String::New("call tags() without parameters or with a string (key)")));
                    }
                    const char* value = wrapped(args.This()).tags().get_value_by_key(*v8::String::Utf8Value(args[0]));
                    return scope.Close(value ? v8::String::New(value) : v8::Undefined());
                }
        }

        return ThrowException(v8::Exception::TypeError(v8::String::New("call tags() without parameters or with a string (key)")));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_id(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).id()));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_uid(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).uid()));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_user(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::String::New(wrapped(info.This()).user()));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_num_changes(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).num_changes()));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_created_at(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).created_at()));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_closed_at(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).closed_at()));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_open(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Boolean::New(wrapped(info.This()).open()));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_closed(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Boolean::New(wrapped(info.This()).closed()));
    }

    v8::Handle<v8::Value> OSMChangesetWrap::get_bounds(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        return create_js_box(wrapped(info.This()).bounds());
    }

} // namespace node_osmium
