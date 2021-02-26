#include "clang/ASTMatchers/ASTMatchers.h"

#include <string>

using namespace std;

using namespace clang;
using namespace ast_matchers;

// Matchers
namespace CustomMatchers{

namespace FunctionMatchers{
DeclarationMatcher getPointerTakesCopy(string funName){
    return functionDecl(hasName(funName), forEachDescendant(decl(
        varDecl(
            hasType(isAnyPointer()), 
            hasInitializer(implicitCastExpr(hasSourceExpression(declRefExpr())))
        ).bind("decl1")
    )));
}

DeclarationMatcher getPointerTakesAddress(string funName){
    return functionDecl(hasName(funName), forEachDescendant(decl(
        varDecl(
            hasType(isAnyPointer()), 
            hasInitializer(unaryOperator(hasOperatorName("&")))
        ).bind("decl2")
    )));
}

DeclarationMatcher getPointerTakesDereference(string funName){
    return functionDecl(hasName(funName), forEachDescendant(decl(
        varDecl(
            hasType(isAnyPointer()), 
            hasInitializer(implicitCastExpr(hasSourceExpression(unaryOperator(hasOperatorName("*")))))
        ).bind("decl3")
    )));
}

DeclarationMatcher getDereferencedPointerTakesCopy(string funName){
    return functionDecl(hasName(funName), forEachDescendant(
        binaryOperator(
            isAssignmentOperator(),
            hasLHS((unaryOperator(hasOperatorName("*")))), 
            hasRHS(implicitCastExpr(hasSourceExpression(declRefExpr())))
        ).bind("decl4")
    ));
}

DeclarationMatcher getDereferencedPointerTakesAddress(string funName){
    return functionDecl(hasName(funName), forEachDescendant(
        binaryOperator(
            isAssignmentOperator(),
            hasLHS((unaryOperator(hasOperatorName("*")))), 
            hasRHS(unaryOperator(hasOperatorName("&")))
        ).bind("decl5")
    ));
}

DeclarationMatcher getDereferencedPointerTakesDereference(string funName){
    return functionDecl(hasName(funName), forEachDescendant(
        binaryOperator(
            isAssignmentOperator(),
            hasLHS((unaryOperator(hasOperatorName("*")))), 
            hasRHS(implicitCastExpr(hasSourceExpression(unaryOperator(hasOperatorName("*")))))
        ).bind("decl6")
    ));
}

}

namespace GlobalMatchers{

DeclarationMatcher getPointerTakesCopy(){
    return decl(forEachDescendant(decl(
        varDecl(
            hasGlobalStorage(),
            hasType(isAnyPointer()), 
            hasInitializer(implicitCastExpr(hasSourceExpression(declRefExpr())))
        ).bind("decl1")
    )));
}

DeclarationMatcher getPointerTakesAddress(){
    return decl(forEachDescendant(decl(
        varDecl(
            hasGlobalStorage(),
            hasType(isAnyPointer()), 
            hasInitializer(unaryOperator(hasOperatorName("&")))
        ).bind("decl2")
    )));
}

DeclarationMatcher getPointerTakesDereference(){
    return decl(forEachDescendant(decl(
        varDecl(
            hasGlobalStorage(),
            hasType(isAnyPointer()), 
            hasInitializer(implicitCastExpr(hasSourceExpression(unaryOperator(hasOperatorName("*")))))
        ).bind("decl3")
    )));
}

DeclarationMatcher getDereferencedPointerTakesCopy(){
    return decl(forEachDescendant(
        binaryOperator(
            isAssignmentOperator(),
            hasLHS(
                unaryOperator(
                    hasOperatorName("*"),
                    hasUnaryOperand(
                        implicitCastExpr(hasSourceExpression(declRefExpr(to(varDecl(hasGlobalStorage())))))
                    )
                )
            ),
            hasRHS(implicitCastExpr(hasSourceExpression(declRefExpr())))
        ).bind("decl4")
    ));
}

DeclarationMatcher getDereferencedPointerTakesAddress(){
    return decl(forEachDescendant(
        binaryOperator(
            isAssignmentOperator(),
            hasLHS(
                unaryOperator(
                    hasOperatorName("*"),
                    hasUnaryOperand(
                        implicitCastExpr(hasSourceExpression(declRefExpr(to(varDecl(hasGlobalStorage())))))
                    )
                )
            ), 
            hasRHS(unaryOperator(hasOperatorName("&")))
        ).bind("decl5")
    ));
}

DeclarationMatcher getDereferencedPointerTakesDereference(){
    return functionDecl(forEachDescendant(
        binaryOperator(
            isAssignmentOperator(),
            hasLHS(
                unaryOperator(
                    hasOperatorName("*"),
                    hasUnaryOperand(
                        implicitCastExpr(hasSourceExpression(declRefExpr(to(varDecl(hasGlobalStorage())))))
                    )
                )
            ), 
            hasRHS(implicitCastExpr(hasSourceExpression(unaryOperator(hasOperatorName("*")))))
        ).bind("decl6")
    ));
}

}

}