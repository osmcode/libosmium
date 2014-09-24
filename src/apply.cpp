
// boost
#include <boost/variant.hpp>

// node
#include <node.h>
#include <node_buffer.h>

// osmium
#include <osmium/io/input_iterator.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/visitor.hpp>

// node-osmium
#include "apply.hpp"
#include "handler.hpp"
#include "location_handler_wrap.hpp"
#include "reader_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    struct visitor_type : public boost::static_visitor<> {

        osmium::OSMEntity& m_entity;

        visitor_type(osmium::OSMEntity& entity) :
            m_entity(entity) {
        }

        void operator()(JSHandler& handler) const {
            handler.dispatch_entity(m_entity);
        }

        void operator()(location_handler_type& handler) const {
            osmium::apply_item(m_entity, handler);
        }

    }; // struct visitor_type

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

    }; // struct visitor_before_after

    typedef boost::variant<location_handler_type&, JSHandler&> some_handler_type;

    template <class TIter>
    v8::Handle<v8::Value> apply_iterator(TIter it, TIter end, std::vector<some_handler_type>& handlers) {
        struct javascript_error {};

        v8::HandleScope scope;
        v8::TryCatch trycatch;
        try {
            osmium::item_type last_type = osmium::item_type::undefined;

            for (; it != end; ++it) {
                visitor_before_after_type visitor_before_after(last_type, it->type());
                visitor_type visitor(*it);

                for (auto& handler : handlers) {
                    if (last_type != it->type()) {
                        boost::apply_visitor(visitor_before_after, handler);
                        if (trycatch.HasCaught()) {
                            throw javascript_error();
                        }
                    }
                    boost::apply_visitor(visitor, handler);
                    if (trycatch.HasCaught()) {
                        throw javascript_error();
                    }
                }

                if (last_type != it->type()) {
                    last_type = it->type();
                }
            }

            visitor_before_after_type visitor_before_after(last_type, osmium::item_type::undefined);
            for (auto& handler : handlers) {
                boost::apply_visitor(visitor_before_after, handler);
                if (trycatch.HasCaught()) {
                    throw javascript_error();
                }
            }
        } catch (const javascript_error& e) {
            trycatch.ReThrow();
        } catch (const std::exception& e) {
            std::string msg("osmium error: ");
            msg += e.what();
            return ThrowException(v8::Exception::Error(v8::String::New(msg.c_str())));
        }
        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> apply(const v8::Arguments& args) {
        v8::HandleScope scope;

        if (args.Length() > 0 && args[0]->IsObject()) {
            std::vector<some_handler_type> handlers;

            for (int i=1; i != args.Length(); ++i) {
                if (!args[i]->IsObject()) {
                    return ThrowException(v8::Exception::TypeError(v8::String::New("please provide handler objects as second and further parameters to apply()")));
                }
                auto obj = args[i]->ToObject();
                if (JSHandler::constructor->HasInstance(obj)) {
                    handlers.push_back(unwrap<JSHandler>(obj));
                } else if (LocationHandlerWrap::constructor->HasInstance(obj)) {
                    handlers.push_back(unwrap<LocationHandlerWrap>(obj));
                }
            }

            auto source = args[0]->ToObject();
            if (ReaderWrap::constructor->HasInstance(source)) {
                osmium::io::Reader& reader = unwrap<ReaderWrap>(source);

                if (reader.eof()) {
                    return ThrowException(v8::Exception::Error(v8::String::New("apply() called on a reader that has reached EOF")));
                }

                typedef osmium::io::InputIterator<osmium::io::Reader, osmium::OSMEntity> input_iterator;

                return scope.Close(apply_iterator(input_iterator{reader}, input_iterator{}, handlers));
            } else if (node::Buffer::HasInstance(source)) {
                osmium::memory::Buffer buffer(reinterpret_cast<unsigned char*>(node::Buffer::Data(source)), node::Buffer::Length(source));

                return scope.Close(apply_iterator(buffer.begin<osmium::OSMEntity>(), buffer.end<osmium::OSMEntity>(), handlers));
            }
        }

        return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a Reader or Buffer object as first parameter")));
    }

} // namespace node_osmium
