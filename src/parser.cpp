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
    };

std::unordered_map<TokenType, ValueType> TYPE_MAP{
    {TypeInt, ValueType::INT},
    {TypeFloat, ValueType::FLOAT},
    {TypeBool, ValueType::BOOL},
    {TypeChar, ValueType::CHAR},
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
        if (tmp->node_type() != Block_N)
            this->nodes.push_back(tmp);
    }
    // this map keeps track of all pointers that have been deleted. A map is used for O(1) lookup times
    std::unordered_map<Node*, bool> freed_ptrs;
    // delete all dynamically allocated nodes and symbol tabkles
    for (int i = 0; i < this->scopes.size(); i++){
        delete this->scopes[i];
        this->scopes[i] = nullptr;
    }
    for (int i = 0; i < this->nodes.size(); i++){
        if (freed_ptrs.count(this->nodes[i]) == 0){
            delete this->nodes[i];
            freed_ptrs[this->nodes[i]] = true;
        }
        this->nodes[i] = nullptr;
    }
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
    else{
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
        EvalBlockNode* eval_block;
        VarNode* var_node;
        SymbolTable* sym_table;
        TokenType op;
        std::string sym;
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
                continue;
            case FloatLiteral:
                float_lit = std::stod(curr_token.txt);
                this->push_node(new LiteralNode(Value::create(FLOAT, float_lit)));
                this->curr_pos++;
                continue;
            case CharLiteral:
                char_lit = curr_token.txt[0];
                this->push_node(new LiteralNode(Value::create(CHAR, char_lit)));
                this->curr_pos++;
                continue;
            case BoolLiteral:
                bool_lit = (curr_token.txt == "true") ? true : false;
                this->push_node(new LiteralNode(Value::create(BOOL, bool_lit)));
                this->curr_pos++;
                continue;
            // Block Nodes
            case Block:
                // create a new block and push it onto the stack
                sym_table = new SymbolTable(this->curr_scope);
                new_block = new BlockNode(sym_table);
                scope_stack.push(sym_table);
                block_stack.push(new_block);
                // set the new block as current
                this->curr_block = new_block;
                this->curr_scope = sym_table;
                this->curr_pos++;
                continue;
            case CondBlock:
            case LoopBlock:
                // read the conditonal expression
                curr_pos++;
                this->parse_expr();
                if (this->node_stack.empty())
                    throw std::runtime_error("syntax error: expected expression (1)");
                condition = this->pop_node();
                // create the block
                if (curr_token.type == CondBlock) {
                    new_block = new CondBlockNode(sym_table, condition);
                } else {
                    new_block = new LoopBlockNode(sym_table, condition);
                }
                sym_table = new SymbolTable(this->curr_scope);
                this->block_stack.push(new_block);
                this->scope_stack.push(sym_table);
                continue;
            case EvalBlock:
                init_count = this->eval_count;
                this->eval_count++;
                eval_block = new EvalBlockNode;
                // read the next singular expression, and assume the next is a closing paren.
                this->curr_pos++;
                this->parse_expr();
                if (this->node_stack.empty())
                    throw std::runtime_error("syntax error: expected expression (2)");
                new_node = this->pop_node();
                eval_block->set_body(new_node);
                // ensure that thd end of the eval node was encountered
                if (this->eval_count > init_count)
                    throw std::runtime_error("syntax error: expected \")\"");
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
                }
                else{
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
            // TODO: Condense this using a porse_binary_expr function
            case And:
            case Or:
                curr_pos++;
                this->parse_expr();
                if (this->stack_size() < 2)
                    throw std::runtime_error("syntax error: expected expression (3)");
                rhs = this->pop_node();
                lhs = this->pop_node();
                new_node = new BoolLogicNode(lhs, rhs, OPERATOR_MAP[curr_token.type]);
                this->push_node(new_node);
                return;
            case Eq:
            case Neq:
            case Greater:
            case Less:
                curr_pos++;
                this->parse_expr();
                if (this->stack_size() < 2)
                    throw std::runtime_error("syntax error: expected expression (4)");
                rhs = this->pop_node();
                lhs = this->pop_node();
                new_node = new CompNode(lhs, rhs, OPERATOR_MAP[curr_token.type]);
                this->push_node(new_node);
                continue;
            case Add:
            case Sub:
            case Mul:
            case Div:
                // read the next expression
                curr_pos++;
                this->parse_expr();
                // ensure at least two nodes are on the stack
                if (this->stack_size() < 2)
                    throw std::runtime_error("syntax error: expected expression (5)");
                //generate  the boolean expression
                rhs = this->pop_node();
                lhs = this->pop_node();
                new_node = new ArithNode(lhs, rhs, OPERATOR_MAP[curr_token.type]);
                this->push_node(new_node);
                continue;
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
                if (lhs->node_type() != Type_N)
                    throw std::runtime_error("syntax error: expected type literal");
                if (rhs->node_type() != Sym_N)
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
                // parse the next two nodes
                curr_pos++;
                this->parse_expr();
                if (this->stack_size() < 2)
                    throw std::runtime_error("syntax error: expected expression (7)");
                rhs = this->pop_node();
                lhs = this->pop_node();
                if (lhs->node_type() != Var_N)
                    throw std::runtime_error("syntax error: cannot assign to expression");
                var_node = static_cast<VarNode*>(lhs);
                new_node = new AsgnNode(var_node, rhs);
                this->push_node(new_node);
                continue;
            case Sym:
                curr_pos++;
                if (curr_scope->exists(curr_token.txt)){
                    var_node = new VarNode(this->curr_scope->get(curr_token.txt), true);
                    this->push_node(var_node);
                    continue;
                }
                else{
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