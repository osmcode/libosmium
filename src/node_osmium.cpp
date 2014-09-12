// v8
#include <v8.h>

// node
#include <node.h>

// osmium
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

// node-osmium
#include "osm_node_wrap.hpp"
#include "osm_way_wrap.hpp"
#include "osm_relation_wrap.hpp"
#include "osm_changeset_wrap.hpp"
#include "handler.hpp"
#include "location_handler_wrap.hpp"
#include "file_wrap.hpp"
#include "reader_wrap.hpp"

namespace node_osmium {

    v8::Persistent<v8::Object> module;
    osmium::geom::WKBFactory<> wkb_factory;
    osmium::geom::WKTFactory<> wkt_factory;

    extern "C" {
        static void start(v8::Handle<v8::Object> target) {
            v8::HandleScope scope;
            module = v8::Persistent<v8::Object>::New(target);

            node_osmium::OSMEntityWrap::Initialize(target);
            node_osmium::OSMObjectWrap::Initialize(target);
            node_osmium::OSMNodeWrap::Initialize(target);
            node_osmium::OSMWayWrap::Initialize(target);
            node_osmium::OSMRelationWrap::Initialize(target);
            node_osmium::OSMChangesetWrap::Initialize(target);
            node_osmium::LocationHandlerWrap::Initialize(target);
            node_osmium::JSHandler::Initialize(target);
            node_osmium::FileWrap::Initialize(target);
            node_osmium::ReaderWrap::Initialize(target);
        }
    }

} // namespace node_osmium

NODE_MODULE(osmium, node_osmium::start)

