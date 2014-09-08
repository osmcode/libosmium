
// c++
#include <exception>
#include <string>
#include <vector>

// boost
#include <boost/variant.hpp>

// node.js
#include <node.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/visitor.hpp>

// node-osmium
#include "reader_wrap.hpp"
#include "file_wrap.hpp"
#include "handler.hpp"
#include "location_handler_wrap.hpp"
#include "osm_object_wrap.hpp"

namespace node_osmium {

    Persistent<FunctionTemplate> ReaderWrap::constructor;

    void ReaderWrap::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(ReaderWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Reader"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "header", header);
        NODE_SET_PROTOTYPE_METHOD(constructor, "apply", apply);
        NODE_SET_PROTOTYPE_METHOD(constructor, "close", close);
        target->Set(String::NewSymbol("Reader"), constructor->GetFunction());
    }

    Handle<Value> ReaderWrap::New(const Arguments& args) {
        HandleScope scope;
        if (!args.IsConstructCall()) {
            return ThrowException(Exception::Error(String::New("Cannot call constructor as function, you need to use 'new' keyword")));
        }
        if (args.Length() < 1 || args.Length() > 2) {
            return ThrowException(Exception::TypeError(String::New("please provide a File object or string for the first argument and optional options Object when creating a Reader")));
        }
        try {
            osmium::osm_entity_bits::type read_which_entities = osmium::osm_entity_bits::all;
            if (args.Length() == 2) {
                if (!args[1]->IsObject()) {
                    return ThrowException(Exception::TypeError(String::New("Second argument to Reader constructor must be object")));
                }
                read_which_entities = osmium::osm_entity_bits::nothing;
                Local<Object> options = args[1]->ToObject();

                Local<Value> want_nodes = options->Get(String::New("node"));
                if (want_nodes->IsBoolean() && want_nodes->BooleanValue()) {
                    read_which_entities |= osmium::osm_entity_bits::node;
                }

                Local<Value> want_ways = options->Get(String::New("way"));
                if (want_ways->IsBoolean() && want_ways->BooleanValue()) {
                    read_which_entities |= osmium::osm_entity_bits::way;
                }

                Local<Value> want_relations = options->Get(String::New("relation"));
                if (want_relations->IsBoolean() && want_relations->BooleanValue()) {
                    read_which_entities |= osmium::osm_entity_bits::relation;
                }

            }
            if (args[0]->IsString()) {
                osmium::io::File file(*String::Utf8Value(args[0]));
                ReaderWrap* q = new ReaderWrap(file, read_which_entities);
                q->Wrap(args.This());
                return args.This();
            } else if (args[0]->IsObject() && FileWrap::constructor->HasInstance(args[0]->ToObject())) {
                Local<Object> file_obj = args[0]->ToObject();
                FileWrap* file_wrap = node::ObjectWrap::Unwrap<FileWrap>(file_obj);
                ReaderWrap* q = new ReaderWrap(*(file_wrap->get()), read_which_entities);
                q->Wrap(args.This());
                return args.This();
            } else {
                return ThrowException(Exception::TypeError(String::New("please provide a File object or string for the first argument when creating a Reader")));
            }
        } catch (const std::exception& ex) {
            return ThrowException(Exception::TypeError(String::New(ex.what())));
        }
        return Undefined();
    }

    Handle<Value> ReaderWrap::header(const Arguments& args) {
        HandleScope scope;
        Local<Object> obj = Object::New();
        ReaderWrap* reader = node::ObjectWrap::Unwrap<ReaderWrap>(args.This());
        const osmium::io::Header& header = reader->m_this->header();
        obj->Set(String::New("generator"), String::New(header.get("generator").c_str()));
        const osmium::Box& bounds = header.box();
        Local<Array> arr = Array::New(4);
        arr->Set(0, Number::New(bounds.bottom_left().lon()));
        arr->Set(1, Number::New(bounds.bottom_left().lat()));
        arr->Set(2, Number::New(bounds.top_right().lon()));
        arr->Set(3, Number::New(bounds.top_right().lat()));
        obj->Set(String::New("bounds"), arr);
        return scope.Close(obj);
    }

    struct visitor_type : public boost::static_visitor<> {

        input_iterator& m_it;

        visitor_type(input_iterator& it) :
            m_it(it) {
        }

        void operator()(JSHandler& handler) const {
            handler.dispatch_object(m_it);
        }

        void operator()(location_handler_type& handler) const {
            osmium::apply_item(*m_it, handler);
        }

    }; // visitor_type

    struct visitor_before_after_type : public boost::static_visitor<> {

        osmium::item_type m_last;
        osmium::item_type m_current;

        visitor_before_after_type(osmium::item_type last, osmium::item_type current) :
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
                    visitor.init();
                    break;
                case osmium::item_type::node:
                    visitor.after_nodes();
                    break;
                case osmium::item_type::way:
                    visitor.after_ways();
                    break;
                case osmium::item_type::relation:
                    visitor.after_relations();
                    break;
                case osmium::item_type::changeset:
                    visitor.after_changesets();
                    break;
                default:
                    break;
            }
            switch (m_current) {
                case osmium::item_type::undefined:
                    visitor.done();
                    break;
                case osmium::item_type::node:
                    visitor.before_nodes();
                    break;
                case osmium::item_type::way:
                    visitor.before_ways();
                    break;
                case osmium::item_type::relation:
                    visitor.before_relations();
                    break;
                case osmium::item_type::changeset:
                    visitor.before_changesets();
                    break;
                default:
                    break;
            }
        }

    }; // visitor_before_after

    Handle<Value> ReaderWrap::apply(const Arguments& args) {
        HandleScope scope;

        try {
            typedef boost::variant<location_handler_type&, JSHandler&> some_handler_type;
            std::vector<some_handler_type> handlers;

            for (int i=0; i != args.Length(); ++i) {
                if (args[i]->IsObject()) {
                    Local<Object> obj = args[i]->ToObject();
                    if (JSHandler::constructor->HasInstance(obj)) {
                        handlers.push_back(*node::ObjectWrap::Unwrap<JSHandler>(obj));
                    } else if (LocationHandlerWrap::constructor->HasInstance(obj)) {
                        location_handler_type* lh = node::ObjectWrap::Unwrap<LocationHandlerWrap>(obj)->get().get();
                        handlers.push_back(*lh);
                    }
                } else {
                    return ThrowException(Exception::TypeError(String::New("please provide a handler object")));
                }
            }

            osmium::io::Reader& reader = wrapped(args.This());

            input_iterator it(reader);
            input_iterator end;

            osmium::item_type last_type = osmium::item_type::undefined;

            for (; it != end; ++it) {
                visitor_before_after_type visitor_before_after(last_type, it->type());
                visitor_type visitor(it);

                for (some_handler_type& handler : handlers) {
                    if (last_type != it->type()) {
                        boost::apply_visitor(visitor_before_after, handler);
                    }
                    boost::apply_visitor(visitor, handler);
                }

                if (last_type != it->type()) {
                    last_type = it->type();
                }
            }

            visitor_before_after_type visitor_before_after(last_type, osmium::item_type::undefined);
            for (auto handler : handlers) {
                boost::apply_visitor(visitor_before_after, handler);
            }
        } catch (std::exception const& ex) {
            std::string msg("during io: osmium says '");
            msg += ex.what();
            msg += "'";
            return ThrowException(Exception::Error(String::New(msg.c_str())));
        }
        return Undefined();
    }

    Handle<Value> ReaderWrap::close(const Arguments& args) {
        wrapped(args.This()).close();
        return Undefined();
    }

} // namespace node_osmium

