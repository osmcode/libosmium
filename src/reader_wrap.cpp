
// c++
#include <exception>
#include <string>
#include <vector>

// boost
#include <boost/variant.hpp>

// node
#include <node.h>

// osmium
#include <osmium/io/input_iterator.hpp>
#include <osmium/visitor.hpp>

// node-osmium
#include "reader_wrap.hpp"
#include "file_wrap.hpp"
#include "handler.hpp"
#include "location_handler_wrap.hpp"
#include "osm_object_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    extern v8::Persistent<v8::Object> module;

    v8::Persistent<v8::FunctionTemplate> ReaderWrap::constructor;

    void ReaderWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(ReaderWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Reader"));
        node::SetPrototypeMethod(constructor, "header", header);
        node::SetPrototypeMethod(constructor, "apply", apply);
        node::SetPrototypeMethod(constructor, "close", close);
        target->Set(v8::String::NewSymbol("Reader"), constructor->GetFunction());
    }

    v8::Handle<v8::Value> ReaderWrap::New(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (!args.IsConstructCall()) {
            return ThrowException(v8::Exception::Error(v8::String::New("Cannot call constructor as function, you need to use 'new' keyword")));
        }
        if (args.Length() < 1 || args.Length() > 2) {
            return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a File object or string for the first argument and optional options v8::Object when creating a Reader")));
        }
        try {
            osmium::osm_entity_bits::type read_which_entities = osmium::osm_entity_bits::all;
            if (args.Length() == 2) {
                if (!args[1]->IsObject()) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("Second argument to Reader constructor must be object")));
                }
                read_which_entities = osmium::osm_entity_bits::nothing;
                v8::Local<v8::Object> options = args[1]->ToObject();

                v8::Local<v8::Value> want_nodes = options->Get(v8::String::NewSymbol("node"));
                if (want_nodes->IsBoolean() && want_nodes->BooleanValue()) {
                    read_which_entities |= osmium::osm_entity_bits::node;
                }

                v8::Local<v8::Value> want_ways = options->Get(v8::String::NewSymbol("way"));
                if (want_ways->IsBoolean() && want_ways->BooleanValue()) {
                    read_which_entities |= osmium::osm_entity_bits::way;
                }

                v8::Local<v8::Value> want_relations = options->Get(v8::String::NewSymbol("relation"));
                if (want_relations->IsBoolean() && want_relations->BooleanValue()) {
                    read_which_entities |= osmium::osm_entity_bits::relation;
                }

                v8::Local<v8::Value> want_changesets = options->Get(v8::String::NewSymbol("changeset"));
                if (want_changesets->IsBoolean() && want_changesets->BooleanValue()) {
                    read_which_entities |= osmium::osm_entity_bits::changeset;
                }

            }
            if (args[0]->IsString()) {
                osmium::io::File file(*v8::String::Utf8Value(args[0]));
                ReaderWrap* q = new ReaderWrap(file, read_which_entities);
                q->Wrap(args.This());
                return args.This();
            } else if (args[0]->IsObject() && FileWrap::constructor->HasInstance(args[0]->ToObject())) {
                v8::Local<v8::Object> file_obj = args[0]->ToObject();
                ReaderWrap* q = new ReaderWrap(unwrap<FileWrap>(file_obj), read_which_entities);
                q->Wrap(args.This());
                return args.This();
            } else {
                return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a File object or string for the first argument when creating a Reader")));
            }
        } catch (const std::exception& ex) {
            return ThrowException(v8::Exception::TypeError(v8::String::New(ex.what())));
        }
        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> ReaderWrap::header(const v8::Arguments& args) {
        v8::HandleScope scope;
        v8::Local<v8::Object> obj = v8::Object::New();
        const osmium::io::Header& header = unwrap<ReaderWrap>(args.This()).header();
        obj->Set(v8::String::NewSymbol("generator"), v8::String::New(header.get("generator").c_str()));

        auto bounds_array = v8::Array::New(header.boxes().size());

        int i=0;
        for (const osmium::Box& box : header.boxes()) {
            bounds_array->Set(i++, create_js_box(box));
        }

        obj->Set(v8::String::NewSymbol("bounds"), bounds_array);
        return scope.Close(obj);
    }

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

    v8::Handle<v8::Value> ReaderWrap::apply(const v8::Arguments& args) {
        v8::HandleScope scope;

        typedef boost::variant<location_handler_type&, JSHandler&> some_handler_type;
        std::vector<some_handler_type> handlers;

        for (int i=0; i != args.Length(); ++i) {
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

        osmium::io::Reader& reader = unwrap<ReaderWrap>(args.This());
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

    v8::Handle<v8::Value> ReaderWrap::close(const v8::Arguments& args) {
        v8::HandleScope scope;
        try {
            unwrap<ReaderWrap>(args.This()).close();
        } catch (const std::exception& e) {
            std::string msg("osmium error: ");
            msg += e.what();
            return ThrowException(v8::Exception::Error(v8::String::New(msg.c_str())));
        }
        return scope.Close(v8::Undefined());
    }

} // namespace node_osmium

