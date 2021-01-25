/*
Declarative Prototype: This is a declarative verifier. 

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

#include <set>
#include <map>
#include <string>

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
    }
    return true;
  }
  set<string> getVariable(){
    return Variables;
  }

private:
  set<string> Variables;
};

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
      Alpha(map<string, Validity>(Alpha)),
      Beta(map<string, set<string>>(Beta)),
      Gamma(map<string, set<string>>(Gamma))
      {}

  // Find declaration statements
  bool VisitVarDecl(VarDecl *Declaration) {
    
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

    return true;

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
      if (Alpha[name] == Undecided){
        GlobalPreconditions.insert(name);
      }
      reassign(name);
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
    return set<string>();
  }

private:
  ASTContext *Context;
  map<string, Function> *FunctionChanges;

  // Contexts
  map<string, Validity> Alpha;     // Variable to valid bit
  map<string, set<string>> Beta;   // Variable to clients
  map<string, set<string>> Gamma;  // Variable to dependencies

  set<string> GlobalPreconditions;  // Global Variables assumed to be true
  set<string> GlobalPostConditions; // Global Variables that exits as true

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

/*
DeclarativeCheckingConsumer:
1. Invokes DeclarativeCheckingFunctionVisitorGlobal on global variables
2. Creates a call graph
3. Invokes a unique DeclarativeCheckingFunctionVisitor to each function in call graph in postorder
*/
class DeclarativeCheckingConsumer : public clang::ASTConsumer {
public:
  explicit DeclarativeCheckingConsumer(ASTContext *Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    // Function map
    map<string, Function> FunctionChanges;

    // Work on global variables
    DeclarativeCheckingFunctionVisitorGlobal GlobalVisitor;
    GlobalVisitor.TraverseDecl(Context.getTranslationUnitDecl());

    // Work on functions in reverse call order
    set<string> Visited;
    CallGraph CG;
    CG.addToCallGraph(Context.getTranslationUnitDecl());
    CallGraphNode *root = CG.getRoot();
    PostTraverseCallGraph(root, Visited, GlobalVisitor, Context, FunctionChanges);
  }

private:
  // Post Traversal of CallGraph
  void PostTraverseCallGraph(CallGraphNode *root, set<string> &Visited, 
          DeclarativeCheckingFunctionVisitorGlobal &GlobalVisitor, ASTContext &Context, map<string, Function> &FunctionChanges){
    for (CallGraphNode *c : root->callees()){
      PostTraverseCallGraph(c, Visited, GlobalVisitor, Context, FunctionChanges);
    }
    if (root->getDecl() != NULL &&
          Visited.find(dyn_cast<FunctionDecl>(root->getDecl())->getNameAsString()) != Visited.end())
      return;

    DeclarativeCheckingFunctionVisitor Visitor(
        &Context,
        GlobalVisitor.getAlpha(),
        GlobalVisitor.getBeta(),
        GlobalVisitor.getGamma(),
        &FunctionChanges
    );
    Visitor.TraverseDecl(root->getDecl());

    for (string n : Visitor.getGlobalPreconditions()){
      llvm::outs() << n;
    }

    for (string n : Visitor.getGlobalPostConditions()){
      llvm::outs() << n;
    }

    if (root->getDecl() != NULL)
      Visited.insert(dyn_cast<FunctionDecl>(root->getDecl())->getNameAsString());
  }
};

class DeclarativeCheckingAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new DeclarativeCheckingConsumer(&Compiler.getASTContext()));
  }
};

static llvm::cl::OptionCategory MyToolCategory("My tool options");
int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<DeclarativeCheckingAction>().get());
}