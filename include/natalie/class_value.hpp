#pragma once

#include <assert.h>

#include "natalie/env.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/macros.hpp"
#include "natalie/module_value.hpp"
#include "natalie/value.hpp"

namespace Natalie {

struct ClassValue : ModuleValue {
    using ModuleValue::ModuleValue;

    ClassValue(Env *);

    ClassValue *subclass(Env *, const char *);

    static ClassValue *bootstrap_class_class(Env *);
    static ClassValue *bootstrap_basic_object(Env *, ClassValue *);

private:
    ClassValue(Env *, ClassValue *);
};

}