/* 
DeclarativeFunctionVisitor: This is a declarative verifier for functions.
Currently ignores the following:
  1. Pointers
  2. Arrays
*/

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/StaticAnalyzer/Declarative/DeclarativeFunctionVisitor.h"
#include "clang/StaticAnalyzer/Declarative/DependencyGraph.h"

using namespace clang;
using namespace declarative;
using namespace visitor;

class DeclarativeFunctionVisitor::DeclarativeFunctionVisitorImpl : 
  public clang::RecursiveASTVisitor<
    DeclarativeFunctionVisitor::DeclarativeFunctionVisitorImpl
  >{

public:
  DeclarativeFunctionVisitorImpl(){
    Graph = new DependencyGraph();
  };
  ~DeclarativeFunctionVisitorImpl(){
    delete(Graph);
  };

  bool VisitVarDecl(VarDecl *Declaration){
    return true;
  }

private:
 DependencyGraph *Graph;
  
}; 

DeclarativeFunctionVisitor::DeclarativeFunctionVisitor():
  Pimpl(new DeclarativeFunctionVisitorImpl()){};
DeclarativeFunctionVisitor::~DeclarativeFunctionVisitor(){
  delete(Pimpl);
};
bool DeclarativeFunctionVisitor::VisitVarDecl(VarDecl *Declaration){
  return Pimpl->VisitVarDecl(Declaration);
}
