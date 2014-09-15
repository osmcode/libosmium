#ifndef READER_WRAP_HPP
#define READER_WRAP_HPP

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
#include <osmium/io/any_input.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>

namespace node_osmium {

    class ReaderWrap : public node::ObjectWrap {

        static v8::Handle<v8::Value> header(const v8::Arguments& args);
        static v8::Handle<v8::Value> close(const v8::Arguments& args);

        std::shared_ptr<osmium::io::Reader> m_this;

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        ReaderWrap(const osmium::io::File& file, osmium::osm_entity_bits::type entities) :
            ObjectWrap(),
            m_this(std::make_shared<osmium::io::Reader>(file, entities)) {
        }

        osmium::io::Reader& get() {
            return *m_this;
        }

    private:

        ~ReaderWrap() {
        }

    }; // class ReaderWrap

} // namespace node_osmium

#endif // READER_WRAP_HPP
