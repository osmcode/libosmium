
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

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> JSHandler::constructor;

    void JSHandler::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(JSHandler::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Handler"));
        node::SetPrototypeMethod(constructor, "on", on);
        node::SetPrototypeMethod(constructor, "options", options);
        target->Set(v8::String::NewSymbol("Handler"), constructor->GetFunction());
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

    v8::Handle<v8::Value> JSHandler::New(const v8::Arguments& args) {
        v8::HandleScope scope;
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

    void JSHandler::print_error_message(v8::TryCatch& trycatch) {
        v8::HandleScope scope;
        v8::Handle<v8::Value> exception = trycatch.Exception();
        v8::String::AsciiValue exception_str(exception);
        v8::Handle<v8::Message> message = trycatch.Message();
        if (message.IsEmpty()) {
            std::cerr << *exception_str << std::endl;
        } else {
            v8::String::Utf8Value filename(message->GetScriptResourceName());
            std::cerr << *filename << ":" << message->GetLineNumber() << ": " << *exception_str << std::endl;

            v8::String::Utf8Value sourceline(message->GetSourceLine());
            std::cerr << *sourceline << std::endl;

            int start = message->GetStartColumn();
            int end = message->GetEndColumn();
            for (int i = 0; i < start; i++) {
                std::cerr << " ";
            }
            for (int i = start; i < end; i++) {
                std::cerr << "^";
            }
            std::cerr << std::endl;
        }
    }

    v8::Handle<v8::Value> JSHandler::options(const v8::Arguments& args) {
        v8::HandleScope scope;
        if (args.Length() == 1) {
            if (args[0]->IsObject()) {
                v8::Local<v8::Value> tagged_nodes_only = args[0]->ToObject()->Get(v8::String::New("tagged_nodes_only"));
                if (tagged_nodes_only->IsBoolean()) {
                    JSHandler* handler = node::ObjectWrap::Unwrap<JSHandler>(args.This());
                    handler->node_callback_for_tagged_only = tagged_nodes_only->BooleanValue();
                }
            }
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
        JSHandler* handler = node::ObjectWrap::Unwrap<JSHandler>(args.This());
        if (callback_name == "node") {
            if (!handler->node_cb.IsEmpty()) {
                handler->node_cb.Dispose();
            }
            handler->node_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "way") {
            if (!handler->way_cb.IsEmpty()) {
                handler->way_cb.Dispose();
            }
            handler->way_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "relation") {
            if (!handler->relation_cb.IsEmpty()) {
                handler->relation_cb.Dispose();
            }
            handler->relation_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "init") {
            if (!handler->init_cb.IsEmpty()) {
                handler->init_cb.Dispose();
            }
            handler->init_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "before_nodes") {
            if (!handler->before_nodes_cb.IsEmpty()) {
                handler->before_nodes_cb.Dispose();
            }
            handler->before_nodes_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "after_nodes") {
            if (!handler->after_nodes_cb.IsEmpty()) {
                handler->after_nodes_cb.Dispose();
            }
            handler->after_nodes_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "before_ways") {
            if (!handler->before_ways_cb.IsEmpty()) {
                handler->before_ways_cb.Dispose();
            }
            handler->before_ways_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "after_ways") {
            if (!handler->after_ways_cb.IsEmpty()) {
                handler->after_ways_cb.Dispose();
            }
            handler->after_ways_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "before_relations") {
            if (!handler->before_relations_cb.IsEmpty()) {
                handler->before_relations_cb.Dispose();
            }
            handler->before_relations_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "after_relations") {
            if (!handler->after_relations_cb.IsEmpty()) {
                handler->after_relations_cb.Dispose();
            }
            handler->after_relations_cb = v8::Persistent<v8::Function>::New(callback);
        } else if (callback_name == "done") {
            if (!handler->done_cb.IsEmpty()) {
                handler->done_cb.Dispose();
            }
            handler->done_cb = v8::Persistent<v8::Function>::New(callback);
        }
        return scope.Close(v8::Undefined());
    }

    void JSHandler::dispatch_object(const input_iterator& it) {
        v8::HandleScope scope;
        switch (it->type()) {
            case osmium::item_type::node:
                if (!node_cb.IsEmpty() && (!node_callback_for_tagged_only || !it->tags().empty())) {
                    const int argc = 1;
                    v8::Local<v8::Object> obj = OSMNodeWrap::create(it);
                    v8::Local<v8::Value> argv[argc] = { obj };

                    v8::TryCatch trycatch;
                    v8::Handle<v8::Value> v = node_cb->Call(v8::Context::GetCurrent()->Global(), argc, argv);
                    if (v.IsEmpty()) {
                        print_error_message(trycatch);
                        exit(1);
                    }
                }
                break;
            case osmium::item_type::way:
                if (!way_cb.IsEmpty()) {
                    const int argc = 1;
                    v8::Local<v8::Object> obj = OSMWayWrap::create(it);
                    v8::Local<v8::Value> argv[argc] = { obj };

                    v8::TryCatch trycatch;
                    v8::Handle<v8::Value> v = way_cb->Call(v8::Context::GetCurrent()->Global(), argc, argv);
                    if (v.IsEmpty()) {
                        print_error_message(trycatch);
                        exit(1);
                    }
                }
                break;
            case osmium::item_type::relation:
                if (!relation_cb.IsEmpty()) {
                    const int argc = 1;
                    v8::Local<v8::Object> obj = OSMRelationWrap::create(it);
                    v8::Local<v8::Value> argv[argc] = { obj };

                    v8::TryCatch trycatch;
                    v8::Handle<v8::Value> v = relation_cb->Call(v8::Context::GetCurrent()->Global(), argc, argv);
                    if (v.IsEmpty()) {
                        print_error_message(trycatch);
                        exit(1);
                    }
                }
                break;
            default:
                break;
        }
    }

    void JSHandler::init() {
        if (!init_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = init_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::before_nodes() {
        if (!before_nodes_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = before_nodes_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::after_nodes() {
        if (!after_nodes_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = after_nodes_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::before_ways() {
        if (!before_ways_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = before_ways_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::after_ways() {
        if (!after_ways_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = after_ways_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::before_relations() {
        if (!before_relations_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = before_relations_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::after_relations() {
        if (!after_relations_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = after_relations_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::before_changesets() {
        if (!before_changesets_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = before_changesets_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::after_changesets() {
        if (!after_changesets_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = after_changesets_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

    void JSHandler::done() {
        if (!done_cb.IsEmpty()) {
            v8::HandleScope scope;
            v8::Local<v8::Value> argv[0] = { };
            v8::TryCatch trycatch;
            v8::Handle<v8::Value> v = done_cb->Call(v8::Context::GetCurrent()->Global(), 0, argv);
            if (v.IsEmpty()) {
                print_error_message(trycatch);
                exit(1);
            }
        }
    }

} // namespace node_osmium

