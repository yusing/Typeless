#include <gtest/gtest.h>
#include <typeless.h>
#include <numeric>

using namespace typeless;

TEST(ObjectTest, InitializationTest) {
    Object obj(123);
    EXPECT_NE(obj.value_, nullptr);
    EXPECT_NE(obj.helper_, nullptr);
    obj = string("123");
    EXPECT_NE(obj.value_, nullptr);
    EXPECT_NE(obj.helper_, nullptr);
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

TEST(ObjectTest, Emplace) {
    Object obj;
    obj.emplace<string>(5, 'A');
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

TEST(ObjectTest, Copy) {
    Object str_obj{string("Some text...")};
    Object copied_obj(str_obj);  // or copied_obj = str_obj
    EXPECT_EQ(
        str_obj.get<string>(),
        copied_obj.get<string>());   // check equality with string::operator==
    EXPECT_EQ(str_obj, copied_obj);  // check equality with helper_.equal
    EXPECT_EQ(copied_obj.type(), typeid(string));
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

TEST(ArrayTest, InitializationTest) {
    Array str_arr{string("foo"), string("bar")};
    Array int_arr{1, 2, 3};
    EXPECT_NE(str_arr.arr_, nullptr);
    EXPECT_NE(str_arr.end_, nullptr);
    EXPECT_NE(str_arr.helper_, nullptr);
    EXPECT_NE(int_arr.arr_, nullptr);
    EXPECT_NE(int_arr.end_, nullptr);
    EXPECT_NE(int_arr.helper_, nullptr);

    EXPECT_EQ(str_arr.size(), 2);
    EXPECT_EQ(int_arr.size(), 3);

    EXPECT_EQ(str_arr.at<string>(0), "foo");
    EXPECT_EQ(str_arr.at<string>(1), "bar");
    EXPECT_EQ(int_arr.at<int>(0), 1);
    EXPECT_EQ(int_arr.at<int>(1), 2);
    EXPECT_EQ(int_arr.at<int>(2), 3);
}

TEST(ArrayTest, Type) {
    Array int_arr{1, 2, 3};
    Array char_arr{'f', 'o', 'o'};
    EXPECT_EQ(int_arr.type(), typeid(int));
    EXPECT_EQ(char_arr.type(), typeid(char));
}

TEST(ArrayTest, Join) {
    Array a1{'f', 'o', 'o', ' ', 'b', 'a', 'r'};
    Array a2 = ArrayInit<string>{"foo", " ", "bar"};
    auto joined = a1.join<char, string>();
    EXPECT_EQ(joined, "foo bar");
    EXPECT_EQ(a2.join<string>(), joined);
}

TEST(ArrayTest, Swap) {
    Array str_arr = ArrayInit<string>{"foo", "bar"};
    Array int_arr{1, 2, 3};
    Array empty;
    str_arr.swap(int_arr);
    EXPECT_EQ(int_arr.at<string>(1), "bar");
    EXPECT_EQ(str_arr.at<int>(2), 3);
    str_arr.swap(empty);
    EXPECT_TRUE(str_arr.empty());
    EXPECT_FALSE(empty.empty());
    EXPECT_EQ(empty.type(), typeid(int));
}

TEST(ArrayTest, Copy) {
    Array str_arr = ArrayInit<string>{"foo", "bar"};
    Array copy(str_arr);
    EXPECT_EQ(str_arr.type(), copy.type());
    EXPECT_EQ(str_arr.join<string>(), copy.join<string>());
}

TEST(ArrayTest, ForEach) {
    int sum = 0;
    Array{1, 2, 3, 4}.for_each<int>([&](int i) { sum += i; });
    EXPECT_EQ(sum, 10);
}

TEST(ArrayTest, Iterator) {
    auto arr = Array{1, 2, 3, 4};
    EXPECT_EQ(std::accumulate<int*>(static_cast<int*>(arr.begin()),
                                    static_cast<int*>(arr.end()), 0),
              10);
}

TEST(ArrayTest, Filter) {
    auto arr = Array{1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto sum = arr.filter<int>([](int i) { return i % 3 == 0; }).join<int>();
    EXPECT_EQ(sum, 18);
}

TEST(ArrayTest, ResizeGrow) {
    auto arr = Array{1, 2, 3};
    arr.resize(5);
    arr.at<int>(3) = 0;
    arr.at<int>(4) = 0;
    EXPECT_EQ(arr.size(), 5);
    EXPECT_EQ(arr.join<int>(), 6);
    arr = ArrayInit<string>{"Hello"};
    arr.resize(3);
    arr.at<string>(1) = "World";
    arr.at<string>(2) = "!";
    EXPECT_EQ(arr.join<string>(), "HelloWorld!");
}

TEST(ArrayTest, ResizeShrink) {
    Array arr = ArrayInit<string>{"Hello", " ", "World", "!"};
    arr.resize(2);
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.join<string>(), "Hello ");
}

class ArrayTester {
   public:
    enum class Which {
        None,
        Constructor,
        CopyConstructor,
        MoveConstructor,
        CopyAssign,
        MoveAssign
    } last_called;
    ArrayTester() noexcept : last_called(Which::None){};
    explicit ArrayTester(int) noexcept : last_called(Which::Constructor) {}
    ArrayTester(const ArrayTester&) noexcept
        : last_called(Which::CopyConstructor) {}
    ArrayTester(ArrayTester&&) noexcept : last_called(Which::MoveConstructor) {}
    ArrayTester& operator=(const ArrayTester&) noexcept {
        last_called = Which::CopyAssign;
        return *this;
    }
    ArrayTester& operator=(ArrayTester&&) noexcept {
        last_called = Which::MoveAssign;
        return *this;
    }
} tester;

TEST(ArrayTest, SetElement) {
    auto arr = Array{tester};
    arr.set(0, tester);
    EXPECT_EQ(arr.at<ArrayTester>(0).last_called,
              ArrayTester::Which::CopyAssign);
}

TEST(ArrayTest, MoveConstructElement) {
    auto arr = Array{tester};
    arr.emplace<ArrayTester>(0, std::move(tester));
    EXPECT_EQ(arr.at<ArrayTester>(0).last_called,
              ArrayTester::Which::MoveConstructor);
}

TEST(ArrayTest, EmplaceElement) {
    auto arr = Array{tester};
    arr.emplace<ArrayTester>(0, 1);
    EXPECT_EQ(arr.at<ArrayTester>(0).last_called,
              ArrayTester::Which::Constructor);
}