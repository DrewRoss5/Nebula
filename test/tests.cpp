#include <gtest/gtest.h>

#include  "../inc/lexer.h"

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

/* LEXER TESTS */
TEST(LexerTests, CharTokens){
    std::vector<TokenType> expected = {Block, EvalBlock, Add, Sub, Mul, Div, Greater, Less, Eq, Neq, Asgn, Not, EvalBlockEnd, BlockEnd};
    std::vector<Token> tokens;
    tokenize("{(+ - * / >< == != = !)}", tokens);
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

int main(int argc, char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}