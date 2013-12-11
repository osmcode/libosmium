
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

#include "osm_way_wrap.hpp"

namespace node_osmium {

    extern osmium::geom::WKBFactory wkb_factory;
    extern osmium::geom::WKTFactory wkt_factory;

    Persistent<FunctionTemplate> OSMWayWrap::constructor;

    void OSMWayWrap::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(OSMWayWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Way"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "tags", tags);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkb", wkb);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkt", wkt);
        NODE_SET_PROTOTYPE_METHOD(constructor, "nodes", nodes);
        target->Set(String::NewSymbol("Way"), constructor->GetFunction());
    }

    OSMWayWrap::OSMWayWrap(const input_iterator& it) :
        OSMObjectWrap(it) {
    }

    OSMWayWrap::~OSMWayWrap() {
    }

    Handle<Value> OSMWayWrap::New(const Arguments& args) {
        HandleScope scope;
        if (args[0]->IsExternal()) {
            Local<External> ext = Local<External>::Cast(args[0]);
            void* ptr = ext->Value();
            OSMWayWrap* way = static_cast<OSMWayWrap*>(ptr);
            way->Wrap(args.This());
            osmium::Way& obj = static_cast<osmium::Way&>(*(way->get()));
            args.This()->Set(String::New("id"), Number::New(obj.id()));
            args.This()->Set(String::New("version"), Number::New(obj.version()));
            args.This()->Set(String::New("changeset"), Number::New(obj.changeset()));
            args.This()->Set(String::New("visible"), Boolean::New(obj.visible()));
            args.This()->Set(String::New("timestamp"), Number::New(obj.timestamp()));
            args.This()->Set(String::New("timestamp_iso"), String::New(obj.timestamp().to_iso().c_str(), obj.timestamp().to_iso().size()));
            args.This()->Set(String::New("uid"), Number::New(obj.uid()));
            args.This()->Set(String::New("user"), String::New(obj.user()));
            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("osmium.Way cannot be created in Javascript")));
        }
        return Undefined();
    }

    Handle<Value> OSMWayWrap::wkb(const Arguments& args) {
        HandleScope scope;

        try {
            std::string wkb { wkb_factory.create_linestring(wrapped(args.This())) };
#if NODE_VERSION_AT_LEAST(0, 10, 0)
            return scope.Close(node::Buffer::New(wkb.data(), wkb.size())->handle_);
#else
            return scope.Close(node::Buffer::New(const_cast<char*>(wkb.data()), wkb.size())->handle_);
#endif
        } catch (osmium::geom::geometry_error&) {
            return scope.Close(Undefined());
        }
    }

    Handle<Value> OSMWayWrap::wkt(const Arguments& args) {
        HandleScope scope;

        try {
            std::string wkt { wkt_factory.create_linestring(wrapped(args.This())) };
            return scope.Close(String::New(wkt.c_str()));
        } catch (osmium::geom::geometry_error&) {
            return scope.Close(Undefined());
        }
    }

    Handle<Value> OSMWayWrap::nodes(const Arguments& args) {
        HandleScope scope;
        osmium::Way& way = static_cast<osmium::Way&>(*(node::ObjectWrap::Unwrap<OSMWayWrap>(args.This())->get()));

        if (args.Length() == 0) {
            Local<Array> nodes = Array::New(way.nodes().size());
            int i = 0;
            for (auto& wn : way.nodes()) {
                nodes->Set(i, Number::New(wn.ref()));
                ++i;
            }
            return scope.Close(nodes);
        } else if (args.Length() == 1) {
            if (args[0]->IsNumber()) {
                int n = static_cast<int>(args[0]->ToNumber()->Value());
                if (n > 0 && n < static_cast<int>(way.nodes().size())) {
                    return scope.Close(Number::New(way.nodes()[n].ref()));
                }
            }
        }
        return Undefined();
    }

} // namespace node_osmium

