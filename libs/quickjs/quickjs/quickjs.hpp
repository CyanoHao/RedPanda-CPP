#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include <QtCore>

#include "quickjs-libc.h"
#include "quickjs.h"

/* Minimal wrapper for QuickJS <=> C++/Qt value conversion. */
namespace RawJs {

template <typename>
constexpr bool dependent_false = false;

JSValue NewValue(JSContext *ctx, const QJsonValue &value) noexcept;
inline JSValue NewBool(JSContext *ctx, bool value) noexcept { return JS_NewBool(ctx, value); }
inline JSValue NewInt(JSContext *ctx, long long value) noexcept { return JS_NewInt64(ctx, value); }
inline JSValue NewFloat(JSContext *ctx, double value) noexcept { return JS_NewFloat64(ctx, value); }
inline JSValue NewString(JSContext *ctx, const char *value) noexcept { return JS_NewString(ctx, value); }
inline JSValue NewString(JSContext *ctx, const std::string &value) noexcept { return JS_NewStringLen(ctx, value.c_str(), value.length()); }
inline JSValue NewString(JSContext *ctx, const QString &value) noexcept { return NewString(ctx, value.toStdString()); }
inline JSValue NewArray(JSContext *ctx, const QStringList &value) noexcept
{
    JSValue arr = JS_NewArray(ctx);
    for (int i = 0; i < value.size(); ++i)
        JS_SetPropertyUint32(ctx, arr, i, NewString(ctx, value[i]));
    return arr;
}
inline JSValue NewArray(JSContext *ctx, const QJsonArray &value) noexcept
{
    JSValue arr = JS_NewArray(ctx);
    for (int i = 0; i < value.size(); ++i)
        JS_SetPropertyUint32(ctx, arr, i, NewValue(ctx, value[i]));
    return arr;
}
inline JSValue NewObject(JSContext *ctx, const QJsonObject &value) noexcept
{
    JSValue obj = JS_NewObject(ctx);
    for (auto it = value.begin(); it != value.end(); ++it)
        JS_SetPropertyStr(ctx, obj, it.key().toUtf8().constData(), NewValue(ctx, it.value()));
    return obj;
}
inline JSValue NewValue(JSContext *ctx, const QJsonValue &value) noexcept
{
    if (value.isUndefined())
        return JS_UNDEFINED;
    else if (value.isNull())
        return JS_NULL;
    else if (value.isBool())
        return JS_NewBool(ctx, value.toBool());
    else if (value.isDouble())
        return JS_NewFloat64(ctx, value.toDouble());
    else if (value.isString())
        return NewString(ctx, value.toString());
    else if (value.isObject())
        return NewObject(ctx, value.toObject());
    else if (value.isArray())
        return NewArray(ctx, value.toArray());
    else
        return JS_UNDEFINED;
}

template <typename T>
JSValue NewValue(JSContext *ctx, const T &value) noexcept
{
    if constexpr (std::is_same<T, bool>::value)
        return NewBool(ctx, value);
    else if constexpr (std::is_integral<T>::value)
        return NewInt(ctx, value);
    else if constexpr (std::is_floating_point<T>::value)
        return NewFloat(ctx, value);
    else if constexpr (std::is_same<T, const char *>::value || std::is_same<T, std::string>::value || std::is_same<T, QString>::value)
        return NewString(ctx, value);
    else if constexpr (std::is_same<T, QStringList>::value || std::is_same<T, QJsonArray>::value)
        return NewArray(ctx, value);
    else if constexpr (std::is_same<T, QJsonObject>::value)
        return NewObject(ctx, value);
    else if constexpr (std::is_same<T, QJsonValue>::value)
        return NewValue(ctx, value);
    else
        static_assert(dependent_false<T>, "NewValue: unknown type");
}

class TypeError : public std::runtime_error {
public:
    TypeError(const std::string &msg) : std::runtime_error("ECMAScript type error: " + msg) {}
};

struct RaiiValue {
    JSValue mValue;
    JSContext *mContext;

    ~RaiiValue() { JS_FreeValue(mContext, mValue); }
    operator JSValue() const { return mValue; }
};

QJsonValue ToQJsonValue(JSContext *ctx, JSValueConst value);
inline bool ToBool(JSContext *ctx, JSValueConst value)
{
    if (!JS_IsBool(value))
        throw TypeError("ToBool: boolean expected");
    return JS_ToBool(ctx, value);
}
inline int64_t ToInt(JSContext *ctx, JSValueConst value)
{
    if (!JS_IsNumber(value))
        throw TypeError("ToInt: number expected");
    int64_t result;
    JS_ToInt64(ctx, &result, value);
    return result;
}
inline double ToFloat(JSContext *ctx, JSValueConst value)
{
    if (!JS_IsNumber(value))
        throw TypeError("ToFloat: number expected");
    double result;
    JS_ToFloat64(ctx, &result, value);
    return result;
}
inline std::string ToString(JSContext *ctx, JSValueConst value)
{
    if (!JS_IsString(value))
        throw TypeError("ToString: string expected");
    size_t len;
    const char *str = JS_ToCStringLen(ctx, &len, value);
    std::string result(str, len);
    JS_FreeCString(ctx, str);
    return result;
}
inline QString ToQString(JSContext *ctx, JSValueConst value) { return QString::fromStdString(ToString(ctx, value)); }
inline QStringList ToQStringList(JSContext *ctx, JSValueConst value)
{
    if (!JS_IsArray(ctx, value))
        throw TypeError("ToQStringList: array expected");
    JSValue jsLen = JS_GetPropertyStr(ctx, value, "length");
    int len = ToInt(ctx, jsLen);
    QStringList result;
    for (int i = 0; i < len; ++i) {
        RaiiValue jsItem{JS_GetPropertyUint32(ctx, value, i), ctx};
        result << ToQString(ctx, jsItem);  // possible exception
    }
    return result;
}
inline QJsonArray ToQJsonArray(JSContext *ctx, JSValueConst value)
{
    if (!JS_IsArray(ctx, value))
        throw TypeError("ToQJsonArray: array expected");
    JSValue jsLen = JS_GetPropertyStr(ctx, value, "length");
    int len = ToInt(ctx, jsLen);
    QJsonArray result;
    for (int i = 0; i < len; ++i) {
        RaiiValue jsItem{JS_GetPropertyUint32(ctx, value, i), ctx};
        result << ToQJsonValue(ctx, jsItem);  // possible exception
    }
    return result;
}
inline QJsonObject ToQJsonObject(JSContext *ctx, JSValueConst value)
{
    if (!JS_IsObject(value) || JS_IsArray(ctx, value))
        throw TypeError("ToQJsonObject: non-array object expected");

    JSPropertyEnum *props;
    uint32_t len;
    JS_GetOwnPropertyNames(ctx, &props, &len, value, JS_GPN_STRING_MASK);
    std::unique_ptr<JSPropertyEnum, std::function<void (JSPropertyEnum *)>> props_{props, [ctx](JSPropertyEnum *p) { js_free(ctx, p); }};

    QJsonObject result;
    for (uint32_t i = 0; i < len; ++i) {
        JSPropertyEnum &prop = props[i];
        RaiiValue jsKey{JS_AtomToString(ctx, prop.atom), ctx};
        RaiiValue jsValue{JS_GetProperty(ctx, value, prop.atom), ctx};
        result.insert(ToQString(ctx, jsKey), ToQJsonValue(ctx, jsValue));  // possible exception
    }
    return result;
}
inline QJsonValue ToQJsonValue(JSContext *ctx, JSValueConst value)
{
    if (JS_IsUndefined(value))
        return QJsonValue::Undefined;
    else if (JS_IsNull(value))
        return QJsonValue::Null;
    else if (JS_IsBool(value))
        return ToBool(ctx, value);
    else if (JS_IsNumber(value))
        return ToFloat(ctx, value);
    else if (JS_IsString(value))
        return ToQString(ctx, value);
    else if (JS_IsArray(ctx, value))
        return ToQJsonArray(ctx, value);
    else if (JS_IsObject(value))
        return ToQJsonObject(ctx, value);
    else
        throw TypeError("ToValue: unknown type");
}

template <typename R>
R ToValue(JSContext *ctx, JSValueConst value)
{
    if constexpr (std::is_same<R, bool>::value)
        return ToBool(ctx, value);
    else if constexpr (std::is_integral<R>::value)
        return ToInt(ctx, value);
    else if constexpr (std::is_floating_point<R>::value)
        return ToFloat(ctx, value);
    else if constexpr (std::is_same<R, std::string>::value)
        return ToString(ctx, value);
    else if constexpr (std::is_same<R, QString>::value)
        return ToQString(ctx, value);
    else if constexpr (std::is_same<R, QStringList>::value)
        return ToQStringList(ctx, value);
    else if constexpr (std::is_same<R, QJsonArray>::value)
        return ToQJsonArray(ctx, value);
    else if constexpr (std::is_same<R, QJsonObject>::value)
        return ToQJsonObject(ctx, value);
    else if constexpr (std::is_same<R, QJsonValue>::value)
        return ToQJsonValue(ctx, value);
    else
        static_assert(dependent_false<R>, "ToValue: unknown type");
}

} // namespace RawJs

/* Minimal RAII wrapper for QuickJS. */
namespace RaiiJs {

class Value {
private:
    Value(JSValue val, JSContext *ctx) : mValue{val}, mContext{ctx} {}
    friend class Context;

public:
    Value(const Value &) = delete;
    Value(Value &&rhs) = default;
    Value &operator=(const Value &) = delete;
    Value &operator=(Value &&) = default;
    ~Value() { JS_FreeValue(mContext, mValue); }

public:
    bool IsException() const { return JS_IsException(mValue); }
    bool IsUndefined() const { return JS_IsUndefined(mValue); }
    bool IsNull() const { return JS_IsNull(mValue); }
    bool IsBool() const { return JS_IsBool(mValue); }
    bool IsNumber() const { return JS_IsNumber(mValue); }
    bool IsString() const { return JS_IsString(mValue); }
    bool IsObject() const { return JS_IsObject(mValue) && !JS_IsArray(mContext, mValue); }
    bool IsArray() const { return JS_IsArray(mContext, mValue); }
    bool IsFunction() const { return JS_IsFunction(mContext, mValue); }

    explicit operator bool() const { return RawJs::ToBool(mContext, mValue); }
    explicit operator int() const { return RawJs::ToInt(mContext, mValue); }
    explicit operator long() const { return RawJs::ToInt(mContext, mValue); }
    explicit operator long long() const { return RawJs::ToInt(mContext, mValue); }
    explicit operator float() const { return RawJs::ToFloat(mContext, mValue); }
    explicit operator double() const { return RawJs::ToFloat(mContext, mValue); }
    explicit operator std::string() const { return RawJs::ToString(mContext, mValue); }
    explicit operator QString() const { return RawJs::ToQString(mContext, mValue); }
    explicit operator QStringList() const { return RawJs::ToQStringList(mContext, mValue); }
    explicit operator QJsonArray() const { return RawJs::ToQJsonArray(mContext, mValue); }
    explicit operator QJsonObject() const { return RawJs::ToQJsonObject(mContext, mValue); }
    explicit operator QJsonValue() const { return RawJs::ToQJsonValue(mContext, mValue); }

    auto exceptionDetail() const {
        auto exception = JS_GetException(mContext);
        auto exception_str = JS_ToCString(mContext, exception);
        auto stack_str = JS_ToCString(mContext, JS_GetPropertyStr(mContext, exception, "stack"));
        auto result = std::make_tuple(exception_str, stack_str);
        JS_FreeCString(mContext, exception_str);
        JS_FreeCString(mContext, stack_str);
        return exception_str;
    }

    Value GetPropertyUint32(int index) const {
        if (!IsArray())
            throw RawJs::TypeError("Value::GetPropertyUint32: array expected");
        return {JS_GetPropertyUint32(mContext, mValue, index), mContext};
    }
    Value GetPropertyStr(const char *key) const {
        if (!IsObject())
            throw RawJs::TypeError("Value::GetPropertyStr: object expected");
        return {JS_GetPropertyStr(mContext, mValue, key), mContext};
    }
    Value GetPropertyStr(const std::string &key) const { return GetPropertyStr(key.c_str()); }
    Value GetPropertyStr(const QString &key) const { return GetPropertyStr(key.toUtf8().constData()); }

    QJsonValue operator[](int index) const { return RawJs::ToQJsonValue(mContext, GetPropertyUint32(index)); }
    QJsonValue operator[](const char *key) const { return RawJs::ToQJsonValue(mContext, GetPropertyStr(key)); }

private:
    JSValue mValue;
    JSContext *mContext;
    operator JSValue() const { return mValue; }
};

class Context {

private:
    Context(JSContext *ctx) : mContext{ctx, &JS_FreeContext} {}
    friend class Runtime;

public:
    Context(const Context &) = delete;
    Context(Context &&) = default;
    Context &operator=(const Context &) = delete;
    Context &operator=(Context &&) = default;
    ~Context() = default;

public:
    Value Call(JSValue func, JSValue this_obj, int argc, JSValue *argv) {
        return {JS_Call(*this, func, this_obj, argc, argv), *this};
    }
    Value Call(Value func, Value this_obj, const std::vector<Value> &args) {
        std::vector<JSValue> argv;
        std::transform(args.begin(), args.end(), std::back_inserter(argv), [](const Value &v) { return JSValue(v); });
        return Call(func, this_obj, argv.size(), argv.data());
    }
    Value Call(Value func, Value this_obj, const QList<Value> &args) {
        std::vector<JSValue> argv;
        std::transform(args.begin(), args.end(), std::back_inserter(argv), [](const Value &v) { return JSValue(v); });
        return Call(func, this_obj, argv.size(), argv.data());
    }
    Value Call(Value func, Value this_obj) { return Call(func, this_obj, 0, nullptr); }

    Value Eval(const char *input, size_t input_len, const char *filename, int eval_flags) {
        return {JS_Eval(*this, input, input_len, filename, eval_flags), *this};
    }
    Value Eval(const std::string &input, const std::string &filename, int eval_flags) {
        return Eval(input.c_str(), input.size(), filename.c_str(), eval_flags);
    }
    Value Eval(const QString &input, const QString &filename, int eval_flags) {
        return Eval(input.toUtf8().constData(), input.size(), filename.toUtf8().constData(), eval_flags);
    }
    Value Eval(const QByteArray &input, const QString &filename, int eval_flags) {
        return Eval(input.constData(), input.size(), filename.toUtf8().constData(), eval_flags);
    }

    Value GetGlobalObject() { return {JS_GetGlobalObject(*this), *this}; }

    Value NewFloat64(double d) { return {JS_NewFloat64(*this, d), *this}; }

    Value NewString(const char *str) { return {JS_NewString(*this, str), *this}; }
    Value NewString(const std::string &str) { return NewString(str.c_str()); }
    Value NewString(const QString &str) { return NewString(str.toUtf8().constData()); }

    void std_add_helpers(int argc, char **argv) { js_std_add_helpers(*this, argc, argv); }

private:
    std::unique_ptr<JSContext, decltype(&JS_FreeContext)> mContext;
    operator JSContext *() const { return mContext.get(); }
};

class Runtime {
public:
    Runtime() : mRuntime{JS_NewRuntime(), &JS_FreeRuntime} {}
    Runtime(const Runtime &) = delete;
    Runtime(Runtime &&) = default;
    Runtime &operator=(const Runtime &) = delete;
    Runtime &operator=(Runtime &&) = default;
    ~Runtime() = default;

public:
    void SetMemoryLimit(size_t limit) { JS_SetMemoryLimit(*this, limit); }
    void SetGCThreshold(size_t gc_threshold) { JS_SetGCThreshold(*this, gc_threshold); }
    void SetMaxStackSize(size_t stack_size) { JS_SetMaxStackSize(*this, stack_size); }

    Context NewContext() { return JS_NewContext(*this); }

private:
    std::unique_ptr<JSRuntime, decltype(&JS_FreeRuntime)> mRuntime;
    operator JSRuntime *() const { return mRuntime.get(); }
};

} // namespace RaiiJs
