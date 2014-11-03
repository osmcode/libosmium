#ifndef MULTIPOLYGON_HANDLER_WRAP_HPP
#define MULTIPOLYGON_HANDLER_WRAP_HPP

// v8/node
#include "include_v8.hpp"
#include <node_object_wrap.h>

// osmium
#include <osmium/area/multipolygon_collector.hpp>
#include <osmium/area/assembler.hpp>

namespace node_osmium {

    class MultipolygonHandlerWrap : public node::ObjectWrap {

        typedef osmium::area::MultipolygonCollector<osmium::area::Assembler>::HandlerPass2 handler_type;
        handler_type m_handler;

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        MultipolygonHandlerWrap(handler_type& handler) :
            node::ObjectWrap(),
            m_handler(handler) {
        }

        handler_type& get() {
            return m_handler;
        }

    private:

        ~MultipolygonHandlerWrap() = default;

    }; // class MultipolygonHandlerWrap

} // namespace node_osmium

#endif // MULTIPOLYGON_HANDLER_WRAP_HPP
