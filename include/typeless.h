#ifndef TYPELESS_H
#define TYPELESS_H

#ifdef __TYPELESS_TEST
#define __TYPELESS_ACCESS_LEVEL public
#else
#define __TYPELESS_ACCESS_LEVEL private
#endif

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace typeless {
struct ObjectBase;
struct ArrayBase;
class Object;
class Array;

using std::string;
using std::type_info;
template <typename T>
using ArrayInit = std::initializer_list<T>;
using ObjectArray = ArrayInit<Object>;

namespace stringizer {
using std::to_string;
inline std::string to_string(const std::string& s);
inline std::string to_string(const char* c_str);
inline std::string to_string(const char& ch);
inline std::string to_string(const Object& obj);
template <class T>
std::string to_string(const T&);
}  // namespace stringizer

namespace internal {
class ObjectHelper {
   public:
    virtual void deleter(void* ptr) = 0;
    virtual void* make_copy(void* src) = 0;
    virtual bool equal(void* lhs, void* rhs) = 0;
    virtual string to_string(const void* ptr) = 0;
    virtual const type_info* type() = 0;
    virtual bool less(const void*, const void*) = 0;
    virtual Object sum(const void* a, const void* b) = 0;
    virtual Object difference(const void* a, const void* b) = 0;
    virtual Object product(const void* a, const void* b) = 0;
    virtual Object quotient(const void* a, const void* b) = 0;
};

class ArrayHelper {
   public:
    virtual void* call_new(size_t size) = 0;
    virtual void deleter(void* ptr) = 0;
    virtual void copy(void* src, void* dst, size_t n) = 0;
    virtual ptrdiff_t distance(void* high, void* low) = 0;
    virtual void* advance(void* ptr, size_t n) = 0;
    virtual const type_info* type() = 0;
};

template <class TValue, class TResult>
void default_join(const TValue& v, TResult& r) {
    r += v;
}

/* forward declaration */
template <class T>
extern ObjectHelper* OBJECT_HELPER;
template <class T>
extern ArrayHelper* ARRAY_HELPER;
};  // namespace internal

struct ObjectBase {
    mutable void* value_;
    mutable internal::ObjectHelper* helper_;
};

struct ArrayBase {
    mutable void* arr_;
    mutable void* end_;
    mutable internal::ArrayHelper* helper_;
};

class Object : __TYPELESS_ACCESS_LEVEL ObjectBase {
   public:
    /* constructor */
    Object();
    ~Object() noexcept;
    template <class T>
    Object(const T& obj);
    Object(const Object& rhs);
    Object(Object&&) noexcept;
    template <class T>
    Object& operator=(const T& obj);
    Object& operator=(const Object& rhs);
    Object& operator=(Object&& rhs) noexcept;
    /* arithmetic operator */
    template <class T>
    friend bool operator==(const Object& obj, const T& v);
    friend bool operator==(const Object& l, const Object& r);
    template <class T>
    friend bool operator!=(const Object& obj, const T& v);
    friend bool operator<(const Object& l, const Object& r);
    friend bool operator>(const Object& l, const Object& r);
    friend bool operator<=(const Object& l, const Object& r);
    friend bool operator>=(const Object& l, const Object& r);
    friend Object operator+(const Object& l, const Object& r);
    friend Object operator-(const Object& l, const Object& r);
    friend Object operator*(const Object& l, const Object& r);
    friend Object operator/(const Object& l, const Object& r);
    /* getter */
    void* data() const noexcept;
    template <class T>
    T& get();
    template <class T>
    T get() const;
    /* setter */
    template <class T, class... Args>
    void emplace(Args&&... args);
    /* utilities */
    Object clone() const;
    bool empty() const noexcept;
    void destroy() const noexcept;
    void swap(Object& right) noexcept;
    std::string to_string() const;
    /* type */
    template <typename T>
    bool has_type() const noexcept;
    const type_info& type() const noexcept;
    const char* type_name() const noexcept;
};

class Array : __TYPELESS_ACCESS_LEVEL ArrayBase {
   public:
    /* constructor */
    Array();
    template <class T>
    Array(ArrayInit<T> init);
    Array(const Array& rhs);
    Array(Array&& rhs) noexcept;
    Array& operator=(const Array& rhs);
    Array& operator=(Array&& rhs) noexcept;
    ~Array() noexcept;
    /* getter */
    template <class T>
    const T* data() const noexcept;
    template <class T>
    T* data() noexcept;
    template <class T>
    T& at(size_t idx);
    template <class T>
    T at(size_t idx) const;
    template <class T>
    /* setter */
    void set_type();
    template <class T, class... Args>
    void emplace(size_t off, Args&&... args);
    template <class T>
    void set(size_t off, T ele);
    /* utilities */
    template <class T, class Callback>
    void for_each(Callback cb) const;
    template <class T, class Fn>
    Array filter(Fn filter_fn) const;
    template <class T, class TResult = T>
    TResult join(void (*cb)(const T&,
                            TResult&) = internal::default_join<T, TResult>);
    bool empty() const noexcept;
    size_t size() const noexcept;
    void resize(size_t new_size);
    void destroy() noexcept;
    void swap(Array& right) noexcept;
    /* type */
    const type_info& type() const noexcept;
    const char* type_name() const noexcept;
    /* iterator */
    void* begin() noexcept;
    void* end() noexcept;
    const void* cbegin() const noexcept;
    const void* cend() const noexcept;
};

#pragma region ObjectImpl
Object::Object() : ObjectBase{nullptr, nullptr} {}
Object::~Object() noexcept { destroy(); }
template <class T>
Object::Object(const T& obj)
    : ObjectBase{static_cast<void*>(new std::decay_t<T>(obj)),
                 internal::OBJECT_HELPER<T>} {}
Object::Object(const Object& rhs) : ObjectBase(rhs) {
    if (helper_) {
        value_ = helper_->make_copy(value_);
    }
}
Object::Object(Object&& rhs) noexcept : ObjectBase(rhs) {
    rhs.value_ = nullptr;
    rhs.helper_ = nullptr;
}

template <class T>
Object& Object::operator=(const T& obj) {
    if (helper_ != nullptr) {
        helper_->deleter(value_);  // destroy the old content with helper
    }
    value_ = new std::decay_t<T>(obj);
    helper_ = internal::OBJECT_HELPER<T>;
    return *this;
}

Object& Object::operator=(const Object& rhs) {
    if (this == &rhs) {
        return *this;
    }
    if (helper_ != nullptr) {
        helper_->deleter(value_);  // destroy the old content with helper
    }
    helper_ = rhs.helper_;
    if (helper_ != nullptr) {
        value_ = helper_->make_copy(rhs.value_);  // construct an object
    }
    return *this;
}

Object& Object::operator=(Object&& rhs) noexcept {
    if (helper_) {
        helper_->deleter(value_);  // destroy the old content with helper
    }
    static_cast<ObjectBase&>(*this) = rhs;
    rhs.value_ = nullptr;
    rhs.helper_ = nullptr;
    return *this;
}

template <class T>
bool operator==(const Object& obj, const T& v) {
    using DecayT = std::decay_t<T>;
    assert(obj.helper_ != nullptr);
    return obj.type() == typeid(DecayT) && obj.get<DecayT>() == v;
}

bool operator==(const Object& l, const Object& r) {
    assert(l.helper_ != nullptr && r.helper_ != nullptr);
    return l.type() == r.type() && l.helper_->equal(l.value_, r.value_);
}

template <class T>
bool operator!=(const Object& obj, const T& v) {
    return !(obj == v);
}

bool operator<(const Object& l, const Object& r) {
    if (l.type() != r.type()) {
        return false;
    }
    return l.helper_->less(l.value_, r.value_);
}

bool operator>(const Object& l, const Object& r) {
    if (l.type() != r.type()) {
        return false;
    }
    return !l.helper_->less(l.value_, r.value_);
}

bool operator<=(const Object& l, const Object& r) {
    if (l.type() != r.type()) {
        return false;
    }
    return l.helper_->less(l.value_, r.value_) ||
           l.helper_->equal(l.value_, r.value_);
}

bool operator>=(const Object& l, const Object& r) {
    if (l.type() != r.type()) {
        return false;
    }
    return !l.helper_->less(l.value_, r.value_) ||
           l.helper_->equal(l.value_, r.value_);
}

Object operator+(const Object& l, const Object& r) {
    if (l.type() != r.type()) {
        return Object();
    }
    return l.helper_->sum(l.value_, r.value_);
}

Object operator-(const Object& l, const Object& r) {
    if (l.type() != r.type()) {
        return Object();
    }
    return l.helper_->difference(l.value_, r.value_);
}

Object operator*(const Object& l, const Object& r) {
    if (l.type() != r.type()) {
        return Object();
    }
    return l.helper_->product(l.value_, r.value_);
}

Object operator/(const Object& l, const Object& r) {
    if (l.type() != r.type()) {
        return Object();
    }
    return l.helper_->quotient(l.value_, r.value_);
}

void* Object::data() const noexcept { return value_; }

template <class T>
T& Object::get() {
    return *static_cast<T*>(value_);
}

template <class T>

T Object::get() const {
    return *static_cast<const T*>(value_);
}

template <class T, class... Args>

void Object::emplace(Args&&... args) {
    if (helper_ != nullptr) {
        helper_->deleter(value_);
    }
    value_ = new std::decay_t<T>(std::forward<Args>(args)...);
    helper_ = internal::OBJECT_HELPER<T>;
}

Object Object::clone() const { return *this; }

bool Object::empty() const noexcept { return value_ == nullptr; }

void Object::destroy() const noexcept {
    if (helper_ == nullptr) {
        return;
    }
    helper_->deleter(value_);
    value_ = nullptr;
    helper_ = nullptr;
    helper_ = nullptr;
}

void Object::swap(Object& right) noexcept { std::swap(*this, right); }

std::string Object::to_string() const {
    if (helper_ == nullptr) {
        return "null";
    }
    return helper_->to_string(value_);
}

template <typename T>

bool Object::has_type() const noexcept {
    return type() == typeid(T);
}

const type_info& Object::type() const noexcept {
    if (helper_ == nullptr) {
        return typeid(std::nullptr_t);
    }
    return *helper_->type();
}

const char* Object::type_name() const noexcept {
    if (helper_ == nullptr) {
        return "void";
    }
    return helper_->type()->name();
}
#pragma endregion ObjectImpl

#pragma region ArrayImpl
Array::Array() : ArrayBase{nullptr, nullptr, nullptr} {}

template <class T>
Array::Array(std::initializer_list<T> init) : ArrayBase() {
    T* ptr = new T[init.size()];
    arr_ = ptr;
    end_ = ptr + init.size();
    helper_ = internal::ARRAY_HELPER<T>;
    for (auto& ele : init) {
        *ptr = ele;
        ++ptr;
    }
}

Array::Array(const Array& rhs) : ArrayBase(rhs) {
    if (helper_) {
        auto n = helper_->distance(end_, arr_);
        void* new_arr = helper_->call_new(n);
        helper_->copy(arr_, new_arr, n);
        arr_ = new_arr;
        end_ = helper_->advance(arr_, n);
    }
}

Array::Array(Array&& rhs) noexcept : ArrayBase(rhs) {
    rhs.arr_ = rhs.end_ = nullptr;
    rhs.helper_ = nullptr;
}

Array& Array::operator=(const Array& rhs) {
    if (this == &rhs) {
        return *this;
    }
    if (helper_ != nullptr) {
        helper_->deleter(arr_);  // destroy the old content with helper
    }
    helper_ = rhs.helper_;
    if (helper_ != nullptr) {
        auto size = helper_->distance(rhs.end_, rhs.arr_);
        arr_ = helper_->call_new(size);
        end_ = helper_->advance(arr_, size);
        helper_->copy(rhs.arr_, arr_, size);
    }
    return *this;
}

Array& Array::operator=(Array&& rhs) noexcept {
    if (helper_ != nullptr) {
        helper_->deleter(arr_);  // destroy the old content with helper
    }
    static_cast<ArrayBase&>(*this) = rhs;
    rhs.arr_ = rhs.end_ = nullptr;
    rhs.helper_ = nullptr;
    return *this;
}

Array::~Array() noexcept { destroy(); }

template <class T>
const T* Array::data() const noexcept {
    return static_cast<const T*>(arr_);
}

template <class T>
T* Array::data() noexcept {
    return static_cast<T*>(arr_);
}

template <class T>

T& Array::at(size_t idx) {
    assert(idx < size());
    return static_cast<T*>(arr_)[idx];
}

template <class T>

T Array::at(size_t idx) const {
    assert(idx < size());
    return static_cast<const T*>(arr_)[idx];
}

template <class T, class... Args>
void Array::emplace(size_t off, Args&&... args) {
    assert(off < size());
    auto& p = static_cast<T*>(arr_)[off];
    p.~T();
    new (&p) T{std::forward<Args>(args)...};
}

template <class T>
void Array::set(size_t off, T ele) {
    assert(off < size());
    auto& p = static_cast<T*>(arr_)[off];
    p = ele;
}

template <class T>
void Array::set_type() {
    helper_ = internal::ARRAY_HELPER<T>;
}

template <class T, class Callback>
void Array::for_each(Callback cb) const {
    if (arr_ == nullptr) return;
    T* ptr = static_cast<T*>(arr_);
    while (ptr < end_) {
        cb(*ptr++);
    }
}

template <class T, class Fn>
Array Array::filter(Fn filter_fn) const {
    if (arr_ == nullptr) return *this;
    Array filtered;
    filtered.helper_ = helper_;
    size_t new_size = 0;
    for (T* p = static_cast<T*>(arr_); p < end_; ++p) {
        // calculate filtered array size
        if (filter_fn(*p)) {
            ++new_size;
        }
    }
    filtered.arr_ = filtered.helper_->call_new(new_size);
    filtered.end_ = filtered.helper_->advance(filtered.arr_, new_size);
    for (T *p = static_cast<T*>(arr_),
           *p_filtered = static_cast<T*>(filtered.arr_);
         p < end_; ++p) {
        if (filter_fn(*p)) {
            *p_filtered++ = *p;
        }
    }
    return filtered;  // moved
}

template <class T, class TResult>
TResult Array::join(void (*cb)(const T&, TResult&)) {
    if (arr_ == nullptr) return {};
    TResult result{};
    T* ptr = static_cast<T*>(arr_);
    while (ptr < end_) {
        cb(*ptr++, result);
    }
    return result;
}

bool Array::empty() const noexcept { return arr_ == nullptr; }

size_t Array::size() const noexcept { return helper_->distance(end_, arr_); }

void Array::resize(size_t new_size) {
    if (helper_ == nullptr) return;
    void* new_arr = helper_->call_new(new_size);
    helper_->copy(arr_, new_arr, std::min(new_size, size()));
    helper_->deleter(arr_);  // destroy old content
    arr_ = new_arr;
    end_ = helper_->advance(arr_, new_size);
}

void Array::destroy() noexcept {
    if (helper_ == nullptr) {
        return;
    }
    helper_->deleter(arr_);
    arr_ = end_ = nullptr;
    helper_ = nullptr;
}

void Array::swap(Array& right) noexcept { std::swap(*this, right); }

const type_info& Array::type() const noexcept {
    if (helper_ == nullptr) {
        return typeid(nullptr);
    }
    return *helper_->type();
}

const char* Array::type_name() const noexcept {
    if (helper_ == nullptr) {
        return "null";
    }
    return helper_->type()->name();
}

void* Array::begin() noexcept { return arr_; }

void* Array::end() noexcept { return end_; }

const void* Array::cbegin() const noexcept { return arr_; }

const void* Array::cend() const noexcept { return end_; }
#pragma endregion ArrayImpl

#pragma region StringizerImpl

inline std::string stringizer::to_string(const std::string& s) { return s; }

inline std::string stringizer::to_string(const char* c_str) { return c_str; }

inline std::string stringizer::to_string(const char& ch) { return {ch}; }

inline std::string stringizer::to_string(const Object& obj) {
    return obj.to_string();
}
template <class T>
std::string stringizer::to_string(const T&) {
#pragma message( \
    "to_string of type T is not implemented. Returning type name by default.")
    return typeid(T).name();
}
#pragma endregion StringizerImpl

#pragma region InternalImpl
namespace internal {
template <class T, class EqualTo>
struct HasOperatorEqualImpl {
    template <class U, class V>
    static auto test(U*) -> decltype(std::declval<U>() == std::declval<V>());
    template <typename, typename>
    static auto test(...) -> std::false_type;

    using type =
        typename std::is_same<bool, decltype(test<T, EqualTo>(nullptr))>::type;
};

template <class T, class EqualTo = T>
struct HasOperatorEqual : HasOperatorEqualImpl<T, EqualTo>::type {};

template <class T,
          typename std::enable_if_t<HasOperatorEqual<T>::value, int> = 0>
bool EqualHelper(const void* a, const void* b) {
    return *static_cast<const T*>(a) == *static_cast<const T*>(b);
}

template <class T,
          typename std::enable_if_t<!HasOperatorEqual<T>::value, int> = 0>
bool EqualHelper(const void* a, const void* b) {
    return false;
}

static const string ARITHMETIC_EXCEPTION_MSG =
    "Attempt to call arithmetic operand on non-arithmetic type ";
template <class T>
class TypedObjectHelperBase : public ObjectHelper {
    void deleter(void* ptr) override { delete static_cast<T*>(ptr); }
    void* make_copy(void* src) override { return new T(*static_cast<T*>(src)); }
    bool equal(void* lhs, void* rhs) override { return false; }
    string to_string(const void* ptr) override {
        return stringizer::to_string(*static_cast<const T*>(ptr));
    }
    const type_info* type() override { return &typeid(T); }
    bool less(const void*, const void*) override {
        throw std::runtime_error(ARITHMETIC_EXCEPTION_MSG + type()->name());
    }
    Object sum(const void* a, const void* b) override {
        throw std::runtime_error(ARITHMETIC_EXCEPTION_MSG + type()->name());
    }
    Object difference(const void* a, const void* b) override {
        throw std::runtime_error(ARITHMETIC_EXCEPTION_MSG + type()->name());
    }
    Object product(const void* a, const void* b) override {
        throw std::runtime_error(ARITHMETIC_EXCEPTION_MSG + type()->name());
    }
    Object quotient(const void* a, const void* b) override {
        throw std::runtime_error(ARITHMETIC_EXCEPTION_MSG + type()->name());
    }
};

template <class T, int = std::is_arithmetic<T>::value>
class TypedObjectHelperArith;

template <class T>
class TypedObjectHelperArith<T, 0> : public TypedObjectHelperBase<T> {};

template <class T>
class TypedObjectHelperArith<T, 1> : public TypedObjectHelperBase<T> {
    inline static const T& val(const void* v) {
        return *static_cast<const T*>(v);
    }
    bool less(const void* a, const void* b) override { return val(a) < val(b); }
    Object sum(const void* a, const void* b) override {
        return Object(val(a) + val(b));
    }
    Object difference(const void* a, const void* b) override {
        return Object(val(a) - val(b));
    }
    Object product(const void* a, const void* b) override {
        return Object(val(a) * val(b));
    }
    Object quotient(const void* a, const void* b) override {
        return Object(val(a) / val(b));
    }
};

template <class T, int = HasOperatorEqual<T>::value>
class TypedObjectHelper;

template <class T>
class TypedObjectHelper<T, 0> : public TypedObjectHelperArith<T> {};

template <class T>
class TypedObjectHelper<T, 1> : public TypedObjectHelperArith<T> {
    bool equal(void* lhs, void* rhs) override {
        return *static_cast<const T*>(lhs) == *static_cast<const T*>(rhs);
    }
};

template <class T>
class TypedArrayHelper : public ArrayHelper {
    void* call_new(size_t size) override { return new T[size]; }
    void deleter(void* ptr) override { delete[] static_cast<T*>(ptr); }
    void copy(void* src, void* dst, size_t n) override {
        auto* s = static_cast<const T*>(src);
        auto* d = static_cast<T*>(dst);
        while (n--) *d++ = *s++;
    }
    ptrdiff_t distance(void* high, void* low) override {
        return static_cast<const T*>(high) - static_cast<const T*>(low);
    }
    void* advance(void* ptr, size_t n) override {
        return static_cast<void*>(static_cast<T*>(ptr) + n);
    }
    const type_info* type() override { return &typeid(T); }
};

template <class T>
ObjectHelper* OBJECT_HELPER = new TypedObjectHelper<std::decay_t<T>>();
template <class T>
ArrayHelper* ARRAY_HELPER = new TypedArrayHelper<std::decay_t<T>>();
}  // namespace internal
#pragma endregion InternalImpl
}  // namespace typeless

static std::ostream& operator<<(std::ostream& os, const typeless::Object& obj) {
    return os << obj.to_string();
}

static std::ostream& operator<<(std::ostream& os, const std::type_info& info) {
    return os << info.name();
}
#endif