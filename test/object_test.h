#ifndef OBJECT_TEST_H
#define OBJECT_TEST_H
#include <gtest/gtest.h>
#include <typeless.h>

using namespace typeless;

TEST(ObjectTest, Initialization) {
    Object obj(123);
    EXPECT_NE(obj.value_, nullptr);
    EXPECT_NE(obj.helper_, nullptr);
    obj = string("123");
    EXPECT_NE(obj.value_, nullptr);
    EXPECT_NE(obj.helper_, nullptr);
}

TEST(ObjectTest, Destruction) {
    Object obj(123);
    obj.~Object();
    EXPECT_EQ(obj.value_, nullptr);
    EXPECT_EQ(obj.helper_, nullptr);
}

TEST(ObjectTest, Copy) {
    Object str_obj{string("Some text...")};
    Object copied_obj(str_obj);
    EXPECT_FALSE(copied_obj.empty());
    EXPECT_NE(str_obj.value_, copied_obj.value_);
    EXPECT_EQ(
        str_obj.get<string>(),
        copied_obj.get<string>());
    EXPECT_EQ(copied_obj.type(), typeid(string));
}

TEST(ObjectTest, Move) {
    Object obj(123);
    Object move_to = std::move(obj);
    EXPECT_EQ(obj.value_, nullptr);
    EXPECT_EQ(obj.helper_, nullptr);
    EXPECT_TRUE(obj.empty());
    EXPECT_FALSE(move_to.empty());
    EXPECT_EQ(move_to, 123);
}

TEST(ObjectTest, Type) {
    Object str_obj{string("Hello World!")};
    Object float_obj{123.f};
    EXPECT_EQ(str_obj.type(), typeid(string));
    EXPECT_EQ(float_obj.type(), typeid(float));
}

TEST(ObjectTest, ValueValidity) {
    Object str_vector(std::vector<int>{0, 1, 2, 3, 4, 5, 6});
    Object clone = str_vector.clone();
    EXPECT_TRUE(str_vector.has_type<std::vector<int>>());
    EXPECT_TRUE(clone.has_type<std::vector<int>>());
    int i = 0;
    for (int ele : str_vector.get<const std::vector<int>>()) {
        EXPECT_EQ(ele, i);
        ++i;
    }
    i = 0;
    for (int ele : clone.get<const std::vector<int>>()) {
        EXPECT_EQ(ele, i);
        ++i;
    }
    EXPECT_NE(str_vector.data(), clone.data());
}

TEST(ObjectTest, Set) {
    Object obj;
    obj.set(string(5, 'A'));
    EXPECT_EQ(obj.get<string>(), "AAAAA");
}

TEST(ObjectTest, Swapping) {
    Object str_obj{string("Hello World!")};
    Object int_obj{123};
    Object empty;
    EXPECT_EQ(str_obj, string("Hello World!"));
    EXPECT_EQ(int_obj, 123);
    str_obj.swap(int_obj);
    EXPECT_EQ(int_obj, string("Hello World!"));
    EXPECT_EQ(str_obj, 123);
    str_obj.swap(int_obj);
    str_obj.swap(empty);
    EXPECT_FALSE(empty.empty());
    EXPECT_TRUE(str_obj.empty());
    EXPECT_EQ(empty, string("Hello World!"));
}

TEST(ObjectTest, Arithmetic) {
    Object i1 = 123, i2 = 456;
    EXPECT_EQ(i1, 123);
    EXPECT_FALSE(i1 == i2);
    EXPECT_TRUE(i1 != i2);
    EXPECT_EQ(i1 + i2, 579);
    EXPECT_EQ(i1 - i2, -333);
    EXPECT_EQ(i1 * i2, 56088);
    EXPECT_EQ(i2 / i1, 3);
    EXPECT_TRUE(i1 < i2);
    EXPECT_TRUE(i1 <= i2);
    EXPECT_TRUE(i2 > i1);
    EXPECT_TRUE(i2 >= i1);
}

TEST(ObjectTest, ArithmeticOp_On_NonArithmetic_Type) {
    Object obj1, obj2;
    obj1 = obj2 = string("");
    EXPECT_ANY_THROW(obj1 + obj2);
    EXPECT_ANY_THROW(obj1 - obj2);
    EXPECT_ANY_THROW(obj1 * obj2);
    EXPECT_ANY_THROW(obj1 / obj2);
    EXPECT_ANY_THROW(obj1 < obj2);
    EXPECT_ANY_THROW(obj1 > obj2);
    EXPECT_ANY_THROW(obj1 <= obj2);
    EXPECT_ANY_THROW(obj1 >= obj2);
}

TEST(ObjectTest, ToString) {
    Object obj(123456789);
    EXPECT_EQ(obj.to_string(), "123456789");
    obj = 456789.0;
    EXPECT_EQ(obj.to_string(), "456789.000000");
    obj = string("Hello World!");
    EXPECT_EQ(obj.to_string(), "Hello World!");
    obj = (const char*)"C-Style string";
    EXPECT_EQ(obj.to_string(), "C-Style string");
    obj = std::vector<int>();
    EXPECT_EQ(obj.to_string(), typeid(std::vector<int>).name());
    obj = 'A';
    EXPECT_EQ(obj.to_string(), "A");
}
#endif