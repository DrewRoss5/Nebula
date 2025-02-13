#include <iostream>
#include <stdexcept>
#include <stack>
#include <vector>
#include <unordered_map>

#include "../inc/values.hpp"
#include "../inc/lexer.h"
#include "../inc/symtable.h"
#include "../inc/nodes.hpp"
#include "../inc/block.h"
#include "../inc/parser.h"

std::unordered_map<TokenType, Operator> OPERATOR_MAP{
        {TokenType::And, Operator::LogicAnd},
        {TokenType::Or, Operator::LogicOr},
        {TokenType::Eq, Operator::Equal},
        {TokenType::Neq, Operator::NEqual},
        {TokenType::Greater, Operator::GreatherThan},
        {TokenType::Less, Operator::LessThan},
        {TokenType::Add, Operator::ArithAdd},
        {TokenType::Sub, Operator::ArithSub},
        {TokenType::Mul, Operator::ArithMul},
        {TokenType::Div, Operator::ArithDiv},
        {TokenType::Mod, Operator::ArithMod},
        {TokenType::Pow, Operator::ArithPow},
        {TokenType::Asgn, Operator::Assignment}
    };

std::unordered_map<TokenType, ValueType> TYPE_MAP{
    {TypeInt, ValueType::INT},
    {TypeFloat, ValueType::FLOAT},
    {TypeBool, ValueType::BOOL},
    {TypeChar, ValueType::CHAR},
};

std::unordered_map<std::string, ValueType> TYPE_STR_MAP{
    {"int", ValueType::INT},
    {"float", ValueType::FLOAT},
    {"bool", ValueType::BOOL},
    {"char", ValueType::CHAR},
};

Parser::Parser(const std::vector<Token>& tokens){
    this->tokens = tokens;
    this->token_count = tokens.size();
    this->curr_scope = &this->global_scope;
}
Parser::~Parser(){
    this->clear();
}

// returns the number of nodes in the current block, or the number of nodes in the global scope if not in a block
size_t Parser::stack_size(){
    if (this->curr_block)
        return this->curr_block->statement_count();
    return this->node_stack.size();
}

// clears all internal member variables of the parser and performs the appropriate cleanup
void Parser::clear(){
    while (!this->block_stack.empty()){
        this->nodes.push_back(this->block_stack.top());
        this->block_stack.pop();
    }
    while (!this->node_stack.empty()){
        Node* tmp = this->pop_node();
        if (tmp->get_node_type() != Block_N)
            this->nodes.push_back(tmp);
    }
    // this map keeps track of all pointers that have been deleted. A map is used for O(1) lookup times
    std::unordered_map<Node*, bool> freed_ptrs;
    for (int i = 0; i < this->nodes.size(); i++){
        if (freed_ptrs.count(this->nodes[i]) == 0){
            delete this->nodes[i];
            freed_ptrs[this->nodes[i]] = true;
        }
        this->nodes[i] = nullptr;
    }
}

bool Parser::validate(std::string& err_msg ){
    if (this->block_stack.size()){
        err_msg = "syntax error: expected \"end\"";
        return false;
    }
    return true;
}

void Parser::reset(const std::vector<Token>& new_tokens){
    this->clear();
    this->tokens = new_tokens;
    this->token_count = new_tokens.size();
    this->curr_pos = 0;
}

// appends a new node to the node stack, and if we're currently in a block, to the current block
void Parser::push_node(Node* node){
    if (this->curr_block)
        this->curr_block->push_statement(node);
    else
        this->node_stack.push_back(node);
}
// pops a node from the stack, if we're currently in a block, pops a node form the current block
Node* Parser::pop_node(){
    Node* ret_val;
    if (this->curr_block)
        ret_val = this->curr_block->pop_statement();
    else {
        ret_val = this->node_stack.back();
        this->node_stack.pop_back();
    }
    // store this node to be deleted later
    this->nodes.push_back(ret_val);
    return ret_val;
}

/* 
    returns the next expression to be evaluated from the node stack, this should only be called after the 
    parse method, because after the tokens are parsed, each node represents the AST of an individual statement
*/
Node* Parser::next_expr(){
    if (!this->node_stack.empty()){
        Node* expr = this->node_stack.front();
        this->node_stack.pop_front();
        this->nodes.push_back(expr);
        return expr;
    }
    return nullptr;
}

// parses all tokens into statements
void Parser::parse(){
    while (this->curr_pos < this->token_count)
        parse_expr();
}

// parses tokens until a complete statement is formed
void Parser::parse_expr(){
    while (this->curr_pos < this->token_count){
        Token curr_token = this->tokens[this->curr_pos];
        int int_lit, init_count;
        double float_lit;
        char char_lit;
        bool bool_lit;
        Node* condition, *new_node, *rhs, *lhs, *to_copy;
        TypeNode* var_type;
        SymNode* var_name;
        BlockNode* new_block;
        CondBlockNode* conditional;
        EvalBlockNode* eval_block;
        VarNode* var_node;
        SymbolTable* sym_table;
        PrintNode* print_node;
        TokenType op;
        std::string sym;
        Token interior;
        switch (curr_token.type){
            // data types
            case TypeInt:
            case TypeFloat:
            case TypeChar:
            case TypeBool:
                this->push_node(new TypeNode(TYPE_MAP[curr_token.type]));
                this->curr_pos++;
                continue;
            // literals
            case IntLiteral:
                int_lit = std::stoi(curr_token.txt);
                this->push_node(new LiteralNode(Value::create(INT, int_lit)));
                this->curr_pos++;
                if (this->return_next){
                    this->return_next = false;
                    return;
                }
                break;
            case FloatLiteral:
                float_lit = std::stod(curr_token.txt);
                this->push_node(new LiteralNode(Value::create(FLOAT, float_lit)));
                this->curr_pos++;
                if (this->return_next){
                    this->return_next = false;
                    return;
                }
                break;
            case CharLiteral:
                char_lit = curr_token.txt[0];
                this->push_node(new LiteralNode(Value::create(CHAR, char_lit)));
                this->curr_pos++;
                if (this->return_next){
                    this->return_next = false;
                    return;
                }
                break;
            case BoolLiteral:
                bool_lit = (curr_token.txt == "true") ? true : false;
                this->push_node(new LiteralNode(Value::create(BOOL, bool_lit)));
                this->curr_pos++;
                if (this->return_next){
                    this->return_next = false;
                    return;
                }
                break;
            // Block Nodes
            case Block:
                // create a new block and push it onto the stack
                sym_table = new SymbolTable(this->curr_scope);
                new_block = new BlockNode(sym_table);
                this->push_block(new_block);
                this->curr_pos++;
                continue;
            case CondBlock:
            case LoopBlock:
                // read the conditonal expression
                curr_pos++;
                this->parse_expr();
                if (this->stack_size() == 0)
                    throw std::runtime_error("syntax error: expected expression (1)");
                condition = this->pop_node();
                // create the block
                sym_table = new SymbolTable(this->curr_scope);
                if (curr_token.type == CondBlock) {
                    new_block = new CondBlockNode(sym_table, condition);
                } else {
                    new_block = new LoopBlockNode(sym_table, condition);
                }
                this->push_block(new_block);
                break;
            case ElseBlock:
                curr_pos++;
                if (this->curr_block->block_type() != Conditional)
                    throw std::runtime_error("syntax error: unexpected token \"else\"");
                conditional = static_cast<CondBlockNode*>(this->curr_block);
                conditional->set_else(new BlockNode(this->curr_scope->get_parent())); // this doesn't leak because the conditional block free's the else clause's memeory
                break;
            case EvalBlock:
                init_count = this->eval_count;
                this->eval_count++;
                eval_block = new EvalBlockNode;
                // read the next singular expression, and assume the next is a closing paren.
                this->curr_pos++;
                while (this->eval_count != init_count){
                    this->return_next = true;
                    this->parse_expr();
                }
                this->return_next = false;
                if (this->stack_size() == 0)
                    throw std::runtime_error("syntax error: expected expression (2)");
                new_node = this->pop_node();
                eval_block->set_body(new_node);
                // ensure that thd end of the eval node was encountered
                this->push_node(eval_block);
                return;
            // Block enders
            case BlockEnd:
                if (this->block_stack.empty())
                    throw std::runtime_error("syntax error: unexpected token \"end\"");
                this->curr_pos++;
                // pop the current block off the stack and append it to the node stack
                this->block_stack.pop();
                this->scopes.push_back(this->scope_stack.top());
                this->scope_stack.pop();
                to_copy = this->curr_block;
                if (this->block_stack.empty()){
                    this->curr_block = nullptr;
                    this->curr_scope = &this->global_scope;
                } else {
                    this->curr_block = this->block_stack.top();
                    this->curr_scope = this->scope_stack.top();
                }
                push_node(to_copy);
                return;
            case EvalBlockEnd:
                if (this->eval_count == 0)
                    throw std::runtime_error("syntax error: unexpected token \")\"");
                this->curr_pos++;
                this->eval_count--;
                return;
            // Binary Expressions
            case And:
            case Or:
                parse_bin_expr(BoolLogic_N, OPERATOR_MAP[curr_token.type]);
                break;
            case Eq:
            case Neq:
            case Greater:
            case Less:
                parse_bin_expr(Comp_N, OPERATOR_MAP[curr_token.type]);
                break;
            case Add:
            case Sub:
            case Mul:
            case Div:
            case Mod:
            case Pow:
                parse_bin_expr(Arith_N, OPERATOR_MAP[curr_token.type]);
                break;
            // Variable-related nodes
            case Defn:
                // parse the next two nodes
                curr_pos++;
                this->parse_expr();
                if (this->stack_size() < 2)
                    throw std::runtime_error("error: expected expression (6)");
                rhs = this->pop_node();
                lhs = this->pop_node();
                // ensure the nodes are a type literal and a symbol, respectively
                if (lhs->get_node_type() != Type_N)
                    throw std::runtime_error("syntax error: expected type literal");
                if (rhs->get_node_type() != Sym_N)
                    throw std::runtime_error("syntax error: invalid variable name");
                // create the variable in the current scope
                var_type = static_cast<TypeNode*>(lhs);
                var_name = static_cast<SymNode*>(rhs);
                sym = var_name->get_sym();
                this->curr_scope->create(sym, var_type->get_type());
                // push the newly created variable onto the node stack
                new_node = new VarNode(curr_scope->get(sym), false);
                this->push_node(new_node);
                continue;
            case Asgn:
                parse_bin_expr(Asgn_N, Assignment);
                break;
            case Print:
            case Println:
                curr_pos++;
                print_node = new PrintNode(curr_token.type == Println);
                init_count = this->stack_size();
                // read every node to the end of the statement as an argument
                this->return_next = false;
                this->parse_expr();
                while (this->stack_size() != init_count)
                    print_node->push_arg(this->pop_node());
                this->push_node(print_node);
                break;
            case ParamOpen:
                if ((curr_pos + 3) > this->token_count ||  this->tokens[curr_pos + 2].type !=  ParamClose)
                    throw std::runtime_error("syntax error: expected token ']");
                interior = this->tokens[curr_pos + 1];
                switch (interior.type){
                    case IntLiteral:
                        new_node = new ParamNode(ParamType::Index, std::stoi(interior.txt));
                        break;
                    case TypeInt:
                    case TypeFloat:
                    case TypeBool:
                    case TypeChar:
                        new_node = new ParamNode(ParamType::Type, TYPE_STR_MAP[curr_token.txt]);
                        break;
                    default:
                        throw std::runtime_error("syntax error: invalid parameter");
                }
                this->curr_pos += 3;
                this->push_node(new_node);
                if (this->return_next)
                    return;
                break;
            case ParamClose:
                throw std::runtime_error("syntax error: unexpected token ']' ");
                break;
            case Sym:
                curr_pos++;
                if (curr_scope->exists(curr_token.txt)){
                    var_node = new VarNode(this->curr_scope->get(curr_token.txt), true);
                    this->push_node(var_node);
                    if (this->return_next)
                        return;
                } else {
                    new_node = new SymNode(curr_token.txt);
                    this->push_node(new_node);
                    return;
                }
                break;
            case Break:
                curr_pos++;
                return;

        }
    }
}

// this function parses a binary expression (such as comparison or arithmetic) and pushes it to the top of the node stack
void Parser::parse_bin_expr(NodeType type, Operator op){
    curr_pos++;
    this->return_next = (type != Asgn_N); // this should always read the next singular expresssion, unless we're assigning to a variable
    this->parse_expr();
    if (this->stack_size() < 2)
        throw std::runtime_error("syntax error: expected expression (7)");
    Node* rhs = this->pop_node();
    Node* lhs = this->pop_node();
    switch (type){
    case Arith_N:
        this->push_node(new ArithNode(lhs, rhs, op));
        break;
    case Comp_N:
        this->push_node(new CompNode(lhs, rhs, op));
        break;
    case BoolLogic_N:
        this->push_node(new BoolLogicNode(lhs, rhs, op));
        break;
    case Asgn_N:
        if (lhs->get_node_type() != Var_N && lhs->get_node_type() != Val_N)
            throw std::runtime_error("syntax error: cannot assign to expression");
        this->push_node(new AsgnNode(static_cast<ValNode*>(lhs), rhs));
        break;
    }
}

// this function creates a new block, and sets it to the current scope 
void Parser::push_block(BlockNode* block){
    this->curr_block = block;
    this->block_stack.push(block);
    this->scope_stack.push(block->get_scope());
    this->curr_scope = block->get_scope();
}