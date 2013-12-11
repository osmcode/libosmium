#ifndef READER_WRAP_HPP
#define READER_WRAP_HPP

// c++11
#include <exception>
#include <memory>

// v8
#include <v8.h>

// node.js
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/io/any_input.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/visitor.hpp>
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
        ReaderWrap(osmium::io::File& infile, osmium::osm_entity::flags entities);

        void _ref() {
            Ref();
        }

        void _unref() {
            Unref();
        }

        reader_ptr get() {
            return this_;
        }

    private:

        ~ReaderWrap();
        reader_ptr this_;
        osmium::io::Header header_;
    };

} // namespace node_osmium

#endif // READER_WRAP_HPP
