#include<set>
#include<string>
#include<iostream>

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
                std::cout << "The variable " << m_name << "is not a pointer.";
            } else {
                std::cout << "The variable " << m_name << "is a pointer.";
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

class AndersenUtils{
    public:

        // These methods are inspired by http://pages.cs.wisc.edu/~fischer/cs701.f08/lectures/Lecture26.4up.pdf
        static void pointerTakesAddress(Node &lhsVar, Node &rhsVar){
            lhsVar.getPointsTo()->insert(&rhsVar);
        }

        static void pointerTakesCopy(Node &lhsVar, Node &rhsVar){
            for (auto n : *(rhsVar.getPointsTo())){
                lhsVar.getPointsTo()->insert(n);
            }
        }

        static void pointerTakesDereference(Node &lhsVar, Node &rhsVar){
            for (auto n1 : *(rhsVar.getPointsTo())){
                for (auto n2 : *(n1->getPointsTo())){
                    lhsVar.getPointsTo()->insert(n2);
                }
            }
        }

        static void dereferencedPointerTakesAddress(Node &lhsVar, Node &rhsVar){
            for (auto n : *(lhsVar.getPointsTo())){
                n->getPointsTo()->insert(&rhsVar);
            }
        }

        static void dereferencedPointerTakesCopy(Node &lhsVar, Node &rhsVar){
            for (auto n1: *(lhsVar.getPointsTo())){
                for (auto n2: *(rhsVar.getPointsTo())){
                    n1->getPointsTo()->insert(n2);
                }
            }
        }

        static void dereferencedPointerTakesDereference(Node &lhsVar, Node &rhsVar){
            for (auto n1: *(lhsVar.getPointsTo())){
                for (auto n2: *(rhsVar.getPointsTo())){
                    for (auto n3: *(n2->getPointsTo())){
                        n1->getPointsTo()->insert(n3);
                    }
                }
            }
        }

};