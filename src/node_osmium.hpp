#ifndef NODE_OSMIUM
#define NODE_OSMIUM

#include "include_v8.hpp"

namespace node_osmium {

    extern v8::Persistent<v8::String> symbol_OSMEntity;
    extern v8::Persistent<v8::String> symbol_OSMObject;

    extern v8::Persistent<v8::String> symbol_Node;
    extern v8::Persistent<v8::String> symbol_node;

    extern v8::Persistent<v8::String> symbol_Way;
    extern v8::Persistent<v8::String> symbol_way;

    extern v8::Persistent<v8::String> symbol_Relation;
    extern v8::Persistent<v8::String> symbol_relation;
    extern v8::Persistent<v8::String> symbol_type;
    extern v8::Persistent<v8::String> symbol_ref;
    extern v8::Persistent<v8::String> symbol_role;

    extern v8::Persistent<v8::String> symbol_Area;
    extern v8::Persistent<v8::String> symbol_area;

    extern v8::Persistent<v8::String> symbol_Changeset;
    extern v8::Persistent<v8::String> symbol_changeset;

    extern v8::Persistent<v8::String> symbol_Coordinates;
    extern v8::Persistent<v8::String> symbol_Box;

    extern v8::Persistent<v8::String> symbol_generator;
    extern v8::Persistent<v8::String> symbol_bounds;

    extern v8::Persistent<v8::String> symbol_Buffer;
    extern v8::Persistent<v8::String> symbol_File;
    extern v8::Persistent<v8::String> symbol_Handler;
    extern v8::Persistent<v8::String> symbol_LocationHandler;
    extern v8::Persistent<v8::String> symbol_MultipolygonCollector;
    extern v8::Persistent<v8::String> symbol_MultipolygonHandler;
    extern v8::Persistent<v8::String> symbol_Reader;

} // namespace node_osmium

#endif // NODE_OSMIUM
