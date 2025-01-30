#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

#include "../inc/lexer.h"

void tokenize(const std::string& expr, std::vector<Token>& tokens){
    size_t str_pos = 0;
    size_t expr_len = expr.length();
    std::unordered_map<char, TokenType> char_tokens = {
        {'+', Add},
        {'-', Sub},
        {'*', Mul},
        {'/', Div},
        {'>', Greater},
        {'<', Less},
        {'{', Block},
        {'}', BlockEnd},
        {'(', EvalBlock},
        {')', EvalBlockEnd},
        {';', Block},
        {'=', Other},
        {'\'', Other},
        {'!', Other},
        {' ', Other},
        {'\n', Other}
    };
    std::unordered_map<std::string, TokenType> word_tokens= {
        {"if", CondBlock},
        {"while", LoopBlock},
        {"int", TypeInt},
        {"float", TypeFloat},
        {"char", TypeChar},
        {"bool", TypeBool},
        {"true", BoolLiteral},
        {"false", BoolLiteral},
        {"let", Defn},
        {"||", Or},
        {"&&", And},

    };
    while (str_pos < expr_len){
        char chr = expr[str_pos++];
        if (char_tokens.find(chr) != char_tokens.end()){
            std::string token_str;
            token_str.push_back(chr);
            TokenType token = char_tokens[chr];
            if (token != Other){
                tokens.push_back({token, token_str});
                continue;
            }
            std::string chr_str;
            switch (chr){
            case '!':
                if (str_pos < expr_len && expr[str_pos] == '='){
                    tokens.push_back({Neq, "!="});
                    str_pos++;
                }
                else
                    tokens.push_back(Not);
            break;
            case '=':
                if (str_pos < expr_len && expr[str_pos] == '='){
                    tokens.push_back({Eq, "=="});
                    str_pos++;
                }
                else
                    tokens.push_back({Asgn, "="});
            break;
            case '\'':
                // ensure the quore proceeds a valid character literal 
                if (str_pos+1 >= expr_len || expr[str_pos+1] != '\'')
                    throw std::runtime_error("invalid character literal");
                chr_str.push_back(expr[str_pos]);
                // push the literal to the token vector
                tokens.push_back({CharLiteral, chr_str});
                str_pos += 2;
            break;
            default: break;
            }
        }
        // parse the token if it's a literal number
        else if ('0' <= chr && chr <= '9'){
            std::string token_str;
            token_str.push_back(chr);
            bool radix_found {false};
            while (str_pos < expr_len && (('0' <= expr[str_pos] && chr <= expr[str_pos]) || expr[str_pos] == '.')){
                if (expr[str_pos] == '.'){
                    if (radix_found)
                        throw std::runtime_error("invalid floating point literal");
                    radix_found = true;
                }
                token_str.push_back(expr[str_pos]);
                str_pos++;
            }
            // append the parsed number to the token vector
            if (radix_found)
                tokens.push_back({FloatLiteral, token_str});
            else
                tokens.push_back({IntLiteral, token_str});
        }
        else{
            // parse to the end of the token
            std::string token_str;
            token_str.push_back(chr);
            while (str_pos < expr_len && char_tokens.find(expr[str_pos]) == char_tokens.end())
                token_str.push_back(expr[str_pos++]);
            // determine the token type
            auto type = word_tokens.find(token_str);
            if (type != word_tokens.end()){
                tokens.push_back({type->second, token_str});
            }
            // we assume any unrecognized character is a user-defined symbol
            else
                tokens.push_back({Sym, token_str});
        }
    }
}