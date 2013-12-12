
#include "location_handler_wrap.hpp"

namespace node_osmium {

    Persistent<FunctionTemplate> LocationHandlerWrap::constructor;

    void LocationHandlerWrap::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LocationHandlerWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("LocationHandler"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "clear", clear);
        target->Set(String::NewSymbol("LocationHandler"), constructor->GetFunction());
    }

    Handle<Value> LocationHandlerWrap::New(const Arguments& args) {
        HandleScope scope;
        try {
            if (!args.IsConstructCall()) {
                return ThrowException(Exception::Error(String::New("Cannot call constructor as function, you need to use 'new' keyword")));
            }
            LocationHandlerWrap* q = new LocationHandlerWrap();
            q->Wrap(args.This());
            return args.This();
        } catch (const std::exception& ex) {
            return ThrowException(Exception::TypeError(String::New(ex.what())));
        }
    }

    Handle<Value> LocationHandlerWrap::clear(const Arguments& args) {
        HandleScope scope;
        // XXX do something here
        return scope.Close(Undefined());
    }

} // namespace node_osmium

