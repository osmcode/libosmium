
// node
#include <node_buffer.h>

// node-osmium
#include "node_osmium.hpp"
#include "multipolygon_handler_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> MultipolygonHandlerWrap::constructor;

    void MultipolygonHandlerWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(MultipolygonHandlerWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(symbol_MultipolygonHandler);
        target->Set(symbol_MultipolygonHandler, constructor->GetFunction());
    }

    v8::Handle<v8::Value> MultipolygonHandlerWrap::New(const v8::Arguments& args) {
        if (args.Length() == 1 && args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            static_cast<MultipolygonHandlerWrap*>(ext->Value())->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.MultipolygonHandler cannot be created in Javascript")));
        }
    }

} // namespace node_osmium

