#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "DependencyGraph.h"
#include <map>
#include <set>
#include <string>


using namespace declarative;

/*
DeclarativeCheckingConsumer:
1. Invokes DeclarativeCheckingFunctionVisitorGlobal on global variables
2. Creates a call graph
3. Invokes a unique DeclarativeCheckingFunctionVisitor to each function in call graph in postorder
*/
class DeclarativeCheckingConsumer : public clang::ASTConsumer {
public:
  explicit DeclarativeCheckingConsumer(clang::ASTContext *Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {

    DependencyGraph *D = new DependencyGraph();
    D->insert("Hello", set<string>());
    set<string> Depends;
    Depends.insert("Hello");
    D->insert("MyMom", Depends);

    Context.getTranslationUnitDecl()->dump();
  }

};

class DeclarativeCheckingAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new DeclarativeCheckingConsumer(&Compiler.getASTContext()));
  }
};

static llvm::cl::OptionCategory MyToolCategory("My tool options");
int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(clang::tooling::newFrontendActionFactory<DeclarativeCheckingAction>().get());
}
