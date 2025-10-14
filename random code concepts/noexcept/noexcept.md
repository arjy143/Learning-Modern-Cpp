# noexcept

This tells the compiler not to throw exceptions in this function, which is beneficial for performance since the compiler does not need to do certain checks or include its own code for exception handling.

This affects exception propagation, important in destructors and move operations.

Also it means that programs will not terminate when exceptions escape from a noexcept function, instead calling std::terminate.

interestingly the compiler will warn you when compiling a noexcept function that throws, but it wont warn you when you make a function that throws, then call that function inside a noexcept function.

Also you can make a noexcept conditional on the outcome of another function, so a noexcept function can still possibly throw exceptions.

noexcept can be useful with move constructors and destructors. However, its still nice to use exceptions if you need to propagate something up the stack.