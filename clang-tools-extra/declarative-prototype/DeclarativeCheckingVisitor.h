/* 
DeclarativeCheckingVisitor: This is a declarative verifier. 

Including the following coding features will produced undefined results.
  1. Pointers
  2. Recursive Functions
  3. Mutually Recursive Functions
  4. Classes and Structs
  5. If statements with variable declaration
  6. Scoping, global and reclaration in the function body
*/

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

#include <set>
#include <map>
#include <string>


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

class DeclarativeCheckingVisitor{
  
  public:
  /* 
  DeclarativeCheckingFunctionVisitor:
  A Declarative Checker that is works on function subtrees
  */
  class DeclarativeCheckingFunctionVisitor
    : public RecursiveASTVisitor<DeclarativeCheckingFunctionVisitor> {
  public:
    explicit DeclarativeCheckingFunctionVisitor(
      ASTContext *Context,
      map<string, Validity> Alpha,
      map<string, set<string>> Beta,
      map<string, set<string>> Gamma,
      map<string, Function> *FunctionChanges
    )
      : Context(Context),
        FunctionChanges(FunctionChanges),
        GlobalAlpha(map<string, Validity>(Alpha)),
        Alpha(map<string, Validity>(Alpha)),
        Beta(map<string, set<string>>(Beta)),
        Gamma(map<string, set<string>>(Gamma))
        {}

    // Find declaration statements
    bool VisitVarDecl(VarDecl *Declaration) {
      modify_env_at_declarations(Declaration);
      return true;
    }

    void modify_env_at_declarations(VarDecl *Declaration){

      string name = VarDeclToString(Declaration);

      // Modify Alpha, Beta, Gamma
      Expr* Initializer = Declaration->getInit();
      if (Declaration->getAnyInitializer() != NULL){
        CollectDeclRefExprVisitor CDREVisitor;
        CDREVisitor.TraverseStmt(dyn_cast<Stmt>(Initializer));
        Alpha.insert(std::pair<string, Validity>(name, Valid));
        Gamma.insert(std::pair<string, set<string>>(name, CDREVisitor.getVariable()));
        UpdateClients(name, CDREVisitor.getVariable());
      } else {
        Alpha.insert(std::pair<string, Validity>(name, Valid));
        Gamma.insert(std::pair<string, set<string>>(name, set<string>()));
      };
      Beta.insert(std::pair<string, set<string>>(name, set<string>()));
    }

    void modify_env_at_declarations(BinaryOperator *BinOperator){

      DeclRefExpr *Declaration = dyn_cast<DeclRefExpr>(BinOperator->getLHS());
      string name = DeclRefExprToString(Declaration);

      // Invalidate the clients
      reassign(name);

      // Let the dependants know that name is no longer a client
      DeregisterClients(name, Gamma[name]);

      // Collect variables
      Expr* Initializer = BinOperator->getRHS();
      CollectDeclRefExprVisitor CDREVisitor;
      CDREVisitor.TraverseStmt(dyn_cast<Stmt>(Initializer));

      // Modify Alpha, Beta, Gamma
      Alpha[name] = Valid;
      Gamma[name] = CDREVisitor.getVariable();
      UpdateClients(name, CDREVisitor.getVariable()); 
    }

    void reassign(string name){
      Alpha[name] = Valid;
      set<string> Clients = Beta[name];
      for (string c : Clients){
        Invalidate(c);
      }
    }

    // Find reassigment statements
    bool VisitBinaryOperator(BinaryOperator *BinOperator) {

      if (BinOperator->isAssignmentOp()) {
        DeclRefExpr *Declaration = dyn_cast<DeclRefExpr>(BinOperator->getLHS());
        string name = DeclRefExprToString(Declaration);

        if (GlobalAlpha.find(name) != GlobalAlpha.end()){
          GlobalPostConditions.insert(name);
        }

        if (Alpha[name] == Undecided){
          GlobalPreconditions.insert(name);
        }
        modify_env_at_declarations(BinOperator);
      }
      return true;
    }

    // Check if usage of variable is valid
    bool VisitDeclRefExpr(DeclRefExpr *Declaration) {

      if(Declaration == 0)
        return true;

      if (clang::VarDecl* VD = dyn_cast<clang::VarDecl>(Declaration->getDecl())){
        string name = VarDeclToString(VD);

        if (Alpha[name] == Undecided){
          GlobalPreconditions.insert(name);
          Alpha[name] = Valid;
        }

        FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());
        if (Alpha[name] == Invalid){
          llvm::outs() << "WARNING! " << name << " is not updated! ";
          if (FullLocation.isValid())
            llvm::outs() << "This error is at: "
                          << FullLocation.getSpellingLineNumber() << ":"
                          << FullLocation.getSpellingColumnNumber() << "\n";
        }
      }
      return true;
    }

    bool TraverseFunctionDecl(FunctionDecl *functionDecl){
      for (unsigned int i=0; i<functionDecl->getNumParams(); i++){
        string name = functionDecl->getParamDecl(i)->getNameAsString();
        FunctionName = name;
        Alpha.insert(pair<string, Validity>(name, Valid));
        Beta.insert(pair<string, set<string>>(name, set<string>()));
        Gamma.insert(pair<string, set<string>>(name, set<string>()));
      }
      TraverseStmt(functionDecl->getBody());
      return true;
    }

    bool TraverseUnaryOperator(UnaryOperator *unaryOperator){
      if (unaryOperator->isIncrementOp() || unaryOperator->isDecrementOp()){
        Expr* var = unaryOperator->getSubExpr();
        string varname = DeclRefExprToString(dyn_cast<DeclRefExpr>(var));
        if (Alpha[varname] ==  Undecided){
          GlobalPreconditions.insert(varname);
          Alpha[varname] = Valid;
        }
        reassign(varname);
      } else {
        // TODO: Fix this to unaryOperator body 
        RecursiveASTVisitor<DeclarativeCheckingFunctionVisitor>::TraverseUnaryOperator(unaryOperator);
      }
      return true;
    }

    bool TraverseCallExpr(CallExpr *CallExpr){

      // Work on arguments of Callee
      for (CallExpr::arg_iterator i = CallExpr->arg_begin(); i != CallExpr->arg_end(); ++i){ 
          Stmt *arg = dyn_cast<Stmt>(*i);
          TraverseStmt(arg);
      } 
      
      // Get name of callee
      Expr* Callee = CallExpr->getCallee();
      CollectDeclRefExprVisitor CDREVisitor;
      CDREVisitor.TraverseStmt(dyn_cast<Stmt>(Callee));
      if (CDREVisitor.getCaller().size() != 1)
        llvm::outs() << "FOUND A CALL EXPRESSION WITH MULTILPE DECLS\n";
      string CalleeName = *(CDREVisitor.getCaller().begin());

      // Use FunctionChanges
      auto PFun = FunctionChanges->find(CalleeName);
      if (PFun != FunctionChanges->end()){
        set<string> Preconditions = PFun->second.Preconditions;
        set<string> Postconditions = PFun->second.Postconditions;

        // Check for preconditions
        for (string s : Preconditions){
          if (Alpha[s] != Valid)
            llvm::outs() << "WARNING! " << s << " is not updated before the function call " << CalleeName << "!\n";
        }

        // Set postconditions
        for (string p : Postconditions){
          reassign(p);
        }

      } else {
        llvm::outs() << "DID NOT FIND CALL FUNCTION FOR " << CalleeName << "\n";
      }

      return true;
    }

    // Do recursive check inside IfStmt
    bool TraverseIfStmt(IfStmt *ifStmt) {

      Stmt* thenStmt = ifStmt->getThen();
      Stmt* elseStmt = ifStmt->getElse();

      if (elseStmt){
        // Recurse on body
        DeclarativeCheckingFunctionVisitor Visitor1(Context, Alpha, Beta, Gamma, FunctionChanges);
        Visitor1.TraverseStmt(thenStmt);

        // Recurse on else
        DeclarativeCheckingFunctionVisitor Visitor2(Context, Alpha, Beta, Gamma, FunctionChanges);
        Visitor2.TraverseStmt(elseStmt);

        // Get Alphas
        map<string, Validity> Alpha1 = Visitor1.getAlpha();
        map<string, Validity> Alpha2 = Visitor2.getAlpha();

        // Union
        for (auto const& pair : Alpha){
          string var = pair.first;
          if (Alpha1[var] == Valid && Alpha2[var] == Valid){
            Alpha[var] = Valid;
          } else {
            Alpha[var] = Invalid;
          }
        }
      } else {
        // Recurse on body
        DeclarativeCheckingFunctionVisitor Visitor1(Context, Alpha, Beta, Gamma, FunctionChanges);
        Visitor1.TraverseStmt(thenStmt);

        // Get Alpha
        map<string, Validity> Alpha1 = Visitor1.getAlpha();

        // Union
        for (auto const& pair : Alpha){
          string var = pair.first;
          if (Alpha[var] == Valid && Alpha1[var] == Valid){
            Alpha[var] = Valid;
          } else {
            Alpha[var] = Invalid;
          }
        }

      }
      return true;
    }

    bool TraverseForStmt(ForStmt *forStmt) {
      // Get init variable
      Stmt* initStmt = forStmt->getInit();
      DeclarativeCheckingFunctionVisitor InitFinder(Context, Alpha, Beta, Gamma, FunctionChanges);
      InitFinder.TraverseStmt(initStmt);
      map<string, Validity> Alpha1 = InitFinder.getAlpha();
      map<string, set<string>> Beta1 = InitFinder.getBeta();
      map<string, set<string>> Gamma1 = InitFinder.getGamma();

      // Get Body
      Stmt* bodyStmt = forStmt->getBody();
      DeclarativeCheckingFunctionVisitor Visitor(Context, Alpha1, Beta1, Gamma1, FunctionChanges);
      Visitor.TraverseStmt(bodyStmt);
      map<string, Validity> Alpha2 = Visitor.getAlpha();

      // Union
      for (auto const& pair : Alpha){
        string var = pair.first;
        if (Alpha[var] == Valid && Alpha2[var] == Valid){
          Alpha[var] = Valid;
        } else {
          Alpha[var] = Invalid;
        }
      }
      return true;
    }

    map<string, Validity> getAlpha(){
      return Alpha;
    }

    map<string, set<string>> getBeta(){
      return Beta;
    }

    map<string, set<string>> getGamma(){
      return Gamma;
    }

    set<string> getGlobalPreconditions(){
      return GlobalPreconditions;
    }

    set<string> getGlobalPostConditions(){
      return GlobalPostConditions;
    }

    Function getFunctionDetails(){
      return {
        .name = FunctionName,
        .Preconditions = getGlobalPreconditions(),
        .Postconditions = getGlobalPostConditions(),
      };
    }

  private:
    ASTContext *Context;
    map<string, Function> *FunctionChanges;

    // Contexts
    map<string, Validity> Alpha;     // Variable to valid bit
    map<string, set<string>> Beta;   // Variable to clients
    map<string, set<string>> Gamma;  // Variable to dependencies

    string FunctionName;

    set<string> GlobalPreconditions;  // Global Variables assumed to be true
    set<string> GlobalPostConditions; // Global Variables that exits as true

    map<string, Validity> GlobalAlpha;          // Alpha for Global Variables

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

    void DeregisterClients(string Var, set<string> Dependencies) {
      set<string>::iterator Dependency = Dependencies.begin();
      while (Dependency != Dependencies.end())
      {
        string DependencyString{*Dependency};
        if (Beta.find(DependencyString) == Beta.end())
          Beta.insert(std::pair<string, set<string>>(DependencyString, set<string>()));
        Beta[DependencyString].erase(Var);
        Dependency++;
      }
    }

    void Invalidate(string Var) {
      Alpha[Var] = Invalid;
      set<string> Clients = Beta[Var];
      for (string c : Clients){
        Invalidate(c);
      }
    }

    void CheckReassigment(set<string> Vars) {
      for (string v : Vars){
        if (Alpha[v] == Invalid)
          llvm::outs() << "WARNING: The variable " << v << " is not updated.";
      }
    }

  };

  /* 
  DeclarativeCheckingFunctionVisitorGlobal:
  A Declarative Checker that is works global variable declarations before functions are defined.
  */
  class DeclarativeCheckingFunctionVisitorGlobal
    : public RecursiveASTVisitor<DeclarativeCheckingFunctionVisitorGlobal> {
  public:
    explicit DeclarativeCheckingFunctionVisitorGlobal() {}

    // Find declaration statements
    bool VisitVarDecl(VarDecl *Declaration) {
      string name = VarDeclToString(Declaration);

      // Modify Alpha, Beta, Gamma
      Expr* Initializer = Declaration->getInit();
      if (Declaration->getAnyInitializer() != NULL){
        CollectDeclRefExprVisitor CDREVisitor;
        CDREVisitor.TraverseStmt(dyn_cast<Stmt>(Initializer));
        Alpha.insert(std::pair<string, Validity>(name, Undecided));
        Gamma.insert(std::pair<string, set<string>>(name, CDREVisitor.getVariable()));
        UpdateClients(name, CDREVisitor.getVariable());
      } else {
        Alpha.insert(std::pair<string, Validity>(name, Undecided));
        Gamma.insert(std::pair<string, set<string>>(name, set<string>()));
      };
      Beta.insert(std::pair<string, set<string>>(name, set<string>()));

      return true;

    }

    bool TraverseFunctionDecl(FunctionDecl *functionDecl){
      return true;
    }
    
    map<string, Validity> getAlpha(){
      return Alpha;
    }

    map<string, set<string>> getBeta(){
      return Beta;
    }

    map<string, set<string>> getGamma(){
      return Gamma;
    }

  private:
    map<string, Validity> Alpha;
    map<string, set<string>> Beta;
    map<string, set<string>> Gamma;

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

  };

};