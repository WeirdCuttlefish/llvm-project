#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Analysis/CallGraph.h"
#include <llvm/ADT/DepthFirstIterator.h>

using namespace clang::tooling;
using namespace llvm;
using namespace clang;

#include <set>
#include <map>
#include <string>

using namespace clang;
using namespace std;

void PostTraverseCallGraph(CallGraphNode *root, int num){
  for (CallGraphNode *c : root->callees()){
    PostTraverseCallGraph(c, num+1);
  }
  llvm::outs() << "level " << num << ":";
  if (root->getDecl() != NULL){
    root->getDefinition()->dump();
  } else {
    root->dump();
  }
  llvm::outs() << "\n";
}

class DeclarativeCheckingConsumer : public clang::ASTConsumer {
public:
  explicit DeclarativeCheckingConsumer(ASTContext *Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Context.getTranslationUnitDecl()->dump();
    /*
    CallGraph CG;
    CG.addToCallGraph(Context.getTranslationUnitDecl());
    CallGraphNode *root = CG.getRoot();
    PostTraverseCallGraph(root, 0);
    */
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
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<DeclarativeCheckingAction>().get());
}