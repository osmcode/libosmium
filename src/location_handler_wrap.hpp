#ifndef LOCATION_HANDLER_WRAP_HPP
#define LOCATION_HANDLER_WRAP_HPP

// c++
#include <memory>

// v8
#include <v8.h>

// node.js
#include <node_object_wrap.h>

// osmium
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/stl_map.hpp>
#include <osmium/index/map/sparse_table.hpp>

using namespace v8;

namespace node_osmium {

    typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
    typedef osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;
    typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

    typedef std::shared_ptr<location_handler_type> location_handler_ptr;

    class LocationHandlerWrap : public node::ObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;

        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> clear(const Arguments& args);
        static Handle<Value> ignoreErrors(const Arguments& args);

        static location_handler_type& wrapped(Local<Object> object) {
            return *(node::ObjectWrap::Unwrap<LocationHandlerWrap>(object)->get());
        }

        LocationHandlerWrap() :
            ObjectWrap(),
            m_index_pos(),
            m_index_neg(),
            m_this(std::make_shared<location_handler_type>(m_index_pos, m_index_neg)) {
        }

        void _ref() {
            Ref();
        }

        void _unref() {
            Unref();
        }

        location_handler_ptr get() {
            return m_this;
        }

    private:

        ~LocationHandlerWrap() {
        }

        index_pos_type m_index_pos;
        index_neg_type m_index_neg;
        location_handler_ptr m_this;

    }; // class LocationHandlerWrap

} // namespace node_osmium

#endif //  LOCATION_HANDLER_WRAP_HPP
