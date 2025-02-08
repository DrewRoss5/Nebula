#ifndef PARSER_H
#define PARSER_H

#include <unordered_map>
#include <vector>
#include <stack>

#include "../inc/values.hpp"
#include "../inc/lexer.h"
#include "../inc/symtable.h"
#include "../inc/nodes.hpp"
#include "../inc/block.h"

class Parser{
    public:
        Parser() {this->curr_scope = &this->global_scope;}
        Parser(const std::vector<Token>& tokens);
        ~Parser();
        void parse(); 
        void reset(const std::vector<Token>& new_tokens);
        bool validate(std::string& error_msg);
        Node* next_expr();
    private:
        Node* pop_node();
        size_t stack_size();
        void push_node(Node* node);
        void push_block(BlockNode* block);
        void parse_expr();
        void parse_bin_expr(NodeType type, Operator op);
        void clear();
        size_t token_count;
        size_t curr_pos {0};
        int eval_count  {0}; // keeps track of the number of eval blocks currentlty open
        bool return_next {false};
        SymbolTable global_scope;
        SymbolTable* curr_scope;
        BlockNode* curr_block {nullptr};
        std::stack<SymbolTable*> scope_stack;
        std::deque<Node*> node_stack;
        std::stack<BlockNode*> block_stack;
        std::vector<Token> tokens;
        std::vector<Node*> statements;
        std::vector<SymbolTable*> scopes; // this is to store scopes that have been declared, but aren't on the stack
        std::vector<Node*> nodes; // see above, but for nodes
};      

#endif