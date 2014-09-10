
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

#include "osm_way_wrap.hpp"

namespace node_osmium {

    extern osmium::geom::WKBFactory<> wkb_factory;
    extern osmium::geom::WKTFactory<> wkt_factory;

    v8::Persistent<v8::FunctionTemplate> OSMWayWrap::constructor;

    void OSMWayWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMWayWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Way"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "tags", tags);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkb", wkb);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkt", wkt);
        NODE_SET_PROTOTYPE_METHOD(constructor, "nodes", nodes);
        enum v8::PropertyAttribute attributes =
            static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
        set_accessor(constructor, "id", get_id, attributes);
        set_accessor(constructor, "version", get_version, attributes);
        set_accessor(constructor, "changeset", get_changeset, attributes);
        set_accessor(constructor, "visible", get_visible, attributes);
        set_accessor(constructor, "timestamp", get_timestamp, attributes);
        set_accessor(constructor, "uid", get_uid, attributes);
        set_accessor(constructor, "user", get_user, attributes);
        target->Set(v8::String::NewSymbol("Way"), constructor->GetFunction());
    }

    OSMWayWrap::OSMWayWrap(const input_iterator& it) :
        OSMObjectWrap(it) {
    }

    OSMWayWrap::~OSMWayWrap() {
    }

    v8::Handle<v8::Value> OSMWayWrap::New(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            void* ptr = ext->Value();
            OSMWayWrap* way = static_cast<OSMWayWrap*>(ptr);
            way->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.Way cannot be created in Javascript")));
        }
        return v8::Undefined();
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

    v8::Handle<v8::Value> OSMWayWrap::nodes(const v8::Arguments& args) {
        v8::HandleScope scope;
        osmium::Way& way = static_cast<osmium::Way&>(*(node::ObjectWrap::Unwrap<OSMWayWrap>(args.This())->get()));

        if (args.Length() == 0) {
            v8::Local<v8::Array> nodes = v8::Array::New(way.nodes().size());
            int i = 0;
            for (auto& wn : way.nodes()) {
                nodes->Set(i, v8::Number::New(wn.ref()));
                ++i;
            }
            return scope.Close(nodes);
        } else if (args.Length() == 1) {
            if (args[0]->IsNumber()) {
                int n = static_cast<int>(args[0]->ToNumber()->Value());
                if (n > 0 && n < static_cast<int>(way.nodes().size())) {
                    return scope.Close(v8::Number::New(way.nodes()[n].ref()));
                }
            }
        }
        return v8::Undefined();
    }

} // namespace node_osmium

