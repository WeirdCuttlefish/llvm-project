#include <set>
#include <map>
#include <string>

#include "Andersen.h"
#include "CallGraph.h"
#include "DeclarativeCheckingVisitor.h"

using namespace clang::tooling;
using namespace llvm;
using namespace clang;
using namespace std;

/*
DeclarativeCheckingConsumer:
1. Invokes DeclarativeCheckingFunctionVisitorGlobal on global variables
2. Creates a call graph
3. Invokes a unique DeclarativeCheckingFunctionVisitor to each function in call graph in postorder
*/
class DeclarativeCheckingConsumer : public clang::ASTConsumer {
public:
  explicit DeclarativeCheckingConsumer(ASTContext *Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {

    #ifdef DEBUG
    Context.getTranslationUnitDecl()->dump();
    #endif

    // Test
    APAMatchFinderUtil APAUtil(&Context);
    APAUtil.run();
    APAUtil.run();
    map<string, Node*>* APA = APAUtil.getGraph();

    #ifdef DEBUG
    for (auto p : *APA){
      llvm::outs() << p.second->toString() << "\n";
    }
    #endif

    // Function map
    map<string, Function> FunctionChanges;

    // Work on global variables
    DeclarativeCheckingVisitor::DeclarativeCheckingFunctionVisitorGlobal GlobalVisitor;
    GlobalVisitor.TraverseDecl(Context.getTranslationUnitDecl());
    map<string, Validity> GlobalAlpha = GlobalVisitor.getAlpha();

    // Create list of graphs to work on
    CallGraphUtil CGU(&Context);
    list<CallGraphNode> LCGU = CGU.generateCallList();

    // Work on functions in reverse call order
    for (CallGraphNode n : LCGU){
      string FunctionName = dyn_cast<FunctionDecl>(n.getDecl())->getNameAsString();
      #ifdef DEBUG
      llvm::outs() << "Running on " + FunctionName + ":\n";
      #endif
      if (FunctionName == "main"){
        for (std::pair<string,Validity> p : GlobalAlpha){
          GlobalAlpha[p.first] = Valid;
        }
      }
      DeclarativeCheckingVisitor::DeclarativeCheckingFunctionVisitor Visitor(
          &Context,
          GlobalAlpha,
          GlobalVisitor.getBeta(),
          GlobalVisitor.getGamma(),
          &FunctionChanges,
          APA
      );
      Visitor.TraverseDecl(n.getDecl());
      FunctionChanges.insert(std::pair<string, Function>(FunctionName, Visitor.getFunctionDetails()));
    }
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