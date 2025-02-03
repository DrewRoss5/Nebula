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
        void pop_statement();
    protected:
        std::stack<Value> eval_stack;
        std::vector<Node*> statements;
        SymbolTable* scope;
};

class EvalBlockNode: public BlockNode{
    public: 
        Value eval() override {return this->body->eval();};
        void set_body(Node* body) {this->body = body;}
    private:
        Node* body;
};

class CondBlockNode: public BlockNode{
    public:
        CondBlockNode(SymbolTable* scope_ptr, Node* cond_ptr);
        Value eval() override;
    private:
        Node* condition;
};

class LoopBlockNode: public BlockNode{
    public:
        LoopBlockNode(SymbolTable* scope_ptr, Node* cond_ptr);
        Value eval() override;
    private:
        Node* condition;
};


#endif