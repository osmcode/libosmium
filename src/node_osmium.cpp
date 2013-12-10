// v8
#include <v8.h>

// node.js
#include <node.h>

// osmium
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

// node-osmium
#include "node.hpp"
#include "way.hpp"
#include "relation.hpp"
#include "handler.hpp"
#include "file.hpp"
#include "reader.hpp"
#include "buffer.hpp"

namespace node_osmium {

    extern "C" {
        static void start(v8::Handle<v8::Object> target) {
            v8::HandleScope scope;
            node_osmium::Node::Initialize(target);
            node_osmium::Way::Initialize(target);
            node_osmium::Relation::Initialize(target);
            node_osmium::JSHandler::Initialize(target);
            node_osmium::Buffer::Initialize(target);
            node_osmium::File::Initialize(target);
            node_osmium::Reader::Initialize(target);
        }
    }

    osmium::geom::WKBFactory wkb_factory;
    osmium::geom::WKTFactory wkt_factory;

} // namespace node_osmium

NODE_MODULE(osmium, node_osmium::start)

