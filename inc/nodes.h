#include <memory>

#include "../inc/values.hpp"

enum NodeType{
    Type,
    Sym,
    Literal, 
    Var,
    Comp,
    Defn,
    Asgn,
    BoolLogic,
    Arith,
};

enum Operator{
    LessThan,
    GreatherThan,
    Equal,
    NEqual,
    LogicOr,
    LogicAnd,
    ArithAdd,
    ArithSub,
    ArithMul,
    ArithDiv
};

// the base class that all nodes in the AST must derive from
class Node{
    public:
        virtual Value eval() = 0;      
    protected:
        NodeType type;
};

// this is the simplest type of node, it simply evaluates to a given value
class LiteralNode: public Node{
    public:
        LiteralNode(const Value& val);
        Value eval() override;
    private:
        Value val;
};

// this node only holds a user defined symbol
class SymNode: public Node{
    public:
         SymNode(const std::string& sym) {this->symbol = sym; this->type = Sym;}
        std::string get_sym() {return std::move(this->symbol);}
    private:
        std::string symbol;
};

// this node only holds the name of a type
class TypeNode: public Node{
    public: 
        TypeNode(ValueType type) {this->val_type = type; this->type = Type;};
        Value eval() {return std::move(Value(NULL_TYPE));};
        ValueType get_type() {return this->val_type;}
    private:
        ValueType val_type;
};

// this node represents a variable
class VarNode: public Node{
    public:
        VarNode(const std::shared_ptr<Value>& val);
        Value eval() override;
        ValueType get_type();
    private:
        std::shared_ptr<Value> val;
};

// this node represents a definition. It serves largely as a place holder as much of the assinmetn logic is performed by the interpreter itself
class DefnNode: public Node{
    public: 
        DefnNode(SymNode* sym_node, TypeNode* type_node);
        SymNode* get_sym() {return this->sym_node;}
        TypeNode* get_type() {return this->type_node;}
    private:
        SymNode* sym_node;
        TypeNode* type_node;
};

// this node assigns a variable to the result of the right child. The right child must evaluate to the same type as the variable. This node evaluates to the new value of the variable
class AsgnNode: public Node{
    public:
        AsgnNode(VarNode* rhs, Node* lhs);
        Value eval() override;
    private:
        VarNode* rhs;
        Node* lhs;
};

// this node will return a value that always evaluates to bool, it accepts any type, but certain opperations only apply to certain types
class CompNode: public Node{
    public:
        CompNode(const Node* lhs, const Node* rhs, Operator op);
        Value eval() override;
        Operator op;
    private:
        Node* lhs;
        Node* rhs;
};

// this node will perform a given logical operation on its children, and evaluates to the result of that operation. The children MUST evaluate to a boolean
class BoolLogicNode: public Node{
    public: 
        BoolLogicNode(const Node* lhs, const Node* rhs, Operator op);
        Value eval() override;
    private:
        Node* lhs;
        Node* rhs;
        Operator op;
};

// this node performans arithmetic on two numeric noes and evaluates to the result
class ArithNode: public Node{
    public:
        ArithNode(const Node* lhs, const Node* rhs, Operator op);
        Value eval() override; 
    private:
        Node* lhs;
        Node* rhs;
        Operator op;

};