#pragma once

#include<set>
#include<map>
#include<string>

using namespace std;
using namespace clang;
using namespace ast_matchers;

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