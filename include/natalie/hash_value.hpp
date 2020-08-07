#pragma once

#include <assert.h>

#include "natalie/block.hpp"
#include "natalie/class_value.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/macros.hpp"
#include "natalie/nil_value.hpp"
#include "natalie/value.hpp"

namespace Natalie {

struct HashValue : Value {
    struct Key {
        Key *prev { nullptr };
        Key *next { nullptr };
        Value *key { nullptr };
        Value *val { nullptr };
        size_t hash { 0 };
        bool removed { false };
    };

    struct Val {
        Key *key { nullptr };
        Value *val { nullptr };
    };

    HashValue(Env *env)
        : HashValue { env, env->Object()->const_get_or_panic(env, "Hash", true)->as_class() } { }

    HashValue(Env *, ClassValue *);

    ~HashValue() {
        destroy_key_list();
        delete m_default_block;
    }

    static Value *square_new(Env *, ssize_t argc, Value **args);

    ssize_t size();
    Value *size(Env *);

    Value *get(Env *, Value *);
    Value *get_default(Env *, Value *);
    void put(Env *, Value *, Value *);
    Value *remove(Env *, Value *);

    const Value *default_value() { return m_default_value; }
    void set_default_value(Value *val) { m_default_value = val; }

    const Block *default_block() { return m_default_block; }
    void set_default_block(Block *block) { m_default_block = block; }

    bool is_iterating() { return m_is_iterating; }
    void set_is_iterating(bool is_iterating) { m_is_iterating = is_iterating; }

    class iterator {
    public:
        iterator(Key *key, HashValue *hash)
            : m_key { key }
            , m_hash { hash } {
            if (m_key) m_hash->set_is_iterating(true);
        }

        iterator operator++() {
            if (m_key->next == nullptr || (!m_key->removed && m_key->next == m_hash->m_key_list)) {
                m_key = nullptr;
                m_hash->set_is_iterating(false);
            } else if (m_key->next->removed) {
                m_key = m_key->next;
                return operator++();
            } else {
                m_key = m_key->next;
            }
            return *this;
        }

        iterator operator++(int _) {
            iterator i = *this;
            operator++();
            return i;
        }

        Key &operator*() { return *m_key; }
        Key *operator->() { return m_key; }

        friend bool operator==(const iterator &i1, const iterator &i2) {
            return i1.m_key == i2.m_key;
        }

        friend bool operator!=(const iterator &i1, const iterator &i2) {
            return i1.m_key != i2.m_key;
        }

    private:
        Key *m_key;
        HashValue *m_hash;
    };

    iterator begin() {
        return iterator { m_key_list, this };
    }

    iterator end() {
        return iterator { nullptr, this };
    }

    Value *initialize(Env *, Value *, Block *);
    Value *inspect(Env *);
    Value *ref(Env *, Value *);
    Value *refeq(Env *, Value *, Value *);
    Value *delete_key(Env *, Value *);
    Value *eq(Env *, Value *);
    Value *each(Env *, Block *);
    Value *keys(Env *);
    Value *values(Env *);
    Value *sort(Env *);
    Value *has_key(Env *, Value *);

private:
    struct value_map;

    void key_list_remove_node(Key *);
    Key *key_list_append(Env *, Value *, Value *);

    void destroy_key_list() {
        if (!m_key_list) return;
        Key *first_key = m_key_list;
        Key *key = m_key_list;
        do {
            Key *next_key = key->next;
            delete key;
            key = next_key;
        } while (key != first_key);
    }

    Key *m_key_list { nullptr };
    value_map *m_hashmap { nullptr };
    bool m_is_iterating { false };
    Value *m_default_value { nullptr };
    Block *m_default_block { nullptr };
};
}
