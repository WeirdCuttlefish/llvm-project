/* 
DeclarativeFunctionVisitor: This is a declarative verifier for functions.
Currently ignores the following:
  1. Pointers
  2. Arrays
*/

#ifndef CLANG_DECLARATIVE_FUNCTIONVISITOR_H
#define CLANG_DECLARATIVE_FUNCTIONVISITOR_H

#include<set>
#include<string>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/StaticAnalyzer/Declarative/DependencyGraph.h"

using namespace clang;
using namespace std;
using namespace declarative;

namespace visitor{

class DeclarativeFunctionVisitor : 
  public clang::RecursiveASTVisitor<DeclarativeFunctionVisitor>{

public:
  DeclarativeFunctionVisitor(ASTContext &Context);
  ~DeclarativeFunctionVisitor();

  bool VisitFunctionDecl(FunctionDecl *Declaration);
  bool VisitDeclRefExpr(DeclRefExpr *Declaration);
  bool TraverseBinaryOperator(BinaryOperator *Operator);
  bool TraverseCompoundAssignOperator(CompoundAssignOperator *Declaration);
  bool TraverseUnaryOperator(UnaryOperator *Declaration);
  bool TraverseIfStmt(IfStmt *If);
  bool TraverseForStmt(ForStmt *For);
  bool TraverseVarDecl(VarDecl *Declaration);
  bool TraverseWhileStmt(WhileStmt *While);

  set<pair<string, Decl*>>* getBugs();

private:
  class DeclarativeFunctionVisitorImpl;
  DeclarativeFunctionVisitorImpl* Pimpl;
  
};
} // end namespace visitor

#endif
