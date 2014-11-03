#ifndef FILE_WRAP_HPP
#define FILE_WRAP_HPP

// c++
#include <memory>
#include <string>

// v8/node
#include "include_v8.hpp"
#include <node_object_wrap.h>

// osmium
#include <osmium/io/file.hpp>

namespace node_osmium {

    class FileWrap : public node::ObjectWrap {

        osmium::io::File m_this;

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        FileWrap(osmium::io::File&& file) :
            node::ObjectWrap(),
            m_this(std::move(file)) {
        }

        osmium::io::File& get() {
            return m_this;
        }

    private:

        ~FileWrap() = default;

    }; // class FileWrap

} // namespace node_osmium

#endif // FILE_WRAP_HPP
