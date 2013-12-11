
// v8
#include <v8.h>

// node.js
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

// osmium
#include <osmium/handler.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>

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

        void dispatch_object(const osmium::io::InputIterator<osmium::io::Reader, osmium::Object>& it);

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

} // namespace node_osmium

