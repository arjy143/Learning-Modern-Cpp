#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static llvm::cl::OptionCategory Category("cout2log");
static llvm::cl::opt<bool> Apply("apply", llvm::cl::cat(Category));

class Handler : public MatchFinder::MatchCallback {
public:
  std::map<std::string, Replacements> *Edits = nullptr;

  // Called once for every match: works out the replacement for one chain.
  void run(const MatchFinder::MatchResult &Result) override {
    SM = Result.SourceManager;
    LangOpts = &Result.Context->getLangOpts();

    const auto *Top = Result.Nodes.getNodeAs<Expr>("op");
    if (Top->getBeginLoc().isMacroID() || !SM->isInMainFile(Top->getBeginLoc()))
      return;

    // Flatten ((cout << a) << b) << c into [a, b, c], ending at the stream.
    std::vector<const Expr *> Operands;
    const Expr *Current = Top;
    while (const auto *Call =
               dyn_cast<CXXOperatorCallExpr>(Current->IgnoreParenImpCasts())) {
      Operands.insert(Operands.begin(), Call->getArg(1));
      Current = Call->getArg(0);
    }

    // The chain must start with std::cout.
    const auto *Stream = dyn_cast<DeclRefExpr>(Current->IgnoreParenImpCasts());
    if (!Stream || !Stream->getDecl()->isInStdNamespace() ||
        Stream->getDecl()->getName() != "cout")
      return;

    std::string Format;
    std::string Args;
    bool EndsWithNewline = false;

    for (const Expr *Operand : Operands) {
      const Expr *Stripped = Operand->IgnoreParenImpCasts();
      const auto *Record = Stripped->getType()->getAsCXXRecordDecl();
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
      } else if (Stripped->getType()->isFunctionType() || Overload) {
        return; // Manipulator such as std::hex; convert by hand.
      } else if (Record && Record->isInStdNamespace() &&
                 Record->getName().starts_with("_")) {
        return; // Manipulator such as std::setw(5); convert by hand.
      }

      if (!IsLiteral) {
        // Anything else becomes a {} placeholder plus an argument.
        Format += "{}";
        Args += ", " + getText(Operand);
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

    // The logger adds its own newline, so drop a trailing one.
    if (EndsWithNewline)
      Format.resize(Format.size() - 2);

    std::string NewText = "logging::log(\"" + Format + "\"" + Args + ")";

    llvm::outs() << Top->getBeginLoc().printToString(*SM) << "\n"
                 << "  - " << getText(Top) << "\n"
                 << "  + " << NewText << "\n";

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
};

int main(int argc, const char **argv) {
  auto Options = CommonOptionsParser::create(argc, argv, Category);
  if (!Options) {
    llvm::errs() << llvm::toString(Options.takeError());
    return 1;
  }

  RefactoringTool Tool(Options->getCompilations(),
                       Options->getSourcePathList());
  Handler Callback;
  Callback.Edits = &Tool.getReplacements();

  // Match the outermost << of each statement-level chain, skipping template
  // instantiations (the template itself is matched once instead).
  MatchFinder Finder;
  Finder.addMatcher(
      cxxOperatorCallExpr(hasOverloadedOperatorName("<<"),
                          unless(isInTemplateInstantiation()),
                          unless(hasParent(expr(unless(exprWithCleanups())))))
          .bind("op"),
      &Callback);

  auto Factory = newFrontendActionFactory(&Finder);
  return Apply ? Tool.runAndSave(Factory.get()) : Tool.run(Factory.get());
}
