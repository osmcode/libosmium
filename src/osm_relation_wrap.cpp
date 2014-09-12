
#include <node.h>

#include <osm_relation_wrap.hpp>

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> OSMRelationWrap::constructor;
    v8::Persistent<v8::String> OSMRelationWrap::symbol_type;
    v8::Persistent<v8::String> OSMRelationWrap::symbol_ref;
    v8::Persistent<v8::String> OSMRelationWrap::symbol_role;

    void OSMRelationWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMRelationWrap::New));
        constructor->Inherit(OSMObjectWrap::constructor);
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Relation"));
        auto attributes = static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
        set_accessor(constructor, "members_count", get_members_count, attributes);
        node::SetPrototypeMethod(constructor, "members", members);
        target->Set(v8::String::NewSymbol("Relation"), constructor->GetFunction());

        symbol_type = NODE_PSYMBOL("type");
        symbol_ref  = NODE_PSYMBOL("ref");
        symbol_role = NODE_PSYMBOL("role");
    }

    v8::Handle<v8::Value> OSMRelationWrap::New(const v8::Arguments& args) {
        if (args.Length() == 1 && args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            static_cast<OSMRelationWrap*>(ext->Value())->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.Relation cannot be created in Javascript")));
        }
    }

    v8::Handle<v8::Value> OSMRelationWrap::get_members_count(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).members().size()));
    }

    v8::Handle<v8::Value> OSMRelationWrap::members(const v8::Arguments& args) {
        v8::HandleScope scope;

        const osmium::Relation& relation = wrapped(args.This());

        switch (args.Length()) {
            case 0:
                {
                    v8::Local<v8::Array> members = v8::Array::New();
                    int i = 0;
                    char typec[2] = " ";
                    for (const auto& member : relation.members()) {
                        v8::Local<v8::Object> jsmember = v8::Object::New();
                        typec[0] = osmium::item_type_to_char(member.type());
                        jsmember->Set(symbol_type, v8::String::New(typec));
                        jsmember->Set(symbol_ref, v8::Number::New(member.ref()));
                        jsmember->Set(symbol_role, v8::String::New(member.role()));
                        members->Set(i, jsmember);
                        ++i;
                    }
                    return scope.Close(members);
                }
            case 1:
                {
                    if (!args[0]->IsNumber()) {
                        return ThrowException(v8::Exception::TypeError(v8::String::New("call members() without parameters or the index of the member you want")));
                    }
                    int n = static_cast<int>(args[0]->ToNumber()->Value());
                    if (n >= 0 && n < static_cast<int>(relation.members().size())) {
                        auto it = relation.members().begin();
                        std::advance(it, n);
                        const osmium::RelationMember& member = *it;
                        v8::Local<v8::Object> jsmember = v8::Object::New();
                        char typec[2] = " ";
                        typec[0] = osmium::item_type_to_char(member.type());
                        jsmember->Set(symbol_type, v8::String::New(typec));
                        jsmember->Set(symbol_ref, v8::Number::New(member.ref()));
                        jsmember->Set(symbol_role, v8::String::New(member.role()));
                        return scope.Close(jsmember);
                    } else {
                        return ThrowException(v8::Exception::RangeError(v8::String::New("argument to members() out of range")));
                    }
                }
        }

        return ThrowException(v8::Exception::TypeError(v8::String::New("call members() without parameters or the index of the member you want")));
    }

} // namespace node_osmium

