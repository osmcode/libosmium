#ifndef LOCATION_HANDLER_WRAP_HPP
#define LOCATION_HANDLER_WRAP_HPP

// c++
#include <memory>

// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

// node
#include <node_object_wrap.h>

// osmium
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/stl_map.hpp>
#include <osmium/index/map/sparse_table.hpp>
#include <osmium/index/map/mmap_vector_anon.hpp>
#include <osmium/index/map/mmap_vector_file.hpp>

namespace node_osmium {

    typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
    typedef osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;
    typedef osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> index_sparsetable_type;
    typedef osmium::index::map::StlMap<osmium::unsigned_object_id_type, osmium::Location> index_stlmap_type;
    typedef osmium::index::map::DenseMapFile<osmium::unsigned_object_id_type, osmium::Location> index_disk_type;

#ifdef __linux__
    typedef osmium::index::map::DenseMapMmap<osmium::unsigned_object_id_type, osmium::Location> index_array_type;
#endif

    typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

    inline index_pos_type* node_cache_factory(const std::string& cache_options) {
        size_t comma = cache_options.find_first_of(',');
        std::string cache_type = comma == std::string::npos ? cache_options : cache_options.substr(0, comma);
        if (cache_type == "sparsetable") {
            return new index_sparsetable_type;
        } else if (cache_type == "stlmap") {
            return new index_stlmap_type;
#ifdef __linux__
        } else if (cache_type == "array") {
            return new index_array_type;
#endif
        } else if (cache_type == "disk") {
            if (comma != std::string::npos) {
                std::string filename = cache_options.substr(comma + 1);
                int fd = ::open(filename.c_str(), O_RDWR);
                if (fd == -1) {
                    throw std::runtime_error(std::string("can't open file '") + filename + "': " + strerror(errno));
                }
                return new index_disk_type(fd);
            } else {
                return new index_disk_type;
            }
        }
        return nullptr;
    }

    class LocationHandlerWrap : public node::ObjectWrap {

        static v8::Handle<v8::Value> ignoreErrors(const v8::Arguments& args);

        std::unique_ptr<index_pos_type> m_index_pos;
        std::unique_ptr<index_neg_type> m_index_neg;

        std::shared_ptr<location_handler_type> m_this;

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;

        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Handle<v8::Value> clear(const v8::Arguments& args);

        static location_handler_type& wrapped(v8::Local<v8::Object> object) {
            return node::ObjectWrap::Unwrap<LocationHandlerWrap>(object)->get();
        }

        LocationHandlerWrap(const std::string& cache_type) :
            ObjectWrap(),
            m_index_pos(node_cache_factory(cache_type)),
            m_index_neg(new index_neg_type),
            m_this(std::make_shared<location_handler_type>(*m_index_pos, *m_index_neg)) {
            if (!m_index_pos) {
                throw std::runtime_error("unknown node cache type");
            }
        }

        void _ref() {
            Ref();
        }

        void _unref() {
            Unref();
        }

        location_handler_type& get() {
            return *m_this;
        }

    private:

        ~LocationHandlerWrap() {
        }

    }; // class LocationHandlerWrap

} // namespace node_osmium

#endif //  LOCATION_HANDLER_WRAP_HPP
