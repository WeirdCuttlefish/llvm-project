//==--- DependencyGraph.cpp - Dependency graph of variables ---------------===//
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Declarative/DependencyGraph.h"
#include "llvm/Support/CommandLine.h"

#include <list>
#include <map>
#include <set>
#include <stack>
#include <string>


struct Hello{
  set<string> S;
};

using namespace std;

using namespace declarative;

// Declarative graph that keeps track of variables. Maintins the
// invariant that if a variable is in the graph, then it is valid.
//
class DependencyGraphElementImpl {

public:

  // Empty DeclarativeGraph
  DependencyGraphElementImpl(){}

  // Copy Constructor
  DependencyGraphElementImpl(const DependencyGraphElementImpl &GraphIn){
    for (pair<string, Node*> P : GraphIn.VarToNode){
      this->VarToNode[P.first] = new Node();
      this->VarToNode[P.first]->Name = P.first;
    }
    for (pair<string, Node*> P : this->VarToNode){
      for (Node* N : GraphIn.VarToNode.at(P.first)->TailOf){
        P.second->TailOf.insert(this->VarToNode[N->Name]);
      }
      for (Node* N : GraphIn.VarToNode.at(P.first)->HeadOf){
        P.second->HeadOf.insert(this->VarToNode[N->Name]);
      }
    }
  }

  // Destructor
  ~DependencyGraphElementImpl(){
    for (pair<string, Node*> P : VarToNode){
      delete(P.second);
    }
  }

  // Printing
  string toString(){
    string GraphString = "Graph:\n";
    for (pair<string, Node*> P : VarToNode){
      GraphString.append("\t");
      GraphString.append(P.second->toString());
      GraphString.append("\n");
    }
    return GraphString;
  }

  // Removes variables from the graph
  void remove(const string Var){
    Node *Target = VarToNode[Var];
    set<Node*> TempHeadOf = Target->HeadOf;
    set<Node*> TempTailOf = Target->TailOf;
    for (Node *N : TempHeadOf){
      N->TailOf.erase(Target);
    }
    for (Node *N : TempTailOf){
      N->HeadOf.erase(Target);
    }
    delete(Target);
    VarToNode.erase(Var);
  }

  // Inserts variables in the graph with dependencies
  void insert(const string Var, const set<string> Rhs){
    // Check if dependent variables are defined already
    set<Node*> TempHeadOf;
    for (string Dep : Rhs){
      if (VarToNode.find(Dep) == VarToNode.end()){
        llvm::outs() << "ERROR: The variable " << Dep << " is undeclared"
          " in this graph when assigning to " << Var << "\n";
      } else {
       TempHeadOf.insert(VarToNode[Dep]);
      }
    }
    // Add to the set of nodes
    Node *VarNode = new Node();
    VarNode->Name = Var;
    VarNode->HeadOf = TempHeadOf;
    // Let dependents know they are dependents
    for (Node *N : TempHeadOf){
      N->TailOf.insert(VarNode);
    }
    VarToNode[Var] = VarNode;
  }

  // Finds reachable variables in the graph
  const set<string> reachable(const string Var){
    set<string> Visited;
    stack<Node*> Stack;
    Stack.push(VarToNode[Var]);

    while (!Stack.empty()){
      Node* CurrNode = Stack.top();
      Stack.pop();
      if (Visited.find(CurrNode->Name) == Visited.end()){
        Visited.insert(CurrNode->Name);
        for (Node *N : VarToNode[CurrNode->Name]->TailOf){
          Stack.push(N);
        }
      }
    }

    return Visited;
  }

  // Shorts the graph
  void shortGraph(const string U){
    for (Node *H : VarToNode[U]->TailOf){
      for (Node *T : VarToNode[U]->HeadOf){
        H->HeadOf.erase(VarToNode[U]);
        T->TailOf.erase(VarToNode[U]);
        H->HeadOf.insert(T);
        T->TailOf.insert(H);
      }
    }
  }

  // Ignores unwanted variables in the graph
  void ignore(const set<string> UnwantedVars){
    for (string U : UnwantedVars){
      shortGraph(U);
    }
  }

  // Figure out if the variable is present in the graph
  bool isPresent(const string Variable){
    return VarToNode.find(Variable) != VarToNode.end();
  }

  // Figure out if the variable is absent in the graph
  bool isAbsent(const string Variable){
    return !isPresent(Variable);
  }

  // Get variables in the graph
  void getVars(set<string> &Vars){
    for (pair<string, Node*> P : VarToNode){
      Vars.insert(P.first);
    }
  }

  // Merge edges of graph
  // (Maintain that Target's varables is subset of this))
  void mergeEdges(DependencyGraphElementImpl *Target){
    set<string> Vars;
    getVars(Vars);
    for (string Var : Vars){
      if (Target->isPresent(Var)){
        for (Node *Tail : Target->VarToNode[Var]->HeadOf){
          this->VarToNode[Var]->HeadOf.insert(Tail);
        }
        for (Node *Head : Target->VarToNode[Var]->TailOf){
          this->VarToNode[Var]->TailOf.insert(Head);
        }
      }
    }
  }

private:

  struct Node {
    string Name;
    set<Node*> TailOf;
    set<Node*> HeadOf;

    string toString(){
      string NodeString = "Node: ";
      NodeString.append("Name(");
      NodeString.append(Name);
      NodeString.append(") ");
      NodeString.append("TailOf(");
      for (Node *N : TailOf){
        NodeString.append(N->Name);
        NodeString.append(", ");
      }
      NodeString.append(")");
      NodeString.append(" ");
      NodeString.append("HeadOf(");
      for (Node *N : HeadOf){
        NodeString.append(N->Name);
        NodeString.append(", ");
      }
      NodeString.append(")");
      return NodeString;
    }

  };

  map<string, Node*> VarToNode;

};


class DependencyGraph::DependencyGraphImpl {

public:
  DependencyGraphImpl(){
    GraphStack.push(new DependencyGraphElementImpl());
  }
  ~DependencyGraphImpl(){
    while (!GraphStack.empty()){
      DependencyGraphElementImpl *N = GraphStack.top();
      delete(N);
      GraphStack.pop();
    }
  }

  // Convert to String
  string toString(){
    return GraphStack.top()->toString();
  }

  // Removes variables from the graph
  void remove(const string Var){
    GraphStack.top()->remove(Var);
  }

  // Inserts variables in the graph with dependencies
  void insert(const string Var, const set<string> Rhs){
    GraphStack.top()->insert(Var, Rhs);
  }

  // Finds reachable variables in the graph
  const set<string> reachable(const string Var){
    return GraphStack.top()->reachable(Var);
  }

  // Ignores unwanted variables in the graph
  void ignore(const set<string> UnwantedVars){
    GraphStack.top()->ignore(UnwantedVars);
  }

  // Figure out if the variable is present in the graph
  bool isPresent(const string Variable){
    return GraphStack.top()->isPresent(Variable);
  }

  // Figure out if the variable is absent in the graph
  bool isAbsent(const string Variable){
    return GraphStack.top()->isAbsent(Variable);
  }

  // Entering an if statement
  void entryScope(){};

  // Entering a branch
  void entryBranch(){
    DependencyGraphElementImpl Top = *GraphStack.top();
    GraphStack.push(new DependencyGraphElementImpl(Top));
  };

  // Exiting a branch (Maybe not needed)
  void exitBranch(){
    MergeStack.push(GraphStack.top());
    GraphStack.pop();
  };

  // Exiting an if statement
  void exitScope(){
    DependencyGraphElementImpl *Curr = GraphStack.top();
    while (!MergeStack.empty()){
      DependencyGraphElementImpl *Target = MergeStack.top();
      MergeStack.pop();

      set<string> Diff;
      diffGraphs(Curr, Target, Diff);

      // FIXME Ignore step might not be needed...
      Curr->ignore(Diff);

      // Merge edges
      Curr->mergeEdges(Target);

      // Invalidate 
      for (string D : Diff){
        set<string> Reach = Curr->reachable(D);
        for (string U : Reach){
          Curr->remove(U);
        }
      }

      delete(Target);
    }
  };


private:
  stack<DependencyGraphElementImpl*> GraphStack;
  stack<DependencyGraphElementImpl*> MergeStack;

  void diffGraphs(DependencyGraphElementImpl *E1, 
                         DependencyGraphElementImpl *E2,
                         set<string> &Diff){
    set<string> E1Vars;
    E1->getVars(E1Vars);
    for (string U : E1Vars){
      if (E2->isAbsent(U)){
        Diff.insert(U);
      }
    }
  }


};



DependencyGraph::DependencyGraph():
  Pimpl(new DependencyGraph::DependencyGraphImpl()){}
DependencyGraph::~DependencyGraph()
  {delete(DependencyGraph::Pimpl);}

// Convert to a string
string DependencyGraph::toString(){
  return Pimpl->toString();
}

// Enter scope
void DependencyGraph::entryScope(){ Pimpl->entryScope(); }

// Exit scope
void DependencyGraph::exitScope(){ Pimpl->exitScope(); }

// Exit branch
void DependencyGraph::entryBranch(){ Pimpl->entryBranch(); }

// Exit branch
void DependencyGraph::exitBranch(){ Pimpl->exitBranch(); }

// Removes variables from the graph
void DependencyGraph::remove(const string Var){ Pimpl->remove(Var); }

// Inserts variables in the graph with dependencies
void DependencyGraph::insert(const string Var, const set<string> Rhs){
  Pimpl->insert(Var, Rhs);
}

// Finds reachable variables in the graph
const set<string> DependencyGraph::reachable(const string Var){
  return Pimpl->reachable(Var);
}

// Ignores unwanted variables in the graph
void DependencyGraph::ignore(const set<string> UnwantedVars){
  Pimpl->ignore(UnwantedVars);
}

// Figure out if the variable is present in the graph
bool DependencyGraph::isPresent(const string Variable){
  return Pimpl->isPresent(Variable);
}

// Figure out if the variable is absent in the graph
bool DependencyGraph::isAbsent(const string Variable){
  return Pimpl->isAbsent(Variable);
}
