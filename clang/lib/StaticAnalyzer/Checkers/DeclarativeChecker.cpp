//===-- DeclarativeChecker.cpp -----------------------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declarative Checker
//
//===----------------------------------------------------------------------===//

#include <set>
#include <string>

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Declarative/DeclarativeFunctionVisitor.h"
#include "llvm/Support/CommandLine.h"
#include <utility>

using namespace clang;
using namespace ento;

class DeclarativeChecker : public Checker<check::ASTDecl<FunctionDecl>> {

public:
  DeclarativeChecker();

  void checkASTDecl(
      const FunctionDecl *D, 
      AnalysisManager &Mgr,
      BugReporter &BR
  ) const;
};

DeclarativeChecker::DeclarativeChecker(){}

void DeclarativeChecker::checkASTDecl(const FunctionDecl *D, 
                                      AnalysisManager &Mgr, 
                                      BugReporter &BR) const {

  if (D->hasBody()){
    visitor::DeclarativeFunctionVisitor DFV;

    llvm::outs() << "Starting Declarative Analysis...\n";
    D->dump();
    DFV.TraverseDecl((Decl*) D);
    llvm::outs() << "Finished Declarative Analysis! By the developer a coffee!\n";
  }

}

void ento::registerDeclarativeChecker(CheckerManager &mgr) {
  mgr.registerChecker<DeclarativeChecker>();
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterDeclarativeChecker(const CheckerManager &mgr) {
  return true;
}
