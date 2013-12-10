// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/osm/relation.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>

using namespace v8;

namespace node_osmium {

    typedef osmium::io::InputIterator<osmium::io::Reader, osmium::Object> input_iterator;

    class Relation : public node::ObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        Relation(const input_iterator&);

        void _ref() {
            Ref();
        }

        void _unref() {
            Unref();
        }

        osmium::Relation& object() {
            return static_cast<osmium::Relation&>(*m_it);
        }

    private:

        input_iterator m_it;

        ~Relation();

    };

    Persistent<FunctionTemplate> Relation::constructor;

    void Relation::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Relation::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Relation"));
        target->Set(String::NewSymbol("Relation"), constructor->GetFunction());
    }

    Relation::Relation(const input_iterator& it) :
        ObjectWrap(),
        m_it(it) {
    }

    Relation::~Relation() {
    }

    Handle<Value> Relation::New(const Arguments& args) {
        HandleScope scope;
        if (args[0]->IsExternal()) {
            Local<External> ext = Local<External>::Cast(args[0]);
            void* ptr = ext->Value();
            Relation* relation = static_cast<Relation*>(ptr);
            relation->Wrap(args.This());
            osmium::Relation& obj = static_cast<osmium::Relation&>(*(relation->m_it));
            args.This()->Set(String::NewSymbol("id"), Number::New(obj.id()));
            args.This()->Set(String::NewSymbol("version"), Number::New(obj.version()));
            args.This()->Set(String::NewSymbol("changeset"), Number::New(obj.changeset()));
            args.This()->Set(String::NewSymbol("visible"), Boolean::New(obj.visible()));
            args.This()->Set(String::NewSymbol("timestamp"), String::New(obj.timestamp().to_iso().c_str(), obj.timestamp().to_iso().size()));
            args.This()->Set(String::NewSymbol("uid"), Number::New(obj.uid()));
            args.This()->Set(String::NewSymbol("user"), String::New(obj.user()));
            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("osmium.Relation cannot be created in Javascript")));
        }
        return Undefined();
    }

} // namespace node_osmium
