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
class DeclarativeCheckingVisitor
  : public RecursiveASTVisitor<DeclarativeCheckingVisitor> {
public:
  explicit DeclarativeCheckingVisitor(
    ASTContext *Context,
    map<string, bool> Alpha,
    map<string, set<string>> Beta,
    map<string, set<string>> Gamma
  )
    : Context(Context),
      Alpha(map<string, bool>(Alpha)),
      Beta(map<string, set<string>>(Beta)),
      Gamma(map<string, set<string>>(Gamma))
      {}

  // Find declaration statements
  bool VisitVarDecl(VarDecl *Declaration) {
    string name = VarDeclToString(Declaration);

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
      string name = DeclRefExprToString(Declaration);
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
    string name = DeclRefExprToString(Declaration);
    FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());
    if (Alpha[name] == false){
      llvm::outs() << "WARNING! " << name << " is not updated! ";
      if (FullLocation.isValid())
        llvm::outs() << "This error is at: "
                      << FullLocation.getSpellingLineNumber() << ":"
                      << FullLocation.getSpellingColumnNumber() << "\n";
    };
    return true;
  }

  // Do recursive check inside IfStmt
  bool TraverseIfStmt(IfStmt *ifStmt) {

    Stmt* thenStmt = ifStmt->getThen();
    Stmt* elseStmt = ifStmt->getElse();

    if (elseStmt){
      // Recurse on body
      DeclarativeCheckingVisitor Visitor1(Context, Alpha, Beta, Gamma);
      Visitor1.TraverseStmt(thenStmt);

      // Recurse on else
      DeclarativeCheckingVisitor Visitor2(Context, Alpha, Beta, Gamma);
      Visitor2.TraverseStmt(elseStmt);

      // Get Alphas
      map<string, bool> Alpha1 = Visitor1.getAlpha();
      map<string, bool> Alpha2 = Visitor2.getAlpha();

      // Union
      for (auto const& pair : Alpha){
        string var = pair.first;
        if (Alpha1[var] && Alpha2[var]){
          Alpha[var] = true;
        } else {
          Alpha[var] = false;
        }
      }
    } else {
      // Recurse on body
      DeclarativeCheckingVisitor Visitor1(Context, Alpha, Beta, Gamma);
      Visitor1.TraverseStmt(thenStmt);

      // Get Alpha
      map<string, bool> Alpha1 = Visitor1.getAlpha();

      // Union
      for (auto const& pair : Alpha){
        string var = pair.first;
        if (Alpha[var] && Alpha1[var]){
          Alpha[var] = true;
        } else {
          Alpha[var] = false;
        }
      }

    }
    return true;
  }

  bool TraverseForStmt(ForStmt *forStmt) {
    // Get init variable
    Stmt* initStmt = forStmt->getInit();
    DeclarativeCheckingVisitor InitFinder(Context, Alpha, Beta, Gamma);
    InitFinder.TraverseStmt(initStmt);
    map<string, bool> Alpha1 = InitFinder.getAlpha();
    map<string, set<string>> Beta1 = InitFinder.getBeta();
    map<string, set<string>> Gamma1 = InitFinder.getGamma();

    // Get Body
    Stmt* bodyStmt = forStmt->getBody();
    DeclarativeCheckingVisitor Visitor(Context, Alpha1, Beta1, Gamma1);
    Visitor.TraverseStmt(bodyStmt);
    map<string, bool> Alpha2 = Visitor.getAlpha();

    // Union
    for (auto const& pair : Alpha){
      string var = pair.first;
      if (Alpha[var] && Alpha2[var]){
        Alpha[var] = true;
      } else {
        Alpha[var] = false;
      }
    }
    return true;
  }

  map<string, bool> getAlpha(){
    return Alpha;
  }

  map<string, set<string>> getBeta(){
    return Beta;
  }

  map<string, set<string>> getGamma(){
    return Gamma;
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

class DeclarativeCheckingConsumer : public clang::ASTConsumer {
public:
  explicit DeclarativeCheckingConsumer(ASTContext *Context)
    : Visitor(Context, map<string, bool>(), map<string, set<string>>(), map<string, set<string>>()) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  DeclarativeCheckingVisitor Visitor;
};

class DeclarativeCheckingAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new DeclarativeCheckingConsumer(&Compiler.getASTContext()));
  }
};

int main(int argc, char **argv) {
  if (argc > 1) {
    clang::tooling::runToolOnCode(std::make_unique<DeclarativeCheckingAction>(), argv[1]);
  }
}