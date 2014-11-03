#ifndef BUFFER_WRAP_HPP
#define BUFFER_WRAP_HPP

// c++
#include <memory>

// v8/node
#include "include_v8.hpp"
#include <node_object_wrap.h>

// osmium
#include <osmium/memory/buffer.hpp>

namespace node_osmium {

    class BufferWrap : public node::ObjectWrap {

        osmium::memory::Buffer m_this;
        osmium::memory::Buffer::iterator m_iterator;

        static v8::Handle<v8::Value> clear(const v8::Arguments& args) {
            BufferWrap* buffer_wrap = node::ObjectWrap::Unwrap<BufferWrap>(args.This());
            buffer_wrap->m_this = std::move(osmium::memory::Buffer());
        }

        static v8::Handle<v8::Value> next(const v8::Arguments& args);
        static v8::Handle<v8::Value> filter_point_in_time(const v8::Arguments& args);

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        BufferWrap(osmium::memory::Buffer&& buffer) :
            ObjectWrap(),
            m_this(std::move(buffer)),
            m_iterator(m_this.begin()) {
        }

        osmium::memory::Buffer& get() {
            return m_this;
        }

    private:

        ~BufferWrap() = default;

    }; // class BufferWrap

} // namespace node_osmium

#endif //  BUFFER_WRAP_HPP
