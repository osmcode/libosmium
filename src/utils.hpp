#ifndef UTILS_HPP
#define UTILS_HPP

#include <osmium/osm/box.hpp>

namespace node_osmium {

    template<class T>
    auto unwrap(const v8::Local<v8::Object>& object) -> decltype(node::ObjectWrap::Unwrap<T>(object)->get()) {
        return node::ObjectWrap::Unwrap<T>(object)->get();
    }

    v8::Handle<v8::Value> create_js_box(const osmium::Box& box);

} // namespace node_osmium

#endif // UTILS_HPP
