#ifndef BLOCK_H
#define BLOCK_H

#include <stack>
#include <vector>

#include "../inc/nodes.hpp"
#include "../inc/symtable.h"

class BlockNode: public Node{
    public:
        BlockNode() {}
        BlockNode(SymbolTable* scope_ptr);
        Value eval() override;
        void push_statement(Node* statement);
    protected:
        std::stack<Value> eval_stack;
        std::vector<Node*> statements;
        SymbolTable* scope;
};

class CondBlock: public BlockNode{
    public:
        CondBlock(SymbolTable* scope_ptr, Node* cond_ptr);
        Value eval() override;
    private:
        Node* condition;
};

class LoopBlock: public BlockNode{
    public:
        LoopBlock(SymbolTable* scope_ptr, Node* cond_pt);
        Value eval() override;
    private:
        Node* condition;
};


#endif