#ifndef APPLY_HPP
#define APPLY_HPP

// v8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

namespace node_osmium {

    v8::Handle<v8::Value> apply(const v8::Arguments& args);

} // namespace node_osmium

#endif // APPLY_HPP
