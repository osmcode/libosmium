#ifndef READER_WRAP_HPP
#define READER_WRAP_HPP

// c++
#include <memory>

// v8
#include <v8.h>

// node.js
#include <node_object_wrap.h>

// osmium
#include <osmium/io/any_input.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>

using namespace v8;

namespace node_osmium {

    typedef std::shared_ptr<osmium::io::Reader> reader_ptr;

    class ReaderWrap : public node::ObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> header(const Arguments& args);
        static Handle<Value> apply(const Arguments& args);
        static Handle<Value> close(const Arguments& args);

        static osmium::io::Reader& wrapped(Local<Object> object) {
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
