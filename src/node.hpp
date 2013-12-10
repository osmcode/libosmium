// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

// osmium
#include <osmium/osm/node.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>

#include "osm_object.hpp"

using namespace v8;

namespace node_osmium {

    extern osmium::geom::WKBFactory wkb_factory;
    extern osmium::geom::WKTFactory wkt_factory;

    class Node : public OSMObject {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> wkb(const Arguments& args);
        static Handle<Value> wkt(const Arguments& args);
        Node(const input_iterator&);

        void _ref() {
            Ref();
        }

        void _unref() {
            Unref();
        }

        osmium::Node& object() {
            return static_cast<osmium::Node&>(*m_it);
        }

    private:

        ~Node();

    };

    Persistent<FunctionTemplate> Node::constructor;

    void Node::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Node::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Node"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "tags", tags);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkb", wkb);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkt", wkt);
        target->Set(String::NewSymbol("Node"), constructor->GetFunction());
    }

    Node::Node(const input_iterator& it) :
        OSMObject(it) {
    }

    Node::~Node() {
    }

    Handle<Value> Node::New(const Arguments& args) {
        HandleScope scope;
        if (args[0]->IsExternal()) {
            Local<External> ext = Local<External>::Cast(args[0]);
            void* ptr = ext->Value();
            Node* node = static_cast<Node*>(ptr);
            node->Wrap(args.This());
            osmium::Node& obj = static_cast<osmium::Node&>(*(node->m_it));
            args.This()->Set(String::NewSymbol("id"), Number::New(obj.id()));
            args.This()->Set(String::NewSymbol("version"), Number::New(obj.version()));
            args.This()->Set(String::NewSymbol("changeset"), Number::New(obj.changeset()));
            args.This()->Set(String::NewSymbol("visible"), Boolean::New(obj.visible()));
            args.This()->Set(String::NewSymbol("timestamp"), Number::New(obj.timestamp()));
            args.This()->Set(String::NewSymbol("timestamp_iso"), String::New(obj.timestamp().to_iso().c_str(), obj.timestamp().to_iso().size()));
            args.This()->Set(String::NewSymbol("uid"), Number::New(obj.uid()));
            args.This()->Set(String::NewSymbol("user"), String::New(obj.user()));
            args.This()->Set(String::NewSymbol("lon"), Number::New(obj.lon()));
            args.This()->Set(String::NewSymbol("lat"), Number::New(obj.lat()));

            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("osmium.Node cannot be created in Javascript")));
        }
        return Undefined();
    }

    Handle<Value> Node::wkb(const Arguments& args) {
        HandleScope scope;
        osmium::Node& node = static_cast<osmium::Node&>(*(node::ObjectWrap::Unwrap<Node>(args.This())->m_it));

        std::string wkb { wkb_factory.create_point(node) };
#if NODE_VERSION_AT_LEAST(0, 10, 0)
        return scope.Close(node::Buffer::New(wkb.data(), wkb.size())->handle_);
#else
        return scope.Close(node::Buffer::New(const_cast<char*>(wkb.data()), wkb.size())->handle_);
#endif
    }

    Handle<Value> Node::wkt(const Arguments& args) {
        HandleScope scope;
        osmium::Node& node = static_cast<osmium::Node&>(*(node::ObjectWrap::Unwrap<Node>(args.This())->m_it));

        std::string wkt { wkt_factory.create_point(node) };

        return scope.Close(String::New(wkt.c_str()));
    }

} // namespace node_osmium
