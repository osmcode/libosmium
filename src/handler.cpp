
// c++
#include <string>

#include "handler.hpp"
#include "osm_node_wrap.hpp"
#include "osm_way_wrap.hpp"
#include "osm_relation_wrap.hpp"

namespace node_osmium {

    Persistent<FunctionTemplate> JSHandler::constructor;

    void JSHandler::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(JSHandler::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Handler"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "on", on);
        NODE_SET_PROTOTYPE_METHOD(constructor, "options", options);
        target->Set(String::NewSymbol("Handler"), constructor->GetFunction());
    }

    JSHandler::JSHandler() :
        ObjectWrap(),
        node_callback_for_tagged_only(false),
        done_cb() {
        done_cb.Clear();
    }

    JSHandler::~JSHandler() {
        if (!done_cb.IsEmpty()) {
            done_cb.Dispose();
        }
        if (!node_cb.IsEmpty()) {
            node_cb.Dispose();
        }
        if (!way_cb.IsEmpty()) {
            way_cb.Dispose();
        }
        if (!relation_cb.IsEmpty()) {
            relation_cb.Dispose();
        }
    }

    Handle<Value> JSHandler::New(const Arguments& args) {
        HandleScope scope;
        if (args[0]->IsExternal()) {
            Local<External> ext = Local<External>::Cast(args[0]);
            void* ptr = ext->Value();
            JSHandler* b =  static_cast<JSHandler*>(ptr);
            b->Wrap(args.This());
            return args.This();
        } else {
            JSHandler* h = new JSHandler();
            h->Wrap(args.This());
            return args.This();
        }
        return Undefined();
    }

    Handle<Value> JSHandler::options(const Arguments& args) {
        if (args.Length() == 1) {
            if (args[0]->IsObject()) {
                Local<Value> tagged_nodes_only = args[0]->ToObject()->Get(String::New("tagged_nodes_only"));
                if (tagged_nodes_only->IsBoolean()) {
                    JSHandler* handler = node::ObjectWrap::Unwrap<JSHandler>(args.This());
                    handler->node_callback_for_tagged_only = tagged_nodes_only->BooleanValue();
                }
            }
        }
        return Undefined();
    }

    Handle<Value> JSHandler::on(const Arguments& args) {
        HandleScope scope;
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
            return ThrowException(Exception::TypeError(String::New("please provide an event name and callback function")));
        }
        std::string callback_name = *String::Utf8Value(args[0]->ToString());
        Local<Function> callback = Local<Function>::Cast(args[1]);
        if (callback->IsNull() || callback->IsUndefined()) {
            return ThrowException(Exception::TypeError(String::New("please provide a valid callback function for second arg")));
        }
        JSHandler* handler = node::ObjectWrap::Unwrap<JSHandler>(args.This());
        if (callback_name == "node") {
            if (!handler->node_cb.IsEmpty()) {
                handler->node_cb.Dispose();
            }
            handler->node_cb = Persistent<Function>::New(callback);
        } else if (callback_name == "way") {
            if (!handler->way_cb.IsEmpty()) {
                handler->way_cb.Dispose();
            }
            handler->way_cb = Persistent<Function>::New(callback);
        } else if (callback_name == "relation") {
            if (!handler->relation_cb.IsEmpty()) {
                handler->relation_cb.Dispose();
            }
            handler->relation_cb = Persistent<Function>::New(callback);
        } else if (callback_name == "done") {
            if (!handler->done_cb.IsEmpty()) {
                handler->done_cb.Dispose();
            }
            handler->done_cb = Persistent<Function>::New(callback);
        }
        return scope.Close(Undefined());
    }

    void JSHandler::dispatch_object(const osmium::io::InputIterator<osmium::io::Reader, osmium::Object>& it) {
        HandleScope scope;
        switch (it->type()) {
            case osmium::item_type::node:
                if (!node_cb.IsEmpty() && (!node_callback_for_tagged_only || !it->tags().empty())) {
                    const int argc = 1;

                    Handle<Value> ext = External::New(new OSMNodeWrap(it));
                    Local<Object> obj = OSMNodeWrap::constructor->GetFunction()->NewInstance(1, &ext);
                    Local<Value> argv[argc] = { obj };

                    TryCatch trycatch;
                    Handle<Value> v = node_cb->Call(Context::GetCurrent()->Global(), argc, argv);
                    if (v.IsEmpty()) {
                        Handle<Value> exception = trycatch.Exception();
                        String::AsciiValue exception_str(exception);
                        printf("Exception: %s\n", *exception_str);
                        exit(1);
                    }
                }
                break;
            case osmium::item_type::way:
                if (!way_cb.IsEmpty()) {
                    const int argc = 1;

                    Handle<Value> ext = External::New(new OSMWayWrap(it));
                    Local<Object> obj = OSMWayWrap::constructor->GetFunction()->NewInstance(1, &ext);
                    Local<Value> argv[argc] = { obj };

                    TryCatch trycatch;
                    Handle<Value> v = way_cb->Call(Context::GetCurrent()->Global(), argc, argv);
                    if (v.IsEmpty()) {
                        Handle<Value> exception = trycatch.Exception();
                        String::AsciiValue exception_str(exception);
                        printf("Exception: %s\n", *exception_str);
                        exit(1);
                    }
                }
                break;
            case osmium::item_type::relation:
                if (!relation_cb.IsEmpty()) {
                    const int argc = 1;

                    Handle<Value> ext = External::New(new OSMRelationWrap(it));
                    Local<Object> obj = OSMRelationWrap::constructor->GetFunction()->NewInstance(1, &ext);
                    Local<Value> argv[argc] = { obj };

                    TryCatch trycatch;
                    Handle<Value> v = relation_cb->Call(Context::GetCurrent()->Global(), argc, argv);
                    if (v.IsEmpty()) {
                        Handle<Value> exception = trycatch.Exception();
                        String::AsciiValue exception_str(exception);
                        printf("Exception: %s\n", *exception_str);
                        exit(1);
                    }
                }
                break;
            default:
                break;
        }
    }

} // namespace node_osmium

