# Modules

Introduced in c++ 20. can use gcc version 13 or greater for it.

Header files have to get parsed every time, and require include guards, and also may have macros that collide with eachother. Also may have a tonne of other include statements that may interfere, and its generally not the cleanest way of managing dependencies. 

Modules are cleaner, they don't involve macros or guards, they can encapsulate things easier, and they can have faster compilation times because they don't do redundant parsing.

You can create a .ixx file which exports a module, and then when compiling, you can simply compile the module once and use it everywhere, unlike headers which have to be compiled individually. However, I don't really see the point if you're using OOP, because it's easy to just expose an interface or abstract class and define your public and private methods, which seems like it would do the same thing. Also, this thing is a pain to compile.

g++ -std=c++20 -fmodules-ts -c maths.ixx -o maths.o will create a gcm.cache, which contains a BMI interface, which sounds like a file containing a binary form of declarations, not definitions. The definitions would be elsewhere,in a .o file, but it didn't work for me, so I was unable to run my simple example.