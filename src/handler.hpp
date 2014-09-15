#ifndef HANDLER_HPP
#define HANDLER_HPP

// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

// node
#include <node_object_wrap.h>

// osmium
namespace osmium {
    class OSMEntity;
}

namespace node_osmium {

    class JSHandler : public node::ObjectWrap {

        static v8::Persistent<v8::String> symbol_tagged_nodes_only;

        bool node_callback_for_tagged_only;

        v8::Persistent<v8::Function> init_cb;

        v8::Persistent<v8::Function> before_nodes_cb;
        v8::Persistent<v8::Function> node_cb;
        v8::Persistent<v8::Function> after_nodes_cb;

        v8::Persistent<v8::Function> before_ways_cb;
        v8::Persistent<v8::Function> way_cb;
        v8::Persistent<v8::Function> after_ways_cb;

        v8::Persistent<v8::Function> before_relations_cb;
        v8::Persistent<v8::Function> relation_cb;
        v8::Persistent<v8::Function> after_relations_cb;

        v8::Persistent<v8::Function> before_changesets_cb;
        v8::Persistent<v8::Function> changeset_cb;
        v8::Persistent<v8::Function> after_changesets_cb;

        v8::Persistent<v8::Function> done_cb;

        static v8::Handle<v8::Value> on(const v8::Arguments& args);
        static v8::Handle<v8::Value> options(const v8::Arguments& args);

    public:

        static v8::Persistent<v8::FunctionTemplate> constructor;
        static void Initialize(v8::Handle<v8::Object> target);
        static v8::Handle<v8::Value> New(const v8::Arguments& args);

        JSHandler();

        JSHandler& get() {
            return *this;
        }

        void dispatch_entity(v8::TryCatch& trycatch, const osmium::OSMEntity& entity) const;

        void init(v8::TryCatch& trycatch) const;
        void before_nodes(v8::TryCatch& trycatch) const;
        void after_nodes(v8::TryCatch& trycatch) const;
        void before_ways(v8::TryCatch& trycatch) const;
        void after_ways(v8::TryCatch& trycatch) const;
        void before_relations(v8::TryCatch& trycatch) const;
        void after_relations(v8::TryCatch& trycatch) const;
        void before_changesets(v8::TryCatch& trycatch) const;
        void after_changesets(v8::TryCatch& trycatch) const;
        void done(v8::TryCatch& trycatch) const;

    private:

        ~JSHandler();

    }; // class JSHandler

} // namespace node_osmium

#endif // HANDLER_HPP
