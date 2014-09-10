#ifndef OSM_RELATION_WRAP_HPP
#define OSM_RELATION_WRAP_HPP

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
#include <osmium/io/reader.hpp>

#include "osm_object_wrap.hpp"

namespace node_osmium {

    class OSMRelationWrap : public OSMObjectWrap {

        static v8::Persistent<v8::FunctionTemplate> constructor;

        static v8::Handle<v8::Value> members(const v8::Arguments& args);

    public:

        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Local<v8::Object> create(const osmium::OSMEntity& entity);

        static const osmium::Relation& wrapped(v8::Local<v8::Object> object) {
            return static_cast<const osmium::Relation&>(OSMObjectWrap::wrapped(object));
        }

        OSMRelationWrap(const osmium::OSMEntity& entity) :
            OSMObjectWrap(entity) {
        }

        const osmium::Relation& object() {
            return static_cast<const osmium::Relation&>(*get());
        }

    private:

        ~OSMRelationWrap() {
        }

    }; // class OSMRelationWrap

} // namespace node_osmium

#endif // OSM_RELATION_WRAP_HPP
