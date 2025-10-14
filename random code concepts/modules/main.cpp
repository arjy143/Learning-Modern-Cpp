import maths;

#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "add " << add(1,4) << "\n";
    std::cout << "mult " << multiply(2,4) << "\n";
    return 0;
}

//using  g++ -std=c++20 -fmodules-ts maths.ixx main.cpp -o main to compile it, but failing