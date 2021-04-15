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

#include "clang/AST/ASTContext.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
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

private:
  void ReportBug(const Decl *D, 
                 string Description, BugReporter &BR, 
                 AnalysisManager &Mgr) const {
    BR.EmitBasicReport(D, 
                       this, 
                       "DeclarativeChecker",
                       "Declarative Bug",
                       Description,
                       PathDiagnosticLocation(
                         D, 
                         Mgr.getSourceManager()));
  }
};


DeclarativeChecker::DeclarativeChecker(){}

void DeclarativeChecker::checkASTDecl(const FunctionDecl *D, 
                                      AnalysisManager &Mgr, 
                                      BugReporter &BR) const {

  if (D->hasBody()){
    unique_ptr<visitor::DeclarativeFunctionVisitor> DFV(
        new visitor::DeclarativeFunctionVisitor(Mgr.getASTContext()));
    DFV->TraverseDecl((Decl*) D);
    set<pair<string,Decl*>> *Bugs =  DFV->getBugs();
    for (pair<string, Decl*> Bug : *Bugs){
      this->ReportBug(Bug.second, Bug.first, BR, Mgr);
    }
  }

}

void ento::registerDeclarativeChecker(CheckerManager &mgr) {
  mgr.registerChecker<DeclarativeChecker>();
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterDeclarativeChecker(const CheckerManager &mgr) {
  return true;
}
