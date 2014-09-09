
#include "location_handler_wrap.hpp"

namespace node_osmium {

    Persistent<FunctionTemplate> LocationHandlerWrap::constructor;

    void LocationHandlerWrap::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LocationHandlerWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("LocationHandler"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "clear", clear);
        NODE_SET_PROTOTYPE_METHOD(constructor, "ignoreErrors", ignoreErrors);
        target->Set(String::NewSymbol("LocationHandler"), constructor->GetFunction());
    }

    Handle<Value> LocationHandlerWrap::New(const Arguments& args) {
        HandleScope scope;
        try {
            if (!args.IsConstructCall()) {
                return ThrowException(Exception::Error(String::New("Cannot call constructor as function, you need to use 'new' keyword")));
            }
            LocationHandlerWrap* q;
            if (args.Length() == 0) {
                q = new LocationHandlerWrap("sparsetable");
            } else {
                if (args.Length() != 1) {
                    return ThrowException(Exception::TypeError(String::New("please provide a node cache type as string when creating a LocationHandler")));
                }
                if (!args[0]->IsString()) {
                    return ThrowException(Exception::TypeError(String::New("please provide a node cache type as string when creating a LocationHandler")));
                }
                q = new LocationHandlerWrap(*String::Utf8Value(args[0]));
            }
            q->Wrap(args.This());
            return args.This();
        } catch (const std::exception& ex) {
            return ThrowException(Exception::TypeError(String::New(ex.what())));
        }
    }

    Handle<Value> LocationHandlerWrap::ignoreErrors(const Arguments& args) {
        HandleScope scope;
        LocationHandlerWrap* handler = node::ObjectWrap::Unwrap<LocationHandlerWrap>(args.This());
        handler->get()->ignore_errors();
        return scope.Close(Undefined());
    }

    Handle<Value> LocationHandlerWrap::clear(const Arguments& args) {
        HandleScope scope;
        // XXX do something here
        return scope.Close(Undefined());
    }

} // namespace node_osmium

