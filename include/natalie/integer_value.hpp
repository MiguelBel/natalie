#pragma once

#include <assert.h>

#include "natalie/class_value.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/macros.hpp"
#include "natalie/value.hpp"

namespace Natalie {

struct IntegerValue : Value {
    IntegerValue(Env *env, int64_t integer)
        : Value { Value::Type::Integer, env->Object()->const_fetch("Integer")->as_class() }
        , m_integer { integer } { }

    int64_t to_int64_t() {
        return m_integer;
    }

    bool is_zero() {
        return m_integer == 0;
    }

    bool is_odd() {
        return m_integer % 2 != 0;
    }

    bool is_even() {
        return m_integer % 2 == 0;
    }

    Value *to_s(Env *);
    Value *to_i();
    Value *add(Env *, Value *);
    Value *sub(Env *, Value *);
    Value *mul(Env *, Value *);
    Value *div(Env *, Value *);
    Value *mod(Env *, Value *);
    Value *pow(Env *, Value *);
    Value *cmp(Env *, Value *);
    Value *eqeqeq(Env *, Value *);
    Value *times(Env *, Block *);
    Value *bitwise_and(Env *, Value *);
    Value *bitwise_or(Env *, Value *);
    Value *succ(Env *);
    Value *coerce(Env *, Value *);
    Value *abs(Env *);
    Value *chr(Env *);

    bool eq(Env *, Value *);
    bool eql(Env *, Value *);
    bool lt(Env *, Value *);
    bool lte(Env *, Value *);
    bool gt(Env *, Value *);
    bool gte(Env *, Value *);

private:
    int64_t m_integer { 0 };
};

}
