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
            if (m_type != Pointer){
                llvm::outs() << "The variable " << m_name << "is not a pointer.";
            } else {
                llvm::outs() << "The variable " << m_name << "is a pointer.";
            }
            #endif
            return &m_pointsTo;
        }

        VariableType getType(){ 
            return m_type;
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
        DeclarationMatcher getPointerAndAddress(){
            return decl(forEachDescendant(decl(varDecl().bind("decl"))));
        }

    private:

        map<string, Node*> Graph;

};

class APAWorker : public MatchFinder::MatchCallback {
public :
  explicit APAWorker(map<string, Node*>* Graph){
      G = Graph;
  }

  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const Decl *D = Result.Nodes.getNodeAs<clang::Decl>("decl"))
      D->dump();
      G->insert(std::pair<string, Node*>("Hello", NULL));
  }

private :
  map<string, Node*>* G;
};
