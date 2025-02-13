#ifndef NODES_H
#define NODES_H

#include <stdexcept>
#include <memory>

#include "../inc/values.hpp"

enum NodeType{
    Type_N,
    Sym_N,
    Literal_N, 
    Val_N,
    Var_N,
    Comp_N,
    Defn_N,
    Asgn_N,
    BoolLogic_N,
    Arith_N,
    Block_N,
    Print_N,
    Param_N
};

enum ParamType{
    Index,
    Type
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
    ArithDiv,
    ArithMod,
    ArithPow,
    Assignment
};

// the base class that all nodes in the AST must derive from
class Node{
    public:
        virtual Value eval() = 0;      
        NodeType get_node_type() {return this->node_type;}
    protected:
        NodeType node_type;
};

// this is the simplest type of node, it simply evaluates to a given value
class LiteralNode: public Node{
    public:
        LiteralNode(const Value& val) {this->value = val; this->node_type = Literal_N;};
        Value eval() override {return this->value;};
    private:
        Value value;
};

// this node only holds a user defined symbol
class SymNode: public Node{
    public:
        SymNode(const std::string& sym) {this->symbol = sym; this->node_type = Sym_N;}
        Value eval() override {return std::move(Value(NULL_TYPE));}
        std::string get_sym() {return std::move(this->symbol);}
    private:
        std::string symbol;
};

// this node only holds the name of a type
class TypeNode: public Node{
    public: 
        TypeNode(ValueType type) {this->val_type = type; this->node_type = Type_N;};
        Value eval() override {return std::move(Value(NULL_TYPE));};
        ValueType get_type() {return this->val_type;}
    private:
        ValueType val_type;
};

// this is an abstract class for nodes that hold a value and can have their values updated
class ValNode: public Node{
    public:
        ValNode() {}
        ValNode(std::shared_ptr<Value>& val_ptr);
        virtual void assign(const Value& new_val);
        virtual ValueType get_type() {return val_ptr->get_type();}
    protected:
        std::shared_ptr<Value> val_ptr;
        
        ValueType val_type;
};

// this node represents a variable
class VarNode: public ValNode{
    public:
        VarNode(ValueType val_type) {this->val_type = val_type; this->initialized = false; this->node_type = Var_N;}
        VarNode(const std::shared_ptr<Value>& val, bool initialized);
        Value eval() override; 
        bool is_initialized() {return this->initialized;}
        bool operator==(VarNode& rhs);
        void assign(const Value& new_val) override;
        void set_ptr(const std::shared_ptr<Value>& val) {this->val_ptr = val;}
    private:
        bool initialized;
        
};

// this node assigns a variable to the result of the right child. The right child must evaluate to the same type as the variable. This node evaluates to the new value of the variable
class AsgnNode: public Node{
    public:
        AsgnNode(ValNode* lhs, Node* rhs);
        Value eval() override;
    private:
        ValNode* lhs;
        Node* rhs;
};

// this node will return a value that always evaluates to bool, it accepts any type, but certain opperations only apply to certain types
class CompNode: public Node{
    public:
        CompNode(Node* lhs, Node* rhs, Operator op);
        Value eval() override;
        Operator op;
        template <typename T>
        bool compare(T lhs_val, T rhs_val, bool is_numeric);
    private:
        Node* lhs;
        Node* rhs;
};
template <typename T> 
bool CompNode::compare(T lhs_val, T rhs_val, bool is_numeric){
    switch (this->op){
        case GreatherThan:
            if (!is_numeric)
                throw std::runtime_error("cannot use the '>' operator on non-numeric values");
            return lhs_val > rhs_val;
        case LessThan:
            if (!is_numeric) 
                throw std::runtime_error("cannot use the '>' operator on non-numeric values");
            return lhs_val < rhs_val;
            break;
        case Equal:
            return lhs_val == rhs_val;
            break;
        case NEqual:
            return lhs_val != rhs_val;
            break;
    }
    return false;
}

// this node will perform a given logical operation on its children, and evaluates to the result of that operation. The children MUST evaluate to a boolean
class BoolLogicNode: public Node{
    public: 
        BoolLogicNode(Node* lhs, Node* rhs, Operator op);
        Value eval() override;
    private:
        Node* lhs;
        Node* rhs;
        Operator op;
};

// this node performans arithmetic on two numeric noes and evaluates to the result
class ArithNode: public Node{
    public:
        ArithNode(Node* lhs, Node* rhs, Operator op);
        Value eval() override; 
        template <typename T>
        Value calculate(T lhs_val, T rhs_val, bool return_int);
    private:
        Node* lhs;
        Node* rhs;
        Operator op;

};

template <typename T>
Value ArithNode::calculate(T lhs_val, T rhs_val, bool return_int){
    double ret_val;
    switch (op){
        case ArithAdd:
            ret_val = lhs_val + rhs_val;
            break;
        case ArithSub:
            ret_val = lhs_val - rhs_val;
            break;
        case ArithMul:
            ret_val = lhs_val * rhs_val;
            break;
        case ArithDiv:
            ret_val = lhs_val / rhs_val;
            break;
        case ArithMod:
            return_int = true;
            ret_val = static_cast<int>(lhs_val) % static_cast<int>(rhs_val);
            break;
        case ArithPow:
            ret_val = 1;
            for (int i = 0; i < lhs_val; i++)
                ret_val *= rhs_val;
            break;
    
    }
    if (return_int)
        return Value::create(INT, static_cast<int>(ret_val));
    else    
        return Value::create(FLOAT, ret_val);
}
// this node represents a print statement
class PrintNode: public Node{
    public:
        PrintNode(bool newline) {this->node_type = Print_N; this->newline = newline;}
        Value eval() override;
        void push_arg(Node* arg) {this->args.push_back(arg);};
    private:
        std::vector<Node*> args;
        bool newline;
};

// this node represents a parameter, be it an index or a type
class ParamNode: public Node{
    public:
        ParamNode(ParamType param_type, unsigned int init_val);
        Value eval() override {return std::move(Value(NULL_TYPE));}
        int get_index();
        ValueType get_val_type();
    private:
        int index_no;
        ValueType val_type;
        ParamType param_type;
};

#endif