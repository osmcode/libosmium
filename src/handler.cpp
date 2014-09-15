
// c++
#include <string>

// v8
#include <v8.h>

// node
#include <node.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/osm/object.hpp>

// node-osmium
#include "handler.hpp"
#include "osm_node_wrap.hpp"
#include "osm_way_wrap.hpp"
#include "osm_relation_wrap.hpp"
#include "osm_changeset_wrap.hpp"
#include "utils.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> JSHandler::constructor;
    v8::Persistent<v8::String> JSHandler::symbol_tagged_nodes_only;

    void JSHandler::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(JSHandler::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Handler"));
        node::SetPrototypeMethod(constructor, "on", on);
        node::SetPrototypeMethod(constructor, "options", options);
        target->Set(v8::String::NewSymbol("Handler"), constructor->GetFunction());

        symbol_tagged_nodes_only = NODE_PSYMBOL("tagged_nodes_only");
    }

    JSHandler::JSHandler() :
        ObjectWrap(),
        node_callback_for_tagged_only(false) {
    }

    JSHandler::~JSHandler() {
        init_cb.Dispose();

        before_nodes_cb.Dispose();
        node_cb.Dispose();
        after_nodes_cb.Dispose();

        before_ways_cb.Dispose();
        way_cb.Dispose();
        after_ways_cb.Dispose();

        before_relations_cb.Dispose();
        relation_cb.Dispose();
        after_relations_cb.Dispose();

        before_changesets_cb.Dispose();
        changeset_cb.Dispose();
        after_changesets_cb.Dispose();

        done_cb.Dispose();
    }

    v8::Handle<v8::Value> JSHandler::New(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (!args.IsConstructCall()) {
            return ThrowException(v8::Exception::Error(v8::String::New("Cannot call constructor as function, you need to use 'new' keyword")));
        }
        if (args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            void* ptr = ext->Value();
            JSHandler* b =  static_cast<JSHandler*>(ptr);
            b->Wrap(args.This());
            return args.This();
        } else {
            JSHandler* h = new JSHandler();
            h->Wrap(args.This());
            return args.This();
        }
        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> JSHandler::options(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (args.Length() != 1 || !args[0]->IsObject()) {
            return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a single object as parameter")));
        }

        v8::Local<v8::Value> tagged_nodes_only = args[0]->ToObject()->Get(symbol_tagged_nodes_only);
        if (tagged_nodes_only->IsBoolean()) {
            unwrap<JSHandler>(args.This()).node_callback_for_tagged_only = tagged_nodes_only->BooleanValue();
        }

        return scope.Close(v8::Undefined());
    }

    v8::Handle<v8::Value> JSHandler::on(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
            return ThrowException(v8::Exception::TypeError(v8::String::New("please provide an event name and callback function")));
        }
        std::string callback_name = *v8::String::Utf8Value(args[0]->ToString());
        v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[1]);
        if (callback->IsNull() || callback->IsUndefined()) {
            return ThrowException(v8::Exception::TypeError(v8::String::New("please provide a valid callback function for second arg")));
        }

        JSHandler& handler = unwrap<JSHandler>(args.This());
        if (callback_name == "node") {
            handler.node_cb.Dispose();
            handler.node_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "way") {
            handler.way_cb.Dispose();
            handler.way_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "relation") {
            handler.relation_cb.Dispose();
            handler.relation_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "changeset") {
            handler.changeset_cb.Dispose();
            handler.changeset_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "init") {
            handler.init_cb.Dispose();
            handler.init_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "before_nodes") {
            handler.before_nodes_cb.Dispose();
            handler.before_nodes_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "after_nodes") {
            handler.after_nodes_cb.Dispose();
            handler.after_nodes_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "before_ways") {
            handler.before_ways_cb.Dispose();
            handler.before_ways_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "after_ways") {
            handler.after_ways_cb.Dispose();
            handler.after_ways_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "before_relations") {
            handler.before_relations_cb.Dispose();
            handler.before_relations_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "after_relations") {
            handler.after_relations_cb.Dispose();
            handler.after_relations_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "before_changesets") {
            handler.before_changesets_cb.Dispose();
            handler.before_changesets_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "after_changesets") {
            handler.after_changesets_cb.Dispose();
            handler.after_changesets_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "done") {
            handler.done_cb.Dispose();
            handler.done_cb = v8::Persistent<v8::Function>::New(callback);
        } else {
            return ThrowException(v8::Exception::RangeError(v8::String::New("unknown callback name as first argument")));
        }

        return scope.Close(v8::Undefined());
    }

    template <class TWrapped>
    void call_callback_with_entity(v8::TryCatch& trycatch, const v8::Persistent<v8::Function>& function, const osmium::OSMEntity& entity) {
        if (function.IsEmpty()) {
            return;
        }

        v8::HandleScope scope;
        v8::Handle<v8::Value> ext = v8::External::New(new TWrapped(entity));
        v8::Local<v8::Object> obj = TWrapped::constructor->GetFunction()->NewInstance(1, &ext);
        v8::Local<v8::Value> argv[1] = { obj };
        function->Call(v8::Context::GetCurrent()->Global(), 1, argv);
    }

    void call_callback(v8::TryCatch& trycatch, const v8::Persistent<v8::Function>& function) {
        if (function.IsEmpty()) {
            return;
        }

        v8::HandleScope scope;
        v8::Local<v8::Value> argv[0] = { };
        function->Call(v8::Context::GetCurrent()->Global(), 0, argv);
    }

    void JSHandler::dispatch_entity(v8::TryCatch& trycatch, const osmium::OSMEntity& entity) const {
        switch (entity.type()) {
            case osmium::item_type::node:
                if (!node_cb.IsEmpty() && (!node_callback_for_tagged_only || !static_cast<const osmium::Node&>(entity).tags().empty())) {
                    call_callback_with_entity<OSMNodeWrap>(trycatch, node_cb, entity);
                }
                break;
            case osmium::item_type::way:
                if (!way_cb.IsEmpty()) {
                    call_callback_with_entity<OSMWayWrap>(trycatch, way_cb, entity);
                }
                break;
            case osmium::item_type::relation:
                if (!relation_cb.IsEmpty()) {
                    call_callback_with_entity<OSMRelationWrap>(trycatch, relation_cb, entity);
                }
                break;
            case osmium::item_type::changeset:
                if (!changeset_cb.IsEmpty()) {
                    call_callback_with_entity<OSMChangesetWrap>(trycatch, changeset_cb, entity);
                }
                break;
            default:
                break;
        }
    }

    void JSHandler::init(v8::TryCatch& trycatch) const {
        call_callback(trycatch, init_cb);
    }

    void JSHandler::before_nodes(v8::TryCatch& trycatch) const {
        call_callback(trycatch, before_nodes_cb);
    }

    void JSHandler::after_nodes(v8::TryCatch& trycatch) const {
        call_callback(trycatch, after_nodes_cb);
    }

    void JSHandler::before_ways(v8::TryCatch& trycatch) const {
        call_callback(trycatch, before_ways_cb);
    }

    void JSHandler::after_ways(v8::TryCatch& trycatch) const {
        call_callback(trycatch, after_ways_cb);
    }

    void JSHandler::before_relations(v8::TryCatch& trycatch) const {
        call_callback(trycatch, before_relations_cb);
    }

    void JSHandler::after_relations(v8::TryCatch& trycatch) const {
        call_callback(trycatch, after_relations_cb);
    }

    void JSHandler::before_changesets(v8::TryCatch& trycatch) const {
        call_callback(trycatch, before_changesets_cb);
    }

    void JSHandler::after_changesets(v8::TryCatch& trycatch) const {
        call_callback(trycatch, after_changesets_cb);
    }

    void JSHandler::done(v8::TryCatch& trycatch) const {
        call_callback(trycatch, done_cb);
    }

} // namespace node_osmium

