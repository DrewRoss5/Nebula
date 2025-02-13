#include <iostream>
#include <memory>
#include <stdio.h>

#include "../inc/values.hpp"
#include "../inc/nodes.hpp"

/*ValNode Functions*/
ValNode::ValNode(std::shared_ptr<Value>& val){
    this->val_ptr = val;
    this->val_type = val->get_type();
    this->node_type = Val_N;
}
void ValNode::assign(const Value& val){
    *this->val_ptr = val;
}


/* VarNode Functions */
VarNode::VarNode(const std::shared_ptr<Value>& val, bool initialize){
    this->val_ptr = val;
    this->val_type = val->get_type();
    this->initialized = initialize;
    this->node_type = NodeType::Var_N;
}
// returns the current value of the variable 
Value VarNode::eval(){
    if (!this->initialized)
        throw std::runtime_error("cannot evaluate an unitialized variable");
    auto tmp = *this->val_ptr;
    return *this->val_ptr;
}
// compares both the content and type of this variable to another
bool VarNode::operator==(VarNode& rhs){
    return this->eval() == rhs.eval();
}
// assigns a new value to the variable. This function assumes types have been checked by the caller
void VarNode::assign(const Value& new_val){
    if (!this->initialized)
        this->initialized = true;
    *this->val_ptr = new_val;
}

/* AsgnNode Functions */
AsgnNode::AsgnNode(ValNode* lhs, Node* rhs){
    this->rhs = rhs;
    this->lhs = lhs;
    this->node_type = NodeType::Asgn_N;
}
// assigns lhs to rhs and returns the new value of lhs. This throws an exception if rhs evaluates to a different type than rhs
Value AsgnNode::eval(){
    Value rhs_val = this->rhs->eval();
    if (rhs_val.get_type() != this->lhs->get_type())
        throw std::runtime_error("cannot assign a variable to a value of a different type");
    this->lhs->assign(rhs_val);
    return this->lhs->eval();
}

/* CompNode functions */
CompNode::CompNode(Node* lhs, Node* rhs, Operator op){
    this->lhs = lhs;
    this->rhs = rhs;
    this->op = op;
    this->node_type = NodeType::Comp_N;
}
Value CompNode::eval(){
    Value lhs_val = this->lhs->eval();
    Value rhs_val = this->rhs->eval();
    if (lhs_val.get_type() != rhs_val.get_type())
        throw std::runtime_error("cannot compare two values of differing types");
    bool result;
    switch (lhs_val.get_type()){
        case INT:
            result = this->compare(lhs_val.as<int>(), rhs_val.as<int>(), true);
            break;
        case FLOAT:
            result = this->compare(lhs_val.as<double>(), rhs_val.as<double>(), true);
            break;
        case CHAR:
            result = this->compare(lhs_val.as<char>(), rhs_val.as<char>(), false);
            break;
        case BOOL:
            result = this->compare(lhs_val.as<bool>(), rhs_val.as<bool>(), false);
            break;
    }
    return std::move(Value::create(ValueType::BOOL, result));
}

/* BoolLogicNode Functions*/
BoolLogicNode::BoolLogicNode(Node* lhs, Node* rhs, Operator op){
    this->lhs = lhs;
    this->rhs = rhs;
    this->op = op;
    this->node_type = NodeType::BoolLogic_N;
}
Value BoolLogicNode::eval(){
    Value lhs_val = lhs->eval();
    Value rhs_val = rhs->eval();
    if (lhs_val.get_type() != BOOL || rhs_val.get_type() != BOOL)
        throw std::runtime_error("invalid opperand types for logical operation");
    bool result;
    switch (this->op){
        case LogicOr:
            result = lhs_val.as<bool>() || rhs_val.as<bool>();
            break;
        case LogicAnd:
            result = lhs_val.as<bool>() && rhs_val.as<bool>();
            break;
    }
    return Value::create(BOOL, result);
}

/* ArithNode functions  */
ArithNode::ArithNode(Node* lhs, Node* rhs, Operator op){
    this->lhs = lhs;
    this->rhs = rhs;
    this->op = op;
    this->node_type = NodeType::Arith_N;
}
Value ArithNode::eval(){
    Value lhs_val = this->lhs->eval();
    Value rhs_val = this->rhs->eval();
    if (lhs_val.get_type() != rhs_val.get_type())
        throw std::runtime_error("cannot perform arithmetic on differing types");
    if (lhs_val.get_type() != INT || lhs_val.get_type() != INT)
        throw std::runtime_error("invalid operation for non-numeric types");
    switch (lhs_val.get_type()){
    case INT:
        return this->calculate(rhs_val.as<int>(), lhs_val.as<int>(), true);
    case FLOAT:
        return this->calculate(rhs_val.as<double>(), lhs_val.as<double>(), false);
    }
    return Value(NULL_TYPE);
}

/* PrintNode functions */
Value PrintNode::eval(){
    for (int i = this->args.size()-1; i >= 0; i--)
        std::cout << args[i]->eval();
    if (this->newline)
        std::cout << std::endl;
    else
        std::cout << std::flush;
    return Value(NULL_TYPE);
}

/* ParamNode Functions */
ParamNode::ParamNode(ParamType param_type, unsigned int init_val){
    this->param_type = param_type;
    this->node_type = Param_N;
    if (this->param_type == Index)
        this->index_no = init_val;
    else
        this->val_type = static_cast<ValueType>(init_val);
}
int ParamNode::get_index(){
    if (this->param_type != Index)
        throw std::runtime_error("Cannot get the index of a non-numeric parameter");
    return this->index_no;
}
ValueType ParamNode::get_val_type(){
    if (this->param_type != Type)
        throw std::runtime_error("Cannot get the type of a non-type parameter");
    return this->val_type;
}