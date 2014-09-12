#ifndef FILE_WRAP_HPP
#define FILE_WRAP_HPP

// c++
#include <memory>
#include <string>

// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

// node
#include <node_object_wrap.h>

// osmium
#include <osmium/io/file.hpp>

namespace node_osmium {

    class FileWrap : public node::ObjectWrap {

        std::shared_ptr<osmium::io::File> m_this;

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        FileWrap(osmium::io::File&& file) :
            node::ObjectWrap(),
            m_this(std::make_shared<osmium::io::File>(file)) {
        }

        osmium::io::File& get() {
            return *m_this;
        }

    private:

        ~FileWrap() {
        }

    }; // class FileWrap

} // namespace node_osmium

#endif // FILE_WRAP_HPP
