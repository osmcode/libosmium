
// node-osmium
#include "node_osmium.hpp"
#include "osm_entity_wrap.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> OSMEntityWrap::constructor;

    void OSMEntityWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMEntityWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(symbol_OSMEntity);
        target->Set(symbol_OSMEntity, constructor->GetFunction());
    }

    v8::Handle<v8::Value> OSMEntityWrap::New(const v8::Arguments& args) {
        if (args.Length() == 1 && args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            static_cast<OSMEntityWrap*>(ext->Value())->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.OSMEntity cannot be created in Javascript")));
        }
    }

} // namespace node_osmium
