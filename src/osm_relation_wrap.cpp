
#include <osm_relation_wrap.hpp>

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> OSMRelationWrap::constructor;

    void OSMRelationWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMRelationWrap::New));
        constructor->Inherit(OSMObjectWrap::constructor);
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("Relation"));
        node::SetPrototypeMethod(constructor, "members", members);
        target->Set(v8::String::NewSymbol("Relation"), constructor->GetFunction());
    }

    OSMRelationWrap::OSMRelationWrap(const input_iterator& it) :
        OSMObjectWrap(it) {
    }

    OSMRelationWrap::~OSMRelationWrap() {
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

    v8::Handle<v8::Value> OSMRelationWrap::members(const v8::Arguments& args) {
        v8::HandleScope scope;
        osmium::Relation& relation = wrapped(args.This());

        if (args.Length() == 0) {
            v8::Local<v8::Array> members = v8::Array::New();
            int i = 0;
            char typec[2] = " ";
            for (auto& member : relation.members()) {
                v8::Local<v8::Object> jsmember = v8::Object::New();
                typec[0] = osmium::item_type_to_char(member.type());
                jsmember->Set(v8::String::NewSymbol("type"), v8::String::New(typec));
                jsmember->Set(v8::String::NewSymbol("ref"), v8::Number::New(member.ref()));
                jsmember->Set(v8::String::NewSymbol("role"), v8::String::New(member.role()));
                members->Set(i, jsmember);
                ++i;
            }
            return scope.Close(members);
        } else if (args.Length() == 1) {
            if (args[0]->IsNumber()) {
                int n = static_cast<int>(args[0]->ToNumber()->Value());
                if (n > 0 && n < static_cast<int>(relation.members().size())) {
                    auto it = relation.members().begin();
                    std::advance(it, n);
                    osmium::RelationMember& member = *it;
                    v8::Local<v8::Object> jsmember = v8::Object::New();
                    char typec[2] = " ";
                    typec[0] = osmium::item_type_to_char(member.type());
                    jsmember->Set(v8::String::NewSymbol("type"), v8::String::New(typec));
                    jsmember->Set(v8::String::NewSymbol("ref"), v8::Number::New(member.ref()));
                    jsmember->Set(v8::String::NewSymbol("role"), v8::String::New(member.role()));
                    return scope.Close(jsmember);
                }
            }
        }

        return scope.Close(v8::Undefined());
    }

} // namespace node_osmium

