// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/dump.hpp>
#include <osmium/visitor.hpp>

using namespace v8;

namespace node_osmium {

    class Buffer : public node::ObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(Arguments const& args);
        static Handle<Value> dump(Arguments const& args);
        Buffer(reader_ptr reader);
        void _ref() {
            Ref();
        }
        void _unref() {
            Unref();
        }
        osmium::memory::Buffer buf;

    private:

        ~Buffer();
    };

    Persistent<FunctionTemplate> Buffer::constructor;

    void Buffer::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Buffer::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Buffer"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "dump", dump);
        target->Set(String::NewSymbol("Buffer"), constructor->GetFunction());
    }

    Buffer::Buffer(reader_ptr reader) :
        ObjectWrap(),
        buf(reader.get()->read()) {
    }

    Buffer::~Buffer() {
    }

    Handle<Value> Buffer::New(const Arguments& args) {
        HandleScope scope;
        if (args[0]->IsExternal()) {
            Local<External> ext = Local<External>::Cast(args[0]);
            void* ptr = ext->Value();
            Buffer* b =  static_cast<Buffer*>(ptr);
            b->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("osmium.Buffer cannot be created in Javascript")));
        }
        return Undefined();
    }


    Handle<Value> Buffer::dump(const Arguments& args) {
        HandleScope scope;
        Buffer* b = node::ObjectWrap::Unwrap<Buffer>(args.This());
        std::ostringstream ss;
        osmium::osm::Dump dump(ss);
        osmium::apply(b->buf, dump);
        Local<String> obj = String::New(ss.str().c_str());
        return scope.Close(obj);
    }

} // namespace node_osmium

