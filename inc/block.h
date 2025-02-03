#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>
#include <stack>
#include <vector>

#include "../inc/nodes.hpp"
#include "../inc/symtable.h"

class BlockNode: public Node{
    public:
        BlockNode() {this->type = Block_N;}
        BlockNode(SymbolTable* scope_ptr);
        virtual Value eval() override;
        virtual void push_statement(Node* statement);
        void pop_statement();
    protected:
        std::stack<Value> eval_stack;
        std::vector<Node*> statements;
        SymbolTable* scope;
};

class EvalBlockNode: public BlockNode{
    public: 
        EvalBlockNode() {this->type = Block_N;}
        Value eval() override;
        void set_body(Node* body) {this->body = body;}
        void push_statement(Node* statement) override {this->body = statement;};
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