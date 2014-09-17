
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
    extern v8::Persistent<v8::Object> module;

    v8::Persistent<v8::FunctionTemplate> OSMWayWrap::constructor;

    void OSMWayWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMWayWrap::New));
        constructor->Inherit(OSMObjectWrap::constructor);
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Way"));
        auto attributes = static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
        set_accessor(constructor, "nodes_count", get_nodes_count, attributes);
        node::SetPrototypeMethod(constructor, "node_refs", node_refs);
        node::SetPrototypeMethod(constructor, "node_coordinates", node_coordinates);
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
        } catch (std::runtime_error& e) {
            return ThrowException(v8::Exception::Error(v8::String::New(e.what())));
        }
    }

    v8::Handle<v8::Value> OSMWayWrap::wkt(const v8::Arguments& args) {
        v8::HandleScope scope;

        try {
            std::string wkt { wkt_factory.create_linestring(wrapped(args.This())) };
            return scope.Close(v8::String::New(wkt.c_str()));
        } catch (std::runtime_error& e) {
            return ThrowException(v8::Exception::Error(v8::String::New(e.what())));
        }
    }

    v8::Handle<v8::Value> OSMWayWrap::get_nodes_count(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).nodes().size()));
    }

    v8::Handle<v8::Value> OSMWayWrap::node_refs(const v8::Arguments& args) {
        v8::HandleScope scope;

        const osmium::Way& way = wrapped(args.This());

        switch (args.Length()) {
            case 0: {
                v8::Local<v8::Array> nodes = v8::Array::New(way.nodes().size());
                int i = 0;
                for (const auto& node_ref : way.nodes()) {
                    nodes->Set(i, v8::Number::New(node_ref.ref()));
                    ++i;
                }
                return scope.Close(nodes);
            }
            case 1: {
                if (!args[0]->IsNumber()) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("call node_refs() without parameters or the index of the node you want")));
                }
                int n = static_cast<int>(args[0]->ToNumber()->Value());
                if (n >= 0 && n < static_cast<int>(way.nodes().size())) {
                    return scope.Close(v8::Number::New(way.nodes()[n].ref()));
                } else {
                    return ThrowException(v8::Exception::RangeError(v8::String::New("argument to node_refs() out of range")));
                }
            }
        }

        return ThrowException(v8::Exception::TypeError(v8::String::New("call node_refs() without parameters or the index of the node you want")));
    }

    v8::Handle<v8::Value> OSMWayWrap::node_coordinates(const v8::Arguments& args) {
        v8::HandleScope scope;

        auto cf = module->Get(v8::String::NewSymbol("Coordinates"));
        assert(cf->IsFunction());

        const osmium::Way& way = wrapped(args.This());

        switch (args.Length()) {
            case 0: {
                try {
                    v8::Local<v8::Array> nodes = v8::Array::New(way.nodes().size());
                    int i = 0;
                    for (const auto& node_ref : way.nodes()) {
                        const osmium::Location location = node_ref.location();
                        v8::Local<v8::Value> argv[2] = { v8::Number::New(location.lon()), v8::Number::New(location.lat()) };
                        nodes->Set(i, v8::Local<v8::Function>::Cast(cf)->NewInstance(2, argv));
                        ++i;
                    }
                    return scope.Close(nodes);
                } catch (osmium::invalid_location&) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("location of at least one of the nodes in this way not set")));
                }
            }
            case 1: {
                if (!args[0]->IsNumber()) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("call node_coordinates() without parameters or the index of the node you want")));
                }
                int n = static_cast<int>(args[0]->ToNumber()->Value());
                if (n >= 0 && n < static_cast<int>(way.nodes().size())) {
                    const osmium::Location location = way.nodes()[n].location();
                    if (location.valid()) {
                        v8::Local<v8::Value> argv[2] = { v8::Number::New(location.lon()), v8::Number::New(location.lat()) };
                        return scope.Close(v8::Local<v8::Function>::Cast(cf)->NewInstance(2, argv));
                    } else {
                        return scope.Close(v8::Undefined());
                    }
                } else {
                    return ThrowException(v8::Exception::RangeError(v8::String::New("argument to node_coordinates() out of range")));
                }
            }
        }

        return ThrowException(v8::Exception::TypeError(v8::String::New("call node_coordinates() without parameters or the index of the node you want")));
    }

} // namespace node_osmium

