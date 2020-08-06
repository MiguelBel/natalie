#include "natalie.hpp"

namespace Natalie {

SymbolValue *SymbolValue::intern(Env *env, const char *name) {
    Value *symbol = env->global_env()->get_symbol(name);
    if (symbol) {
        return symbol->as_symbol();
    } else {
        symbol = new SymbolValue { env, name };
        env->global_env()->add_symbol(name, symbol);
        return symbol->as_symbol();
    }
}

StringValue *SymbolValue::inspect(Env *env) {
    StringValue *string = new StringValue { env, ":" };
    ssize_t len = strlen(m_name);
    bool quote = false;
    for (ssize_t i = 0; i < len; i++) {
        char c = m_name[i];
        if (!((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == '_')) {
            quote = true;
            break;
        }
    };
    if (quote) {
        StringValue *quoted = StringValue { env, m_name }.inspect(env);
        string->append_string(env, quoted);
    } else {
        string->append(env, m_name);
    }
    return string;
}

ProcValue *SymbolValue::to_proc(Env *env) {
    Env block_env = Env::new_detatched_block_env(env);
    block_env.var_set("name", 0, true, this);
    Block *proc_block = new Block { block_env, this, SymbolValue::to_proc_block_fn };
    return new ProcValue { env, proc_block };
}

Value *SymbolValue::to_proc_block_fn(Env *env, Value *self_value, ssize_t argc, Value **args, Block *block) {
    NAT_ASSERT_ARGC(1);
    SymbolValue *name_obj = env->outer()->var_get("name", 0)->as_symbol();
    assert(name_obj);
    const char *name = name_obj->c_str();
    return args[0]->send(env, name);
}

Value *SymbolValue::cmp(Env *env, Value *other_value) {
    if (!other_value->is_symbol()) return env->nil_obj();
    SymbolValue *other = other_value->as_symbol();
    int diff = strcmp(m_name, other->m_name);
    int result;
    if (diff < 0) {
        result = -1;
    } else if (diff > 0) {
        result = 1;
    } else {
        result = 0;
    }
    return new IntegerValue { env, result };
}

}
