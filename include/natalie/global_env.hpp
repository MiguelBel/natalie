#pragma once

#include <stdlib.h>

#include "natalie/forward.hpp"
#include <string>
#include <unordered_map>

namespace Natalie {

extern "C" {
#include "hashmap.h"
#include "onigmo.h"
}

struct GlobalEnv {
    GlobalEnv() {
        m_globals = static_cast<struct hashmap *>(malloc(sizeof(struct hashmap)));
        hashmap_init(m_globals, hashmap_hash_string, hashmap_compare_string, 100);
        hashmap_set_key_alloc_funcs(m_globals, hashmap_alloc_key_string, free);
    }

    ~GlobalEnv() {
        hashmap_destroy(m_globals);
    }

    struct hashmap *globals() {
        return m_globals;
    }

    Value *get_symbol(const char *name) {
        auto result = m_symbols.find(name);
        if (result == m_symbols.end()) {
            return nullptr;
        }
        return result->second;
    }

    void add_symbol(const char *name, Value *value) {
        m_symbols[name] = value;
    }

    ClassValue *Object() { return m_Object; }
    void set_Object(ClassValue *Object) { m_Object = Object; }

    NilValue *nil_obj() { return m_nil_obj; }
    void set_nil_obj(NilValue *nil_obj) { m_nil_obj = nil_obj; }

    TrueValue *true_obj() { return m_true_obj; }
    void set_true_obj(TrueValue *true_obj) { m_true_obj = true_obj; }

    FalseValue *false_obj() { return m_false_obj; }
    void set_false_obj(FalseValue *false_obj) { m_false_obj = false_obj; }

private:
    struct hashmap *m_globals { nullptr };
    std::unordered_map<std::string, Value *> m_symbols {};
    ClassValue *m_Object { nullptr };
    NilValue *m_nil_obj { nullptr };
    TrueValue *m_true_obj { nullptr };
    FalseValue *m_false_obj { nullptr };
};

}
