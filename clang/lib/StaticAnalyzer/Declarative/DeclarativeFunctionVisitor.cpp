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
    delete(BugReports);
  };

  bool VisitFunctionDecl(FunctionDecl *Declaration){
    set<string> EmptySet;
    for (ParmVarDecl *Param : Declaration->parameters()){
      string ParamName = (dyn_cast<VarDecl>(Param))->getNameAsString();
      Graph->insert(ParamName, EmptySet);
    }
    return true;
  }

  bool TraverseForStmt(ForStmt *For){
    Graph->entryScope();

    Graph->entryBranch();
    TraverseStmt(For->getInit());
    TraverseStmt(For->getBody());
    Graph->exitBranch();

    Graph->exitScope();
    return true;
  }

  bool TraverseCompoundAssignOperator(CompoundAssignOperator *Operator){
    TraverseStmt(Operator->getRHS());
    TraverseStmt(Operator->getLHS());

    if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Operator->getLHS())){
      if (clang::VarDecl* VD = dyn_cast<clang::VarDecl>(DRE->getDecl())){
        if (!VD->getType()->isPointerType()){
          CollectDeclRefExprVisitor Collector;

          Collector.TraverseStmt(Operator->getRHS());
          set<string> *Us = Collector.getVariable();

          string LhsName = VD->getNameAsString();

          if (Graph->isPresent(LhsName)){
            set<string> Reach;
            Graph->reachable(LhsName, Reach);
            for (string r : Reach){
              if (Graph->isPresent(r) && r != LhsName){
                Graph->remove(r, LhsName);
              }
            }
          }

          for (string U : *Us){
            Graph->insertEdge(LhsName, U);
          }
        }
      }
    }
    return true;
  }

  bool TraverseVarDecl(VarDecl *Declaration){
    if (Declaration->getInit() != nullptr){
      TraverseStmt(Declaration->getInit());
    }
    if (!Declaration->getType()->isPointerType()){
      CollectDeclRefExprVisitor Collector;
      set<string> Empty = set<string>();
      set<string> *Us = &Empty;
      if (Declaration->getInit() != nullptr){
        Collector.TraverseStmt(Declaration->getInit());
        Us = Collector.getVariable();
      }
      for (string U : *Us){
        Empty.insert(U);
      }
      for (string U : Empty){
        if (Graph->isAbsent(U)){
          Us->erase(U);
        }
      }

      Graph->insert(Declaration->getNameAsString(), *Us);
    }
    return true;
  }

  bool TraverseBinaryOperator(BinaryOperator *Operator){
    TraverseStmt(Operator->getRHS());

    if (Operator->isAssignmentOp()){
      if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Operator->getLHS())){
        if (clang::VarDecl* VD = dyn_cast<clang::VarDecl>(DRE->getDecl())){
          if (!VD->getType()->isPointerType()){
            CollectDeclRefExprVisitor Collector;

            Collector.TraverseStmt(Operator->getRHS());
            set<string> *Us = Collector.getVariable();

            string LhsName = VD->getNameAsString();

            if (Graph->isPresent(LhsName)){
              set<string> Reach;
              Graph->reachable(LhsName, Reach);
              for (string r : Reach){
                if (Graph->isPresent(r)){
                  Graph->remove(r, LhsName);
                }
              }
            }
            Graph->insert(LhsName, *Us);
          }
        }
      }
    }
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *Declaration){
    if (VarDecl* VD = dyn_cast<VarDecl>(Declaration->getDecl())){
      if (VD->isLocalVarDeclOrParm() && !VD->getType()->isPointerType()){
        string Name = VD->getNameAsString();
        if (Graph->isAbsent(Name)){
          BugReports->insert(
              pair<string, Decl*>(
                Name + " is no longer valid " + "due to change in " +
                (!Graph->getRemovalReason(Name).empty() ? Graph->getRemovalReason(Name) : "UNKNOWN")
                + " in " + 
                Declaration->getLocation().printToString(
                Context.getSourceManager()),
                VD
              )
          );
        }
      }
    }
    return true;
  }

  bool TraverseIfStmt(IfStmt *If){
    Stmt *Cond = If->getCond();
    Stmt *Then = If->getThen();
    Stmt *Else = If->getElse();

    Graph->entryScope();
    
    Graph->entryBranch();
    this->TraverseStmt(Cond);
    this->TraverseStmt(Then);
    Graph->exitBranch();

    if (Else != NULL){
      Graph->entryBranch();
      this->TraverseStmt(Cond);
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

  set<pair<string, Decl*>>* GetBugs(){
    return BugReports;
  }

private:
 DependencyGraph *Graph;
 ASTContext &Context;
 set<pair<string, Decl*>> *BugReports = new set<pair<string,Decl*>>();
  
}; 

DeclarativeFunctionVisitor::DeclarativeFunctionVisitor(ASTContext &Context):
  Pimpl(new DeclarativeFunctionVisitorImpl(Context)){}
DeclarativeFunctionVisitor::~DeclarativeFunctionVisitor(){
  delete(Pimpl);
}
bool DeclarativeFunctionVisitor::VisitFunctionDecl(FunctionDecl *Declaration){
  return Pimpl->VisitFunctionDecl(Declaration);
}
bool DeclarativeFunctionVisitor::TraverseCompoundAssignOperator(CompoundAssignOperator *Operator){
  return Pimpl->TraverseCompoundAssignOperator(Operator);
}
bool DeclarativeFunctionVisitor::TraverseVarDecl(VarDecl *Declaration){
  return Pimpl->TraverseVarDecl(Declaration);
}
bool DeclarativeFunctionVisitor::TraverseBinaryOperator(BinaryOperator *Operator){
  return Pimpl->TraverseBinaryOperator(Operator);
}
bool DeclarativeFunctionVisitor::VisitDeclRefExpr(DeclRefExpr *Declaration){
  return Pimpl->VisitDeclRefExpr(Declaration);
}
bool DeclarativeFunctionVisitor::TraverseIfStmt(IfStmt *If){
  return Pimpl->TraverseIfStmt(If);
}
bool DeclarativeFunctionVisitor::TraverseForStmt(ForStmt *For){
  return Pimpl->TraverseForStmt(For);
}
bool DeclarativeFunctionVisitor::TraverseWhileStmt(WhileStmt *While){
  return Pimpl->TraverseWhileStmt(While);
}
set<pair<string, Decl*>>* DeclarativeFunctionVisitor::getBugs(){
  return Pimpl->GetBugs();
}
