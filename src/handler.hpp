
// v8
#include <v8.h>

// node.js
#include <node_object_wrap.h>

// node-osmium
#include "osm_object_wrap.hpp"

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

        void dispatch_object(const input_iterator& it);

        void done() {
            if (!done_cb.IsEmpty()) {
                Local<Value> argv[0] = { };
                TryCatch trycatch;
                Handle<Value> v = done_cb->Call(Context::GetCurrent()->Global(), 0, argv);
                if (v.IsEmpty()) {
                    Handle<Value> exception = trycatch.Exception();
                    String::AsciiValue exception_str(exception);
                    printf("Exception: %s\n", *exception_str);
                    exit(1);
                }
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

} // namespace node_osmium

