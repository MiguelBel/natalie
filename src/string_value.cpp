#include "ctype.h"
#include "natalie.hpp"
#include "string.h"

namespace Natalie {

void StringValue::grow(Env *env, ssize_t new_capacity) {
    assert(new_capacity >= m_length);
    m_str = static_cast<char *>(realloc(m_str, new_capacity + 1));
    m_capacity = new_capacity;
}

void StringValue::grow_at_least(Env *env, ssize_t min_capacity) {
    if (m_capacity >= min_capacity) return;
    if (m_capacity > 0 && min_capacity <= m_capacity * STRING_GROW_FACTOR) {
        grow(env, m_capacity * STRING_GROW_FACTOR);
    } else {
        grow(env, min_capacity);
    }
}

void StringValue::prepend_char(Env *env, char c) {
    ssize_t total_length = m_length + 1;
    grow_at_least(env, total_length);
    memmove(m_str + 1, m_str, m_length);
    m_str[0] = c;
    m_length = total_length;
}

void StringValue::insert(Env *env, ssize_t position, char c) {
    assert(position < m_length);
    grow_at_least(env, m_length + 1);
    m_length++;
    size_t nbytes = m_length - position + 1; // 1 extra for null terminator
    memmove(m_str + position + 1, m_str + position, nbytes);
    m_str[position] = c;
}

void StringValue::append(Env *env, const char *str) {
    if (str == nullptr) return;
    ssize_t new_length = strlen(str);
    if (new_length == 0) return;
    ssize_t total_length = m_length + new_length;
    grow_at_least(env, total_length);
    strcat(m_str, str);
    m_length = total_length;
}

void StringValue::append_char(Env *env, char c) {
    ssize_t total_length = m_length + 1;
    grow_at_least(env, total_length);
    m_str[total_length - 1] = c;
    m_str[total_length] = 0;
    m_length = total_length;
}

void StringValue::append_string(Env *env, Value *value) {
    append_string(env, value->as_string());
}

void StringValue::append_string(Env *env, StringValue *string2) {
    if (string2->length() == 0) return;
    ssize_t total_length = m_length + string2->length();
    grow_at_least(env, total_length);
    strncat(m_str, string2->c_str(), string2->length());
    m_length = total_length;
}

void StringValue::raise_encoding_invalid_byte_sequence_error(Env *env, ssize_t index) {
    StringValue *message = sprintf(env, "invalid byte sequence at index %i in string of size %i (string not long enough)", index, length());
    ClassValue *Encoding = env->Object()->const_find(env, "Encoding")->as_class();
    ClassValue *InvalidByteSequenceError = Encoding->const_find(env, "InvalidByteSequenceError")->as_class();
    ExceptionValue *exception = new ExceptionValue { env, InvalidByteSequenceError, message->c_str() };
    env->raise_exception(exception);
}

ArrayValue *StringValue::chars(Env *env) {
    ArrayValue *ary = new ArrayValue { env };
    StringValue *c;
    char buffer[5];
    switch (m_encoding) {
    case Encoding::UTF_8:
        for (ssize_t i = 0; i < m_length; i++) {
            buffer[0] = m_str[i];
            if (((unsigned char)buffer[0] >> 3) == 30) { // 11110xxx, 4 bytes
                if (i + 3 >= m_length) raise_encoding_invalid_byte_sequence_error(env, i);
                buffer[1] = m_str[++i];
                buffer[2] = m_str[++i];
                buffer[3] = m_str[++i];
                buffer[4] = 0;
            } else if (((unsigned char)buffer[0] >> 4) == 14) { // 1110xxxx, 3 bytes
                if (i + 2 >= m_length) raise_encoding_invalid_byte_sequence_error(env, i);
                buffer[1] = m_str[++i];
                buffer[2] = m_str[++i];
                buffer[3] = 0;
            } else if (((unsigned char)buffer[0] >> 5) == 6) { // 110xxxxx, 2 bytes
                if (i + 1 >= m_length) raise_encoding_invalid_byte_sequence_error(env, i);
                buffer[1] = m_str[++i];
                buffer[2] = 0;
            } else {
                buffer[1] = 0;
            }
            c = new StringValue { env, buffer };
            c->set_encoding(Encoding::UTF_8);
            ary->push(c);
        }
        break;
    case Encoding::ASCII_8BIT:
        for (ssize_t i = 0; i < m_length; i++) {
            buffer[0] = m_str[i];
            buffer[1] = 0;
            c = new StringValue { env, buffer };
            c->set_encoding(Encoding::ASCII_8BIT);
            ary->push(c);
        }
        break;
    }
    return ary;
}

SymbolValue *StringValue::to_symbol(Env *env) {
    return SymbolValue::intern(env, m_str);
}

Value *StringValue::to_sym(Env *env) {
    return to_symbol(env);
}

StringValue *StringValue::inspect(Env *env) {
    StringValue *out = new StringValue { env, "\"" };
    for (ssize_t i = 0; i < m_length; i++) {
        char c = m_str[i];
        if (c == '"' || c == '\\' || c == '#') {
            out->append_char(env, '\\');
            out->append_char(env, c);
        } else if (c == '\n') {
            out->append(env, "\\n");
        } else if (c == '\t') {
            out->append(env, "\\t");
        } else {
            out->append_char(env, c);
        }
    }
    out->append_char(env, '"');
    return out;
}

bool StringValue::operator==(const Value &other) const {
    if (!other.is_string()) return false;
    const StringValue *rhs_string = const_cast<Value &>(other).as_string();
    return length() == rhs_string->length() && strcmp(c_str(), rhs_string->c_str()) == 0;
}

StringValue *StringValue::successive(Env *env) {
    StringValue *result = new StringValue { env, m_str };
    ssize_t index = m_length - 1;
    char last_char = m_str[index];
    if (last_char == 'z') {
        result->increment_successive_char(env, 'a', 'a', 'z');
    } else if (last_char == 'Z') {
        result->increment_successive_char(env, 'A', 'A', 'Z');
    } else if (last_char == '9') {
        result->increment_successive_char(env, '0', '1', '9');
    } else {
        char *next = strdup(result->c_str());
        next[index]++;
        result->set_str(next);
        free(next);
    }
    return result;
}

void StringValue::increment_successive_char(Env *env, char append_char, char begin_char, char end_char) {
    ssize_t index = m_length - 1;
    char last_char = m_str[index];
    while (last_char == end_char) {
        m_str[index] = begin_char;
        last_char = m_str[--index];
    }
    if (index == -1) {
        this->append_char(env, append_char);
    } else {
        m_str[index]++;
    }
}

Value *StringValue::index(Env *env, Value *needle) {
    return index(env, needle, 0);
}

// FIXME: this does not honor multi-byte characters :-(
Value *StringValue::index(Env *env, Value *needle, ssize_t start) {
    ssize_t i = index_ssize_t(env, needle, start);
    if (i == -1) {
        return env->nil_obj();
    }
    return new IntegerValue { env, i };
}

// FIXME: this does not honor multi-byte characters :-(
ssize_t StringValue::index_ssize_t(Env *env, Value *needle, ssize_t start) {
    NAT_ASSERT_TYPE(needle, Value::Type::String, "String");
    const char *ptr = strstr(c_str() + start, needle->as_string()->c_str());
    if (ptr == nullptr) {
        return -1;
    }
    return ptr - c_str();
}

StringValue *StringValue::sprintf(Env *env, const char *format, ...) {
    va_list args;
    va_start(args, format);
    StringValue *out = vsprintf(env, format, args);
    va_end(args);
    return out;
}

StringValue *StringValue::vsprintf(Env *env, const char *format, va_list args) {
    StringValue *out = new StringValue { env, "" };
    ssize_t len = strlen(format);
    StringValue *inspected;
    char buf[NAT_INT_64_MAX_BUF_LEN];
    for (ssize_t i = 0; i < len; i++) {
        char c = format[i];
        if (c == '%') {
            char c2 = format[++i];
            switch (c2) {
            case 's':
                out->append(env, va_arg(args, char *));
                break;
            case 'S':
                out->append_string(env, va_arg(args, StringValue *));
                break;
            case 'i':
            case 'd':
                int_to_string(va_arg(args, int64_t), buf);
                out->append(env, buf);
                break;
            case 'x':
                int_to_hex_string(va_arg(args, int64_t), buf, false);
                out->append(env, buf);
                break;
            case 'X':
                int_to_hex_string(va_arg(args, int64_t), buf, true);
                out->append(env, buf);
                break;
            case 'v':
                inspected = va_arg(args, Value *)->send(env, "inspect")->as_string();
                out->append_string(env, inspected);
                break;
            case '%':
                out->append_char(env, '%');
                break;
            default:
                fprintf(stderr, "Unknown format specifier: %%%c", c2);
                abort();
            }
        } else {
            out->append_char(env, c);
        }
    }
    return out;
}

Value *StringValue::initialize(Env *env, Value *arg) {
    if (arg) {
        NAT_ASSERT_TYPE(arg, Value::Type::String, "String");
        set_str(arg->as_string()->c_str());
    }
    return this;
}

Value *StringValue::ltlt(Env *env, Value *arg) {
    NAT_ASSERT_NOT_FROZEN(this);
    if (arg->is_string()) {
        append_string(env, arg->as_string());
    } else {
        Value *str_obj = arg->send(env, "to_s");
        NAT_ASSERT_TYPE(str_obj, Value::Type::String, "String");
        append_string(env, str_obj->as_string());
    }
    return this;
}

Value *StringValue::add(Env *env, Value *arg) {
    const char *str;
    if (arg->is_string()) {
        str = arg->as_string()->c_str();
    } else {
        StringValue *str_obj = arg->send(env, "to_s")->as_string();
        NAT_ASSERT_TYPE(str_obj, Value::Type::String, "String");
        str = str_obj->c_str();
    }
    StringValue *new_string = new StringValue { env, c_str() };
    new_string->append(env, str);
    return new_string;
}

Value *StringValue::mul(Env *env, Value *arg) {
    NAT_ASSERT_TYPE(arg, Value::Type::Integer, "Integer");
    StringValue *new_string = new StringValue { env, "" };
    for (int64_t i = 0; i < arg->as_integer()->to_int64_t(); i++) {
        new_string->append_string(env, this);
    }
    return new_string;
}

Value *StringValue::cmp(Env *env, Value *other) {
    if (other->type() != Value::Type::String) return env->nil_obj();
    int diff = strcmp(c_str(), other->as_string()->c_str());
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

Value *StringValue::eqtilde(Env *env, Value *other) {
    NAT_ASSERT_TYPE(other, Value::Type::Regexp, "Regexp");
    return other->as_regexp()->eqtilde(env, this);
}

Value *StringValue::match(Env *env, Value *other) {
    NAT_ASSERT_TYPE(other, Value::Type::Regexp, "Regexp");
    return other->as_regexp()->match(env, this);
}

Value *StringValue::ord(Env *env) {
    ArrayValue *chars = this->chars(env);
    if (chars->size() == 0) {
        NAT_RAISE(env, "ArgumentError", "empty string");
    }
    StringValue *c = (*chars)[0]->as_string();
    assert(c->length() > 0);
    unsigned int code;
    const char *str = c->c_str();
    switch (c->length()) {
    case 0:
        NAT_UNREACHABLE();
    case 1:
        code = (unsigned char)str[0];
        break;
    case 2:
        code = (((unsigned char)str[0] ^ 0xC0) << 6) + (((unsigned char)str[1] ^ 0x80) << 0);
        break;
    case 3:
        code = (((unsigned char)str[0] ^ 0xE0) << 12) + (((unsigned char)str[1] ^ 0x80) << 6) + (((unsigned char)str[2] ^ 0x80) << 0);
        break;
    case 4:
        code = (((unsigned char)str[0] ^ 0xF0) << 18) + (((unsigned char)str[1] ^ 0x80) << 12) + (((unsigned char)str[2] ^ 0x80) << 6) + (((unsigned char)str[3] ^ 0x80) << 0);
        break;
    default:
        NAT_UNREACHABLE();
    }
    return new IntegerValue { env, code };
}

Value *StringValue::bytes(Env *env) {
    ArrayValue *ary = new ArrayValue { env };
    for (ssize_t i = 0; i < m_length; i++) {
        ary->push(new IntegerValue { env, m_str[i] });
    }
    return ary;
}

Value *StringValue::size(Env *env) {
    return new IntegerValue { env, chars(env)->size() };
}

Value *StringValue::encoding(Env *env) {
    ClassValue *Encoding = env->Object()->const_find(env, "Encoding")->as_class();
    switch (m_encoding) {
    case Encoding::ASCII_8BIT:
        return Encoding->const_find(env, "ASCII_8BIT");
    case Encoding::UTF_8:
        return Encoding->const_find(env, "UTF_8");
    }
    NAT_UNREACHABLE();
}

static char *lcase_string(const char *str) {
    char *lcase_str = strdup(str);
    for (int i = 0; lcase_str[i]; i++) {
        lcase_str[i] = tolower(lcase_str[i]);
    }
    return lcase_str;
}

static EncodingValue *find_encoding_by_name(Env *env, const char *name) {
    char *lcase_name = lcase_string(name);
    ArrayValue *list = EncodingValue::list(env);
    for (ssize_t i = 0; i < list->size(); i++) {
        EncodingValue *encoding = (*list)[i]->as_encoding();
        ArrayValue *names = encoding->names(env);
        for (ssize_t n = 0; n < names->size(); n++) {
            StringValue *name_obj = (*names)[n]->as_string();
            char *name = lcase_string(name_obj->c_str());
            if (strcmp(name, lcase_name) == 0) {
                free(name);
                free(lcase_name);
                return encoding;
            }
            free(name);
        }
    }
    free(lcase_name);
    NAT_RAISE(env, "ArgumentError", "unknown encoding name - %s", name);
}

Value *StringValue::encode(Env *env, Value *encoding) {
    Encoding orig_encoding = m_encoding;
    StringValue *copy = dup(env)->as_string();
    copy->force_encoding(env, encoding);
    ClassValue *Encoding = env->Object()->const_find(env, "Encoding")->as_class();
    if (orig_encoding == copy->encoding()) {
        return copy;
    } else if (orig_encoding == Encoding::UTF_8 && copy->encoding() == Encoding::ASCII_8BIT) {
        ArrayValue *chars = this->chars(env);
        for (ssize_t i = 0; i < chars->size(); i++) {
            StringValue *char_obj = (*chars)[i]->as_string();
            if (char_obj->length() > 1) {
                Value *ord = char_obj->ord(env);
                Value *message = StringValue::sprintf(env, "U+%X from UTF-8 to ASCII-8BIT", ord->as_integer()->to_int64_t());
                StringValue zero_x { env, "0X" };
                StringValue blank { env, "" };
                message = message->as_string()->sub(env, &zero_x, &blank);
                env->raise(Encoding->const_find(env, "UndefinedConversionError")->as_class(), "%S", message);
                abort();
            }
        }
        return copy;
    } else if (orig_encoding == Encoding::ASCII_8BIT && copy->encoding() == Encoding::UTF_8) {
        return copy;
    } else {
        env->raise(Encoding->const_find(env, "ConverterNotFoundError")->as_class(), "code converter not found");
        abort();
    }
}

Value *StringValue::force_encoding(Env *env, Value *encoding) {
    switch (encoding->type()) {
    case Value::Type::Encoding:
        set_encoding(encoding->as_encoding()->num());
        break;
    case Value::Type::String:
        set_encoding(find_encoding_by_name(env, encoding->as_string()->c_str())->num());
        break;
    default:
        NAT_RAISE(env, "TypeError", "no implicit conversion of %s into String", encoding->klass()->class_name());
    }
    return this;
}

Value *StringValue::ref(Env *env, Value *index_obj) {
    // not sure how we'd handle a string that big anyway
    assert(m_length < INT64_MAX);

    if (index_obj->is_integer()) {
        int64_t index = index_obj->as_integer()->to_int64_t();

        ArrayValue *chars = this->chars(env);
        if (index < 0) {
            index = chars->size() + index;
        }

        if (index < 0 || index >= (int64_t)chars->size()) {
            return env->nil_obj();
        }
        return (*chars)[index];
    } else if (index_obj->is_range()) {
        RangeValue *range = index_obj->as_range();

        NAT_ASSERT_TYPE(range->begin(), Value::Type::Integer, "Integer");
        NAT_ASSERT_TYPE(range->end(), Value::Type::Integer, "Integer");

        int64_t begin = range->begin()->as_integer()->to_int64_t();
        int64_t end = range->end()->as_integer()->to_int64_t();

        ArrayValue *chars = this->chars(env);

        if (begin < 0) begin = chars->size() + begin;
        if (end < 0) end = chars->size() + end;

        if (begin < 0 || end < 0) return env->nil_obj();
        if (begin >= chars->size()) return env->nil_obj();

        if (!range->exclude_end()) end++;

        ssize_t max = chars->size();
        end = end > max ? max : end;
        StringValue *result = new StringValue { env };
        for (int64_t i = begin; i < end; i++) {
            result->append_string(env, (*chars)[i]);
        }

        return result;
    }
    NAT_ASSERT_TYPE(index_obj, Value::Type::Integer, "Integer");
    abort();
}

StringValue *StringValue::sub(Env *env, Value *find, Value *replacement) {
    NAT_ASSERT_TYPE(replacement, Value::Type::String, "String");
    if (find->is_string()) {
        ssize_t index = this->index_ssize_t(env, find->as_string(), 0);
        if (index == -1) {
            return dup(env)->as_string();
        }
        StringValue *out = new StringValue { env, m_str, index };
        out->append_string(env, replacement->as_string());
        out->append(env, &m_str[index + find->as_string()->length()]);
        return out;
    } else if (find->is_regexp()) {
        Value *match = find->as_regexp()->match(env, this);
        if (match == env->nil_obj()) {
            return dup(env)->as_string();
        }
        size_t length = match->as_match_data()->group(env, 0)->as_string()->length();
        int64_t index = match->as_match_data()->index(0);
        StringValue *out = new StringValue { env, m_str, index };
        out->append_string(env, replacement->as_string());
        out->append(env, &m_str[index + length]);
        return out;
    } else {
        NAT_RAISE(env, "TypeError", "wrong argument type %s (expected Regexp)", find->klass()->class_name());
    }
}

Value *StringValue::to_i(Env *env, Value *base_obj) {
    int base = 10;
    if (base_obj) {
        NAT_ASSERT_TYPE(base_obj, Value::Type::Integer, "Integer");
        base = base_obj->as_integer()->to_int64_t();
    }
    int64_t number = strtoll(m_str, nullptr, base);
    return new IntegerValue { env, number };
}

Value *StringValue::split(Env *env, Value *splitter) {
    ArrayValue *ary = new ArrayValue { env };
    if (!splitter) {
        splitter = new RegexpValue { env, "\\s+" };
    }
    if (m_length == 0) {
        return ary;
    } else if (splitter->is_regexp()) {
        ssize_t last_index = 0;
        ssize_t index, len;
        OnigRegion *region = onig_region_new();
        int result = splitter->as_regexp()->search(m_str, region, ONIG_OPTION_NONE);
        if (result == ONIG_MISMATCH) {
            ary->push(dup(env));
        } else {
            do {
                index = region->beg[0];
                len = region->end[0] - region->beg[0];
                ary->push(new StringValue { env, &m_str[last_index], index - last_index });
                last_index = index + len;
                result = splitter->as_regexp()->search(m_str, last_index, region, ONIG_OPTION_NONE);
            } while (result != ONIG_MISMATCH);
            ary->push(new StringValue { env, &m_str[last_index] });
        }
        onig_region_free(region, true);
        return ary;
    } else if (splitter->is_string()) {
        ssize_t last_index = 0;
        ssize_t index = index_ssize_t(env, splitter->as_string(), 0);
        if (index == -1) {
            ary->push(dup(env));
        } else {
            do {
                ary->push(new StringValue { env, &m_str[last_index], index - last_index });
                last_index = index + splitter->as_string()->length();
                index = index_ssize_t(env, splitter->as_string(), last_index);
            } while (index != -1);
            ary->push(new StringValue { env, &m_str[last_index] });
        }
        return ary;
    } else {
        NAT_RAISE(env, "TypeError", "wrong argument type %s (expected Regexp))", splitter->klass()->class_name());
    }
}

Value *StringValue::ljust(Env *env, Value *length_obj, Value *pad_obj) {
    NAT_ASSERT_TYPE(length_obj, Value::Type::Integer, "Integer");
    ssize_t length = length_obj->as_integer()->to_int64_t() < 0 ? 0 : length_obj->as_integer()->to_int64_t();
    StringValue *padstr;
    if (pad_obj) {
        NAT_ASSERT_TYPE(pad_obj, Value::Type::String, "String");
        padstr = pad_obj->as_string();
    } else {
        padstr = new StringValue { env, " " };
    }
    StringValue *copy = dup(env)->as_string();
    while (copy->length() < length) {
        bool truncate = copy->length() + padstr->length() > length;
        copy->append_string(env, padstr);
        if (truncate) {
            copy->truncate(length);
        }
    }
    return copy;
}

}
