
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

#include "osm_node_wrap.hpp"

namespace node_osmium {

    extern osmium::geom::WKBFactory wkb_factory;
    extern osmium::geom::WKTFactory wkt_factory;

    Persistent<FunctionTemplate> OSMNodeWrap::constructor;

    void OSMNodeWrap::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(OSMNodeWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Node"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "tags", tags);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkb", wkb);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkt", wkt);
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
            osmium::Node& obj = static_cast<osmium::Node&>(*(node->get()));
            args.This()->Set(String::New("id"), Number::New(obj.id()));
            args.This()->Set(String::New("version"), Number::New(obj.version()));
            args.This()->Set(String::New("changeset"), Number::New(obj.changeset()));
            args.This()->Set(String::New("visible"), Boolean::New(obj.visible()));
            args.This()->Set(String::New("timestamp"), Number::New(obj.timestamp()));
            args.This()->Set(String::New("timestamp_iso"), String::New(obj.timestamp().to_iso().c_str(), obj.timestamp().to_iso().size()));
            args.This()->Set(String::New("uid"), Number::New(obj.uid()));
            args.This()->Set(String::New("user"), String::New(obj.user()));
            args.This()->Set(String::New("lon"), Number::New(obj.lon()));
            args.This()->Set(String::New("lat"), Number::New(obj.lat()));

            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("osmium.Node cannot be created in Javascript")));
        }
        return scope.Close(Undefined());
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
