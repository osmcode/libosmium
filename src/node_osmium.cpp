// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

// node
#include <node.h>

// osmium
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/visitor.hpp>

// node-osmium
#include "apply.hpp"
#include "buffer_wrap.hpp"
#include "file_wrap.hpp"
#include "handler.hpp"
#include "location_handler_wrap.hpp"
#include "osm_changeset_wrap.hpp"
#include "osm_node_wrap.hpp"
#include "osm_relation_wrap.hpp"
#include "osm_way_wrap.hpp"
#include "reader_wrap.hpp"

namespace node_osmium {

    v8::Persistent<v8::Object> module;
    osmium::geom::WKBFactory<> wkb_factory;
    osmium::geom::WKTFactory<> wkt_factory;

    v8::Persistent<v8::String> symbol_Node;
    v8::Persistent<v8::String> symbol_node;

    v8::Persistent<v8::String> symbol_Way;
    v8::Persistent<v8::String> symbol_way;

    v8::Persistent<v8::String> symbol_Relation;
    v8::Persistent<v8::String> symbol_relation;
    v8::Persistent<v8::String> symbol_type;
    v8::Persistent<v8::String> symbol_ref;
    v8::Persistent<v8::String> symbol_role;

    v8::Persistent<v8::String> symbol_Changeset;
    v8::Persistent<v8::String> symbol_changeset;

    extern "C" {
        static void start(v8::Handle<v8::Object> target) {
            v8::HandleScope scope;
            module = v8::Persistent<v8::Object>::New(target);

            node::SetMethod(target, "apply", node_osmium::apply);

            symbol_Node      = NODE_PSYMBOL("Node");
            symbol_node      = NODE_PSYMBOL("node");
            symbol_Way       = NODE_PSYMBOL("Way");
            symbol_way       = NODE_PSYMBOL("way");
            symbol_Relation  = NODE_PSYMBOL("Relation");
            symbol_relation  = NODE_PSYMBOL("relation");
            symbol_type      = NODE_PSYMBOL("type");
            symbol_ref       = NODE_PSYMBOL("ref");
            symbol_role      = NODE_PSYMBOL("role");
            symbol_Changeset = NODE_PSYMBOL("Changeset");
            symbol_changeset = NODE_PSYMBOL("changeset");

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
            node_osmium::BufferWrap::Initialize(target);
        }
    }

} // namespace node_osmium

NODE_MODULE(osmium, node_osmium::start)

