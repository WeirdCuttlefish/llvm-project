#include <list>
#include <set>

#include "clang/Analysis/CallGraph.h"
#include "llvm/Support/CommandLine.h"

using namespace std;
using namespace clang;

class CallGraphUtil{

    public:

    explicit CallGraphUtil(ASTContext *Context) : m_ASTContext(Context){}

    list<CallGraphNode> generateCallList(){
        list<CallGraphNode> Nodes;
        set<CallGraphNode*> Visited;
        CallGraph CG;
        CG.addToCallGraph(m_ASTContext->getTranslationUnitDecl());
        PostTraverseCallGraph(CG.getRoot(), CG.getRoot(), Visited, &Nodes);
        return Nodes;
    }

    private:

    ASTContext *m_ASTContext = NULL;

    void PostTraverseCallGraph(CallGraphNode *root, CallGraphNode *origin, 
            set<CallGraphNode*> &Visited, list<CallGraphNode> *Nodes){

        if (root == NULL){
            return;
        }
        else if (root == origin){
            for (CallGraphNode *c : root->callees()){
                PostTraverseCallGraph(c, origin, Visited, Nodes);
            }
            Visited.insert(root);
        } else {
            if (Visited.find(root) != Visited.end()){
                return;
            }
            if (root->getDecl() == NULL){
                llvm::outs() << "ERROR: PostTraverseCallGraph found a nonDecl.";
                return;
            }
            for (CallGraphNode *c : root->callees()){
                PostTraverseCallGraph(c, origin, Visited, Nodes);
            }

            Nodes->push_back(*root);

            Visited.insert(root);
        }
    }
};