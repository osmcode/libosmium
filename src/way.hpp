// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/osm/way.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>

#include "osm_object.hpp"

using namespace v8;

namespace node_osmium {

    class Way : public OSMObject {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> wkb(const Arguments& args);
        static Handle<Value> wkt(const Arguments& args);
        static Handle<Value> nodes(const Arguments& args);
        Way(const input_iterator&);

        void _ref() {
            Ref();
        }

        void _unref() {
            Unref();
        }

        osmium::Way& object() {
            return static_cast<osmium::Way&>(*m_it);
        }

    private:

        ~Way();

    };

    Persistent<FunctionTemplate> Way::constructor;

    void Way::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Way::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Way"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "tags", tags);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkb", wkb);
        NODE_SET_PROTOTYPE_METHOD(constructor, "wkt", wkt);
        NODE_SET_PROTOTYPE_METHOD(constructor, "nodes", nodes);
        target->Set(String::NewSymbol("Way"), constructor->GetFunction());
    }

    Way::Way(const input_iterator& it) :
        OSMObject(it) {
    }

    Way::~Way() {
    }

    Handle<Value> Way::New(const Arguments& args) {
        HandleScope scope;
        if (args[0]->IsExternal()) {
            Local<External> ext = Local<External>::Cast(args[0]);
            void* ptr = ext->Value();
            Way* way = static_cast<Way*>(ptr);
            way->Wrap(args.This());
            osmium::Way& obj = static_cast<osmium::Way&>(*(way->m_it));
            args.This()->Set(String::NewSymbol("id"), Number::New(obj.id()));
            args.This()->Set(String::NewSymbol("version"), Number::New(obj.version()));
            args.This()->Set(String::NewSymbol("changeset"), Number::New(obj.changeset()));
            args.This()->Set(String::NewSymbol("visible"), Boolean::New(obj.visible()));
            args.This()->Set(String::NewSymbol("timestamp"), String::New(obj.timestamp().to_iso().c_str(), obj.timestamp().to_iso().size()));
            args.This()->Set(String::NewSymbol("uid"), Number::New(obj.uid()));
            args.This()->Set(String::NewSymbol("user"), String::New(obj.user()));
            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("osmium.Way cannot be created in Javascript")));
        }
        return Undefined();
    }

    Handle<Value> Way::wkb(const Arguments& args) {
        HandleScope scope;
        osmium::Way& way = static_cast<osmium::Way&>(*(node::ObjectWrap::Unwrap<Way>(args.This())->m_it));

        try {
            std::string wkb { wkb_factory.create_linestring(way) };
#if NODE_VERSION_AT_LEAST(0, 10, 0)
            return scope.Close(node::Buffer::New(wkb.data(), wkb.size())->handle_);
#else
            return scope.Close(node::Buffer::New(const_cast<char*>(wkb.data()), wkb.size())->handle_);
#endif
        } catch (osmium::geom::geometry_error&) {
            return scope.Close(Undefined());
        }
    }

    Handle<Value> Way::wkt(const Arguments& args) {
        HandleScope scope;
        osmium::Way& way = static_cast<osmium::Way&>(*(node::ObjectWrap::Unwrap<Way>(args.This())->m_it));

        try {
            std::string wkt { wkt_factory.create_linestring(way) };
            return scope.Close(String::New(wkt.c_str()));
        } catch (osmium::geom::geometry_error&) {
            return scope.Close(Undefined());
        }
    }

    Handle<Value> Way::nodes(const Arguments& args) {
        HandleScope scope;
        osmium::Way& way = static_cast<osmium::Way&>(*(node::ObjectWrap::Unwrap<Way>(args.This())->m_it));

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
