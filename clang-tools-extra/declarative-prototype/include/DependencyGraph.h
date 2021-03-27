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

  // Removes variables from the graph
  void remove(const string Var);

  // Inserts variables in the graph with dependencies
  void insert(const string Var, const set<string> Rhs);

  // Finds reachable variables in the graph
  const set<string> reachable(const string Var);

  // Ignores unwanted variables in the graph
  void ignore(const set<string> UnwantedVars);

  // Figure out if the variable is present in the graph
  bool isPresent(const string Variable);

  // Figure out if the variable is absent in the graph
  bool isAbsent(const string Variable);

};


} // end namespace declarative

#endif
