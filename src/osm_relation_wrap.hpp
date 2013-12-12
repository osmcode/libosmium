#ifndef RELATION_WRAP_HPP
#define RELATION_WRAP_HPP

// c++
#include <iterator>

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/osm/relation.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>

#include "osm_object_wrap.hpp"

using namespace v8;

namespace node_osmium {

    class OSMRelationWrap : public OSMObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> members(const Arguments& args);

        static osmium::Relation& wrapped(Local<Object> object) {
            return static_cast<osmium::Relation&>(OSMObjectWrap::wrapped(object));
        }

        OSMRelationWrap(const input_iterator&);

        osmium::Relation& object() {
            return static_cast<osmium::Relation&>(*get());
        }

    private:

        ~OSMRelationWrap();

    };

} // namespace node_osmium

#endif // RELATION_WRAP_HPP
