/* 
DeclarativeFunctionVisitor: This is a declarative verifier for functions.
Currently ignores the following:
  1. Pointers
  2. Arrays
*/

#include <set>
#include <string>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/StaticAnalyzer/Declarative/CollectDeclRefExprVisitor.h"
#include "clang/StaticAnalyzer/Declarative/DeclarativeFunctionVisitor.h"
#include "clang/StaticAnalyzer/Declarative/DependencyGraph.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace declarative;
using namespace visitor;
using namespace std;

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
    CollectDeclRefExprVisitor Collector;
    set<string> Us;
    if (Declaration->getInit() != nullptr){
      Collector.TraverseStmt(Declaration->getInit());
      Us = Collector.getVariable();
    } else {
      Us = set<string>();
    }

    Graph->insert(Declaration->getNameAsString(), Us);

    llvm::outs() << Graph->toString();
    return true;
  }

  bool VisitBinaryOperator(BinaryOperator *Operator){

    if (Operator->isAssignmentOp()){

      CollectDeclRefExprVisitor Collector;

      Collector.TraverseStmt(Operator->getRHS());
      set<string> Us = Collector.getVariable();

      string LhsName = dyn_cast<DeclRefExpr>(Operator->getLHS())
                       ->getNameInfo()
                       .getAsString();

      for (string reachable : Graph->reachable(LhsName)){
        Graph->remove(reachable);
      }

      Graph->insert(LhsName, Us);

      llvm::outs() << Graph->toString();
    }
    return true;
  }

private:
 DependencyGraph *Graph;
  
}; 

DeclarativeFunctionVisitor::DeclarativeFunctionVisitor():
  Pimpl(new DeclarativeFunctionVisitorImpl()){}
DeclarativeFunctionVisitor::~DeclarativeFunctionVisitor(){
  delete(Pimpl);
}
bool DeclarativeFunctionVisitor::VisitVarDecl(VarDecl *Declaration){
  return Pimpl->VisitVarDecl(Declaration);
}
bool DeclarativeFunctionVisitor::VisitBinaryOperator(BinaryOperator *Operator){
  return Pimpl->VisitBinaryOperator(Operator);
}
