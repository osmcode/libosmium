#ifndef FILE_WRAP_HPP
#define FILE_WRAP_HPP

// c++11
#include <exception>
#include <memory>
#include <string>

// v8
#include <v8.h>

// node.js
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/io/file.hpp>

using namespace v8;

namespace node_osmium {

    typedef std::shared_ptr<osmium::io::File> file_ptr;

    class FileWrap : public node::ObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);

        FileWrap(const std::string& filename = "", const std::string& format = "");

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

        ~FileWrap() {
        }

        file_ptr this_;

    };

} // namespace node_osmium

#endif // FILE_WRAP_HPP
