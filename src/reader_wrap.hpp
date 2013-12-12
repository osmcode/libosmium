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
#include <osmium/osm/entity_flags.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/stl_map.hpp>
#include <osmium/index/map/sparse_table.hpp>

typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
typedef osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;
typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

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

        ReaderWrap(const osmium::io::File& file, osmium::osm_entity::flags entities) :
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
    };

} // namespace node_osmium

#endif // READER_WRAP_HPP
