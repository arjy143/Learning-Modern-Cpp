cmake_minimum_required(VERSION 3.20)
project(cout2log CXX)

find_package(Clang REQUIRED CONFIG)

add_executable(cout2log cout2log.cpp)
target_include_directories(cout2log PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
target_compile_options(cout2log PRIVATE -std=c++17 -fno-rtti)

# Most installs ship one shared clang-cpp library; otherwise link the parts.
if(TARGET clang-cpp)
  target_link_libraries(cout2log PRIVATE clang-cpp)
else()
  target_link_libraries(cout2log PRIVATE clangTooling clangToolingCore
    clangFrontend clangASTMatchers clangAST clangLex clangBasic clangRewrite)
endif()
if(TARGET LLVM)
  target_link_libraries(cout2log PRIVATE LLVM)
else()
  target_link_libraries(cout2log PRIVATE LLVMSupport LLVMOption LLVMFrontendOpenMP)
endif()

# Clang's own headers (stddef.h etc.), which the tool needs to parse code.
set(RESOURCE_DIR "${LLVM_LIBRARY_DIR}/clang/${LLVM_VERSION_MAJOR}")
target_compile_definitions(cout2log PRIVATE COUT2LOG_RESOURCE_DIR="${RESOURCE_DIR}")
