// c++11
#include <string>

// v8
#include <v8.h>

// node.js
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

// osmium
#include <osmium/handler.hpp>
#include <osmium/osm.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

namespace node_osmium {

    using namespace v8;

    class JSHandler : public node::ObjectWrap {

    public:

        static Persistent<FunctionTemplate> constructor;
        static void Initialize(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> on(const Arguments& args);
        static Handle<Value> options(const Arguments& args);
        JSHandler();

        void _ref() {
            Ref();
        }

        void _unref() {
            Unref();
        }

        void doit(const osmium::io::InputIterator<osmium::io::Reader, osmium::Object>& it) {
            HandleScope scope;
            switch (it->type()) {
                case osmium::item_type::node:
                    if (!node_cb.IsEmpty() && (!node_callback_for_tagged_only || !it->tags().empty())) {
                        const int argc = 1;

                        Node* node = new Node(it);
                        Handle<Value> ext = External::New(node);
                        Local<Object> obj = Node::constructor->GetFunction()->NewInstance(1, &ext);
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

                        Way* way = new Way(it);
                        Handle<Value> ext = External::New(way);
                        Local<Object> obj = Way::constructor->GetFunction()->NewInstance(1, &ext);
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

                        Relation* relation = new Relation(it);
                        Handle<Value> ext = External::New(relation);
                        Local<Object> obj = Relation::constructor->GetFunction()->NewInstance(1, &ext);
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

        void done() {
            if (!done_cb.IsEmpty()) {
                Local<Value> argv[0] = { };
                done_cb->Call(Context::GetCurrent()->Global(), 0, argv);
            }
        }

        bool node_callback_for_tagged_only;
        Persistent<Function> node_cb;
        Persistent<Function> way_cb;
        Persistent<Function> relation_cb;
        Persistent<Function> done_cb;

    private:

        ~JSHandler();

    };

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
        JSHandler * handler = node::ObjectWrap::Unwrap<JSHandler>(args.This());
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

} // namespace node_osmium

