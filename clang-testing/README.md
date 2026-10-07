# cout2log

A small Clang LibTooling tool that rewrites `std::cout` chains into `logging::log` calls with `{}` placeholders.

```cpp
std::cout << "name=" << name << ", size=" << v.size() << std::endl;
// becomes
logging::log("name={}, size={}", name, v.size());
```

## Build

Needs the Clang/LLVM development libraries (tested with LLVM 18).

```sh
# Ubuntu/Debian: sudo apt install libclang-18-dev llvm-18-dev cmake ninja-build
cmake -S . -B build -G Ninja -DClang_DIR=/usr/lib/llvm-18/lib/cmake/clang
cmake --build build
```

On macOS with Homebrew, use `-DClang_DIR=$(brew --prefix llvm)/lib/cmake/clang`.

## Use

Your project needs a `compile_commands.json` (configure it with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`).

```sh
cout2log -p build src/foo.cpp           # preview the changes
cout2log -p build --apply src/foo.cpp   # write them
```

Afterwards, add your logging header to the changed files and rebuild.

## What it handles

- Chains spread across several lines, with variables and text interleaved
- `std::endl` and `"\n"`; a trailing newline is dropped, since the logger adds one
- Braces in text, which are escaped to `{{` and `}}`
- `using namespace std`, templates, lambdas, and macros used as values

It leaves these for you to convert by hand: stream manipulators (`std::hex`, `std::setw`, ...), chains written inside macros, and chains whose result is used, such as `if (std::cout << x)`.

Running `grep -rn "std::cout"` afterwards shows what's left.

## Known differences

- `bool` prints as `1`/`0` with iostream but `true`/`false` with `{}`.
- Types that only have an `operator<<` need a formatter for your logger.
- A `cout` with no newline that the next `cout` continues will be split onto two lines.
