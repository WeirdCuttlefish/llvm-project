#include <set>
#include <map>
#include <string>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Analysis/CallGraph.h"

using namespace clang::tooling;
using namespace llvm;
using namespace clang;
using namespace std;


enum Validity {Valid, Invalid, Undecided};

struct Function {
  string name;
  set<string> Preconditions;    // Guaranteed valid going in
  set<string> Postconditions;   // Guaranteed valid going out
};

string VarDeclToString(VarDecl *x){
  return x->getNameAsString();
}

string DeclRefExprToString(DeclRefExpr *x){
  return x->getNameInfo().getAsString();
}

void PrintGamma(map<string, set<string>> m) {
  llvm::outs() << "Gamma Begin:\n";
  map<string, set<string>>::iterator it;
  for (it = m.begin(); it != m.end(); it++)
  {
    llvm::outs() << "Variable " << it->first << " depends on: ";
    set<string>::iterator it2 = (it->second).begin();
    while (it2 != (it->second).end())
    {
        llvm::outs() << *it2 << ", ";
        it2++;
    }
    llvm::outs() << "\n";
  }
  llvm::outs() << "Gamma End:\n";
}

void PrintBeta(map<string, set<string>> m) {
  llvm::outs() << "Beta Begin:\n";
  map<string, set<string>>::iterator it;
  for (it = m.begin(); it != m.end(); it++)
  {
    llvm::outs() << "Variable " << it->first << " has clients: ";
    set<string>::iterator it2 = (it->second).begin();
    while (it2 != (it->second).end())
    {
        llvm::outs() << *it2 << ", ";
        it2++;
    }
    llvm::outs() << "\n";
  }
  llvm::outs() << "Beta End:\n";
}

void PrintAlpha(map<string, Validity> m) {
  llvm::outs() << "Alpha Begin:\n";
  map<string, Validity>::iterator it;
  for (it = m.begin(); it != m.end(); it++)
  {
    switch(it->second) {
      case Valid: llvm::outs() << "Variable " << it->first << " is: valid.\n"; break;
      case Invalid: llvm::outs() << "Variable " << it->first << " is: invalid.\n"; break;
      case Undecided: llvm::outs() << "Variable " << it->first << " is: undecided.\n"; break;
    }
  }
  llvm::outs() << "Alpha End:\n";
}

void PrintVarDeclLoc(VarDecl *Declaration, ASTContext *Context) {
  FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());
  string name = Declaration->getNameAsString();
  if (FullLocation.isValid())
      llvm::outs() << "Found declaration " << name << " at: "
                    << FullLocation.getSpellingLineNumber() << ":"
                    << FullLocation.getSpellingColumnNumber() << "\n";
}

void PrintBinaryOperatorLoc(BinaryOperator *BinOperator, ASTContext *Context) {
    if (BinOperator->isAssignmentOp()) {
      DeclRefExpr *Declaration = dyn_cast<DeclRefExpr>(BinOperator->getLHS());
      FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());
      DeclarationNameInfo Id = Declaration->getNameInfo();
      string name = Id.getAsString();
      if (FullLocation.isValid())
        llvm::outs() << "Found reassignment " << name << " at: "
                      << FullLocation.getSpellingLineNumber() << ":"
                      << FullLocation.getSpellingColumnNumber() << "\n";
    }
}

void PrintDeclRefExprLoc(DeclRefExpr *Declaration, ASTContext *Context) {
  FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());
  DeclarationNameInfo Id = Declaration->getNameInfo();
  string name = Id.getAsString();
  if (FullLocation.isValid())
    llvm::outs() << "--Inside expression we have " << name << " variable at: "
                  << FullLocation.getSpellingLineNumber() << ":"
                  << FullLocation.getSpellingColumnNumber() << "\n";
}

/* 
CollectDeclRefExprVisitor:
Collects variables inside a statement
*/
class CollectDeclRefExprVisitor
  : public RecursiveASTVisitor<CollectDeclRefExprVisitor> {
public:
  explicit CollectDeclRefExprVisitor() {}

  bool VisitDeclRefExpr(DeclRefExpr *Declaration) {
    if(Declaration == 0)
      return true;
    if (clang::VarDecl* VD = dyn_cast<clang::VarDecl>(Declaration->getDecl())){
      string name = VarDeclToString(VD);
      Variables.insert(name);
    } else if (FunctionDecl* FD = dyn_cast<FunctionDecl>(Declaration->getDecl())) {
      string name = FD->getNameAsString();
      Caller.insert(name);
    }
    return true;
  }
  set<string> getVariable(){
    return Variables;
  }

  set<string> getCaller(){
    return Caller;
  }

private:
  set<string> Variables;
  set<string> Caller;
};
