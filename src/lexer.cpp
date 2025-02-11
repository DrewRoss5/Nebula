#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

#include "../inc/lexer.h"

Token parse_token(const std::string& expr, size_t& str_pos, const std::unordered_map<char, TokenType>& char_tokens, const std::unordered_map<std::string, TokenType>& word_tokens){
    // read to the end of the current token
    std::string token_str;
    token_str.push_back(expr[str_pos - 1]);
    while (str_pos < expr.size() && !char_tokens.count(expr[str_pos])){
        token_str.push_back(expr[str_pos]);
        ++str_pos;
    }
    // determine the token type
    auto type = word_tokens.find(token_str);
    if (type != word_tokens.end()) 
        return Token(type->second, token_str);
    // we assume any unrecognized character is a user-defined symbol
    return Token(Sym, token_str);
}

Token parse_num(const std::string& expr, size_t& str_pos){
    std::string token_str;
    token_str.push_back(expr[str_pos-1]);
    bool radix_found {false};
        while (str_pos < expr.size() && (('0' <= expr[str_pos] && expr[str_pos] <= '9') || expr[str_pos] == '.')){
            if (expr[str_pos] == '.'){
                if (radix_found)
                    throw std::runtime_error("invalid floating point literal");
                radix_found = true;
            }
            token_str.push_back(expr[str_pos]);
            str_pos++;
        }
        if (radix_found)
            return Token(FloatLiteral, token_str);
        return Token(IntLiteral, token_str);
}

void tokenize(const std::string& expr, std::vector<Token>& tokens){
    size_t str_pos = 0;
    size_t expr_len = expr.length();
    std::unordered_map<char, TokenType> char_tokens = {
        {'+', Add},
        {'-', Sub},
        {'*', Other},
        {'/', Div},
        {'%', Mod},
        {'>', Greater},
        {'<', Less},
        {'(', EvalBlock},
        {')', EvalBlockEnd},
        {'[', ParamOpen},
        {']', ParamClose},
        {';', Break},
        {'\n', Break},
        {'=', Other},
        {'\'', Other},
        {'!', Other},
        {' ', Other},
        {'\t', Other}
    };
    std::unordered_map<std::string, TokenType> word_tokens= {
        {"if", CondBlock},
        {"else", ElseBlock},
        {"while", LoopBlock},
        {"block", Block},
        {"int", TypeInt},
        {"float", TypeFloat},
        {"char", TypeChar},
        {"bool", TypeBool},
        {"true", BoolLiteral},
        {"false", BoolLiteral},
        {"arr", Arr},
        {"let", Defn},
        {"begin", Block},
        {"end", BlockEnd},
        {"||", Or},
        {"&&", And},
        {"print", Print},
        {"println", Println}

    };
    std::string token_str;
    while (str_pos < expr_len){
        token_str.clear();
        char chr = expr[str_pos];
        str_pos += 1;
        auto type_itt = char_tokens.find(chr);
        if (type_itt != char_tokens.end()){
            token_str.push_back(chr);
            TokenType token = type_itt->second;
            if (token != Other){
                tokens.push_back({token, token_str});
                continue;
            }
            std::string chr_str;
            switch (chr){
            case '*':
                if (str_pos < expr_len && expr[str_pos] == '*'){
                    tokens.push_back({Pow, "**"});
                    str_pos++;
                } else { 
                    tokens.push_back({Mul, "*"});
                }
                break;
            case '!':
                if (str_pos < expr_len && expr[str_pos] == '='){
                    tokens.push_back({Neq, "!="});
                    str_pos++;
                } else {
                    tokens.push_back(Not);
                }
            break;
            case '=':
                if (str_pos < expr_len && expr[str_pos] == '='){
                    tokens.push_back({Eq, "=="});
                    str_pos++;
                } else {
                    tokens.push_back({Asgn, "="});
                }
            break;
            case '\'':
                // ensure the quore proceeds a valid character literal 
                if (str_pos >= expr_len || expr[str_pos+1] != '\'')
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
        else if ('0' <= chr && chr <= '9')
            tokens.push_back(parse_num(expr, str_pos));
        else
            tokens.push_back(parse_token(expr, str_pos, char_tokens, word_tokens));
    }
}