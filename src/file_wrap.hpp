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

    typedef std::shared_ptr<osmium::io::File> file_ptr;

    class FileWrap : public node::ObjectWrap {

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

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
