/* 
DeclarativeFunctionVisitor: This is a declarative verifier for functions.
Currently ignores the following:
  1. Pointers
  2. Arrays
*/

#ifndef CLANG_DECLARATIVE_FUNCTIONVISITOR_H
#define CLANG_DECLARATIVE_FUNCTIONVISITOR_H

#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;

namespace visitor{

class DeclarativeFunctionVisitor : 
  public clang::RecursiveASTVisitor<DeclarativeFunctionVisitor>{

public:
  DeclarativeFunctionVisitor();
  ~DeclarativeFunctionVisitor();

  bool VisitVarDecl(VarDecl *Declaration);
  bool VisitBinaryOperator(BinaryOperator *Operator);

private:
  class DeclarativeFunctionVisitorImpl;
  DeclarativeFunctionVisitorImpl* Pimpl;
  
};
} // end namespace visitor

#endif
