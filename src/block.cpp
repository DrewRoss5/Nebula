#include <stdexcept>
#include <vector>

#include "../inc/nodes.hpp"
#include "../inc/block.h"

/* Base block methods */
BlockNode::BlockNode(SymbolTable* scope_ptr){
    this->scope = scope_ptr;
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

/* Conditional Block Methods */
CondBlock::CondBlock(SymbolTable* scope_ptr, Node* cond_ptr){
    this->scope = scope_ptr;
    this->condition = cond_ptr;
}
Value CondBlock::eval(){
    Value cond_val = this->condition->eval();
    if (cond_val.get_type() != BOOL)
        throw std::runtime_error("invalid conditional");
    if (cond_val.as<bool>())
        return BlockNode::eval();
    return Value(NULL_TYPE);
}

/* Loop Block Functions */
LoopBlock::LoopBlock(SymbolTable* scope_ptr, Node* cond_ptr){
    this->scope = scope_ptr;
    this->condition = cond_ptr;
}
Value LoopBlock::eval(){
    Value cond_val = this->condition->eval();
    if (cond_val.get_type() != BOOL)
        throw std::runtime_error("invalid conditional");
    while (cond_val.as<bool>())
        this->eval_stack.push(std::move(BlockNode::eval()));
    return Value(NULL_TYPE); 
}
