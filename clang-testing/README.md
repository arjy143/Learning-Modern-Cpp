# cout2log

A Clang LibTooling tool that rewrites `std::cout` chains into calls to a `{}`-style logging function.

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
cout2log -p build src/foo.cpp           # preview the changes to one file
cout2log -p build                       # preview every file in compile_commands.json
cout2log -p build --apply src/foo.cpp   # write the changes
```

Afterwards, add your logging header to the changed files and rebuild.

| Option | Effect |
|---|---|
| `--apply` | Write the changes. Without it, the tool only prints them. |
| `--function=NAME` | Function to call instead of `logging::log`, e.g. `--function=spdlog::info` |
| `--stream=NAME` or `--stream=NAME=FUNCTION` | Which streams to convert (repeatable). Default is `cout`. e.g. `--stream=cout --stream=cerr=logging::error` |
| `--keep-newline` | Keep a trailing `\n`/`std::endl` in the format string, for functions that don't add one (such as `fmt::print`) |
| `--include-headers` | Also rewrite your own headers, not just the `.cpp` files |

## What it handles

- Chains spread across several lines, with variables and text interleaved
- `std::endl` and `"\n"`; by default a trailing newline is dropped, since most loggers add one
- Braces in text, which are escaped to `{{` and `}}`
- `using namespace std`, templates, lambdas, and macros used as values

It never rewrites code inside the logging function's own namespace (e.g. anything in `logging::`), so your logger won't end up calling itself.

## What it reports

**Skipped chains** are listed with a reason, for you to convert by hand:
- stream manipulators (`std::hex`, `std::setw(5)`, ...)
- chains written inside a macro
- chains whose result is used, such as `if (std::cout << x)`
- template code where part of the chain depends on the template parameters

**Warnings** (`!`) mark changes that may print differently or not compile:
- `bool` prints `1`/`0` with `<<` but `true`/`false` with `{}`
- non-`char` pointers may need a cast to `(const void *)`
- class types that only have an `operator<<` need a formatter

## Known differences

- A `cout` with no newline that the next `cout` continues will be split onto two lines.
- Joining a multi-line chain onto one line shifts line numbers, so `__LINE__` values change.

## Tests

```sh
tests/run.sh build/cout2log
```

This checks the preview output against `tests/expected.txt`, then applies the changes, builds the converted program and checks that it prints the same as the original. It runs on GitHub Actions for every change to this folder.
