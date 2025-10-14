#include <iostream>
#include <stdexcept>

void willThrow()
{
    throw std::runtime_error("runtime errror");
}

void neverThrows() noexcept 
{
    std::cout << "noexcept function"<< "\n";
    throw std::runtime_error("runtime errror");
}

class Test
{
    public:
        Test() = default;
        Test(const Test&) 
        {
            std::cout << "copy constructor" << "\n";
        }
        Test(Test&&) noexcept 
        {
            std::cout << "move constructor with noexcept" << "\n";
        }

        // ~Test()
        // {
        //     std::cout << "destructor" << "\n";
        // }

};

int main(int argc, char** argv)
{

    {Test test;
    Test test2 = std::move(test);}
    try
    {
        neverThrows();
    }
    catch(const std::exception& e)
    {
        //can't catch exceptions thrown from noexcept function, std::terminate will always be called, ending the program.
        std::cerr << "caught exception in noexcept function "<< e.what() << '\n';
    }
    

    neverThrows(); 
    //willThrow();
}