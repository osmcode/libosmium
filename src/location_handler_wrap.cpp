
#include "location_handler_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> LocationHandlerWrap::constructor;

    void LocationHandlerWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(LocationHandlerWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("LocationHandler"));
        node::SetPrototypeMethod(constructor, "clear", clear);
        node::SetPrototypeMethod(constructor, "ignoreErrors", ignoreErrors);
        target->Set(v8::String::NewSymbol("LocationHandler"), constructor->GetFunction());
    }

    v8::Handle<v8::Value> LocationHandlerWrap::New(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (!args.IsConstructCall()) {
            return ThrowException(v8::Exception::Error(v8::String::New("Cannot call constructor as function, you need to use 'new' keyword")));
        }

        try {
            LocationHandlerWrap* location_handler_wrap;
            if (args.Length() == 0) {
                location_handler_wrap = new LocationHandlerWrap("sparsetable");
            } else {
                if (args.Length() != 1) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a node cache type as string when creating a LocationHandler")));
                }
                if (!args[0]->IsString()) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a node cache type as string when creating a LocationHandler")));
                }
                location_handler_wrap = new LocationHandlerWrap(*v8::String::Utf8Value(args[0]));
            }
            location_handler_wrap->Wrap(args.This());
            return args.This();
        } catch (const std::exception& ex) {
            return ThrowException(v8::Exception::TypeError(v8::String::New(ex.what())));
        }
    }

    v8::Handle<v8::Value> LocationHandlerWrap::ignoreErrors(const v8::Arguments& args) {
        v8::HandleScope scope;
        unwrap<LocationHandlerWrap>(args.This()).ignore_errors();
        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> LocationHandlerWrap::clear(const v8::Arguments& args) {
        v8::HandleScope scope;
        // XXX do something here
        return scope.Close(v8::Undefined());
    }

} // namespace node_osmium

