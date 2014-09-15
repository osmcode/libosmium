// v8
#include <v8.h>

// boost
#include <boost/variant.hpp>

// node
#include <node.h>

// osmium
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/visitor.hpp>

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

    struct visitor_type : public boost::static_visitor<> {

        v8::TryCatch& m_trycatch;
        osmium::OSMEntity& m_entity;

        visitor_type(v8::TryCatch& trycatch, osmium::OSMEntity& entity) :
            m_trycatch(trycatch),
            m_entity(entity) {
        }

        void operator()(JSHandler& handler) const {
            handler.dispatch_entity(m_trycatch, m_entity);
        }

        void operator()(location_handler_type& handler) const {
            osmium::apply_item(m_entity, handler);
        }

    }; // visitor_type

    struct visitor_before_after_type : public boost::static_visitor<> {

        v8::TryCatch& m_trycatch;
        osmium::item_type m_last;
        osmium::item_type m_current;

        visitor_before_after_type(v8::TryCatch& trycatch, osmium::item_type last, osmium::item_type current) :
            m_trycatch(trycatch),
            m_last(last),
            m_current(current) {
        }

        // The operator() is overloaded just for location_handler and
        // for JSHandler. Currently these are the only handlers allowed
        // anyways. But this needs to be fixed at some point to allow
        // any handler. Unfortunately a template function is not allowed
        // here.
        void operator()(location_handler_type& visitor) const {
        }

        void operator()(JSHandler& visitor) const {
            switch (m_last) {
                case osmium::item_type::undefined:
                    visitor.init(m_trycatch);
                    break;
                case osmium::item_type::node:
                    visitor.after_nodes(m_trycatch);
                    break;
                case osmium::item_type::way:
                    visitor.after_ways(m_trycatch);
                    break;
                case osmium::item_type::relation:
                    visitor.after_relations(m_trycatch);
                    break;
                case osmium::item_type::changeset:
                    visitor.after_changesets(m_trycatch);
                    break;
                default:
                    break;
            }
            switch (m_current) {
                case osmium::item_type::undefined:
                    visitor.done(m_trycatch);
                    break;
                case osmium::item_type::node:
                    visitor.before_nodes(m_trycatch);
                    break;
                case osmium::item_type::way:
                    visitor.before_ways(m_trycatch);
                    break;
                case osmium::item_type::relation:
                    visitor.before_relations(m_trycatch);
                    break;
                case osmium::item_type::changeset:
                    visitor.before_changesets(m_trycatch);
                    break;
                default:
                    break;
            }
        }

    }; // visitor_before_after

    v8::Handle<v8::Value> apply(const v8::Arguments& args) {
        v8::HandleScope scope;

        typedef boost::variant<location_handler_type&, JSHandler&> some_handler_type;
        std::vector<some_handler_type> handlers;

        for (int i=1; i != args.Length(); ++i) {
            if (args[i]->IsObject()) {
                auto obj = args[i]->ToObject();
                if (JSHandler::constructor->HasInstance(obj)) {
                    handlers.push_back(unwrap<JSHandler>(obj));
                } else if (LocationHandlerWrap::constructor->HasInstance(obj)) {
                    handlers.push_back(unwrap<LocationHandlerWrap>(obj));
                }
            } else {
                return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a handler object")));
            }
        }

        osmium::io::Reader& reader = unwrap<ReaderWrap>(args[0]->ToObject());
        if (reader.eof()) {
            return ThrowException(v8::Exception::Error(v8::String::New("apply() called on a reader that has reached EOF")));
        }

        try {
            typedef osmium::io::InputIterator<osmium::io::Reader, osmium::OSMEntity> input_iterator;
            input_iterator it(reader);
            input_iterator end;

            osmium::item_type last_type = osmium::item_type::undefined;

            v8::TryCatch trycatch;
            for (; it != end; ++it) {
                visitor_before_after_type visitor_before_after(trycatch, last_type, it->type());
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                }
                visitor_type visitor(trycatch, *it);

                for (auto& handler : handlers) {
                    if (last_type != it->type()) {
                        boost::apply_visitor(visitor_before_after, handler);
                        if (trycatch.HasCaught()) {
                            trycatch.ReThrow();
                        }
                    }
                    boost::apply_visitor(visitor, handler);
                }

                if (last_type != it->type()) {
                    last_type = it->type();
                }
            }

            visitor_before_after_type visitor_before_after(trycatch, last_type, osmium::item_type::undefined);
            for (auto& handler : handlers) {
                boost::apply_visitor(visitor_before_after, handler);
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                }
            }
        } catch (const std::exception& e) {
            std::string msg("osmium error: ");
            msg += e.what();
            return ThrowException(v8::Exception::Error(v8::String::New(msg.c_str())));
        }
        return scope.Close(v8::Undefined());
    }

    extern "C" {
        static void start(v8::Handle<v8::Object> target) {
            v8::HandleScope scope;
            module = v8::Persistent<v8::Object>::New(target);

            node::SetMethod(target, "apply", apply);

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

