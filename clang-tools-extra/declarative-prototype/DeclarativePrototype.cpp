#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <set>
#include <map>
#include <string>

using namespace clang;
using namespace std;

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

void PrintAlpha(map<string, bool> m) {
  llvm::outs() << "Alpha Begin:\n";
  map<string, bool>::iterator it;
  for (it = m.begin(); it != m.end(); it++)
  {
    llvm::outs() << "Variable " << it->first << " is: " << it->second << "\n";
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

// Collect variables inside expression
class CollectDeclRefExprVisitor
  : public RecursiveASTVisitor<CollectDeclRefExprVisitor> {
public:
  explicit CollectDeclRefExprVisitor(ASTContext *Context)
    : Context(Context) {}

  bool VisitDeclRefExpr(DeclRefExpr *Declaration) {
    string name = Declaration->getNameInfo().getAsString();
    Variables.insert(name);
    return true;
  }

  set<string> getVariable(){
    return Variables;
  }

private:
  ASTContext *Context;   // Probably important in the future for analysis with functions
  set<string> Variables;
};

// Recurse through all AST
class FindNamedClassVisitor
  : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext *Context)
    : Context(Context) {}

  // Find declaration statements
  bool VisitVarDecl(VarDecl *Declaration) {
    string name = Declaration->getNameAsString();

    // Modify Alpha, Beta, Gamma
    Expr* Initializer = Declaration->getInit();
    if (Declaration->getAnyInitializer() != NULL){
      CollectDeclRefExprVisitor CDREVisitor(Context);
      CDREVisitor.TraverseStmt(dyn_cast<Stmt>(Initializer));
      Alpha.insert(std::pair<string, bool>(name, true));
      Gamma.insert(std::pair<string, set<string>>(name, CDREVisitor.getVariable()));
      UpdateClients(name, CDREVisitor.getVariable());
    } else {
      Alpha.insert(std::pair<string, bool>(name, true));
      Gamma.insert(std::pair<string, set<string>>(name, set<string>()));
    };
    Beta.insert(std::pair<string, set<string>>(name, set<string>()));

    // Debugging print all
    /*
    llvm::outs() << "---------------  ROUND    --------------------\n";
    PrintAlpha(Alpha);
    PrintBeta(Beta);
    PrintGamma(Gamma);
    llvm::outs() << "----------------------------------------------\n";
    */
   
    return true;
    
  }

  // Find reassigment statements
  bool VisitBinaryOperator(BinaryOperator *BinOperator) {
    if (BinOperator->isAssignmentOp()) {
      DeclRefExpr *Declaration = dyn_cast<DeclRefExpr>(BinOperator->getLHS());
      string name = Declaration->getNameInfo().getAsString();
      Expr *Rhs = BinOperator->getRHS();
      
      // CollectDeclRefExprVisitor CDREVisitor(Context);
      // CDREVisitor.TraverseStmt(dyn_cast<Stmt>(Rhs));
      // CheckReassigment(CDREVisitor.getVariable());

      Alpha[name] = true;
      set<string> Clients = Beta[name];
      for (string c : Clients){
        Invalidate(c);
      }
    }
    return true;
  }

  // Check if usage of variable is valid
  bool VisitDeclRefExpr(DeclRefExpr *Declaration) {
    string name = Declaration->getNameInfo().getAsString();
    if (Alpha[name] == false){
      llvm::outs() << "WARNING: " << name << " is not updated!\n";
    };
    return true;
  }

private:
  ASTContext *Context;

  // Contexts
  map<string, bool> Alpha;         // Variable to valid bit
  map<string, set<string>> Beta;   // Variable to clients
  map<string, set<string>> Gamma;  // Variable to dependencies

  void UpdateClients(string Var, set<string> Dependencies) {
    set<string>::iterator Dependency = Dependencies.begin();
    while (Dependency != Dependencies.end())
    {
      string DependencyString{*Dependency};
      if (Beta.find(DependencyString) == Beta.end())
        Beta.insert(std::pair<string, set<string>>(DependencyString, set<string>()));
      Beta[DependencyString].insert(Var);
      Dependency++;
    }
  }

  void Invalidate(string Var) {
    Alpha[Var] = false;
    set<string> Clients = Beta[Var];
    for (string c : Clients){
      Invalidate(c);
    }
  }

  void CheckReassigment(set<string> Vars) {
    for (string v : Vars){
      if (Alpha[v] == false){
        llvm::outs() << "WARNING: The variable " << v << " is not updated."; 
      }
    }
  }

};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
  explicit FindNamedClassConsumer(ASTContext *Context)
    : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new FindNamedClassConsumer(&Compiler.getASTContext()));
  }
};

int main(int argc, char **argv) {
  if (argc > 1) {
    clang::tooling::runToolOnCode(std::make_unique<FindNamedClassAction>(), argv[1]);
  }
}