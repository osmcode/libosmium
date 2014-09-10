
// c++
#include <exception>

// v8
#include <v8.h>

#include "file_wrap.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> FileWrap::constructor;

    void FileWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(FileWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("File"));
        target->Set(v8::String::NewSymbol("File"), constructor->GetFunction());
    }

    v8::Handle<v8::Value> FileWrap::New(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (!args.IsConstructCall()) {
            return ThrowException(v8::Exception::Error(v8::String::New("Cannot call constructor as function, you need to use 'new' keyword")));
        }
        try {
            if (args.Length() > 2) {
                return ThrowException(v8::Exception::TypeError(v8::String::New("File is constructed with a maximum of two arguments")));
            }

            std::string filename;
            std::string format;
            if (args.Length() > 0) {
                if (!args[0]->IsString()) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("first argument to File constructor must be a string")));
                }
                filename = *v8::String::Utf8Value(args[0]);
            }
            if (args.Length() > 1) {
                if (!args[1]->IsString()) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("second argument to File constructor must be a string")));
                }
                format = *v8::String::Utf8Value(args[1]);
            }
            FileWrap* q = new FileWrap(filename, format);
            q->Wrap(args.This());
            return args.This();
        } catch (const std::exception& ex) {
            return ThrowException(v8::Exception::TypeError(v8::String::New(ex.what())));
        }
        return scope.Close(v8::Undefined());
    }

} // namespace node_osmium

