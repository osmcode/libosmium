
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

        void init();
        void before_nodes();
        void after_nodes();
        void before_ways();
        void after_ways();
        void before_relations();
        void after_relations();
        void before_changesets();
        void after_changesets();
        void done();

        bool node_callback_for_tagged_only;

        Persistent<Function> init_cb;

        Persistent<Function> before_nodes_cb;
        Persistent<Function> node_cb;
        Persistent<Function> after_nodes_cb;

        Persistent<Function> before_ways_cb;
        Persistent<Function> way_cb;
        Persistent<Function> after_ways_cb;

        Persistent<Function> before_relations_cb;
        Persistent<Function> relation_cb;
        Persistent<Function> after_relations_cb;

        Persistent<Function> before_changesets_cb;
        Persistent<Function> changeset_cb;
        Persistent<Function> after_changesets_cb;

        Persistent<Function> done_cb;

    private:

        ~JSHandler();

    };

} // namespace node_osmium

