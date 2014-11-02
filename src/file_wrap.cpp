
// c++
#include <exception>

// v8
#include <v8.h>

// node
#include <node_buffer.h>

// node-osmium
#include "node_osmium.hpp"
#include "file_wrap.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> FileWrap::constructor;

    void FileWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(FileWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(symbol_File);
        target->Set(symbol_File, constructor->GetFunction());
    }

    v8::Handle<v8::Value> FileWrap::New(const v8::Arguments& args) {
        v8::HandleScope scope;

        if (!args.IsConstructCall()) {
            return ThrowException(v8::Exception::Error(v8::String::New("Cannot call constructor as function, you need to use 'new' keyword")));
        }

        if (args.Length() == 0 || args.Length() > 2) {
            return ThrowException(v8::Exception::TypeError(v8::String::New("File is constructed with one or two arguments: filename or node.Buffer and optional format")));
        }

        std::string format;
        if (args.Length() == 2) {
            if (!args[1]->IsString()) {
                return ThrowException(v8::Exception::TypeError(v8::String::New("second argument to File constructor (format) must be a string")));
            }
            format = *v8::String::Utf8Value(args[1]);
        }

        try {
            osmium::io::File file;

            if (args[0]->IsString()) {
                std::string filename = *v8::String::Utf8Value(args[0]);
                file = osmium::io::File(filename, format);
            } else if (args[0]->IsObject() && node::Buffer::HasInstance(args[0]->ToObject())) {
                auto source = args[0]->ToObject();
                file = osmium::io::File(node::Buffer::Data(source), node::Buffer::Length(source), format);
            } else {
                return ThrowException(v8::Exception::TypeError(v8::String::New("first argument to File constructor must be a string (filename) or node.Buffer")));
            }

            file.check();
            FileWrap* file_wrap = new FileWrap(std::move(file));
            file_wrap->Wrap(args.This());
            return args.This();
        } catch (const std::exception& e) {
            return ThrowException(v8::Exception::TypeError(v8::String::New(e.what())));
        }

        return scope.Close(v8::Undefined());
    }

} // namespace node_osmium

