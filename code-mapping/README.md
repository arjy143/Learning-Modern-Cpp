# codemap

A Clang LibTooling tool that maps which files in a project `#include` which, and
writes a Markdown report with [Mermaid](https://mermaid.js.org) diagrams. GitHub,
GitLab and VS Code's Markdown preview draw the diagrams, so there's nothing else
to install to read it. Each diagram is followed by the same information as a
plain list, so the report also reads well in a terminal with
[glow](https://github.com/charmbracelet/glow) or `less`, which don't draw Mermaid.

See [`example/codemap.md`](example/codemap.md) for the report on the small
project in `example/`.

The report has four parts:

1. **Folders**: one box per folder, with arrows for "something in A includes
   something in B" and a count on each. Outside libraries (zlib, boost, ...)
   appear as hexagons. Start here on a big codebase. The list below it gives
   each folder and what it includes.
2. **Files**: every project file, boxed by folder, with an arrow per `#include`.
   Includes that are part of a cycle are drawn in red. The list below it gives
   each file and what it includes, with `(cycle)` after any include in a loop.
3. **Where to start reading**: the most-included headers (the core types) and
   the files with the most includes (the glue code).
4. **Include cycles**: each `#include` that is part of a loop.

With `--dot`, it also writes the folder and file graphs as `folders.dot` and
`files.dot` (see below).

It only runs the preprocessor, never the parser, so it's fast: yaml-cpp's 35
source files take about 1.5 seconds.

## Build

Needs the Clang/LLVM development libraries (tested with LLVM 18).

```sh
# Ubuntu/Debian: sudo apt install libclang-18-dev llvm-18-dev cmake ninja-build
cmake -S . -B build -G Ninja -DClang_DIR=/usr/lib/llvm-18/lib/cmake/clang
cmake --build build
```

On macOS with Homebrew, use `-DClang_DIR=$(brew --prefix llvm)/lib/cmake/clang`.

## Use

Your project needs a `compile_commands.json` (configure it with
`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`). Run codemap from the project's root
directory; only files under it are mapped.

```sh
cd example
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
../build/codemap build > codemap.md
glow codemap.md     # or open it on GitHub / in VS Code for the diagrams
```

### Diagrams without GitHub or VS Code

Add `--dot` to also write the two graphs as `folders.dot` and `files.dot`, in
Graphviz's format. Both tools below are standard Ubuntu/Debian packages:

```sh
sudo apt install graphviz libgraph-easy-perl

../build/codemap build --dot > codemap.md
graph-easy --as=boxart folders.dot           # folder graph as boxes and arrows, in the terminal
dot -Tsvg files.dot -o files.svg             # full file graph as an image (cycles in red)
xdg-open files.svg
```

`graph-easy` is best kept to the folder graph: on a big project the file graph
gets too wide for a terminal and slow to lay out. `dot` handles it in a fraction
of a second.

## How it works

| Part of `codemap.cpp` | What it does |
|---|---|
| `IncludeCollector` | Clang calls `InclusionDirective` for every `#include`. A project header becomes an edge in `edges`; anything else is recorded under its library name in `libraries`. |
| `IncludeAction` | Runs only the preprocessor, with the collector attached. |
| `relative()` | Makes a path project-relative, or returns `""` for files outside the project. This one check separates project files from system headers. |
| `libraryName()` | Guesses a library from how the header is spelled: `<boost/asio.hpp>` → boost, `<zlib.h>` → zlib, `<vector>` → std. |
| `id()` | Gives every file, folder and library a short Mermaid id (`n0`, `n1`, ...), since ids can't contain `/` or `.`. |
| `reaches()` | Follows includes from one file to see if it gets back to another; an edge `a → b` is in a cycle when `b` reaches `a`. |
| `main` | Loads the compilation database, runs the tool over every file, then prints the four sections. The two diagram sections each print a Mermaid block followed by the same edges as a nested list. With `--dot`, section 5 writes the same two graphs as DOT files, where real paths can be node names, so no ids are needed. |

## Limits

- Only files reached through `compile_commands.json` are seen; a header nothing
  includes won't appear.
- `#include`s inside `#if` branches that are switched off for this build are
  not seen, because the preprocessor skips them.
- Library names are a guess from the spelling: `<sys/stat.h>` shows up as `sys`.
- LLVM 19 and newer add a `bool` parameter to `InclusionDirective`, just before
  the last one. Add `bool,` there if the override doesn't compile.
