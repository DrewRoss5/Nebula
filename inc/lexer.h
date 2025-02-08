#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum TokenType{
    // data types
    TypeInt,
    TypeFloat,
    TypeBool,
    TypeChar,
    // literal types
    IntLiteral,
    FloatLiteral,
    CharLiteral,
    BoolLiteral,
    // block types
    Block,
    CondBlock,
    ElseBlock,
    LoopBlock,
    EvalBlock,
    BlockEnd,
    EvalBlockEnd,
    // comparison operators
    And,
    Or,
    Not,
    Eq,
    Neq,
    Greater,
    Less,
    // arithmetic operators
    Add,
    Sub,
    Div,
    Mul,
    Pow,
    Mod,
    // io operators
    Print,
    Println,
    // other types
    Defn,
    Sym,
    Asgn,
    Break,
    Other // this should only be used by the lexer itself for ambiguous cases
};

struct Token{
    Token(TokenType type) {this->type = type; this->txt = "";};
    Token(TokenType type, const std::string& txt) {this->type = type; this->txt = txt;};
    TokenType type;
    std::string txt; 
};

void tokenize(const std::string& statement, std::vector<Token>& tokens);

#endif