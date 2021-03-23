#ifndef INTERNAL_TEST_H
#define INTERNAL_TEST_H
#include <gtest/gtest.h>

template <typename T>
class TestAllocator {
    T placeholder_[100];

public:
    size_t size_allocated = 0;
    T* allocate(size_t n) {
        size_allocated += n;
        return placeholder_;
    }
    void deallocate(T*, size_t n) {
        size_allocated -= n;
    }
    void reset() {
        size_allocated = 0;
    }
};

#define __TYPELESS_ALLOCATOR TestAllocator
#include <typeless.h>

using namespace typeless;
using namespace typeless::internal;

TEST(ObjectHelper, AllocDealloc) {
    ObjectHelper* helper = GetObjectHelper<int>();
    TestAllocator<int>* allocator = static_cast<TestAllocator<int>*>(helper->get_allocator());
    void* ptr = helper->allocate();
    EXPECT_EQ(allocator->size_allocated, 1);
    helper->destroy_deallocate(ptr);
    EXPECT_EQ(allocator->size_allocated, 0);
    allocator->reset();
}

TEST(ObjectHelper, MakeCopy) {
    ObjectHelper* helper = GetObjectHelper<int>();
    TestAllocator<int>* allocator = static_cast<TestAllocator<int>*>(helper->get_allocator());
    int i = 12345;
    void* ptr = helper->make_copy(&i);
    EXPECT_EQ(allocator->size_allocated, 1);
    EXPECT_EQ(*static_cast<int*>(ptr), i);
    helper->destroy_deallocate(ptr);
    EXPECT_EQ(allocator->size_allocated, 0);
    allocator->reset();
}

TEST(ObjectHelper, Equal) {
    ObjectHelper* helper = GetObjectHelper<int>();
    int a = 123, b = 123, c = 456;
    EXPECT_TRUE(helper->equal(&a, &b));
    EXPECT_FALSE(helper->equal(&a, &c));
    EXPECT_FALSE(helper->equal(&b, &c));
}

TEST(ObjectHelper, ToString) {
    ObjectHelper* helper = GetObjectHelper<int>();
    int i = 123456;
    EXPECT_EQ(helper->to_string(&i), string("123456"));
}

TEST(ObjectHelper, Type) {
    ObjectHelper* helper = GetObjectHelper<int>();
    EXPECT_STREQ(helper->type()->name(), typeid(int).name());
}

TEST(ObjectHelper, Arithmetic) {
    ObjectHelper* helper = GetObjectHelper<int>();
    int a = 100, b = 200;
    EXPECT_TRUE(helper->less(&a, &b));
    EXPECT_EQ(helper->sum(&a, &b).get<int>(), 300);
    EXPECT_EQ(helper->difference(&a, &b).get<int>(), -100);
    EXPECT_EQ(helper->product(&a, &b).get<int>(), 20000);
    EXPECT_EQ(helper->quotient(&a, &b).get<int>(), 0);
    EXPECT_EQ(helper->quotient(&b, &a).get<int>(), 2);
}

TEST(ArrayHelper, AllocDealloc) {
    ArrayHelper* helper = GetArrayHelper<int>();
    TestAllocator<int>* allocator = static_cast<TestAllocator<int>*>(helper->get_allocator());
    void* ptr = helper->allocate(100);
    EXPECT_EQ(allocator->size_allocated, 100);
    helper->destroy_deallocate(ptr, 100);
    EXPECT_EQ(allocator->size_allocated, 0);
    allocator->reset();
}

TEST(ArrayHelper, ConstructDestruct) {
    ArrayHelper* helper = GetArrayHelper<int>();
    TestAllocator<int>* allocator = static_cast<TestAllocator<int>*>(helper->get_allocator());
    void* ptr = helper->allocate(100);
    helper->construct(ptr, allocator->size_allocated);
    int *it = static_cast<int*>(ptr), *const end = it + 100;
    while (it != end) {
        EXPECT_EQ(*it, 0);
        ++it;
    }
    int value = 123;
    helper->construct(ptr, &value);
    EXPECT_EQ(*static_cast<int*>(ptr), value);
    helper->destroy_deallocate(ptr, 100);
    EXPECT_EQ(allocator->size_allocated, 0);
    allocator->reset();
}

TEST(ArrayHelper, MakeCopy) {
    ArrayHelper* helper = GetArrayHelper<int>();
    int arr[100];
    for (int i = 0; i < 100; ++i) {
        arr[i] = i;
    }
    int* copied_arr = static_cast<int*>(helper->make_copy(arr, 100));
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(copied_arr[i], i);
    }
}

TEST(ArrayHelper, Distance) {
    ArrayHelper* helper = GetArrayHelper<int>();
    int a, b;
    EXPECT_EQ(helper->distance(&a, &b), &a - &b);
}

TEST(ArrayHelper, Advance) {
    ArrayHelper* helper = GetArrayHelper<int>();
    int a;
    const int b{};
    EXPECT_EQ(helper->advance(&a, 100), &a + 100);
    EXPECT_EQ(helper->advance(&b, 100), &b + 100);
}

TEST(ArrayHelper, Type) {
    ArrayHelper* helper = GetArrayHelper<int>();
    EXPECT_STREQ(helper->type()->name(), typeid(int).name());
}

struct EqualOperatorTester {
    friend bool operator==(EqualOperatorTester, int) { return true; }
};

TEST(HasOperatorEqual, value) {
    EXPECT_TRUE((HasOperatorEqual<int, int>::value));
    EXPECT_TRUE((HasOperatorEqual<int, float>::value));
    EXPECT_TRUE((HasOperatorEqual<EqualOperatorTester, int>::value));
    EXPECT_TRUE((HasOperatorEqual<EqualOperatorTester, EqualOperatorTester>::value));
    EXPECT_FALSE((HasOperatorEqual<EqualOperatorTester, float>::value));
}
#endif