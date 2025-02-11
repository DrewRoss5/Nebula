#ifndef BLOCK_H
#define BLOCK_H

#include <stack>
#include <vector>

#include "../inc/nodes.hpp"
#include "../inc/symtable.h"

enum BlockType{
    Base,
    Eval,
    Conditional,
    Loop
};

class BlockNode: public Node{
    public:
        BlockNode() {this->node_type = Block_N; this->block_t = Base;}
        BlockNode(SymbolTable* scope_ptr);
        ~BlockNode();
        BlockType block_type() {return this->block_t;}
        SymbolTable* get_scope() {return this->scope;}
        virtual Node* pop_statement();
        virtual size_t statement_count() {return this->statements.size();}
        virtual Value eval() override;
        virtual void push_statement(Node* statement);
    protected:
        std::stack<Value> eval_stack;
        std::vector<Node*> statements;
        SymbolTable* scope;
        BlockType block_t;
};

class EvalBlockNode: public BlockNode{
    public: 
        EvalBlockNode() {this->node_type = Block_N; this->block_t = Eval;}
        Value eval() override;
        void set_body(Node* body) {this->body = body;}
        void push_statement(Node* statement) override {this->body = statement;};
    private:
        Node* body;
};

class CondBlockNode: public BlockNode{
    public:
        CondBlockNode(SymbolTable* scope_ptr, Node* cond_ptr);
        ~CondBlockNode();
        Value eval() override;
        void set_else(BlockNode* else_body);
        Node* pop_statement() override;
        void push_statement(Node* statement) override;
        size_t statement_count() override;
    private:
        bool eval_else {false};
        Node* condition;
        BlockNode* else_body {nullptr};
};

class LoopBlockNode: public BlockNode{
    public:
        LoopBlockNode(SymbolTable* scope_ptr, Node* cond_ptr);
        Value eval() override;
    private:
        Node* condition;
};


#endif