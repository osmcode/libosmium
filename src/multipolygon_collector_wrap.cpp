
// node
#include <node_buffer.h>

// node-osmium
#include "node_osmium.hpp"
#include "multipolygon_collector_wrap.hpp"
#include "multipolygon_handler_wrap.hpp"
#include "handler.hpp"
#include "reader_wrap.hpp"
#include "utils.hpp"
#include "apply.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> MultipolygonCollectorWrap::constructor;

    void MultipolygonCollectorWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(MultipolygonCollectorWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(symbol_MultipolygonCollector);
        node::SetPrototypeMethod(constructor, "read_relations", read_relations);
        node::SetPrototypeMethod(constructor, "handler", handler);
        target->Set(symbol_MultipolygonCollector, constructor->GetFunction());
    }

    v8::Handle<v8::Value> MultipolygonCollectorWrap::New(const v8::Arguments& args) {
        v8::HandleScope scope;

        if (!args.IsConstructCall()) {
            return ThrowException(v8::Exception::Error(v8::String::New("Cannot call constructor as function, you need to use 'new' keyword")));
        }

        if (args.Length() != 0) {
            return ThrowException(v8::Exception::TypeError(v8::String::New("MultipolygonCollector is constructed without arguments")));
        }

        try {
            MultipolygonCollectorWrap* multipolygon_collector_wrap = new MultipolygonCollectorWrap();
            multipolygon_collector_wrap->Wrap(args.This());
            return args.This();
        } catch (const std::exception& e) {
            return ThrowException(v8::Exception::TypeError(v8::String::New(e.what())));
        }

        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> MultipolygonCollectorWrap::read_relations(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (args.Length() != 1 || !args[0]->IsObject() || !ReaderWrap::constructor->HasInstance(args[0]->ToObject())) {
            return ThrowException(v8::Exception::Error(v8::String::New("call MultipolygonCollector.read_relation() with Reader object")));
        }
        try {
            osmium::io::Reader& reader = unwrap<ReaderWrap>(args[0]->ToObject());
            unwrap<MultipolygonCollectorWrap>(args.This()).read_relations(reader);
        } catch (const std::exception& e) {
            std::string msg("osmium error: ");
            msg += e.what();
            return ThrowException(v8::Exception::Error(v8::String::New(msg.c_str())));
        }
        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> MultipolygonCollectorWrap::handler(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (args.Length() != 1 || !args[0]->IsObject() || !JSHandler::constructor->HasInstance(args[0]->ToObject())) {
            return ThrowException(v8::Exception::Error(v8::String::New("call MultipolygonCollector.handler() with Handler object")));
        }
        try {
            JSHandler& handler = unwrap<JSHandler>(args[0]->ToObject());
            auto& mc_handler = unwrap<MultipolygonCollectorWrap>(args.This()).handler([&handler](const osmium::memory::Buffer& area_buffer) {
                for (const osmium::OSMEntity& entity : area_buffer) {
                    handler.dispatch_entity(entity);
                }
            });
            return scope.Close(new_external<MultipolygonHandlerWrap>(mc_handler));

        } catch (const std::exception& e) {
            std::string msg("osmium error: ");
            msg += e.what();
            return ThrowException(v8::Exception::Error(v8::String::New(msg.c_str())));
        }
        return scope.Close(v8::Undefined());
    }

} // namespace node_osmium

