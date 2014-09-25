#ifndef BUFFER_WRAP_HPP
#define BUFFER_WRAP_HPP

// c++
#include <memory>

// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/memory/buffer.hpp>

namespace node_osmium {

    class BufferWrap : public node::ObjectWrap {

        std::shared_ptr<osmium::memory::Buffer> m_this;
        osmium::memory::Buffer::iterator m_iterator;

        static v8::Handle<v8::Value> next(const v8::Arguments& args);

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        BufferWrap(osmium::memory::Buffer&& buffer) :
            ObjectWrap(),
            m_this(std::make_shared<osmium::memory::Buffer>(std::move(buffer))),
            m_iterator(m_this->begin()) {
        }

        osmium::memory::Buffer& get() {
            return *m_this;
        }

    private:

        ~BufferWrap() {
        }

    }; // class BufferWrap

} // namespace node_osmium

#endif //  BUFFER_WRAP_HPP
