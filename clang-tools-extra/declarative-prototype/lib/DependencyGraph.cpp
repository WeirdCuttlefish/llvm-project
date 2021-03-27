//==--- DependencyGraph.cpp - Dependency graph of variables ---------------===//
//
//===----------------------------------------------------------------------===//

// #include "llvm/Support/CommandLine.h"

#include "DependencyGraph.h"

#include <map>
#include <set>
#include <stack>
#include <string>


using namespace std;

using namespace declarative;

// Declarative graph that keeps track of variables. Maintins the
// invariant that if a variable is in the graph, then it is valid.
class DependencyGraph::DependencyGraphImpl {

public:

  // Empty DeclarativeGraph
  DependencyGraphImpl(){};

  // Destructor
  ~DependencyGraphImpl(){
    for (pair<string, Node*> P : VarToNode){
      delete(P.second);
    }
  };

  // Removes variables from the graph
  void remove(const string Var){
    Node *Target = VarToNode[Var];
    set<Node*> TempHeadOf = Target->HeadOf;
    set<Node*> TempTailOf = Target->TailOf;
    for (Node *N : TempHeadOf){
      N->TailOf.erase(Target);
    }
    for (Node *N : TempTailOf){
      N->TailOf.erase(Target);
    }
    delete(Target);
    VarToNode.erase(Var);
  };

  // Inserts variables in the graph with dependencies
  void insert(const string Var, const set<string> Rhs){
    // Check if dependent variables are defined already
    set<Node*> TempHeadOf;
    for (string Dep : Rhs){
      if (VarToNode.find(Dep) == VarToNode.end()){
        // llvm::outs() << "ERROR: The variable " << Dep << " is undeclared"
        //  " in this graph when assigning to " << Var << "\n";
      } else {
        TempHeadOf.insert(VarToNode[Dep]);
      }
    }
    // Add to the set of nodes
    Node *VarNode = new Node();
    VarNode->HeadOf = TempHeadOf;
    // Let dependents know they are dependents
    for (Node *N : TempHeadOf){
      N->TailOf.insert(VarNode);
    }
  }

  // Finds reachable variables in the graph
  const set<string> reachable(const string Var){
    set<string> Visited;
    stack<Node*> Stack;
    Stack.push(VarToNode[Var]);

    while (!Stack.empty()){
      Node* CurrNode = Stack.top();
      Stack.pop();
      if (Visited.find(CurrNode->Name) != Visited.end()){
        Visited.insert(Var);
        for (Node *N : VarToNode[Var]->TailOf){
          Stack.push(N);
        }
      }
    }

    return Visited;
  }

  // Ignores unwanted variables in the graph
  void ignore(const set<string> UnwantedVars){
  }

  // Figure out if the variable is present in the graph
  bool isPresent(const string Variable){
    return VarToNode.find(Variable) != VarToNode.end();
  };

  // Figure out if the variable is absent in the graph
  bool isAbsent(const string Variable){
    return !isPresent(Variable);
  };

private:

  struct Node {
    string Name;
    set<Node*> TailOf;
    set<Node*> HeadOf;
  };

  map<string, Node*> VarToNode;

};

DependencyGraph::DependencyGraph():
  Pimpl(new DependencyGraph::DependencyGraphImpl()){};
DependencyGraph::~DependencyGraph()
  {delete(DependencyGraph::Pimpl);};

// Removes variables from the graph
void DependencyGraph::remove(const string Var){ Pimpl->remove(Var); };

// Inserts variables in the graph with dependencies
void DependencyGraph::insert(const string Var, const set<string> Rhs){
  Pimpl->insert(Var, Rhs);
};

// Finds reachable variables in the graph
const set<string> DependencyGraph::reachable(const string Var){
  return Pimpl->reachable(Var);
};

// Ignores unwanted variables in the graph
void DependencyGraph::ignore(const set<string> UnwantedVars){
  Pimpl->ignore(UnwantedVars);
};

// Figure out if the variable is present in the graph
bool DependencyGraph::isPresent(const string Variable){
  return Pimpl->isPresent(Variable);
};

// Figure out if the variable is absent in the graph
bool DependencyGraph::isAbsent(const string Variable){
  return Pimpl->isAbsent(Variable);
};

