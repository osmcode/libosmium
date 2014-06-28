
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

#include "osm_node_wrap.hpp"

namespace node_osmium {

    extern osmium::geom::WKBFactory<> wkb_factory;
    extern osmium::geom::WKTFactory<> wkt_factory;

    Persistent<FunctionTemplate> OSMNodeWrap::constructor;

    void OSMNodeWrap::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(OSMNodeWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Node"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "tags", tags);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkb", wkb);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkt", wkt);
        enum PropertyAttribute attributes =
            static_cast<PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
        SET_ACCESSOR(constructor, "id", get_id, attributes);
        SET_ACCESSOR(constructor, "version", get_version, attributes);
        SET_ACCESSOR(constructor, "changeset", get_changeset, attributes);
        SET_ACCESSOR(constructor, "visible", get_visible, attributes);
        SET_ACCESSOR(constructor, "timestamp", get_timestamp, attributes);
        SET_ACCESSOR(constructor, "uid", get_uid, attributes);
        SET_ACCESSOR(constructor, "user", get_user, attributes);
        SET_ACCESSOR(constructor, "lon", get_lon, attributes);
        SET_ACCESSOR(constructor, "lat", get_lat, attributes);
        target->Set(String::NewSymbol("Node"), constructor->GetFunction());
    }

    OSMNodeWrap::OSMNodeWrap(const input_iterator& it) :
        OSMObjectWrap(it) {
    }

    OSMNodeWrap::~OSMNodeWrap() {
    }

    Handle<Value> OSMNodeWrap::New(const Arguments& args) {
        HandleScope scope;
        if (args[0]->IsExternal()) {
            Local<External> ext = Local<External>::Cast(args[0]);
            void* ptr = ext->Value();
            OSMNodeWrap* node = static_cast<OSMNodeWrap*>(ptr);
            node->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("osmium.Node cannot be created in Javascript")));
        }
        return scope.Close(Undefined());
    }

    Handle<Value> OSMNodeWrap::get_lon(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(Number::New(wrapped(info.This()).lon()));
    }

    Handle<Value> OSMNodeWrap::get_lat(Local<String> property,const AccessorInfo& info) {
        HandleScope scope;
        return scope.Close(Number::New(wrapped(info.This()).lat()));
    }

    Handle<Value> OSMNodeWrap::wkb(const Arguments& args) {
        HandleScope scope;

        std::string wkb { wkb_factory.create_point(wrapped(args.This())) };
#if NODE_VERSION_AT_LEAST(0, 10, 0)
        return scope.Close(node::Buffer::New(wkb.data(), wkb.size())->handle_);
#else
        return scope.Close(node::Buffer::New(const_cast<char*>(wkb.data()), wkb.size())->handle_);
#endif
    }

    Handle<Value> OSMNodeWrap::wkt(const Arguments& args) {
        HandleScope scope;

        std::string wkt { wkt_factory.create_point(wrapped(args.This())) };

        return scope.Close(String::New(wkt.c_str()));
    }
} // namespace node_osmium
