
// node
#include <node_buffer.h>

// osmium
#include <osmium/diff_iterator.hpp>

// node-osmium
#include "node_osmium.hpp"
#include "buffer_wrap.hpp"
#include "osm_node_wrap.hpp"
#include "osm_way_wrap.hpp"
#include "osm_relation_wrap.hpp"
#include "osm_area_wrap.hpp"
#include "osm_changeset_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> BufferWrap::constructor;

    void BufferWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(BufferWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(symbol_Buffer);
        node::SetPrototypeMethod(constructor, "clear", clear);
        node::SetPrototypeMethod(constructor, "next", next);
        node::SetPrototypeMethod(constructor, "filter_point_in_time", filter_point_in_time);
        target->Set(symbol_Buffer, constructor->GetFunction());
    }

    v8::Handle<v8::Value> BufferWrap::New(const v8::Arguments& args) {
        if (args.Length() == 1 && args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            static_cast<BufferWrap*>(ext->Value())->Wrap(args.This());
            return args.This();
        } else if (args.Length() == 1 && args[0]->IsObject()) {
            auto obj = args[0]->ToObject();
            if (node::Buffer::HasInstance(obj)) {
                osmium::memory::Buffer buffer(reinterpret_cast<unsigned char*>(node::Buffer::Data(obj)), node::Buffer::Length(obj));
                BufferWrap* buffer_wrap = new BufferWrap(std::move(buffer));
                buffer_wrap->Wrap(args.This());
                return args.This();
            }
        }
        return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.Buffer takes a single argument, a node::Buffer")));
    }

    v8::Handle<v8::Value> BufferWrap::next(const v8::Arguments& args) {
        BufferWrap* buffer_wrap = node::ObjectWrap::Unwrap<BufferWrap>(args.This());
        v8::HandleScope scope;
        if (buffer_wrap->m_iterator == buffer_wrap->m_this.end()) {
            return scope.Close(v8::Undefined());
        }
        osmium::OSMEntity& entity = *buffer_wrap->m_iterator;
        ++buffer_wrap->m_iterator;
        switch (entity.type()) {
            case osmium::item_type::node: {
                return scope.Close(new_external<OSMNodeWrap>(entity));
            }
            case osmium::item_type::way: {
                return scope.Close(new_external<OSMWayWrap>(entity));
            }
            case osmium::item_type::relation: {
                return scope.Close(new_external<OSMRelationWrap>(entity));
            }
            case osmium::item_type::area: {
                return scope.Close(new_external<OSMAreaWrap>(entity));
            }
            case osmium::item_type::changeset: {
                return scope.Close(new_external<OSMChangesetWrap>(entity));
            }
            default:
                break;
        }
        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> BufferWrap::filter_point_in_time(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (args.Length() != 1) {
            return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a point in time as first and only argument")));
        }

        osmium::Timestamp point_in_time;
        if (args[0]->IsInt32()) {
            point_in_time = args[0]->Int32Value();
        } else if (args[0]->IsString()) {
            point_in_time = osmium::Timestamp(*v8::String::Utf8Value(args[0]->ToString()));
        } else if (args[0]->IsDate()) {
            point_in_time = osmium::Timestamp(static_cast<int32_t>(v8::Date::Cast(*args[0])->NumberValue() / 1000));
        }

        typedef osmium::DiffIterator<osmium::memory::Buffer::t_iterator<osmium::OSMObject>> diff_iterator;
        osmium::memory::Buffer& buffer = unwrap<BufferWrap>(args.This());
        osmium::memory::Buffer fbuffer(buffer.committed(), osmium::memory::Buffer::auto_grow::yes);
        {
            auto dbegin = diff_iterator(buffer.begin<osmium::OSMObject>(), buffer.end<osmium::OSMObject>());
            auto dend   = diff_iterator(buffer.end<osmium::OSMObject>(), buffer.end<osmium::OSMObject>());

            std::for_each(dbegin, dend, [point_in_time, &fbuffer](const osmium::DiffObject& d) {
                if (((d.end_time() == 0 || d.end_time() > point_in_time) &&
                        d.start_time() <= point_in_time) &&
                    d.curr().visible()) {
                    fbuffer.add_item(d.curr());
                    fbuffer.commit();
                }
            });
        }

        return scope.Close(new_external<BufferWrap>(std::move(fbuffer)));
    }

} // namespace node_osmium

