#include<set>
#include<map>
#include<string>

#include "llvm/Support/CommandLine.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace std;
using namespace clang;
using namespace ast_matchers;

#define DEBUG

enum VariableType {Pointer, Variable};

class Node{
    public:
        
        explicit Node(std::string name, VariableType type){
            m_name = name;
            m_pointsTo = std::set<Node*>();
            m_type = type;
        }
        explicit Node(Node &node){
            m_pointsTo = node.m_pointsTo;
            m_type = node.m_type;
        }

        std::string getName(){
            return m_name;
        }

        std::set<Node*>* getPointsTo(){
            #ifdef DEBUG
            /*
            if (m_type != Pointer){
                llvm::outs() << "The variable " << m_name << " is not a pointer.";
            } else {
                llvm::outs() << "The variable " << m_name << " is a pointer.";
            }
            */
            #endif
            return &m_pointsTo;
        }

        VariableType getType(){ 
            return m_type;
        }

        std::string toString(){
            std::string out = "Node " + m_name + ": ";
            for (Node* n : m_pointsTo){
                out += (n->getName() + ", ");
            }
            return out;
        }

    private:
        std::string m_name;
        std::set<Node*> m_pointsTo;
        VariableType m_type;
        
};

class APAGraph{
    public:

        explicit APAGraph(){}

        // These methods are inspired by http://pages.cs.wisc.edu/~fischer/cs701.f08/lectures/Lecture26.4up.pdf
        void pointerTakesAddress(Node &lhsVar, Node &rhsVar){
            lhsVar.getPointsTo()->insert(&rhsVar);
        }

        void pointerTakesCopy(Node &lhsVar, Node &rhsVar){
            for (auto n : *(rhsVar.getPointsTo())){
                lhsVar.getPointsTo()->insert(n);
            }
        }

        void pointerTakesDereference(Node &lhsVar, Node &rhsVar){
            for (auto n1 : *(rhsVar.getPointsTo())){
                for (auto n2 : *(n1->getPointsTo())){
                    lhsVar.getPointsTo()->insert(n2);
                }
            }
        }

        void dereferencedPointerTakesAddress(Node &lhsVar, Node &rhsVar){
            for (auto n : *(lhsVar.getPointsTo())){
                n->getPointsTo()->insert(&rhsVar);
            }
        }

        void dereferencedPointerTakesCopy(Node &lhsVar, Node &rhsVar){
            for (auto n1: *(lhsVar.getPointsTo())){
                for (auto n2: *(rhsVar.getPointsTo())){
                    n1->getPointsTo()->insert(n2);
                }
            }
        }

        void dereferencedPointerTakesDereference(Node &lhsVar, Node &rhsVar){
            for (auto n1: *(lhsVar.getPointsTo())){
                for (auto n2: *(rhsVar.getPointsTo())){
                    for (auto n3: *(n2->getPointsTo())){
                        n1->getPointsTo()->insert(n3);
                    }
                }
            }
        }

        // Matchers
        DeclarationMatcher getPointerTakesCopy(){
            return decl(forEachDescendant(decl(
                varDecl(
                    hasType(isAnyPointer()), 
                    hasInitializer(implicitCastExpr(hasSourceExpression(declRefExpr())))
                ).bind("decl1")
            )));
        }

        DeclarationMatcher getPointerTakesAddress(){
            return decl(forEachDescendant(decl(
                varDecl(
                    hasType(isAnyPointer()), 
                    hasInitializer(unaryOperator(hasOperatorName("&")))
                ).bind("decl2")
            )));
        }

        DeclarationMatcher getPointerTakesDereference(){
            return decl(forEachDescendant(decl(
                varDecl(
                    hasType(isAnyPointer()), 
                    hasInitializer(implicitCastExpr(hasSourceExpression(unaryOperator(hasOperatorName("*")))))
                ).bind("decl3")
            )));
        }

        DeclarationMatcher getDereferencedPointerTakesCopy(){
            return decl(forEachDescendant(
                binaryOperator(
                    isAssignmentOperator(),
                    hasLHS((unaryOperator(hasOperatorName("*")))), 
                    hasRHS(implicitCastExpr(hasSourceExpression(declRefExpr())))
                ).bind("decl4")
            ));
        }

        DeclarationMatcher getDereferencedPointerTakesAddress(){
            return decl(forEachDescendant(
                binaryOperator(
                    isAssignmentOperator(),
                    hasLHS((unaryOperator(hasOperatorName("*")))), 
                    hasRHS(unaryOperator(hasOperatorName("&")))
                ).bind("decl5")
            ));
        }

        DeclarationMatcher getDereferencedPointerTakesDereference(){
            return decl(forEachDescendant(
                binaryOperator(
                    isAssignmentOperator(),
                    hasLHS((unaryOperator(hasOperatorName("*")))), 
                    hasRHS(implicitCastExpr(hasSourceExpression(unaryOperator(hasOperatorName("*")))))
                ).bind("decl6")
            ));
        }

    private:

        map<string, Node*> Graph;

};

class APAPointerTakesCopyWorker : public MatchFinder::MatchCallback {
public :
  explicit APAPointerTakesCopyWorker(map<string, Node*>* Graph){
      G = Graph;
  }

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const Decl *ResultNode = Result.Nodes.getNodeAs<clang::Decl>("decl1")){
      string LVarName = dyn_cast<VarDecl>(ResultNode)->getNameAsString();
      string RVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<ImplicitCastExpr>(
          dyn_cast<VarDecl>(ResultNode)->getInit()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      if (G->find(LVarName) == G->end()){
          G->insert(pair<string, Node*>(LVarName, new Node(LVarName, Pointer)));
      }
      if (G->find(RVarName) == G->end()){
          G->insert(pair<string, Node*>(RVarName, new Node(RVarName, Pointer)));
      }
      APAGraph GraphUtil;
      GraphUtil.pointerTakesCopy(*(G->at(LVarName)), *(G->at(RVarName)));
      #ifdef DEBUG
      llvm::outs() << "FINISHED PONTER TAKES COPY\n";
      #endif
    }
  }

private :
  map<string, Node*>* G;
};

class APAPointerTakesAddressWorker : public MatchFinder::MatchCallback {
public :
  explicit APAPointerTakesAddressWorker(map<string, Node*>* Graph){
      G = Graph;
  }

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const Decl *ResultNode = Result.Nodes.getNodeAs<clang::Decl>("decl2")){
      string LVarName = dyn_cast<VarDecl>(ResultNode)->getNameAsString();
      string RVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<UnaryOperator>(
          dyn_cast<VarDecl>(ResultNode)->getInit()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      if (G->find(LVarName) == G->end()){
          G->insert(pair<string, Node*>(LVarName, new Node(LVarName, Pointer)));
      }
      if (G->find(RVarName) == G->end()){
          G->insert(pair<string, Node*>(RVarName, new Node(RVarName, Pointer)));
      }
      APAGraph GraphUtil;
      GraphUtil.pointerTakesAddress(*(G->at(LVarName)), *(G->at(RVarName)));
      #ifdef DEBUG
      llvm::outs() << "FINISHED PONTER TAKES ADDRESS\n";
      #endif
    }
  }

private :
  map<string, Node*>* G;
};

class APAPointerTakesDereferenceWorker : public MatchFinder::MatchCallback {
public :
  explicit APAPointerTakesDereferenceWorker(map<string, Node*>* Graph){
      G = Graph;
  }

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const Decl *ResultNode = Result.Nodes.getNodeAs<clang::Decl>("decl3")){
      string LVarName = dyn_cast<VarDecl>(ResultNode)->getNameAsString();
      string RVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<ImplicitCastExpr>(
          dyn_cast<UnaryOperator>(
            dyn_cast<ImplicitCastExpr>(
              dyn_cast<VarDecl>(ResultNode)->getInit()
            )->getSubExpr()
          )->getSubExpr()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      if (G->find(LVarName) == G->end()){
          G->insert(pair<string, Node*>(LVarName, new Node(LVarName, Pointer)));
      }
      if (G->find(RVarName) == G->end()){
          G->insert(pair<string, Node*>(RVarName, new Node(RVarName, Pointer)));
      }
      APAGraph GraphUtil;
      GraphUtil.pointerTakesDereference(*(G->at(LVarName)), *(G->at(RVarName)));
      #ifdef DEBUG
      llvm::outs() << "FINISHED PONTER TAKES DEREFERENCED\n";
      #endif
    }
  }
private :
  map<string, Node*>* G;
};

class APADereferencedPointerTakesCopyWorker : public MatchFinder::MatchCallback {
public :
  explicit APADereferencedPointerTakesCopyWorker(map<string, Node*>* Graph){
      G = Graph;
  }

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const BinaryOperator *ResultNode = Result.Nodes.getNodeAs<clang::BinaryOperator>("decl4")){
      string LVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<ImplicitCastExpr>(
          dyn_cast<UnaryOperator>(
            dyn_cast<BinaryOperator>(ResultNode)->getLHS()
          )->getSubExpr()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      string RVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<ImplicitCastExpr>(
          dyn_cast<BinaryOperator>(ResultNode)->getRHS()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      if (G->find(LVarName) == G->end()){
          G->insert(pair<string, Node*>(LVarName, new Node(LVarName, Pointer)));
      }
      if (G->find(RVarName) == G->end()){
          G->insert(pair<string, Node*>(RVarName, new Node(RVarName, Pointer)));
      }
      APAGraph GraphUtil;
      GraphUtil.dereferencedPointerTakesCopy(*(G->at(LVarName)), *(G->at(RVarName)));
      #ifdef DEBUG
      llvm::outs() << "FINISHED DEREFERENCED PONTER TAKES COPY\n";
      #endif
    }
  }

private :
  map<string, Node*>* G;
};

class APADereferencedPointerTakesAddressWorker : public MatchFinder::MatchCallback {
public :
  explicit APADereferencedPointerTakesAddressWorker(map<string, Node*>* Graph){
      G = Graph;
  }

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const BinaryOperator *ResultNode = Result.Nodes.getNodeAs<clang::BinaryOperator>("decl5")){
      string LVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<ImplicitCastExpr>(
          dyn_cast<UnaryOperator>(
            dyn_cast<BinaryOperator>(ResultNode)->getLHS()
          )->getSubExpr()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      string RVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<UnaryOperator>(
          dyn_cast<BinaryOperator>(ResultNode)->getRHS()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      if (G->find(LVarName) == G->end()){
          G->insert(pair<string, Node*>(LVarName, new Node(LVarName, Pointer)));
      }
      if (G->find(RVarName) == G->end()){
          G->insert(pair<string, Node*>(RVarName, new Node(RVarName, Pointer)));
      }
      APAGraph GraphUtil;
      GraphUtil.dereferencedPointerTakesAddress(*(G->at(LVarName)), *(G->at(RVarName)));
      #ifdef DEBUG
      llvm::outs() << "FINISHED DEREFERENCED PONTER TAKES ADDRESS\n";
      #endif
    }
  }

private :
  map<string, Node*>* G;
};

class APADereferencedPointerTakesDereferenceWorker : public MatchFinder::MatchCallback {
public :
  explicit APADereferencedPointerTakesDereferenceWorker(map<string, Node*>* Graph){
      G = Graph;
  }

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const BinaryOperator *ResultNode = Result.Nodes.getNodeAs<clang::BinaryOperator>("decl6")){
      string LVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<ImplicitCastExpr>(
          dyn_cast<UnaryOperator>(
            dyn_cast<BinaryOperator>(ResultNode)->getLHS()
          )->getSubExpr()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      string RVarName = dyn_cast<DeclRefExpr>(
        dyn_cast<ImplicitCastExpr>(
          dyn_cast<UnaryOperator>(
            dyn_cast<ImplicitCastExpr>(
              dyn_cast<BinaryOperator>(ResultNode)->getRHS()
            )->getSubExpr()
          )->getSubExpr()
        )->getSubExpr()
      )->getNameInfo().getAsString();
      if (G->find(LVarName) == G->end()){
          G->insert(pair<string, Node*>(LVarName, new Node(LVarName, Pointer)));
      }
      if (G->find(RVarName) == G->end()){
          G->insert(pair<string, Node*>(RVarName, new Node(RVarName, Pointer)));
      }
      APAGraph GraphUtil;
      GraphUtil.dereferencedPointerTakesDereference(*(G->at(LVarName)), *(G->at(RVarName)));
      #ifdef DEBUG
      llvm::outs() << "FINISHED DEREFERENCED PONTER TAKES DEREFERENCE\n";
      #endif
    }
  }

private :
  map<string, Node*>* G;
};


class APAMatchFinderUtil {
public :
  explicit APAMatchFinderUtil(ASTContext *Context) : 
    Context(Context),
    Graph(new map<string, Node*>())
    {}

  void run(){
    MatchFinder Finder;
    APAGraph APAUtils;

    APAPointerTakesCopyWorker Worker1(Graph);
    APAPointerTakesAddressWorker Worker2(Graph);
    APAPointerTakesDereferenceWorker Worker3(Graph);
    APADereferencedPointerTakesCopyWorker Worker4(Graph);
    APADereferencedPointerTakesAddressWorker Worker5(Graph);
    APADereferencedPointerTakesDereferenceWorker Worker6(Graph);

    Finder.addMatcher(APAUtils.getPointerTakesCopy(), &Worker1);
    Finder.addMatcher(APAUtils.getPointerTakesAddress(), &Worker2);
    Finder.addMatcher(APAUtils.getPointerTakesDereference(), &Worker3);
    Finder.addMatcher(APAUtils.getDereferencedPointerTakesCopy(), &Worker4);
    Finder.addMatcher(APAUtils.getDereferencedPointerTakesAddress(), &Worker5);
    Finder.addMatcher(APAUtils.getDereferencedPointerTakesDereference(), &Worker6);

    Finder.match(*(Context->getTranslationUnitDecl()), *Context);
  }

  map<string, Node*>* getGraph(){
      return Graph;
  }

private :
  ASTContext *Context;
  map<string, Node*>* Graph;

};