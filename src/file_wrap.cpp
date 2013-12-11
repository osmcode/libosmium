
// c++
#include <exception>

#include "file_wrap.hpp"

namespace node_osmium {

    Persistent<FunctionTemplate> FileWrap::constructor;

    void FileWrap::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FileWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("File"));
        target->Set(String::NewSymbol("File"), constructor->GetFunction());
    }

    Handle<Value> FileWrap::New(const Arguments& args) {
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
            FileWrap* q = new FileWrap(filename, format);
            q->Wrap(args.This());
            return args.This();
        } catch (const std::exception& ex) {
            return ThrowException(Exception::TypeError(String::New(ex.what())));
        }
        return Undefined();
    }

} // namespace node_osmium

