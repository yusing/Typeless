#include <typeless.h>
using namespace typeless;

void CharArrayToString() {
    auto arr = Array{'H','e','l','l','o'};
    auto str = arr.join<char, std::string>(); // Hello
    std::cout << str << std::endl;
}

void SumOfEvenNumbers() {
    auto arr = Array{1,2,3,4,5,6,7,8,9};
    auto sum = arr.filter<int>([](int i) { return i % 2 == 0; }).join<int>(); // 20
    std::cout << sum << std::endl;
}

void Print(const Array& args) { // print arguments with different types
    args.for_each<Object>([] (const Object& obj) {
        std::cout << obj.to_string();
    });
}

int main() {
    CharArrayToString();
    SumOfEvenNumbers();
    Print(ObjectArray{123, '+', 456, string(" = "), 123+456});
}