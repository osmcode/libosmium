#ifndef OSM_OBJECT_WRAP_HPP
#define OSM_OBJECT_WRAP_HPP

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

// osmium
#include <osmium/io/reader.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/osm/object.hpp>

using namespace v8;

namespace node_osmium {

    typedef osmium::io::InputIterator<osmium::io::Reader, osmium::Object> input_iterator;

    class OSMObjectWrap : public node::ObjectWrap {

    protected:

        input_iterator m_it;

    public:

        static Handle<Value> tags(const Arguments& args);

        OSMObjectWrap(const input_iterator& it) :
            m_it(it) {
        }

    }; // class OSMObjectWrap

} // namespace node_osmium

#endif // OSM_OBJECT_WRAP_HPP
