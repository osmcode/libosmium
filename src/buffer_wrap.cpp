
// node
#include <node_buffer.h>

// node-osmium
#include "node_osmium.hpp"
#include "buffer_wrap.hpp"
#include "osm_node_wrap.hpp"
#include "osm_way_wrap.hpp"
#include "osm_relation_wrap.hpp"
#include "osm_changeset_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> BufferWrap::constructor;

    void BufferWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(BufferWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(symbol_Buffer);
        node::SetPrototypeMethod(constructor, "next", next);
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
                BufferWrap* bw = new BufferWrap(std::move(buffer));
                bw->Wrap(args.This());
                return args.This();
            }
        }
        return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.Buffer takes a single argument, a node::Buffer")));
    }

    v8::Handle<v8::Value> BufferWrap::next(const v8::Arguments& args) {
        BufferWrap* buffer_wrap = node::ObjectWrap::Unwrap<BufferWrap>(args.This());
        v8::HandleScope scope;
        if (buffer_wrap->m_iterator == buffer_wrap->m_this->end()) {
            return scope.Close(v8::Undefined());
        }
        osmium::OSMEntity& entity = *buffer_wrap->m_iterator;
        ++buffer_wrap->m_iterator;
        switch (entity.type()) {
            case osmium::item_type::node: {
                v8::Handle<v8::Value> ext = v8::External::New(new OSMNodeWrap(entity));
                v8::Local<v8::Object> obj = OSMNodeWrap::constructor->GetFunction()->NewInstance(1, &ext);
                return scope.Close(obj);
            }
            case osmium::item_type::way: {
                v8::Handle<v8::Value> ext = v8::External::New(new OSMWayWrap(entity));
                v8::Local<v8::Object> obj = OSMWayWrap::constructor->GetFunction()->NewInstance(1, &ext);
                return scope.Close(obj);
            }
            case osmium::item_type::relation: {
                v8::Handle<v8::Value> ext = v8::External::New(new OSMRelationWrap(entity));
                v8::Local<v8::Object> obj = OSMRelationWrap::constructor->GetFunction()->NewInstance(1, &ext);
                return scope.Close(obj);
            }
            case osmium::item_type::changeset: {
                v8::Handle<v8::Value> ext = v8::External::New(new OSMChangesetWrap(entity));
                v8::Local<v8::Object> obj = OSMChangesetWrap::constructor->GetFunction()->NewInstance(1, &ext);
                return scope.Close(obj);
            }
            default:
                break;
        }
        return scope.Close(v8::Undefined());
    }

} // namespace node_osmium

