#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "llvm/Support/Regex.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static llvm::cl::OptionCategory Category("cout2log options");

static llvm::cl::opt<bool> Apply(
    "apply", llvm::cl::desc("Write the changes (default is a dry run)"),
    llvm::cl::cat(Category));

static llvm::cl::opt<std::string> Function(
    "function", llvm::cl::desc("Logging function to call (default: logging::log)"),
    llvm::cl::init("logging::log"), llvm::cl::cat(Category));

static llvm::cl::list<std::string> Streams(
    "stream",
    llvm::cl::desc("Stream to convert, as NAME or NAME=FUNCTION. Repeatable.\n"
                   "Default: cout. Example: --stream=cerr=logging::error"),
    llvm::cl::cat(Category));

static llvm::cl::opt<bool> KeepNewline(
    "keep-newline",
    llvm::cl::desc("Keep a trailing newline in the format string (for loggers\n"
                   "that don't add one, such as fmt::print)"),
    llvm::cl::cat(Category));

static llvm::cl::opt<bool> IncludeHeaders(
    "include-headers",
    llvm::cl::desc("Also rewrite your own (non-system) headers"),
    llvm::cl::cat(Category));

// Stream name (e.g. "cout") -> logging function to call instead.
static std::map<std::string, std::string> StreamFunctions;

class Handler : public MatchFinder::MatchCallback {
public:
  std::map<std::string, Replacements> *Edits = nullptr;
  int Changes = 0;
  int Skipped = 0;

  // Called once for every match: works out the replacement for one chain.
  void run(const MatchFinder::MatchResult &Result) override {
    SM = Result.SourceManager;
    LangOpts = &Result.Context->getLangOpts();

    // Only touch the files the user asked for (and their headers if wanted).
    const auto *Top = Result.Nodes.getNodeAs<Expr>("op");
    SourceLocation Where = SM->getExpansionLoc(Top->getBeginLoc());
    if (SM->isInSystemHeader(Where) ||
        (!IncludeHeaders && !SM->isInMainFile(Where)))
      return;

    // Flatten ((cout << a) << b) << c into [a, b, c], ending at the stream.
    std::vector<const Expr *> Operands;
    const Expr *Current = Top;
    while (const auto *Call =
               dyn_cast<CXXOperatorCallExpr>(Current->IgnoreParenImpCasts())) {
      Operands.insert(Operands.begin(), Call->getArg(1));
      Current = Call->getArg(0);
    }

    // The chain must start with one of the std streams being converted.
    const auto *Stream = dyn_cast<DeclRefExpr>(Current->IgnoreParenImpCasts());
    if (!Stream || !Stream->getDecl()->isInStdNamespace())
      return;
    auto Target = StreamFunctions.find(Stream->getDecl()->getNameAsString());
    if (Target == StreamFunctions.end())
      return;

    // A header included by several files is seen several times; do it once.
    std::string Key = Where.printToString(*SM);
    if (!Seen.insert(Key).second)
      return;

    // Reasons the chain can't be converted automatically.
    std::string Reason;
    const auto Parents = Result.Context->getParents(*Top);
    const Expr *Parent = Parents.empty() ? nullptr : Parents[0].get<Expr>();
    if (Top->getBeginLoc().isMacroID() || Top->getEndLoc().isMacroID())
      Reason = "the chain is inside a macro";
    else if (Parent && Parent->isTypeDependent())
      Reason = "part of the chain depends on a template parameter";
    else if (Parent && !isa<ExprWithCleanups>(Parent))
      Reason = "the result is used, e.g. in an if condition";

    std::string Format;
    std::string Args;
    std::vector<std::string> Warnings;
    bool EndsWithNewline = false;

    for (const Expr *Operand : Operands) {
      if (!Reason.empty())
        break;
      const Expr *Stripped = Operand->IgnoreParenImpCasts();
      const QualType Type = Stripped->getType();
      const auto *Record = Type->getAsCXXRecordDecl();
      const auto *Ref = dyn_cast<DeclRefExpr>(Stripped);
      const auto *Overload = dyn_cast<OverloadExpr>(Stripped);

      // Work out whether this operand is literal text (and what it is).
      std::string Literal;
      bool IsLiteral = false;
      if (Stripped->getBeginLoc().isMacroID()) {
        IsLiteral = false;
      } else if (const auto *String = dyn_cast<StringLiteral>(Stripped)) {
        Literal = String->getString().str();
        IsLiteral = true;
      } else if (const auto *Char = dyn_cast<CharacterLiteral>(Stripped)) {
        Literal = std::string(1, static_cast<char>(Char->getValue()));
        IsLiteral = true;
      } else if ((Ref && Ref->getDecl()->getName() == "endl") ||
                 (Overload && Overload->getName().getAsString() == "endl")) {
        Literal = "\n";
        IsLiteral = true;
      } else if (Type->isFunctionType() || Overload ||
                 (Record && Record->isInStdNamespace() &&
                  Record->getName().starts_with("_"))) {
        Reason = "it uses the stream manipulator " + getText(Operand);
        break;
      }

      if (!IsLiteral) {
        // Things that print differently with {} than with <<.
        std::string Name = getText(Operand);
        if (Type->isBooleanType())
          Warnings.push_back(Name + " is a bool: << prints 1/0, {} prints "
                                    "true/false");
        else if (Type->isPointerType() && !Type->getPointeeType()->isCharType())
          Warnings.push_back(Name + " is a pointer: it may need a cast to "
                                    "(const void *)");
        else if (Record && !(Record->isInStdNamespace() &&
                             Record->getName().starts_with("basic_string")))
          Warnings.push_back(Name + " has type " + Type.getAsString() +
                             ", which needs a formatter");

        // Anything else becomes a {} placeholder plus an argument.
        Format += "{}";
        Args += ", " + Name;
        EndsWithNewline = false;
        continue;
      }

      // Escape the text as a C++ string literal, doubling braces so they
      // aren't treated as placeholders.
      for (char C : Literal) {
        if (C == '\n') {
          Format += "\\n";
        } else if (C == '\t') {
          Format += "\\t";
        } else {
          if (C == '"' || C == '\\')
            Format += '\\';
          Format += C;
          if (C == '{' || C == '}')
            Format += C;
        }
      }
      if (!Literal.empty())
        EndsWithNewline = Literal.back() == '\n';
    }

    if (!Reason.empty()) {
      llvm::outs() << Key << ": skipped because " << Reason << "\n"
                   << "  " << getText(Top) << "\n";
      ++Skipped;
      return;
    }

    // Most loggers add their own newline, so drop a trailing one.
    if (EndsWithNewline && !KeepNewline)
      Format.resize(Format.size() - 2);

    std::string NewText = Target->second + "(\"" + Format + "\"" + Args + ")";

    llvm::outs() << Key << "\n"
                 << "  - " << getText(Top) << "\n"
                 << "  + " << NewText << "\n";
    for (const std::string &Warning : Warnings)
      llvm::outs() << "  ! " << Warning << "\n";
    ++Changes;

    CharSourceRange Range = Lexer::makeFileCharRange(
        CharSourceRange::getTokenRange(Top->getSourceRange()), *SM, *LangOpts);
    Replacement Rep(*SM, Range, NewText, *LangOpts);
    llvm::consumeError((*Edits)[Rep.getFilePath().str()].add(Rep));
  }

private:
  // The source code text of an expression, as written in the file.
  std::string getText(const Expr *E) {
    CharSourceRange Range = Lexer::makeFileCharRange(
        CharSourceRange::getTokenRange(E->getSourceRange()), *SM, *LangOpts);
    return Lexer::getSourceText(Range, *SM, *LangOpts).str();
  }

  const SourceManager *SM = nullptr;
  const LangOptions *LangOpts = nullptr;
  std::set<std::string> Seen;
};

int main(int argc, const char **argv) {
  auto Options = CommonOptionsParser::create(argc, argv, Category,
                                             llvm::cl::ZeroOrMore);
  if (!Options) {
    llvm::errs() << llvm::toString(Options.takeError());
    return 1;
  }

  // Which streams to convert, and to what.
  if (Streams.empty())
    StreamFunctions["cout"] = Function;
  for (StringRef Entry : Streams) {
    auto [Name, Target] = Entry.split('=');
    Name.consume_front("std::");
    StreamFunctions[Name.str()] = Target.empty() ? Function : Target.str();
  }

  // With no files given, convert every file in compile_commands.json. The
  // options parser doesn't load the database in that case, so load it here.
  std::vector<std::string> Files = Options->getSourcePathList();
  std::unique_ptr<CompilationDatabase> Database;
  if (Files.empty()) {
    std::string BuildDir = ".";
    for (int I = 1; I + 1 < argc; ++I)
      if (StringRef(argv[I]) == "-p")
        BuildDir = argv[I + 1];
    std::string Error;
    Database = CompilationDatabase::autoDetectFromDirectory(BuildDir, Error);
    if (!Database) {
      llvm::errs() << Error << "\n";
      return 1;
    }
    Files = Database->getAllFiles();
  }
  std::sort(Files.begin(), Files.end()); // Same output order every run.

  RefactoringTool Tool(Database ? *Database : Options->getCompilations(), Files);

  // Find clang's own headers (stddef.h etc.), hide compiler warnings, and
  // ignore warning flags that only GCC understands.
  for (const char *Arg : {"-resource-dir=" COUT2LOG_RESOURCE_DIR, "-w",
                          "-Wno-unknown-warning-option"})
    Tool.appendArgumentsAdjuster(
        getInsertArgumentAdjuster(Arg, ArgumentInsertPosition::END));

  Handler Callback;
  Callback.Edits = &Tool.getReplacements();

  // Never rewrite the logger itself, or it would end up calling itself:
  // skip code inside the target functions' namespace (logging::log ->
  // anything in ::logging::), or inside the function if it has none.
  std::string LoggerCode;
  for (const auto &[Stream, Target] : StreamFunctions) {
    StringRef Name = StringRef(Target).trim();
    Name.consume_front("::");
    size_t Colons = Name.rfind("::");
    LoggerCode += LoggerCode.empty() ? "" : "|";
    LoggerCode += Colons == StringRef::npos
                      ? "^::" + llvm::Regex::escape(Name) + "$"
                      : "^::" + llvm::Regex::escape(Name.substr(0, Colons)) + "::";
  }

  // Match the outermost << of each chain, skipping template instantiations
  // (the template itself is matched once instead).
  MatchFinder Finder;
  auto Shift = cxxOperatorCallExpr(hasOverloadedOperatorName("<<"));
  Finder.addMatcher(cxxOperatorCallExpr(hasOverloadedOperatorName("<<"),
                                        unless(isInTemplateInstantiation()),
                                        unless(hasAncestor(functionDecl(
                                            matchesName(LoggerCode)))),
                                        unless(hasParent(Shift)),
                                        unless(hasParent(parenExpr(hasParent(Shift)))))
                        .bind("op"),
                    &Callback);

  auto Factory = newFrontendActionFactory(&Finder);
  int Status = Apply ? Tool.runAndSave(Factory.get()) : Tool.run(Factory.get());

  llvm::outs() << Callback.Changes << " change(s), " << Callback.Skipped
               << " skipped.";
  if (!Apply && Callback.Changes > 0)
    llvm::outs() << " Dry run: re-run with --apply to write them.";
  llvm::outs() << "\n";
  return Status;
}
