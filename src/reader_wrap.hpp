#ifndef READER_WRAP_HPP
#define READER_WRAP_HPP

// c++
#include <memory>

// v8
#include <v8.h>

// node
#include <node_object_wrap.h>

// osmium
#include <osmium/io/any_input.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>

namespace node_osmium {

    typedef std::shared_ptr<osmium::io::Reader> reader_ptr;

    class ReaderWrap : public node::ObjectWrap {

        static v8::Handle<v8::Value> header(const v8::Arguments& args);
        static v8::Handle<v8::Value> apply(const v8::Arguments& args);
        static v8::Handle<v8::Value> close(const v8::Arguments& args);

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        static osmium::io::Reader& wrapped(v8::Local<v8::Object> object) {
            return *(node::ObjectWrap::Unwrap<ReaderWrap>(object)->get());
        }

        ReaderWrap(const osmium::io::File& file, osmium::osm_entity_bits::type entities) :
            ObjectWrap(),
            m_this(std::make_shared<osmium::io::Reader>(file, entities)) {
        }

        reader_ptr get() {
            return m_this;
        }

    private:

        ~ReaderWrap() {
        }

        reader_ptr m_this;

    }; // class ReaderWrap

} // namespace node_osmium

#endif // READER_WRAP_HPP
