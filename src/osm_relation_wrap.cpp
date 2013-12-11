
#include <osm_relation_wrap.hpp>

namespace node_osmium {

    Persistent<FunctionTemplate> OSMRelationWrap::constructor;

    void OSMRelationWrap::Initialize(Handle<Object> target) {
        HandleScope scope;
        constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(OSMRelationWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(String::NewSymbol("Relation"));
        NODE_SET_PROTOTYPE_METHOD(constructor, "tags", tags);
        NODE_SET_PROTOTYPE_METHOD(constructor, "members", members);
        target->Set(String::NewSymbol("Relation"), constructor->GetFunction());
    }

    OSMRelationWrap::OSMRelationWrap(const input_iterator& it) :
        OSMObjectWrap(it) {
    }

    OSMRelationWrap::~OSMRelationWrap() {
    }

    Handle<Value> OSMRelationWrap::New(const Arguments& args) {
        HandleScope scope;
        if (args[0]->IsExternal()) {
            Local<External> ext = Local<External>::Cast(args[0]);
            void* ptr = ext->Value();
            OSMRelationWrap* relation = static_cast<OSMRelationWrap*>(ptr);
            relation->Wrap(args.This());
            osmium::Relation& obj = static_cast<osmium::Relation&>(*(relation->get()));
            args.This()->Set(String::New("id"), Number::New(obj.id()));
            args.This()->Set(String::New("version"), Number::New(obj.version()));
            args.This()->Set(String::New("changeset"), Number::New(obj.changeset()));
            args.This()->Set(String::New("visible"), Boolean::New(obj.visible()));
            args.This()->Set(String::New("timestamp"), Number::New(obj.timestamp()));
            args.This()->Set(String::New("timestamp_iso"), String::New(obj.timestamp().to_iso().c_str(), obj.timestamp().to_iso().size()));
            args.This()->Set(String::New("uid"), Number::New(obj.uid()));
            args.This()->Set(String::New("user"), String::New(obj.user()));
            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("osmium.Relation cannot be created in Javascript")));
        }
        return Undefined();
    }

    Handle<Value> OSMRelationWrap::members(const Arguments& args) {
        HandleScope scope;
        osmium::Relation& relation = wrapped(args.This());

        if (args.Length() == 0) {
            Local<Array> members = Array::New();
            int i = 0;
            char typec[2] = " ";
            for (auto& member : relation.members()) {
                Local<Object> jsmember = Object::New();
                typec[0] = osmium::item_type_to_char(member.type());
                jsmember->Set(String::New("type"), String::New(typec));
                jsmember->Set(String::New("ref"), Number::New(member.ref()));
                jsmember->Set(String::New("role"), String::New(member.role()));
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
                    Local<Object> jsmember = Object::New();
                    char typec[2] = " ";
                    typec[0] = osmium::item_type_to_char(member.type());
                    jsmember->Set(String::New("type"), String::New(typec));
                    jsmember->Set(String::New("ref"), Number::New(member.ref()));
                    jsmember->Set(String::New("role"), String::New(member.role()));
                    return scope.Close(jsmember);
                }
            }
        }

        return Undefined();
    }

} // namespace node_osmium

