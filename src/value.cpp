#include "natalie.hpp"
#include "natalie/forward.hpp"

namespace Natalie {

NilValue *Value::as_nil() {
    assert(is_nil());
    return static_cast<NilValue *>(this);
}

ArrayValue *Value::as_array() {
    assert(is_array());
    return static_cast<ArrayValue *>(this);
}

ModuleValue *Value::as_module() {
    assert(is_module() || is_class());
    return static_cast<ModuleValue *>(this);
}

ClassValue *Value::as_class() {
    assert(is_class());
    return static_cast<ClassValue *>(this);
}

EncodingValue *Value::as_encoding() {
    assert(is_encoding());
    return static_cast<EncodingValue *>(this);
}

ExceptionValue *Value::as_exception() {
    assert(is_exception());
    return static_cast<ExceptionValue *>(this);
}

HashValue *Value::as_hash() {
    assert(is_hash());
    return static_cast<HashValue *>(this);
}

IntegerValue *Value::as_integer() {
    assert(is_integer());
    return static_cast<IntegerValue *>(this);
}

IoValue *Value::as_io() {
    assert(is_io());
    return static_cast<IoValue *>(this);
}

MatchDataValue *Value::as_match_data() {
    assert(is_match_data());
    return static_cast<MatchDataValue *>(this);
}

ProcValue *Value::as_proc() {
    assert(is_proc());
    return static_cast<ProcValue *>(this);
}

RangeValue *Value::as_range() {
    assert(is_range());
    return static_cast<RangeValue *>(this);
}

RegexpValue *Value::as_regexp() {
    assert(is_regexp());
    return static_cast<RegexpValue *>(this);
}

StringValue *Value::as_string() {
    assert(is_string());
    return static_cast<StringValue *>(this);
}

SymbolValue *Value::as_symbol() {
    assert(is_symbol());
    return static_cast<SymbolValue *>(this);
}

VoidPValue *Value::as_void_p() {
    assert(is_void_p());
    return static_cast<VoidPValue *>(this);
}

const char *Value::identifier_str(Env *env, Conversion conversion) {
    if (is_symbol()) {
        return as_symbol()->symbol;
    } else if (is_string()) {
        return as_string()->str;
    } else if (conversion == Conversion::NullAllowed) {
        return nullptr;
    } else {
        NAT_RAISE(env, "TypeError", "%s is not a symbol nor a string", send(env, this, "inspect", 0, nullptr, nullptr));
    }
}

SymbolValue *Value::to_symbol(Env *env, Conversion conversion) {
    if (is_symbol()) {
        return as_symbol();
    } else if (is_string()) {
        return as_string()->to_symbol(env);
    } else if (conversion == Conversion::NullAllowed) {
        return nullptr;
    } else {
        NAT_RAISE(env, "TypeError", "%s is not a symbol nor a string", send(env, this, "inspect", 0, nullptr, nullptr));
    }
}

Value *const_get(Env *env, Value *parent, const char *name, bool strict) {
    Value *val = const_get_or_null(env, parent, name, strict, false);
    if (val) {
        return val;
    } else if (strict) {
        NAT_RAISE(env, "NameError", "uninitialized constant %S::%s", send(env, parent, "inspect", 0, nullptr, nullptr), name);
    } else {
        NAT_RAISE(env, "NameError", "uninitialized constant %s", name);
    }
}

Value *const_get_or_null(Env *env, Value *parent, const char *name, bool strict, bool define) {
    if (!parent->is_module()) {
        parent = parent->klass;
        assert(parent);
    }

    ModuleValue *search_parent;
    Value *val;

    if (!strict) {
        // first search in parent namespaces (not including global, i.e. Object namespace)
        search_parent = parent->as_module();
        while (!(val = static_cast<Value *>(hashmap_get(&search_parent->constants, name))) && search_parent->owner && search_parent->owner != NAT_OBJECT) {
            search_parent = search_parent->owner;
        }
        if (val) return val;
    }

    if (define) {
        // don't search superclasses
        val = static_cast<Value *>(hashmap_get(&parent->constants, name));
        if (val) return val;
    } else {
        // search in superclass hierarchy
        search_parent = parent->as_module();
        while (!(val = static_cast<Value *>(hashmap_get(&search_parent->constants, name))) && search_parent->superclass) {
            search_parent = search_parent->superclass;
        }
        if (val) return val;
    }

    if (!strict) {
        // lastly, search on the global, i.e. Object namespace
        val = static_cast<Value *>(hashmap_get(&NAT_OBJECT->constants, name));
        if (val) return val;
    }

    return nullptr;
}

Value *const_set(Env *env, Value *parent, const char *name, Value *val) {
    if (!NAT_IS_MODULE_OR_CLASS(parent)) {
        parent = NAT_OBJ_CLASS(parent);
        assert(parent);
    }
    hashmap_remove(&parent->constants, name);
    hashmap_put(&parent->constants, name, val);
    if (val->is_module() && !val->owner) {
        val->owner = parent->as_module();
    }
    return val;
}

Value *Value::ivar_get(Env *env, const char *name) {
    assert(strlen(name) > 0);
    if (name[0] != '@') {
        NAT_RAISE(env, "NameError", "`%s' is not allowed as an instance variable name", name);
    }
    init_ivars();
    Value *val = static_cast<Value *>(hashmap_get(&this->ivars, name));
    if (val) {
        return val;
    } else {
        return NAT_NIL;
    }
}

Value *Value::ivar_set(Env *env, const char *name, Value *val) {
    assert(strlen(name) > 0);
    if (name[0] != '@') {
        NAT_RAISE(env, "NameError", "`%s' is not allowed as an instance variable name", name);
    }
    init_ivars();
    hashmap_remove(&this->ivars, name);
    hashmap_put(&this->ivars, name, val);
    return val;
}

void Value::init_ivars() {
    if (this->ivars.table) return;
    hashmap_init(&this->ivars, hashmap_hash_string, hashmap_compare_string, 100);
    hashmap_set_key_alloc_funcs(&this->ivars, hashmap_alloc_key_string, free);
}

Value *Value::cvar_get(Env *env, const char *name) {
    Value *val = this->cvar_get_or_null(env, name);
    if (val) {
        return val;
    } else {
        ModuleValue *module;
        if (this->is_module()) {
            module = this->as_module();
        } else {
            module = this->klass;
        }
        NAT_RAISE(env, "NameError", "uninitialized class variable %s in %s", name, module->class_name);
    }
}

Value *Value::cvar_get_or_null(Env *env, const char *name) {
    assert(strlen(name) > 1);
    if (name[0] != '@' || name[1] != '@') {
        NAT_RAISE(env, "NameError", "`%s' is not allowed as a class variable name", name);
    }

    ModuleValue *module;
    if (this->is_module()) {
        module = this->as_module();
    } else {
        module = this->klass;
    }

    Value *val = nullptr;
    while (1) {
        if (module->cvars.table) {
            val = static_cast<Value *>(hashmap_get(&module->cvars, name));
            if (val) {
                return val;
            }
        }
        if (!module->superclass) {
            return nullptr;
        }
        module = module->superclass;
    }
}

Value *Value::cvar_set(Env *env, const char *name, Value *val) {
    assert(strlen(name) > 1);
    if (name[0] != '@' || name[1] != '@') {
        NAT_RAISE(env, "NameError", "`%s' is not allowed as a class variable name", name);
    }

    ModuleValue *module;
    if (this->is_module()) {
        module = this->as_module();
    } else {
        module = this->klass;
    }

    ModuleValue *current = module;

    Value *exists = nullptr;
    while (1) {
        if (current->cvars.table) {
            exists = static_cast<Value *>(hashmap_get(&current->cvars, name));
            if (exists) {
                hashmap_remove(&current->cvars, name);
                hashmap_put(&current->cvars, name, val);
                return val;
            }
        }
        if (!current->superclass) {
            if (module->cvars.table == nullptr) {
                hashmap_init(&module->cvars, hashmap_hash_string, hashmap_compare_string, 10);
                hashmap_set_key_alloc_funcs(&module->cvars, hashmap_alloc_key_string, free);
            }
            hashmap_remove(&module->cvars, name);
            hashmap_put(&module->cvars, name, val);
            return val;
        }
        current = current->superclass;
    }
}

}