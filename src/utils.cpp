
// osmium
#include <osmium/osm/box.hpp>

#include "node_osmium.hpp"
#include "utils.hpp"

namespace node_osmium {

    extern v8::Persistent<v8::Object> module;

    v8::Handle<v8::Value> create_js_box(const osmium::Box& box) {
        v8::HandleScope scope;

        if (!box.valid()) {
            return scope.Close(v8::Undefined());
        }

        auto cf = module->Get(symbol_Coordinates);
        assert(cf->IsFunction());
        auto bf = module->Get(symbol_Box);
        assert(bf->IsFunction());

        v8::Local<v8::Value> argv_bl[2] = { v8::Number::New(box.bottom_left().lon()), v8::Number::New(box.bottom_left().lat()) };
        auto bottom_left = v8::Local<v8::Function>::Cast(cf)->NewInstance(2, argv_bl);

        v8::Local<v8::Value> argv_tr[2] = { v8::Number::New(box.top_right().lon()), v8::Number::New(box.top_right().lat()) };
        auto top_right = v8::Local<v8::Function>::Cast(cf)->NewInstance(2, argv_tr);

        v8::Local<v8::Value> argv_box[2] = { bottom_left, top_right };
        return scope.Close(v8::Local<v8::Function>::Cast(bf)->NewInstance(2, argv_box));
    }

} // namespace node_osmium
