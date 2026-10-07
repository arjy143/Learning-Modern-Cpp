cmake_minimum_required(VERSION 3.20)
project(codemap CXX)

find_package(Clang REQUIRED CONFIG)

add_executable(codemap codemap.cpp)
target_include_directories(codemap PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
target_compile_options(codemap PRIVATE -std=c++17 -fno-rtti)

# Most installs ship one shared clang-cpp library; otherwise link the parts.
if(TARGET clang-cpp)
  target_link_libraries(codemap PRIVATE clang-cpp)
else()
  target_link_libraries(codemap PRIVATE clangTooling clangFrontend clangLex clangBasic)
endif()
if(TARGET LLVM)
  target_link_libraries(codemap PRIVATE LLVM)
else()
  target_link_libraries(codemap PRIVATE LLVMSupport LLVMOption LLVMFrontendOpenMP)
endif()

# Clang's own headers (stddef.h etc.), which the preprocessor needs.
set(RESOURCE_DIR "${LLVM_LIBRARY_DIR}/clang/${LLVM_VERSION_MAJOR}")
target_compile_definitions(codemap PRIVATE CODEMAP_RESOURCE_DIR="${RESOURCE_DIR}")
