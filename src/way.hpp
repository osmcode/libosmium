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

using namespace v8;

namespace node_osmium {

    typedef osmium::io::InputIterator<osmium::io::Reader, osmium::Object> input_iterator;

    class Way : public node::ObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
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

        input_iterator m_it;

        ~Way();

    };

    Persistent<FunctionTemplate> Way::constructor;

    void Way::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Way::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Way"));
        target->Set(String::NewSymbol("Way"), constructor->GetFunction());
    }

    Way::Way(const input_iterator& it) :
        ObjectWrap(),
        m_it(it) {
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

} // namespace node_osmium
