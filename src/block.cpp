#include <iostream>
#include <stdexcept>
#include <vector>

#include "../inc/nodes.hpp"
#include "../inc/block.h"

/* Base block methods */
BlockNode::BlockNode(SymbolTable* scope_ptr){
    this->scope = scope_ptr;
    this->type = Block_N;
    this->block_t = Base;
}
BlockNode::~BlockNode(){
    for (int i = 0; i < this->statements.size(); i++)
        delete this->statements[i];
    delete this->scope;
}
Value BlockNode::eval(){
    size_t statement_count = statements.size();
    for (int i = 0; i < statement_count; i++)
        this->eval_stack.push(std::move(statements[i]->eval()));
    return this->eval_stack.top();
}
void BlockNode::push_statement(Node* statement){
    this->statements.push_back(statement);
}
Node* BlockNode::pop_statement(){
    Node* retval = this->statements.back();
    this->statements.pop_back();
    return retval;
}

/* Eval block methods */
Value EvalBlockNode::eval(){
    return this->body->eval();
}

/* Conditional Block Methods */
CondBlockNode::CondBlockNode(SymbolTable* scope_ptr, Node* cond_ptr){
    this->scope = scope_ptr;
    this->condition = cond_ptr;
    this->type = Block_N;
    this->block_t = Conditional;
    this->eval_stack.push(Value(NULL_TYPE)); // this is so that the node can evaluate to something, even if the condition is false
}
CondBlockNode::~CondBlockNode(){
    delete this->else_body;
    BlockNode::~BlockNode();
}
/* 
    this sets the given block to the conditional block's else clause, all nodes to pushed to the conditional after this function is
    called will be pused to the else clause
*/
void CondBlockNode::set_else(BlockNode* else_body){
    if (this->else_body)
        throw std::runtime_error("cannot have multiple else clauses in one if body");
    this->else_body = else_body;
    this->eval_else = true;
}
void CondBlockNode::push_statement(Node* statement){
    if (this->eval_else)
        this->else_body->push_statement(statement);
    else
        this->statements.push_back(statement);
}
Node* CondBlockNode::pop_statement(){
    if (this->eval_else)
        return this->else_body->pop_statement();
    return BlockNode::pop_statement();   
}
size_t CondBlockNode::statement_count(){
    if (this->else_body)
        return this->else_body->statement_count();
    return this->statements.size();
}
Value CondBlockNode::eval(){
    Value cond_val = this->condition->eval();
    if (cond_val.get_type() != BOOL)
        throw std::runtime_error("invalid conditional");
    if (cond_val.as<bool>())
        return BlockNode::eval();
    if (this->else_body)
        return else_body->eval();
    return Value(this->eval_stack.top());
}

/* Loop Block Functions */
LoopBlockNode::LoopBlockNode(SymbolTable* scope_ptr, Node* cond_ptr){
    this->scope = scope_ptr;
    this->condition = cond_ptr;
    this->type = Block_N;
    this->block_t = Loop;
    this->eval_stack.push(Value(NULL_TYPE)); // this is so that the node can evaluate to something, even if the loop never runs
}
Value LoopBlockNode::eval(){
    Value cond_val = this->condition->eval();
    if (cond_val.get_type() != BOOL)
        throw std::runtime_error("invalid conditional");
    while (this->condition->eval().as<bool>()){
        this->eval_stack.push(BlockNode::eval());
        //this->scope->clear();
    }
    return Value(this->eval_stack.top()); 
}