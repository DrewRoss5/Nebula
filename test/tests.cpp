#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include <gtest/gtest.h>

#include  "../inc/lexer.h"
#include "../inc/values.hpp"
#include "../inc/nodes.hpp"
#include "../inc/symtable.h"
#include "../inc/block.h"
#include "../inc/parser.h"

/* DEBUG FUNCTIONS */
void display_token(Token& token){
    std::cout << "Token ID: " << token.type << ", Token content: " << token.txt << std::endl;
}
bool comp_token_types(const std::vector<Token>& tokens, const std::vector<TokenType>& expected){
    if (tokens.size() != expected.size()){
        std::cout << "Non-matching sizes: expected " << expected.size() << " tokens, got " << tokens.size() << std::endl; 
        return false;
    }
    for (int i = 0; i < expected.size(); i++){
        if ((int) tokens[i].type != (int) expected[i]){
            std::cout << "Discrepency found at position " << i << std::endl;
            return false;
        }
    }
    return true;
}
bool comp_token_text(const std::vector<Token>& tokens, const std::vector<std::string>& expected){
    for (int i = 0; i < tokens.size(); i++){
        if (tokens[i].txt != expected[i]){
            std::cout << "Expected: " << expected[i] << " got " << tokens[i].txt << std::endl;
            return false;
        }
    }
    return true;
}

// this is a simplified version of the interpreter for testing purposes
Value interpret(const std::string& expr){
    std::vector<Token> tokens;
    tokenize(expr, tokens);
    Parser parser(tokens);
    parser.parse();
    Value val;
    Node* tmp;
    do{
        tmp = parser.next_expr();
        if (tmp)
            val = tmp->eval();
    } 
    while(tmp);
    return val;
}

/* TESTS FOR THE LEXER */
TEST(LexerTests, CharTokens){
    std::vector<TokenType> expected = {EvalBlock, Add, Sub, Mul, Div, Greater, Less, Eq, Neq, Asgn, Not, EvalBlockEnd};
    std::vector<Token> tokens;
    tokenize("(+ - * / >< == != = !)", tokens);
    EXPECT_TRUE(comp_token_types(tokens, expected));
}
TEST(LexerTests, WordTokens){
    std::vector<TokenType> expected = {CondBlock, LoopBlock, TypeInt, TypeFloat, TypeChar, TypeBool, Defn, Or, And};
    std::vector<Token> tokens;
    tokenize("if while int float char bool let || &&", tokens);
    EXPECT_TRUE(comp_token_types(tokens, expected));
}
TEST(LexerTests, Literals){
    std::vector<TokenType> expected = {IntLiteral, FloatLiteral, CharLiteral, BoolLiteral};
    std::vector<Token> tokens;
    tokenize("123 1.23 'a' true", tokens);
    EXPECT_TRUE(comp_token_types(tokens, expected));
    EXPECT_TRUE(comp_token_text(tokens, {"123", "1.23", "a", "true"}));
}
TEST(LexerTests, Symbols){
    std::vector<Token> tokens;
    tokenize("these are user defined", tokens);
    EXPECT_TRUE(comp_token_types(tokens, {Sym, Sym, Sym, Sym}));
    EXPECT_TRUE(comp_token_text(tokens, {"these", "are", "user", "defined"}));
}

/* SYMBOL TABLE TESTS */
TEST(SymbolTableTests, General){   
    Value int_val = Value::create(INT, 15);
    Value char_val = Value::create(CHAR, '!');
    SymbolTable sym_table;
    SymbolTable child_table(&sym_table);
    sym_table.create("int_var", INT);
    child_table.create("char_var", CHAR);
    // ensure scopes work properly
    EXPECT_TRUE(sym_table.exists("int_var"));
    EXPECT_FALSE(sym_table.exists("char_var"));
    EXPECT_TRUE(child_table.exists("int_var"));
    EXPECT_TRUE(child_table.exists("char_var"));
    // ensure that pointers are disributed properly
    VarNode int_var(sym_table.get("int_var"), true);
    VarNode int_var_copy(child_table.get("int_var"), true);
    int_var.assign(std::move(Value::create(INT, 256)));
    EXPECT_EQ(int_var_copy.eval().as<int>(), 256);

}

/* Parser Tests */
TEST(ParserTests, Basic){
    Value val = interpret("5 + 10;");
    EXPECT_EQ(val.as<int>(), 15);
    val = interpret("let float x = 10.5;");
    EXPECT_EQ(val.as<double>(), 10.5);
}
TEST(ParserTests, BasicCompound){
    // this checks if compound expressions work by doing a simple interpretation of defining and then using a variable
    Value val = interpret("let int num = 12; num * 2;");
    EXPECT_EQ(val.as<int>(), 24);
    // this tests evaluating variables
    val = interpret("let int foo = 10;"\
                    "let int bar = 2;"\
                    "(foo * bar) == 20;");
    EXPECT_EQ(val.as<bool>(), true);
}   
TEST(ParserTest, Blocks){
    // test a simple block
    Value val = interpret(
        R"(
        begin
            let char a = 'a';
            let char b = 'b';
            a == b;
        end
        )"
    );
}

int main(int argc, char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}