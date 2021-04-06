//== CollectDeclRefExprVisitor.h - Collects DeclRefExprs----------------==//
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

using namespace clang;
using namespace std;

class CollectDeclRefExprVisitor
  : public RecursiveASTVisitor<CollectDeclRefExprVisitor> {
public:
  CollectDeclRefExprVisitor();

  bool VisitDeclRefExpr(DeclRefExpr *Declaration);

  set<string> getVariable();

  set<string> getCaller();

private:
  class CollectDeclRefExprVisitorImpl;
  CollectDeclRefExprVisitorImpl* Pimpl;

};

