#ifndef ARRAY_TEST_H
#define ARRAY_TEST_H
#include <gtest/gtest.h>
#include <numeric>
#include <typeless.h>


using namespace typeless;

TEST(ArrayTest, Initialization) {
    Array str_arr = StringArray{"foo", "bar"}; // StringArray = std::initializer_list<std::string>
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

TEST(ArrayTest, IteratorRangeInit) {
    std::vector<string> v{"ab", "cd", "ef", "gh", "ij"};
    Array a(v.begin(), v.end());
    EXPECT_NE(a.arr_, nullptr);
    EXPECT_NE(a.end_, nullptr);
    EXPECT_NE(a.helper_, nullptr);
    EXPECT_EQ(a.size(), v.size());
    for(size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(v[i], a.at<string>(i));
    }
}

TEST(ArrayTest, Destruction) {
    Array arr{1, 2, 3};
    arr.~Array();
    EXPECT_EQ(arr.arr_, nullptr);
    EXPECT_EQ(arr.end_, nullptr);
    EXPECT_EQ(arr.helper_, nullptr);
}

TEST(ArrayTest, Move) {
    Array arr{1, 2, 3, 4};
    Array move_to = std::move(arr);
    EXPECT_TRUE(arr.empty());
    EXPECT_FALSE(move_to.empty());
    EXPECT_EQ(arr.arr_, nullptr);
    EXPECT_EQ(arr.end_, nullptr);
    EXPECT_EQ(arr.helper_, nullptr);
    EXPECT_EQ(move_to.size(), 4);
    EXPECT_EQ(move_to.join<int>(), 10); // sum
}

TEST(ArrayTest, Copy) {
    Array str_arr = StringArray{"foo", "bar"};
    Array copy(str_arr);
    EXPECT_EQ(str_arr.type(), copy.type());
    EXPECT_EQ(str_arr.join<string>(), copy.join<string>());
}

TEST(ArrayTest, Type) {
    Array int_arr{1, 2, 3};
    Array char_arr{'f', 'o', 'o'};
    EXPECT_EQ(int_arr.type(), typeid(int));
    EXPECT_EQ(char_arr.type(), typeid(char));
}

TEST(ArrayTest, Join) {
    Array a1{'f', 'o', 'o', ' ', 'b', 'a', 'r'};
    Array a2 = StringArray{"foo", " ", "bar"};
    auto joined = a1.join<char, string>();
    EXPECT_EQ(joined, "foo bar");
    EXPECT_EQ(a2.join<string>(), joined);
}

TEST(ArrayTest, Swap) {
    Array str_arr = StringArray{"foo", "bar"};
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
    arr = StringArray{"Hello"};
    arr.resize(3);
    arr.at<string>(1) = "World";
    arr.at<string>(2) = "!";
    EXPECT_EQ(arr.join<string>(), "HelloWorld!");
}

TEST(ArrayTest, ResizeShrink) {
    Array arr = StringArray{"Hello", " ", "World", "!"};
    arr.resize(2);
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.join<string>(), "Hello ");
}

int tester_constructor_called = 0;
int tester_destructor_called = 0;

class ArrayTester {
public:
    ArrayTester() {
        ++tester_constructor_called;
    }
    ~ArrayTester() {
        ++tester_destructor_called;
    }
};

TEST(ArrayTest, ConstructionDestruction) {
    Array arr(ArrayInit<ArrayTester>{{}, {}, {}, {}, {}});
    EXPECT_EQ(arr.size(), 5);
    EXPECT_EQ(tester_constructor_called, 5);
    arr.~Array();
    EXPECT_EQ(tester_destructor_called, 10); // 5 for initializer_list and 5 for array
}
#endif