
#include <memory>
#include <string>

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/io/file.hpp>

using namespace v8;

namespace node_osmium {

    typedef std::shared_ptr<osmium::io::File> file_ptr;

    class File : public node::ObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(Arguments const& args);

        File(const std::string& filename = "", const std::string& format = "");

        void _ref() {
            Ref();
        }

        void _unref() {
            Unref();
        }

        file_ptr get() {
            return this_;
        }

    private:

        ~File() {
        }

        file_ptr this_;

    };

    Persistent<FunctionTemplate> File::constructor;

    void File::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(File::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("File"));
        target->Set(String::NewSymbol("File"), constructor->GetFunction());
    }

    File::File(const std::string& filename, const std::string& format) :
        ObjectWrap(),
        this_(std::make_shared<osmium::io::File>(filename, format)) {
    }

    Handle<Value> File::New(Arguments const& args) {
        HandleScope scope;
        if (!args.IsConstructCall()) {
            return ThrowException(Exception::Error(String::New("Cannot call constructor as function, you need to use 'new' keyword")));
        }
        try {
            if (args.Length() > 2) {
                return ThrowException(Exception::TypeError(String::New("File is constructed with a maximum of two arguments")));
            }

            std::string filename;
            std::string format;
            if (args.Length() > 0) {
                if (!args[0]->IsString()) {
                    return ThrowException(Exception::TypeError(String::New("first argument to File constructor must be a string")));
                }
                filename = *String::Utf8Value(args[0]);
            }
            if (args.Length() > 1) {
                if (!args[1]->IsString()) {
                    return ThrowException(Exception::TypeError(String::New("second argument to File constructor must be a string")));
                }
                format = *String::Utf8Value(args[1]);
            }
            File* q = new File(filename, format);
            q->Wrap(args.This());
            return args.This();
        } catch (std::exception const& ex) {
            return ThrowException(Exception::TypeError(String::New(ex.what())));
        }
        return Undefined();
    }

} // namespace node_osmium

