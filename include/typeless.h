#ifndef TYPELESS_H
#define TYPELESS_H

#ifdef __TYPELESS_TEST
#define __TYPELESS_ACCESS_LEVEL public
#else
#define __TYPELESS_ACCESS_LEVEL private
#endif

#ifndef __TYPELESS_ALLOCATOR
#define __TYPELESS_ALLOCATOR std::allocator
#endif

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <memory>
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
    using StringArray = ArrayInit<string>;

    namespace stringizer {
        using std::to_string;
        inline std::string to_string(const std::string& s);
        inline std::string to_string(const char* c_str);
        inline std::string to_string(const char& ch);
        inline std::string to_string(const Object& obj);
        template <class T>
        std::string to_string(const T&);
    } // namespace stringizer

    namespace internal {
        class ObjectHelper {
        public:
            virtual void* get_allocator() = 0;
            virtual void* allocate() = 0;
            virtual void destroy_deallocate(void* ptr) = 0;
            virtual void* make_copy(const void* src) = 0;
            virtual bool equal(const void* lhs, const void* rhs) = 0;
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
            virtual void* get_allocator() = 0;
            virtual void* allocate(size_t size) = 0;                             // allocate new array with n size
            virtual void destroy_deallocate(void* ptr, size_t n) = 0;            // destroy and deallocate whole array
            virtual void construct(void* ptr, size_t n) = 0;                     // call constructor of elements from [ptr] to [ptr+n]
            virtual void construct(void* ptr, const void* value) = 0;            // construct single element
            virtual void destruct(void* ptr) = 0;                                // destruct single element
            virtual void* make_copy(const void* src, size_t n) = 0;              // make a copy of array [src]
            virtual void* make_copy(const void* src, size_t size, size_t n) = 0; // make a copy of array [src] but only n is copied
            virtual ptrdiff_t distance(const void* high, const void* low) = 0;   // like std::distance
            virtual void* advance(void* ptr, size_t n) = 0;                      // like std::advance
            virtual const void* advance(const void* ptr, size_t n) = 0;          // like std::advance
            virtual const type_info* type() = 0;
        };

        template <class TValue, class TResult>
        void default_join(const TValue& v, TResult& r) {
            r += v;
        }

        template <class T>
        void copy_construct_at(T* p, const T& value);
        template <class T>
        void destroy_at(T* p);
        template <class T>
        void destroy_n(T* p, size_t n);
        template <class Iterator>
        void move(Iterator src, Iterator dst);

        template <class T, class Allocator_ = __TYPELESS_ALLOCATOR<T>>
        ObjectHelper* GetObjectHelper();
        template <class T, class Allocator_ = __TYPELESS_ALLOCATOR<T>>
        ArrayHelper* GetArrayHelper();
    }; // namespace internal

    struct ObjectBase {
        mutable internal::ObjectHelper* helper_;
        mutable void* value_;
    };

    struct ArrayBase {
        mutable internal::ArrayHelper* helper_;
        mutable void* arr_;
        mutable void* end_;
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
        template <class T>
        void set(const T& val);
        /* utilities */
        Object clone() const;
        bool empty() const noexcept;
        void destroy() const noexcept;
        void invalidate() const noexcept;
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
        template <class Iterator>
        Array(Iterator first, Iterator last);
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
        /* setter */
        template <class T>
        void set_type();
        template <class T>
        void set(size_t off, const T& ele);
        /* utilities */
        template <class T, class Callback>
        void for_each(Callback cb) const;
        template <class T, class Fn>
        Array filter(Fn filter_fn) const;
        template <class T, class TResult = T>
        TResult join(void (*cb)(const T&, TResult&) = internal::default_join<T, TResult>);
        bool empty() const noexcept;
        size_t size() const noexcept;
        void resize(size_t new_size);
        void destroy() noexcept;
        void invalidate() noexcept;
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
    inline Object::Object() : ObjectBase{nullptr, nullptr} {
    }
    inline Object::~Object() noexcept {
        destroy();
        invalidate();
    }
    template <class T>
    Object::Object(const T& obj) : ObjectBase() {
        helper_ = internal::GetObjectHelper<T>();
        value_ = helper_->make_copy(&obj);
    }
    inline Object::Object(const Object& rhs) : ObjectBase(rhs) {
        if (helper_ != nullptr) {
            value_ = helper_->make_copy(value_);
        }
    }
    inline Object::Object(Object&& rhs) noexcept : ObjectBase(rhs) {
        rhs.invalidate();
    }

    inline Object& Object::operator=(const Object& rhs) {
        if (this == &rhs) {
            return *this;
        }
        destroy();
        helper_ = rhs.helper_;
        if (helper_ != nullptr) {
            value_ = helper_->make_copy(rhs.value_); // construct an object
        }
        return *this;
    }

    inline Object& Object::operator=(Object&& rhs) noexcept {
        destroy();
        static_cast<ObjectBase&>(*this) = rhs;
        rhs.invalidate();
        return *this;
    }

    template <class T>
    bool operator==(const Object& obj, const T& v) {
        assert(obj.helper_ != nullptr);
        return obj.type() == typeid(T) && obj.get<T>() == v;
    }

    inline bool operator==(const Object& l, const Object& r) {
        assert(l.helper_ != nullptr && r.helper_ != nullptr);
        return l.type() == r.type() && l.helper_->equal(l.value_, r.value_);
    }

    template <class T>
    bool operator!=(const Object& obj, const T& v) {
        return !(obj == v);
    }

    inline bool operator<(const Object& l, const Object& r) {
        if (l.type() != r.type()) {
            return false;
        }
        return l.helper_->less(l.value_, r.value_);
    }

    inline bool operator>(const Object& l, const Object& r) {
        if (l.type() != r.type()) {
            return false;
        }
        return !l.helper_->less(l.value_, r.value_);
    }

    inline bool operator<=(const Object& l, const Object& r) {
        if (l.type() != r.type()) {
            return false;
        }
        return l.helper_->less(l.value_, r.value_) ||
               l.helper_->equal(l.value_, r.value_);
    }

    inline bool operator>=(const Object& l, const Object& r) {
        if (l.type() != r.type()) {
            return false;
        }
        return !l.helper_->less(l.value_, r.value_) ||
               l.helper_->equal(l.value_, r.value_);
    }

    inline Object operator+(const Object& l, const Object& r) {
        if (l.type() != r.type()) {
            return Object();
        }
        return l.helper_->sum(l.value_, r.value_);
    }

    inline Object operator-(const Object& l, const Object& r) {
        if (l.type() != r.type()) {
            return Object();
        }
        return l.helper_->difference(l.value_, r.value_);
    }

    inline Object operator*(const Object& l, const Object& r) {
        if (l.type() != r.type()) {
            return Object();
        }
        return l.helper_->product(l.value_, r.value_);
    }

    inline Object operator/(const Object& l, const Object& r) {
        if (l.type() != r.type()) {
            return Object();
        }
        return l.helper_->quotient(l.value_, r.value_);
    }

    inline void* Object::data() const noexcept { return value_; }

    template <class T>
    T& Object::get() {
        return *static_cast<T*>(value_);
    }

    template <class T>
    T Object::get() const {
        return *static_cast<const T*>(value_);
    }

    template <class T>
    void Object::set(const T& val) {
        if (helper_ != nullptr) {
            helper_->destroy_deallocate(value_);
        }
        helper_ = internal::GetObjectHelper<T>();
        value_ = helper_->make_copy(&val);
    }

    inline Object Object::clone() const { return *this; }

    inline bool Object::empty() const noexcept { return value_ == nullptr; }

    /// \brief call destructor on object
    inline void Object::destroy() const noexcept {
        if (helper_ == nullptr) {
            return;
        }
        helper_->destroy_deallocate(value_);
    }

    /// \brief invalidate object (called after destroy())
    inline void Object::invalidate() const noexcept {
        helper_ = nullptr;
        value_ = nullptr;
    }

    inline void Object::swap(Object& right) noexcept { std::swap(*this, right); }

    inline std::string Object::to_string() const {
        if (helper_ == nullptr) {
            return "null";
        }
        return helper_->to_string(value_);
    }

    template <typename T>
    bool Object::has_type() const noexcept {
        return type() == typeid(T);
    }

    inline const type_info& Object::type() const noexcept {
        if (helper_ == nullptr) {
            return typeid(std::nullptr_t);
        }
        return *helper_->type();
    }

    inline const char* Object::type_name() const noexcept {
        if (helper_ == nullptr) {
            return "void";
        }
        return helper_->type()->name();
    }
#pragma endregion ObjectImpl

#pragma region ArrayImpl
    inline Array::Array() : ArrayBase{nullptr, nullptr, nullptr} {
    }

    template <class T>
    Array::Array(ArrayInit<T> init) : ArrayBase() {
        helper_ = internal::GetArrayHelper<T>();
        auto n = init.size();
        arr_ = helper_->make_copy(init.begin(), n);
        end_ = helper_->advance(arr_, n);
    }

    template <class Iterator>
    Array::Array(Iterator first, Iterator last) : ArrayBase() {
        using T = std::decay_t<decltype(*first)>;
        helper_ = internal::GetArrayHelper<T>();
        auto n = last - first;
        arr_ = helper_->allocate(n);
        end_ = helper_->advance(arr_, n);
        T* p = static_cast<T*>(arr_);
        while(n--) {
            internal::copy_construct_at(p, *first);
            ++first;
            ++p;
        }
    }

    inline Array::Array(const Array& rhs) : ArrayBase(rhs) {
        if (helper_) {
            auto n = helper_->distance(end_, arr_);
            arr_ = helper_->make_copy(arr_, n);
            end_ = helper_->advance(arr_, n);
        }
    }

    inline Array::Array(Array&& rhs) noexcept : ArrayBase(rhs) { rhs.invalidate(); }

    inline Array& Array::operator=(const Array& rhs) {
        if (this == &rhs) {
            return *this;
        }
        destroy();
        helper_ = rhs.helper_;
        if (helper_ != nullptr) {
            auto n = helper_->distance(rhs.end_, rhs.arr_);
            arr_ = helper_->make_copy(rhs.arr_, n);
            end_ = helper_->advance(arr_, n);
        }
        return *this;
    }

    inline Array& Array::operator=(Array&& rhs) noexcept {
        destroy();
        static_cast<ArrayBase&>(*this) = rhs;
        rhs.invalidate();
        return *this;
    }

    inline Array::~Array() noexcept {
        destroy();
        invalidate();
    }

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

    template <class T>
    void Array::set(size_t off, const T& ele) {
        assert(off < size());
        void* ptr = helper_->advance(arr_, off);
        helper_->destruct(ptr);
        internal::copy_construct_at(ptr, ele);
    }

    template <class T>
    void Array::set_type() {
        destroy();
        helper_ = internal::GetArrayHelper<T>();
    }

    template <class T, class Callback>
    void Array::for_each(Callback cb) const {
        if (arr_ == nullptr)
            return;
        T* ptr = static_cast<T*>(arr_);
        while (ptr < end_) {
            cb(*ptr++);
        }
    }

    template <class T, class Fn>
    Array Array::filter(Fn filter_fn) const {
        if (arr_ == nullptr)
            return *this;
        size_t n = size();
        Array filtered;
        filtered.helper_ = helper_;
        filtered.arr_ = filtered.helper_->allocate(n);
        filtered.end_ = filtered.helper_->advance(filtered.arr_, n);

        T* p = static_cast<T*>(arr_);
        T* p_filtered = static_cast<T*>(filtered.arr_);
        while (p != end_) {
            if (filter_fn(*p)) {
                helper_->construct(p_filtered, p);
                ++p_filtered;
            }
            ++p;
        }
        if (p_filtered != filtered.end_) {
            T default_value{};
            do {
                helper_->construct(p_filtered, &default_value);
                ++p_filtered;
            } while (p_filtered != filtered.end_);
        }
        return filtered;
    }

    template <class T, class TResult>
    TResult Array::join(void (*cb)(const T&, TResult&)) {
        if (arr_ == nullptr)
            return {};
        TResult result{};
        T* ptr = static_cast<T*>(arr_);
        while (ptr < end_) {
            cb(*ptr++, result);
        }
        return result;
    }

    inline bool Array::empty() const noexcept { return arr_ == nullptr; }

    inline size_t Array::size() const noexcept {
        return helper_->distance(end_, arr_);
    }

    inline void Array::resize(size_t new_size) {
        if (helper_ == nullptr)
            return;
        void* old_arr = arr_;
        size_t old_size = size();
        arr_ = helper_->make_copy(arr_, new_size, std::min(old_size, new_size));
        end_ = helper_->advance(arr_, new_size);
        helper_->destroy_deallocate(old_arr, old_size); // destroy old content
        if (new_size > old_size) {
            helper_->construct(helper_->advance(arr_, old_size),
                               new_size - old_size);
        }
    }

    /// \brief  Destroy all elements in the array and release memory.
    ///         Type data is not erased.
    ///         Calling invalidate() is needed,
    ///         otherwise double free will occur when destructor is called
    inline void Array::destroy() noexcept {
        if (helper_ == nullptr) {
            return;
        }
        helper_->destroy_deallocate(arr_, size());
    }

    inline void Array::invalidate() noexcept {
        helper_ = nullptr;
        arr_ = end_ = nullptr;
    }

    inline void Array::swap(Array& right) noexcept { std::swap(*this, right); }

    inline const type_info& Array::type() const noexcept {
        if (helper_ == nullptr) {
            return typeid(nullptr);
        }
        return *helper_->type();
    }

    inline const char* Array::type_name() const noexcept {
        if (helper_ == nullptr) {
            return "null";
        }
        return helper_->type()->name();
    }

    inline void* Array::begin() noexcept { return arr_; }
    inline void* Array::end() noexcept { return end_; }
    inline const void* Array::cbegin() const noexcept { return arr_; }
    inline const void* Array::cend() const noexcept { return end_; }
#pragma endregion ArrayImpl

#pragma region StringizerImpl

    inline std::string stringizer::to_string(const std::string& s) { return s; }
    inline std::string stringizer::to_string(const char* c_str) { return c_str; }
    inline std::string stringizer::to_string(const char& ch) { return {ch}; }
    inline std::string stringizer::to_string(const Object& obj) { return obj.to_string(); }
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
        struct HasOperatorEqual : HasOperatorEqualImpl<T, EqualTo>::type {
        };

        template <class T, typename std::enable_if_t<HasOperatorEqual<T>::value, int> = 0>
        bool EqualHelper(const void* a, const void* b) {
            return *static_cast<const T*>(a) == *static_cast<const T*>(b);
        }

        template <class T, typename std::enable_if_t<!HasOperatorEqual<T>::value, int> = 0>
        bool EqualHelper(const void* a, const void* b) {
            return false;
        }

        template <class T, class Allocator_>
        class TypedObjectHelperBase : public ObjectHelper {
            Allocator_ allocator;
            void* get_allocator() override { return &allocator; }
            void* allocate() override { return allocator.allocate(1); }
            void destroy_deallocate(void* ptr) override {
                internal::destroy_at(static_cast<T*>(ptr));
                allocator.deallocate(static_cast<T*>(ptr), 1);
            }
            void* make_copy(const void* src) override {
                T* ptr = allocator.allocate(1);
                internal::copy_construct_at(ptr, *static_cast<const T*>(src));
                return ptr;
            }
            bool equal(const void* lhs, const void* rhs) override { return false; }
            string to_string(const void* ptr) override {
                return stringizer::to_string(*static_cast<const T*>(ptr));
            }
            const type_info* type() override { return &typeid(T); }
            std::runtime_error exception() {
                return std::runtime_error(
                    string("Attempt to call arithmetic operand on non-arithmetic type ") +
                    type()->name());
            }
            bool less(const void*, const void*) override { throw exception(); }
            Object sum(const void* a, const void* b) override { throw exception(); }
            Object difference(const void* a, const void* b) override {
                throw exception();
            }
            Object product(const void* a, const void* b) override { throw exception(); }
            Object quotient(const void* a, const void* b) override {
                throw exception();
            }
        };

        template <class T, class Allocator_, int = std::is_arithmetic<T>::value>
        class TypedObjectHelperArith;

        template <class T, class Allocator_>
        class TypedObjectHelperArith<T, Allocator_, 0>
            : public TypedObjectHelperBase<T, Allocator_> {
        };

        template <class T, class Allocator_>
        class TypedObjectHelperArith<T, Allocator_, 1>
            : public TypedObjectHelperBase<T, Allocator_> {
            inline static const T& val(const void* v) {
                return *static_cast<const T*>(v);
            }
            bool less(const void* a, const void* b) override { return val(a) < val(b); }
            Object sum(const void* a, const void* b) override {
                return {val(a) + val(b)};
            }
            Object difference(const void* a, const void* b) override {
                return {val(a) - val(b)};
            }
            Object product(const void* a, const void* b) override {
                return {val(a) * val(b)};
            }
            Object quotient(const void* a, const void* b) override {
                return {val(a) / val(b)};
            }
        };

        template <class T, class Allocator_, int = HasOperatorEqual<T>::value>
        class TypedObjectHelper;

        template <class T, class Allocator_>
        class TypedObjectHelper<T, Allocator_, 0>
            : public TypedObjectHelperArith<T, Allocator_> {
        };

        template <class T, class Allocator_>
        class TypedObjectHelper<T, Allocator_, 1>
            : public TypedObjectHelperArith<T, Allocator_> {
            bool equal(const void* lhs, const void* rhs) override {
                return *static_cast<const T*>(lhs) == *static_cast<const T*>(rhs);
            }
        };

        template <class T, class Allocator_>
        class TypedArrayHelper : public ArrayHelper {
            Allocator_ allocator;
            void* get_allocator() override { return &allocator; }

            void* allocate(size_t size) override { return allocator.allocate(size); }

            void destroy_deallocate(void* ptr, size_t n) override {
                T* _ptr = static_cast<T*>(ptr);
                internal::destroy_n(_ptr, n);
                allocator.deallocate(_ptr, n);
            }

            void construct(void* ptr, size_t n) override {
                T* begin = static_cast<T*>(ptr);
                const T* end = begin + n;
                T default_value{};
                while (begin != end) {
                    internal::copy_construct_at(begin, default_value);
                    ++begin;
                }
            }

            void construct(void* ptr, const void* value) override {
                internal::copy_construct_at(static_cast<T*>(ptr),
                                            *static_cast<const T*>(value));
            }

            void destruct(void* ptr) override {
                internal::destroy_at(static_cast<T*>(ptr));
            }

            void* make_copy(const void* src, size_t n) override {
                void* arr = allocate(n);
                void* dst = arr;
                while (n--) {
                    construct(dst, src);
                    src = advance(src, 1);
                    dst = advance(dst, 1);
                }
                return arr;
            }
            void* make_copy(const void* src, size_t size, size_t n) override {
                void* arr = allocate(size);
                void* dst = arr;
                size -= n;
                while (n--) {
                    construct(dst, src);
                    src = advance(src, 1);
                    dst = advance(dst, 1);
                }
                if (size > 0) { // construct default value for elements beyond [n]
                    construct(dst, size);
                }
                return arr;
            }
            ptrdiff_t distance(const void* high, const void* low) override {
                return static_cast<const T*>(high) - static_cast<const T*>(low);
            }
            void* advance(void* ptr, size_t n) override {
                return static_cast<void*>(static_cast<T*>(ptr) + n);
            }
            const void* advance(const void* ptr, size_t n) override {
                return static_cast<const void*>(static_cast<const T*>(ptr) + n);
            }
            const type_info* type() override { return &typeid(T); }
        };

        template <class T, class Allocator_>
        ObjectHelper* OBJECT_HELPER = new TypedObjectHelper<T, Allocator_>();

        template <class T, class Allocator_>
        ArrayHelper* ARRAY_HELPER = new TypedArrayHelper<T, Allocator_>();

        template <class T>
        void copy_construct_at(T* p, const T& value) {
            ::new (p) T(value);
        }

        template <class T>
        void destroy_at(T* p) {
            p->~T();
        }

        template <class T>
        void destroy_n(T* p, size_t n) {
            for (const T* end = p + n; p != end; ++p)
                internal::destroy_at(p);
        }

        template <class Iterator>
        void move(Iterator src, Iterator dst) {
            using T = std::decay_t<decltype(*src)>;
            new (src) T(std::move(*dst));
        }

        template <class T, class Allocator_>
        ObjectHelper* GetObjectHelper() {
            return OBJECT_HELPER<T, Allocator_>;
        }
        template <class T, class Allocator_>
        ArrayHelper* GetArrayHelper() {
            return ARRAY_HELPER<T, Allocator_>;
        }
    } // namespace internal
#pragma endregion InternalImpl
} // namespace typeless

static std::ostream& operator<<(std::ostream& os, const typeless::Object& obj) {
    return os << obj.to_string();
}

static std::ostream& operator<<(std::ostream& os, const std::type_info& info) {
    return os << info.name();
}
#endif
// TODO: add operator+ for generic types
// TODO: add push_back, erase
// TODO: add TypedArray conversion