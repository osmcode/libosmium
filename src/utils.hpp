#ifndef UTILS_HPP
#define UTILS_HPP

// c++
#include <utility>

// v8/node
#include "include_v8.hpp"
#include <node_object_wrap.h>

// osmium
namespace osmium {
    class Box;
}

namespace node_osmium {

    template<class T>
    auto unwrap(const v8::Local<v8::Object>& object) -> decltype(node::ObjectWrap::Unwrap<T>(object)->get()) {
        return node::ObjectWrap::Unwrap<T>(object)->get();
    }

    template<class T, class... Args>
    v8::Local<v8::Object> new_external(Args&&... args) {
        v8::HandleScope scope;
        v8::Handle<v8::Value> ext = v8::External::New(new T(std::forward<Args>(args)...));
        return scope.Close(T::constructor->GetFunction()->NewInstance(1, &ext));
    }

    v8::Handle<v8::Value> create_js_box(const osmium::Box& box);

} // namespace node_osmium

#endif // UTILS_HPP
