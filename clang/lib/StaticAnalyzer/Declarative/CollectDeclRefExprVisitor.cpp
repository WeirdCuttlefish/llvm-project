//== CollectDeclRefExprVisitor.cpp - Collects DeclRefExprs----------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// 
// This gets all the variable instances in an expression
//
//===----------------------------------------------------------------------===//

#include <set>
#include <string>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/StaticAnalyzer/Declarative/CollectDeclRefExprVisitor.h"

using namespace clang;
using namespace std;

class CollectDeclRefExprVisitor::CollectDeclRefExprVisitorImpl
  : public RecursiveASTVisitor<CollectDeclRefExprVisitor> {
public:
  explicit CollectDeclRefExprVisitorImpl() : Variables(new set<string>()) {}
  ~CollectDeclRefExprVisitorImpl() { delete(Variables); }

  bool VisitDeclRefExpr(DeclRefExpr *Declaration) {
    if(Declaration == 0)
      return true;
    if (clang::VarDecl* VD = dyn_cast<clang::VarDecl>(Declaration->getDecl())){
      string name = VD->getNameAsString();
      Variables->insert(name);
    } else if (FunctionDecl* FD = dyn_cast<FunctionDecl>(Declaration->getDecl())) {
      string name = FD->getNameAsString();
      Caller.insert(name);
    }
    return true;
  }
  set<string> *getVariable(){
    return Variables;
  }

  set<string> getCaller(){
    return Caller;
  }

private:
  set<string> *Variables;
  set<string> Caller;

};

CollectDeclRefExprVisitor::CollectDeclRefExprVisitor():
  Pimpl(new CollectDeclRefExprVisitor::CollectDeclRefExprVisitorImpl()){}
CollectDeclRefExprVisitor::~CollectDeclRefExprVisitor(){ delete(Pimpl); }

bool CollectDeclRefExprVisitor::VisitDeclRefExpr(DeclRefExpr *Declaration){
  return Pimpl->VisitDeclRefExpr(Declaration);
}
set<string>* CollectDeclRefExprVisitor::getVariable(){
  return Pimpl->getVariable();
}
set<string> CollectDeclRefExprVisitor::getCaller(){
  return Pimpl->getCaller();
}

