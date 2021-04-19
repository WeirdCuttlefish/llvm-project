//==--- DependencyGraph.h - Dependency graph of variables -----------------===//
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_DECLARATIVE_PROTOTYPE_DEPENDENCYGRAPH_H
#define CLANG_DECLARATIVE_PROTOTYPE_DEPENDENCYGRAPH_H

#include <set>
#include <string>

using namespace std;

namespace declarative {

// Declarative graph that keeps track of variables. Maintins the
// invariant that if a variable is in the graph, then it is valid.
class DependencyGraph {

public:

  // Constructor and destructors
  DependencyGraph();
  ~DependencyGraph();

  // Printing
  string toString();

  // Entering an if statement
  void entryScope();

  // Entering a branch
  void entryBranch();

  // Exiting a branch
  void exitBranch();

  // Exiting an if statement
  void exitScope();

  // Removes variables from the graph
  // FIXME: Hack is to keep the reason with the string.
  void remove(const string Var, const string Reason);

  // Inserts variables in the graph with dependencies
  void insert(const string Var, const set<string> &Rhs);

  // Inserts edge in the graph with dependencies
  void insertEdge(const string Var, const string U);

  // Finds reachable variables in the graph
  void reachable(const string Var, set<string> &Visited);

  // Ignores unwanted variables in the graph
  void ignore(const set<string> &UnwantedVars);

  // Figure out if the variable is present in the graph
  bool isPresent(const string Variable);

  // Figure out if the variable is absent in the graph
  bool isAbsent(const string Variable);

  // Get state of variable at vardecl
  string getState(const string Variable);

  // Set state of variable at vardecl
  void setState(const string Variable);

  // Get reason of removal for variable
  string getRemovalReason(const string Var);

private:

  class DependencyGraphImpl;
  DependencyGraphImpl* Pimpl;

};


} // end namespace declarative

#endif
