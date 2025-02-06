#include <iostream>
#include <stdexcept>
#include <fstream>

#include "../inc/interpreter.h"
#include "../inc/lexer.h"
#include "../inc/parser.h"
#include "../inc/nodes.hpp"
#include "../inc/block.h"

// this displays the last thrown erro message
void Interpreter::display_err(){
    std::cout << "\033[31mnebula error: \033[0m"  << this->err_msg << std::endl;
}
// tokenizes a string and updates the interpreter's tokens, returns 0 on success and 1 on failure
int Interpreter::set_tokens(const std::string& expr){
    this->tokens.clear();
    try{
        tokenize(expr, this->tokens);
        return 0;
    }
    catch (std::runtime_error e){
        this->err_msg = e.what();
        return 1;
    }
}
// returns the top value on the eval stack, or an empty value if nothing's on the stack
Value Interpreter::result(){
    if (this->eval_stack.empty())
        return Value(NULL_TYPE);
    return this->eval_stack.top();
}
// reads source code from a provided file and evaluates it, returns 0 for succss and 1 for failure
int Interpreter::run_file(const std::string& file_path){
    std::ifstream in(file_path);
    if (!in.good()){
        this->err_msg = "failed to read source file: \""+file_path +"\"";
        return 1;
    }
    std::string tmp, src_code;
    while (std::getline(in, tmp))
        src_code += tmp;
    in.close();
    return this->run(src_code);
}
// runs the given expression/source code, returns 1 on error
int Interpreter::run(const std::string& statements){
    // empty the eval stack, if it isn't already
    while (!this->eval_stack.empty())
        this->eval_stack.pop();
    // toenize the expression
    int res = this->set_tokens(statements);
    if (res == 1)
        return 1;
    // parse the tokens into expression
    this->parser.reset(this->tokens);
    try{
        this->parser.parse();
    }
    catch (std::runtime_error e) {
        this->err_msg = e.what();
        return 1;
    }
    // ensure the expression was parsed correctly
    if (!this->parser.validate(this->err_msg))
        return 1;
    // evaluate each expression
    Node* expr;
    BlockNode* block;
    Value val;
    try{
        do{
            expr = this->parser.next_expr();
            if (!expr)
                break;
            // if the value is a block, ensure the correct eval function is called
            else if (expr->node_type() == Block_N){
                block = static_cast<BlockNode*>(expr);
                switch (block->block_type()){
                    case BlockType::Conditional:
                        this->eval_stack.push(static_cast<CondBlockNode*>(expr)->eval());
                        break;
                    case BlockType::Loop:
                        this->eval_stack.push(static_cast<LoopBlockNode*>(expr)->eval());
                        break;
                    default:
                        this->eval_stack.push(block->eval());
                        break;
                }
            }
            else
                this->eval_stack.push(expr->eval());
        }
        while(expr); 
    } 
    catch (std::runtime_error e){
        this->err_msg = e.what();
        return 1;
    }
    return 0;
}