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
      VarToNode[P.first] = new Node();
      VarToNode[P.first]->Name = P.first;
      VarToNode[P.first]->State = P.second->State;
    }
    for (pair<string, Node*> P : GraphIn.VarToNode){
      for (Node* N : P.second->TailOf){
        VarToNode[P.first]->TailOf.insert(VarToNode[N->Name]);
      }
      for (Node* N : P.second->HeadOf){
        VarToNode[P.first]->HeadOf.insert(VarToNode[N->Name]);
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
  void remove(const string Var, const string Reason, map<string, string> &InvalidationMap){
    InvalidationMap[Var] = Var + " was deleted due to change in " + Reason + ".\n" +
      "The dependency graph at the time of declaration was:\n" + VarToNode[Var]->State + "\n";
    deleteNode(Var);
  }

  // Inserts variables in the graph with dependencies
  void insert(const string Var, const set<string> &Rhs){
    addNode(Var);
    for (string S : Rhs){
      addEdge(S, Var);
    }
  }

  // Inserts edge in the graph with dependencies
  void insertEdge(const string Var, const string U){
    addEdge(Var, U);
  }

  // Finds reachable variables in the graph
  void reachable(const string Var, set<string> &Visited){
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
  }

  // Shorts the graph
  void shortGraph(const string U){
    for (Node *H : VarToNode[U]->TailOf){
      for (Node *T : VarToNode[U]->HeadOf){
        addEdge(T->Name, H->Name);
      }
    }
    deleteNode(U);
  }

  // Ignores unwanted variables in the graph
  void ignore(const set<string> &UnwantedVars){
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
    set<pair<string, string>> Edges;
    Target->getEdges(Edges);
    for (pair<string, string> Edge : Edges){
      addEdge(Edge.first, Edge.second);
    }
  }

  // Get State of var
  string getState(const string Var){
    if (isPresent(Var))
      return VarToNode[Var]->State;
    return "NOT A NODE";
  }

  // Set State of var
  void setState(const string Var){
    if (isPresent(Var)){
      VarToNode[Var]->State = toString();
    }
  }

private:

  struct Node {

    string Name;
    string State = "";
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

  void addNode(string S){
    VarToNode[S] = new Node();
    VarToNode[S]->Name = S;
  }

  void addEdge(string S1, string S2){
    if (VarToNode.find(S1) == VarToNode.end() ||
        VarToNode.find(S2) == VarToNode.end())
      return;
    VarToNode[S1]->TailOf.insert(VarToNode[S2]);
    VarToNode[S2]->HeadOf.insert(VarToNode[S1]);
  }

  void deleteNode(string S){
    if (VarToNode.find(S) == VarToNode.end())
      return;
    set<pair<string,string>> Edges;
    getEdges(S, Edges);
    for (pair<string, string> E : Edges){
      deleteEdge(E.first, E.second);
    }
    delete(VarToNode[S]);
    VarToNode.erase(S);
  }

  void deleteEdge(string S1, string S2){
    if (VarToNode.find(S1) == VarToNode.end() ||
        VarToNode.find(S2) == VarToNode.end())
      return;
    if (VarToNode[S1]->TailOf.find(VarToNode[S2]) == VarToNode[S1]->TailOf.end() ||
        VarToNode[S2]->HeadOf.find(VarToNode[S1]) == VarToNode[S2]->HeadOf.end())
      return;
    VarToNode[S1]->TailOf.erase(VarToNode[S2]);
    VarToNode[S2]->HeadOf.erase(VarToNode[S1]);
  }

  void getEdges(set<pair<string,string>> &S){
    for (pair<string, Node*> P : VarToNode){
      for (Node *N : P.second->TailOf){
        S.insert(pair<string, string>(P.second->Name, N->Name));
      }
    }
  }

  void getEdges(string U, set<pair<string,string>> &S){
    if (VarToNode.find(U) == VarToNode.end())
      return;
    for (Node *N : VarToNode[U]->TailOf){
      S.insert(pair<string,string>(U, N->Name));
    }
    for (Node *N : VarToNode[U]->HeadOf){
      S.insert(pair<string,string>(N->Name, U));
    }
  }

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
  // FIXME Hack: Second argument tells reason
  void remove(const string Var, const string Reason){
    GraphStack.top()->remove(Var, Reason, InvalidationMap);
  }

  // Inserts variables in the graph with dependencies
  void insert(const string Var, const set<string> &Rhs){
    GraphStack.top()->insert(Var, Rhs);
  }

  // Inserts edge in the graph with dependencies
  void insertEdge(const string Var, const string U){
    GraphStack.top()->insertEdge(Var, U);
  }

  // Finds reachable variables in the graph
  void reachable(const string Var, set<string> &Visited){
    GraphStack.top()->reachable(Var, Visited);
  }

  // Ignores unwanted variables in the graph
  void ignore(const set<string> &UnwantedVars){
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
    DependencyGraphElementImpl *DGE = new DependencyGraphElementImpl(*GraphStack.top());
    GraphStack.push(DGE);
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

      // Get rid of variables in target but not in Curr
      set<string> Diff;
      diffGraphs(Target, Curr, Diff);
      Target->ignore(Diff);

      // Merge edges has precondition that target <= curr
      Curr->mergeEdges(Target);

      // Invalidate 
      set<string> Diff2;
      diffGraphs(Curr, Target, Diff2);
      for (string D : Diff2){
        if (Curr->isPresent(D)){
          set<string> Reach;
          Curr->reachable(D, Reach);
          for (string U : Reach){
            Curr->remove(U, D, InvalidationMap);
          }
        }
      }

      delete(Target);
    }
  };

  // Get reason of removal of function
  string getRemovalReason(const string Var){
    if (InvalidationMap.find(Var) != InvalidationMap.end()){
      return InvalidationMap[Var];
    }
    return "";
  };

  // Get State of var
  string getState(const string Var){
    return GraphStack.top()->getState(Var);
  }

  // Set state of var
  void setState(const string Var){
    GraphStack.top()->setState(Var);
  }

  // Entering a lambda statement
  void entryScopeNonDestructive(){
    DependencyGraphElementImpl *DGE = new DependencyGraphElementImpl(*GraphStack.top());
    GraphStack.push(DGE);
  };

  // Leaving a lambda statement
  void exitScopeNonDestructive(){
    GraphStack.pop();
  };


private:
  stack<DependencyGraphElementImpl*> GraphStack;
  stack<DependencyGraphElementImpl*> MergeStack;
  map<string, string> InvalidationMap;

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

// Enter scope for Lambda
void DependencyGraph::entryScopeNonDestructive(){ Pimpl->entryScopeNonDestructive(); }

// Exit scope for Lambda
void DependencyGraph::exitScopeNonDestructive(){ Pimpl->exitScopeNonDestructive(); }

// Removes variables from the graph
void DependencyGraph::remove(const string Var, const string Reason){ 
  Pimpl->remove(Var, Reason); 
}

// Gets the reason for removal of variable
string DependencyGraph::getRemovalReason(const string Var){ 
  return Pimpl->getRemovalReason(Var); 
}

// Gets the reason for removal of variable
string DependencyGraph::getState(const string Var){ 
  return Pimpl->getState(Var); 
}

// Inserts variables in the graph with dependencies
void DependencyGraph::insert(const string Var, const set<string> &Rhs){
  Pimpl->insert(Var, Rhs);
}

// Inserts edge in the graph with dependencies
void DependencyGraph::insertEdge(const string Var, const string U){
  Pimpl->insertEdge(Var, U);
}

// Finds reachable variables in the graph
void DependencyGraph::reachable(const string Var, set<string> &Visited){
  Pimpl->reachable(Var, Visited);
}

// Ignores unwanted variables in the graph
void DependencyGraph::ignore(const set<string> &UnwantedVars){
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

// set state
void DependencyGraph::setState(const string Variable){
  return Pimpl->setState(Variable);
}
