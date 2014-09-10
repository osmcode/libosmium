
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

#include "osm_node_wrap.hpp"

namespace node_osmium {

    extern osmium::geom::WKBFactory<> wkb_factory;
    extern osmium::geom::WKTFactory<> wkt_factory;

    v8::Persistent<v8::FunctionTemplate> OSMNodeWrap::constructor;

    void OSMNodeWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMNodeWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Node"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "tags", tags);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkb", wkb);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkt", wkt);
        enum v8::PropertyAttribute attributes =
            static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
        set_accessor(constructor, "id", get_id, attributes);
        set_accessor(constructor, "version", get_version, attributes);
        set_accessor(constructor, "changeset", get_changeset, attributes);
        set_accessor(constructor, "visible", get_visible, attributes);
        set_accessor(constructor, "timestamp", get_timestamp, attributes);
        set_accessor(constructor, "uid", get_uid, attributes);
        set_accessor(constructor, "user", get_user, attributes);
        set_accessor(constructor, "lon", get_lon, attributes);
        set_accessor(constructor, "lat", get_lat, attributes);
        target->Set(v8::String::NewSymbol("Node"), constructor->GetFunction());
    }

    OSMNodeWrap::OSMNodeWrap(const input_iterator& it) :
        OSMObjectWrap(it) {
    }

    OSMNodeWrap::~OSMNodeWrap() {
    }

    v8::Handle<v8::Value> OSMNodeWrap::New(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            void* ptr = ext->Value();
            OSMNodeWrap* node = static_cast<OSMNodeWrap*>(ptr);
            node->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.Node cannot be created in Javascript")));
        }
        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> OSMNodeWrap::get_lon(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).location().lon()));
    }

    v8::Handle<v8::Value> OSMNodeWrap::get_lat(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).location().lat()));
    }

    v8::Handle<v8::Value> OSMNodeWrap::wkb(const v8::Arguments& args) {
        v8::HandleScope scope;

        std::string wkb { wkb_factory.create_point(wrapped(args.This())) };
#if NODE_VERSION_AT_LEAST(0, 10, 0)
        return scope.Close(node::Buffer::New(wkb.data(), wkb.size())->handle_);
#else
        return scope.Close(node::Buffer::New(const_cast<char*>(wkb.data()), wkb.size())->handle_);
#endif
    }

    v8::Handle<v8::Value> OSMNodeWrap::wkt(const v8::Arguments& args) {
        v8::HandleScope scope;

        std::string wkt { wkt_factory.create_point(wrapped(args.This())) };

        return scope.Close(v8::String::New(wkt.c_str()));
    }
} // namespace node_osmium
