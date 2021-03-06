#pragma once

#include <assert.h>

#include "natalie/block.hpp"
#include "natalie/env.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/macros.hpp"
#include "natalie/module_value.hpp"
#include "natalie/value.hpp"

namespace Natalie {

struct ClassValue : ModuleValue {
    ClassValue(Env *env)
        : ClassValue { env, env->Object()->const_fetch("Class")->as_class() } { }

    ClassValue(Env *env, ClassValue *klass)
        : ModuleValue { env, Value::Type::Class, klass } { }

    ClassValue *subclass(Env *, const char * = nullptr);
    ClassValue *subclass(Env *, const char *, Type);

    static ClassValue *bootstrap_class_class(Env *);
    static ClassValue *bootstrap_basic_object(Env *, ClassValue *);

    Type object_type() { return m_object_type; }

    Value *initialize(Env *, Value *, Block *);

    static Value *new_method(Env *env, Value *superclass, Block *block) {
        if (superclass) {
            if (!superclass->is_class()) {
                NAT_RAISE(env, "TypeError", "superclass must be a Class (%s given)", superclass->klass()->class_name());
            }
        } else {
            superclass = env->Object();
        }
        Value *klass = superclass->as_class()->subclass(env);
        if (block) {
            block->set_self(klass);
            NAT_RUN_BLOCK_AND_POSSIBLY_BREAK(env, block, 0, nullptr, nullptr);
        }
        return klass;
    }

private:
    Type m_object_type { Type::Object };
};

}
