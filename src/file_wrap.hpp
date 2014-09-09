#ifndef FILE_WRAP_HPP
#define FILE_WRAP_HPP

// c++
#include <memory>
#include <string>

// v8
#include <v8.h>

// node.js
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

        FileWrap(const std::string& filename, const std::string& format) :
            ObjectWrap(),
            m_this(std::make_shared<osmium::io::File>(filename, format)) {
        }

        file_ptr get() {
            return m_this;
        }

    private:

        ~FileWrap() {
        }

        file_ptr m_this;

    }; // class FileWrap

} // namespace node_osmium

#endif // FILE_WRAP_HPP
