// codemap: maps which files in a project #include which, and prints a Markdown
// report with Mermaid diagrams (GitHub and VS Code's preview draw them).
// Run it from your project's root directory:   codemap build > codemap.md
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace clang;
using Edge = std::pair<std::string, std::string>;

llvm::SmallString<256> root;                     // project directory
std::set<Edge> edges;                            // (file, project file it includes)
std::map<std::string, std::set<std::string>> libraries;  // outside library -> files that include it

// Path relative to the project root, or "" for files outside it.
std::string relative(llvm::StringRef path) {
    if (!path.consume_front(root)) return "";
    return path.ltrim('/').str();
}

// Library name from how an outside header is spelled:
// <boost/asio.hpp> -> boost, <zlib.h> -> zlib, <vector> -> std.
std::string libraryName(llvm::StringRef spelled) {
    if (spelled.contains('/')) return spelled.split('/').first.str();
    if (!spelled.contains('.')) return "std";
    return spelled.split('.').first.str();
}

// The preprocessor calls this for every #include it sees.
struct IncludeCollector : PPCallbacks {
    SourceManager& sm;
    IncludeCollector(SourceManager& sm) : sm(sm) {}

    void InclusionDirective(SourceLocation hash, const Token&, StringRef spelled, bool, CharSourceRange,
                            OptionalFileEntryRef included, StringRef, StringRef, const Module*,
                            SrcMgr::CharacteristicKind) override {
        auto includer = sm.getFileEntryRefForID(sm.getFileID(hash));
        if (!includer) return;
        std::string from = relative(includer->getFileEntry().tryGetRealPathName());
        if (from.empty()) return;  // an #include inside a system header
        std::string to = included ? relative(included->getFileEntry().tryGetRealPathName()) : "";
        if (!to.empty()) edges.insert({from, to});
        else libraries[libraryName(spelled)].insert(from);
    }
};

// Only runs the preprocessor (no parsing), with our collector attached.
struct IncludeAction : PreprocessOnlyAction {
    bool BeginSourceFileAction(CompilerInstance& ci) override {
        ci.getPreprocessor().addPPCallbacks(std::make_unique<IncludeCollector>(ci.getSourceManager()));
        return true;
    }
};

// Mermaid ids can't contain '/' or '.', so every name gets a short id: n0, n1, ...
std::map<std::string, int> ids;
std::string id(const std::string& name) {
    ids.insert({name, ids.size()});
    return "n" + std::to_string(ids[name]);
}

std::string folder(const std::string& file) {
    std::string dir = llvm::sys::path::parent_path(file).str();
    return dir.empty() ? "(root)" : dir;
}

// True if `target` can be reached from `start` by following includes.
bool reaches(const std::string& start, const std::string& target) {
    std::set<std::string> seen;
    std::vector<std::string> todo = {start};
    while (!todo.empty()) {
        std::string file = todo.back();
        todo.pop_back();
        if (file == target) return true;
        if (!seen.insert(file).second) continue;
        for (auto& [from, to] : edges)
            if (from == file) todo.push_back(to);
    }
    return false;
}

// Prints the top 10 of `counts` as table rows, largest first.
void top10(const std::map<std::string, int>& counts) {
    std::vector<std::pair<int, std::string>> sorted;
    for (auto& [file, n] : counts) sorted.push_back({-n, file});
    std::sort(sorted.begin(), sorted.end());
    for (size_t i = 0; i < sorted.size() && i < 10; ++i)
        llvm::outs() << "| `" << sorted[i].second << "` | " << -sorted[i].first << " |\n";
}

int main(int argc, char** argv) {
    if (argc != 2) {
        llvm::errs() << "usage: codemap <build dir containing compile_commands.json>\n";
        return 1;
    }
    llvm::sys::fs::real_path(".", root);

    std::string error;
    auto db = tooling::CompilationDatabase::loadFromDirectory(argv[1], error);
    if (!db) {
        llvm::errs() << error << "\n";
        return 1;
    }
    tooling::ClangTool tool(*db, db->getAllFiles());
    // Clang's own headers (stddef.h etc.) live next to the LLVM install, not next to this binary.
    tool.appendArgumentsAdjuster(tooling::getInsertArgumentAdjuster(
        "-resource-dir=" CODEMAP_RESOURCE_DIR, tooling::ArgumentInsertPosition::BEGIN));
    tool.run(tooling::newFrontendActionFactory<IncludeAction>().get());

    // 1. Folders: one node per folder, plus the outside libraries each folder uses.
    std::map<Edge, int> folderEdges;
    for (auto& [from, to] : edges)
        if (folder(from) != folder(to)) folderEdges[{folder(from), folder(to)}]++;
    for (auto& [lib, files] : libraries)
        if (lib != "std")
            for (auto& file : files) folderEdges[{folder(file), lib}]++;

    llvm::outs() << "# Code map\n\n## Folders\n\nArrows point from a folder to what it includes; "
                 << "numbers count the #include pairs. Hexagons are outside libraries.\n\n```mermaid\nflowchart LR\n";
    for (auto& [lib, files] : libraries)
        if (lib != "std") llvm::outs() << "  " << id("lib " + lib) << "{{\"" << lib << "\"}}\n";
    for (auto& [pair, n] : folderEdges) {
        auto& [from, to] = pair;
        std::string target = libraries.count(to) ? id("lib " + to) : id("dir " + to);
        llvm::outs() << "  " << id("dir " + from) << "[\"" << from << "\"] -->|" << n << "| " << target << "\n";
        if (!libraries.count(to)) llvm::outs() << "  " << target << "[\"" << to << "\"]\n";
    }
    llvm::outs() << "```\n\n";

    // 2. Files, boxed by folder. Includes that are part of a cycle are drawn in red.
    std::map<std::string, std::set<std::string>> byFolder;
    for (auto& [from, to] : edges) {
        byFolder[folder(from)].insert(from);
        byFolder[folder(to)].insert(to);
    }
    llvm::outs() << "## Files\n\n```mermaid\nflowchart LR\n";
    for (auto& [dir, files] : byFolder) {
        llvm::outs() << "  subgraph " << id("box " + dir) << "[\"" << dir << "\"]\n";
        for (auto& file : files)
            llvm::outs() << "    " << id(file) << "[\"" << llvm::sys::path::filename(file) << "\"]\n";
        llvm::outs() << "  end\n";
    }
    std::vector<Edge> cycles;
    std::string red;
    int n = 0;
    for (auto& [from, to] : edges) {
        llvm::outs() << "  " << id(from) << " --> " << id(to) << "\n";
        if (reaches(to, from)) {
            cycles.push_back({from, to});
            red += (red.empty() ? "" : ",") + std::to_string(n);
        }
        n++;
    }
    if (!red.empty()) llvm::outs() << "  linkStyle " << red << " stroke:#d33,stroke-width:3px\n";
    llvm::outs() << "```\n\n";

    // 3. Where to start reading: the most-included headers, and the files that include the most.
    std::map<std::string, int> includedBy, includes;
    for (auto& [from, to] : edges) {
        includes[from]++;
        includedBy[to]++;
    }
    llvm::outs() << "## Where to start reading\n\nMost included (the core everything builds on):\n\n"
                 << "| File | Included by |\n| --- | --- |\n";
    top10(includedBy);
    llvm::outs() << "\nMost includes (the code that ties things together):\n\n| File | Includes |\n| --- | --- |\n";
    top10(includes);

    // 4. Include cycles.
    llvm::outs() << "\n## Include cycles\n\n";
    if (cycles.empty()) llvm::outs() << "None found.\n";
    for (auto& [from, to] : cycles) llvm::outs() << "- `" << from << "` includes `" << to << "`\n";
}
