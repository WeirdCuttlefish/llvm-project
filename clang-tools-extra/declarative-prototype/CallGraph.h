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


// Extra Code 

/*
        map<string, Validity> GlobalAlpha = GlobalVisitor.getAlpha();

        // If the function is main assume the global variables to be valid
        string FunctionName = dyn_cast<FunctionDecl>(root->getDecl())->getNameAsString();
        if (FunctionName == "main"){
            for (std::pair<string,Validity> p : GlobalAlpha){
            GlobalAlpha[p.first] = Valid;
            }
        }

        DeclarativeCheckingFunctionVisitor Visitor(
            &Context,
            GlobalAlpha,
            GlobalVisitor.getBeta(),
            GlobalVisitor.getGamma(),
            &FunctionChanges
        );

        Visitor.TraverseDecl(root->getDecl());

        // APA
        map<string, Node*> Graph;
        APAWorker Worker(&Graph);
        MatchFinder Finder;
        APAGraph APA;
        Finder.addMatcher(APA.getPointerAndAddress(), &Worker);
        Finder.match(*(root->getDecl()), Context);

        for (auto p : Graph){
            llvm::outs() << p.first;
        }

        FunctionChanges.insert(std::pair<string, Function>(FunctionName, Visitor.getFunctionDetails()));

        Visited.insert(FunctionName);
        }
    }
*/