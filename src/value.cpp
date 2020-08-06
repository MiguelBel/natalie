#include "natalie.hpp"
#include "natalie/forward.hpp"

#include <string>
#include <unordered_map>

namespace Natalie {

struct Value::value_map {
    std::unordered_map<std::string, Value *> m_map {};
};

Value::Value(const Value &other)
    : m_klass { other.m_klass }
    , m_type { other.m_type }
    , m_singleton_class { other.m_singleton_class ? new ClassValue { *other.m_singleton_class } : nullptr }
    , m_owner { other.m_owner }
    , m_flags { other.m_flags } {
    init_ivars();
    m_ivars->m_map = other.m_ivars->m_map;
}

Value *Value::_new(Env *env, Value *klass_value, ssize_t argc, Value **args, Block *block) {
    ClassValue *klass = klass_value->as_class();
    Value *obj;
    switch (klass->object_type()) {
    case Value::Type::Array:
        obj = new ArrayValue { env, klass };
        break;

    case Value::Type::Class:
        obj = new ClassValue { env, klass };
        break;

    case Value::Type::Encoding:
        obj = new EncodingValue { env, klass };
        break;

    case Value::Type::Exception:
        obj = new ExceptionValue { env, klass };
        break;

    case Value::Type::Hash:
        obj = new HashValue { env, klass };
        break;

    case Value::Type::Io:
        obj = new IoValue { env, klass };
        break;

    case Value::Type::MatchData:
        obj = new MatchDataValue { env, klass };
        break;

    case Value::Type::Module:
        obj = new ModuleValue { env, klass };
        break;

    case Value::Type::Object:
        obj = new Value { klass };
        break;

    case Value::Type::Proc:
        obj = new ProcValue { env, klass };
        break;

    case Value::Type::Range:
        obj = new RangeValue { env, klass };
        break;

    case Value::Type::Regexp:
        obj = new RegexpValue { env, klass };
        break;

    case Value::Type::String:
        obj = new StringValue { env, klass };
        break;

    case Value::Type::VoidP:
        obj = new VoidPValue { env, klass };
        break;

    case Value::Type::Nil:
    case Value::Type::False:
    case Value::Type::True:
    case Value::Type::Integer:
    case Value::Type::Float:
    case Value::Type::Symbol:
        NAT_UNREACHABLE();
    }

    return obj->initialize(env, argc, args, block);
}

Value *Value::initialize(Env *env, ssize_t argc, Value **args, Block *block) {
    ModuleValue *matching_class_or_module;
    Method *method = m_klass->find_method("initialize", &matching_class_or_module);
    if (method) {
        m_klass->call_method(env, m_klass, "initialize", this, argc, args, block);
    }
    return this;
}

NilValue *Value::as_nil() {
    assert(is_nil());
    return static_cast<NilValue *>(this);
}

ArrayValue *Value::as_array() {
    assert(is_array());
    return static_cast<ArrayValue *>(this);
}

ModuleValue *Value::as_module() {
    assert(is_module());
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

FalseValue *Value::as_false() {
    assert(is_false());
    return static_cast<FalseValue *>(this);
}

FloatValue *Value::as_float() {
    assert(is_float());
    return static_cast<FloatValue *>(this);
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

FileValue *Value::as_file() {
    assert(is_io());
    return static_cast<FileValue *>(this);
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

TrueValue *Value::as_true() {
    assert(is_true());
    return static_cast<TrueValue *>(this);
}

VoidPValue *Value::as_void_p() {
    assert(is_void_p());
    return static_cast<VoidPValue *>(this);
}

KernelModule *Value::as_kernel_module_for_method_binding() {
    return static_cast<KernelModule *>(this);
}

EnvValue *Value::as_env_value_for_method_binding() {
    return static_cast<EnvValue *>(this);
}

const char *Value::identifier_str(Env *env, Conversion conversion) {
    if (is_symbol()) {
        return as_symbol()->c_str();
    } else if (is_string()) {
        return as_string()->c_str();
    } else if (conversion == Conversion::NullAllowed) {
        return nullptr;
    } else {
        NAT_RAISE(env, "TypeError", "%s is not a symbol nor a string", send(env, "inspect", 0, nullptr, nullptr));
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
        NAT_RAISE(env, "TypeError", "%s is not a symbol nor a string", send(env, "inspect", 0, nullptr, nullptr));
    }
}

ClassValue *Value::singleton_class(Env *env) {
    if (is_integer() || is_float() || is_symbol()) {
        NAT_RAISE(env, "TypeError", "can't define singleton");
    }
    if (!m_singleton_class) {
        m_singleton_class = m_klass->subclass(env);
    }
    return m_singleton_class;
}

Value *Value::const_get(Env *env, const char *name, bool strict) {
    return m_klass->const_get(env, name, strict);
}

Value *Value::const_get_or_panic(Env *env, const char *name, bool strict) {
    return m_klass->const_get_or_panic(env, name, strict);
}

Value *Value::const_get_or_null(Env *env, const char *name, bool strict, bool define) {
    return m_klass->const_get_or_null(env, name, strict, define);
}

Value *Value::const_set(Env *env, const char *name, Value *val) {
    return m_klass->const_set(env, name, val);
}

Value *Value::ivar_get(Env *env, const char *name) {
    assert(strlen(name) > 0);
    if (name[0] != '@') {
        NAT_RAISE(env, "NameError", "`%s' is not allowed as an instance variable name", name);
    }
    init_ivars();
    auto result = m_ivars->m_map.find(name);
    if (result != m_ivars->m_map.end()) {
        return result->second;
    } else {
        return env->nil_obj();
    }
}

Value *Value::ivar_set(Env *env, const char *name, Value *val) {
    assert(strlen(name) > 0);
    if (name[0] != '@') {
        NAT_RAISE(env, "NameError", "`%s' is not allowed as an instance variable name", name);
    }
    init_ivars();
    m_ivars->m_map[name] = val;
    return val;
}

Value *Value::ivars(Env *env) {
    ArrayValue *ary = new ArrayValue { env };
    if (m_type == Value::Type::Integer) {
        return ary;
    }
    if (m_ivars) {
        for (std::pair<std::string, Value *> name_and_value : m_ivars->m_map) {
            auto name = name_and_value.first;
            ary->push(SymbolValue::intern(env, strdup(name.c_str())));
        }
    }
    return ary;
}

void Value::init_ivars() {
    if (m_ivars) return;
    m_ivars = new value_map {};
}

Value *Value::cvar_get(Env *env, const char *name) {
    Value *val = cvar_get_or_null(env, name);
    if (val) {
        return val;
    } else {
        ModuleValue *module;
        if (is_module()) {
            module = as_module();
        } else {
            module = m_klass;
        }
        NAT_RAISE(env, "NameError", "uninitialized class variable %s in %s", name, module->class_name());
    }
}

Value *Value::cvar_get_or_null(Env *env, const char *name) {
    assert(strlen(name) > 1);
    if (name[0] != '@' || name[1] != '@') {
        NAT_RAISE(env, "NameError", "`%s' is not allowed as a class variable name", name);
    }

    return m_klass->cvar_get_or_null(env, name);
}

Value *Value::cvar_set(Env *env, const char *name, Value *val) {
    assert(strlen(name) > 1);
    if (name[0] != '@' || name[1] != '@') {
        NAT_RAISE(env, "NameError", "`%s' is not allowed as a class variable name", name);
    }

    return m_klass->cvar_set(env, name, val);
}

void Value::alias(Env *env, const char *new_name, const char *old_name) {
    if (is_integer() || is_symbol()) {
        NAT_RAISE(env, "TypeError", "no klass to make alias");
    }
    if (is_main_object()) {
        m_klass->alias(env, new_name, old_name);
    } else if (is_module()) {
        as_module()->alias(env, new_name, old_name);
    } else {
        singleton_class(env)->alias(env, new_name, old_name);
    }
}

void Value::define_singleton_method(Env *env, const char *name, Value *(*fn)(Env *, Value *, ssize_t, Value **, Block *block)) {
    ClassValue *klass = singleton_class(env);
    klass->define_method(env, name, fn);
}

void Value::define_singleton_method_with_block(Env *env, const char *name, Block *block) {
    ClassValue *klass = singleton_class(env);
    klass->define_method_with_block(env, name, block);
}

void Value::undefine_singleton_method(Env *env, const char *name) {
    define_singleton_method(env, name, nullptr);
}

void Value::define_method(Env *env, const char *name, Value *(*fn)(Env *, Value *, ssize_t, Value **, Block *block)) {
    if (!is_main_object()) {
        printf("tried to call define_method on something that has no methods\n");
        abort();
    }
    m_klass->define_method(env, name, fn);
}

void Value::define_method_with_block(Env *env, const char *name, Block *block) {
    if (!is_main_object()) {
        printf("tried to call define_method on something that has no methods\n");
        abort();
    }
    m_klass->define_method_with_block(env, name, block);
}

void Value::undefine_method(Env *env, const char *name) {
    if (!is_main_object()) {
        printf("tried to call define_method on something that has no methods\n");
        abort();
    }
    m_klass->undefine_method(env, name);
}

Value *Value::send(Env *env, const char *sym, ssize_t argc, Value **args, Block *block) {
    if (singleton_class()) {
        ModuleValue *matching_class_or_module;
        Method *method = singleton_class()->find_method(sym, &matching_class_or_module);
        if (method) {
            if (method->is_undefined()) {
                NAT_RAISE(env, "NoMethodError", "undefined method `%s' for %s:Class", sym, m_klass->class_name());
            }
            return singleton_class()->call_method(env, m_klass, sym, this, argc, args, block);
        }
    }
    return m_klass->call_method(env, m_klass, sym, this, argc, args, block);
}

Value *Value::dup(Env *env) {
    switch (m_type) {
    case Value::Type::Array:
        return new ArrayValue { *as_array() };
    case Value::Type::String:
        return new StringValue { *as_string() };
    case Value::Type::False:
    case Value::Type::Float:
    case Value::Type::Integer:
    case Value::Type::Nil:
    case Value::Type::Symbol:
    case Value::Type::True:
        return this;
    case Value::Type::Object:
        return new Value { *this };
    default:
        fprintf(stderr, "I don't know how to dup this kind of object yet %s (type = %d).\n", m_klass->class_name(), static_cast<int>(m_type));
        abort();
    }
}

bool Value::is_a(Env *env, Value *val) {
    if (!val->is_module()) return false;
    ModuleValue *module = val->as_module();
    if (this == module) {
        return true;
    } else {
        ArrayValue *ancestors = m_klass->ancestors(env);
        for (Value *m : *ancestors) {
            if (module == m->as_module()) {
                return true;
            }
        }
        return false;
    }
}

bool Value::respond_to(Env *env, const char *name) {
    ModuleValue *matching_class_or_module;
    if (singleton_class() && singleton_class()->find_method_without_undefined(name, &matching_class_or_module)) {
        return true;
    } else if (m_klass->find_method_without_undefined(name, &matching_class_or_module)) {
        return true;
    } else {
        return false;
    }
}

bool Value::respond_to(Env *env, Value *name_val) {
    const char *name = name_val->identifier_str(env, Value::Conversion::NullAllowed);
    return !!(name && respond_to(env, name));
}

const char *Value::defined(Env *env, const char *name) {
    Value *obj;
    if (is_constant_name(name)) {
        obj = const_get_or_null(env, name, false, false);
        if (obj) return "constant";
    } else if (is_global_name(name)) {
        obj = env->global_get(name);
        if (obj != env->nil_obj()) return "global-variable";
    } else if (respond_to(env, name)) {
        return "method";
    }
    return nullptr;
}

Value *Value::defined_obj(Env *env, const char *name) {
    const char *result = defined(env, name);
    if (result) {
        return new StringValue { env, result };
    } else {
        return env->nil_obj();
    }
}

ProcValue *Value::to_proc(Env *env) {
    if (respond_to(env, "to_proc")) {
        return send(env, "to_proc")->as_proc();
    } else {
        NAT_RAISE(env, "TypeError", "wrong argument type %s (expected Proc)", m_klass->class_name());
    }
}

Value *Value::instance_eval(Env *env, Value *string, Block *block) {
    if (string || !block) {
        NAT_RAISE(env, "ArgumentError", "Natalie only supports instance_eval with a block");
    }
    if (is_module()) {
        // I *think* this is right... instance_eval, when called on a class/module,
        // evals with self set to the singleton class
        block->set_self(singleton_class(env));
    } else {
        block->set_self(this);
    }
    NAT_RUN_BLOCK_AND_POSSIBLY_BREAK(env, block, 0, nullptr, nullptr);
    return env->nil_obj();
}

}
