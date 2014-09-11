
// node
#include <node_buffer.h>
#include <node_version.h>

// osmium
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

// node-osmium
#include "osm_way_wrap.hpp"

namespace node_osmium {

    extern osmium::geom::WKBFactory<> wkb_factory;
    extern osmium::geom::WKTFactory<> wkt_factory;

    v8::Persistent<v8::FunctionTemplate> OSMWayWrap::constructor;

    void OSMWayWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMWayWrap::New));
        constructor->Inherit(OSMObjectWrap::constructor);
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Way"));
        auto attributes = static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
        set_accessor(constructor, "nodes_count", get_nodes_count, attributes);
        node::SetPrototypeMethod(constructor, "nodes", nodes);
        node::SetPrototypeMethod(constructor, "wkb", wkb);
        node::SetPrototypeMethod(constructor, "wkt", wkt);
        target->Set(v8::String::NewSymbol("Way"), constructor->GetFunction());
    }

    v8::Handle<v8::Value> OSMWayWrap::New(const v8::Arguments& args) {
        if (args.Length() == 1 && args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            static_cast<OSMWayWrap*>(ext->Value())->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.Way cannot be created in Javascript")));
        }
    }

    v8::Handle<v8::Value> OSMWayWrap::wkb(const v8::Arguments& args) {
        v8::HandleScope scope;

        try {
            std::string wkb { wkb_factory.create_linestring(wrapped(args.This())) };
#if NODE_VERSION_AT_LEAST(0, 10, 0)
            return scope.Close(node::Buffer::New(wkb.data(), wkb.size())->handle_);
#else
            return scope.Close(node::Buffer::New(const_cast<char*>(wkb.data()), wkb.size())->handle_);
#endif
        } catch (osmium::geometry_error&) {
            return scope.Close(v8::Undefined());
        }
    }

    v8::Handle<v8::Value> OSMWayWrap::wkt(const v8::Arguments& args) {
        v8::HandleScope scope;

        try {
            std::string wkt { wkt_factory.create_linestring(wrapped(args.This())) };
            return scope.Close(v8::String::New(wkt.c_str()));
        } catch (osmium::geometry_error&) {
            return scope.Close(v8::Undefined());
        }
    }

    v8::Handle<v8::Value> OSMWayWrap::get_nodes_count(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).nodes().size()));
    }

    v8::Handle<v8::Value> OSMWayWrap::nodes(const v8::Arguments& args) {
        v8::HandleScope scope;

        const osmium::Way& way = wrapped(args.This());

        switch (args.Length()) {
            case 0:
                {
                    v8::Local<v8::Array> nodes = v8::Array::New(way.nodes().size());
                    int i = 0;
                    for (const auto& node_ref : way.nodes()) {
                        nodes->Set(i, v8::Number::New(node_ref.ref()));
                        ++i;
                    }
                    return scope.Close(nodes);
                }
            case 1:
                {
                    if (!args[0]->IsNumber()) {
                        return ThrowException(v8::Exception::TypeError(v8::String::New("call nodes() without parameters or the index of the node you want")));
                    }
                    int n = static_cast<int>(args[0]->ToNumber()->Value());
                    if (n >= 0 && n < static_cast<int>(way.nodes().size())) {
                        return scope.Close(v8::Number::New(way.nodes()[n].ref()));
                    } else {
                        return ThrowException(v8::Exception::RangeError(v8::String::New("argument to nodes() out of range")));
                    }
                }
        }

        return ThrowException(v8::Exception::TypeError(v8::String::New("call nodes() without parameters or the index of the node you want")));
    }

} // namespace node_osmium

