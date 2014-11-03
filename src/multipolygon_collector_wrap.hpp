#ifndef MULTIPOLYGON_COLLECTOR_WRAP_HPP
#define MULTIPOLYGON_COLLECTOR_WRAP_HPP

// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

// node
#include <node_object_wrap.h>

// osmium
#include <osmium/area/multipolygon_collector.hpp>
#include <osmium/area/assembler.hpp>

namespace node_osmium {

    class MultipolygonCollectorWrap : public node::ObjectWrap {

        static v8::Handle<v8::Value> read_relations(const v8::Arguments& args);
        static v8::Handle<v8::Value> handler(const v8::Arguments& args);

        osmium::area::Assembler::config_type m_assembler_config;
        osmium::area::MultipolygonCollector<osmium::area::Assembler> m_collector;

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        MultipolygonCollectorWrap() :
            node::ObjectWrap(),
            m_assembler_config(),
            m_collector(m_assembler_config) {
        }

        osmium::area::MultipolygonCollector<osmium::area::Assembler>& get() {
            return m_collector;
        }

    private:

        ~MultipolygonCollectorWrap() {
        }

    }; // class MultipolygonCollectorWrap

} // namespace node_osmium

#endif // MULTIPOLYGON_COLLECTOR_WRAP_HPP
