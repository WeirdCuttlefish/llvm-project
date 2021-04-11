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
  DeclarativeFunctionVisitorImpl(ASTContext &Context):
    Graph(new DependencyGraph()),
    Context(Context)
    {};
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
    return true;
  }

  bool VisitBinaryOperator(BinaryOperator *Operator){

    if (Operator->isAssignmentOp()){

      DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Operator->getLHS());
      if (clang::VarDecl* VD = dyn_cast<clang::VarDecl>(DRE->getDecl())){
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

      }
    }
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *Declaration){
    string Name = Declaration->getNameInfo().getAsString();
    if (Graph->isAbsent(Name)){
      BugReports->insert(Name + " is no longer valid in " + 
          Declaration->getLocation().printToString(
            Context.getSourceManager()));
    }
    return true;
  }

  bool TraverseIfStmt(IfStmt *If){
    Stmt *Then = If->getThen();
    Stmt *Else = If->getElse();

    Graph->entryScope();
    
    Graph->entryBranch();
    this->TraverseStmt(Then);
    Graph->exitBranch();

    if (Else != NULL){
      Graph->entryBranch();
      this->TraverseStmt(Else);
      Graph->exitBranch();
    }

    Graph->exitScope();

    return true;
  }

  bool TraverseWhileStmt(WhileStmt *While){
    Stmt *Body = While->getBody();

    Graph->entryScope();

    Graph->entryBranch();
    this->TraverseStmt(Body);
    Graph->exitBranch();

    Graph->exitScope();

    return true;
  }

  set<string>* GetBugs(){
    return BugReports;
  }

private:
 DependencyGraph *Graph;
 ASTContext &Context;
 set<string> *BugReports = new set<string>();
  
}; 

DeclarativeFunctionVisitor::DeclarativeFunctionVisitor(ASTContext &Context):
  Pimpl(new DeclarativeFunctionVisitorImpl(Context)){}
DeclarativeFunctionVisitor::~DeclarativeFunctionVisitor(){
  delete(Pimpl);
}
bool DeclarativeFunctionVisitor::VisitVarDecl(VarDecl *Declaration){
  return Pimpl->VisitVarDecl(Declaration);
}
bool DeclarativeFunctionVisitor::VisitBinaryOperator(BinaryOperator *Operator){
  return Pimpl->VisitBinaryOperator(Operator);
}
bool DeclarativeFunctionVisitor::VisitDeclRefExpr(DeclRefExpr *Declaration){
  return Pimpl->VisitDeclRefExpr(Declaration);
}
bool DeclarativeFunctionVisitor::TraverseIfStmt(IfStmt *If){
  return Pimpl->TraverseIfStmt(If);
}
bool DeclarativeFunctionVisitor::TraverseWhileStmt(WhileStmt *While){
  return Pimpl->TraverseWhileStmt(While);
}
set<string>* DeclarativeFunctionVisitor::getBugs(){
  return Pimpl->GetBugs();
}
