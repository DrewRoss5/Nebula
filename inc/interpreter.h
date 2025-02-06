#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <vector>
#include <stack>

#include "block.h"
#include "lexer.h"
#include "parser.h"
#include "values.hpp"
#include "nodes.hpp"

class Interpreter{
    public:
        Interpreter() {};
        int run_file(const std::string& file_path);
        int run(const std::string& expr);
        Value result();
        void display_err();
    private:
        int set_tokens(const std::string& expr);
        std::string err_msg;
        std::vector<Token> tokens;
        std::stack<Value> eval_stack;
        Parser parser;
};

#endif