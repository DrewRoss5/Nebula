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
#include "../inc/interpreter.h"

/* DEBUG FUNCTIONS */
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

TEST(ParserTest, Basic){
    // this checks if compound expressions work by doing a simple interpretation of defining and then using a variable
    Interpreter interpreter;
    interpreter.run("let int num = 12; num * 2;");
    Value val = interpreter.result();
    EXPECT_EQ(val.as<int>(), 24);
    // this tests evaluating variables
    interpreter.run(R"(
                        let int foo = 10;
                        let int bar = 2;
                        (foo * bar) == (3 + 2 * 3);
                    )");
    val = interpreter.result();
    EXPECT_EQ(val.as<bool>(), false);
}   
TEST(ParserTest, Blocks){
    // test a simple block
    Interpreter interpreter;
    interpreter.run(
        R"(
        begin
            let char a = 'a';
            let char b = 'b';
            a != b;
        end
        )"
    );
    Value val = interpreter.result();
    EXPECT_EQ(val.as<bool>(), true);
    // test that blocks can access an outer scope
    interpreter.run(
        R"(
            let float tmp = 5.0;
            begin
                tmp==5.0;
            end
            tmp;
        )"
    );
    val = interpreter.result();
    EXPECT_EQ(val.as<double>(), 5.0);
    // test that an outerscope cannot access a block's variables
    int res = interpreter.run(
        R"(
            begin
                let float tmp = 10.0;
            end
            tmp + 2.0;
        )"
    );
    EXPECT_EQ(res, 1);
    // test that conditionals work
    interpreter.run(
        R"(
            let int node = 5;
            if (true)
                node = 20;
            end
            node;
        )"
    );
    val = interpreter.result();
    EXPECT_EQ(val.as<int>(), 20);
    // test a false condition
    interpreter.run(
        R"(
            let int five = 5;
            if (false)
                five = 20;
            end
            five;

        )"
    );
    val = interpreter.result();
    EXPECT_EQ(val.as<int>(), 5);
    // test a  while loop
    interpreter.run(
        R"(
            let int ctr = 0;
            while (ctr != 10)
                ctr = (ctr + 1);
            end
            ctr;
        )"
    );
    val = interpreter.result();
    EXPECT_EQ(val.as<int>(), 10);
}
TEST(ParserTest, Printing){
    std::cout << "This should print all numbers between one and 5, followed by all numbers 6-10" << std::endl;
    Interpreter interpreter;
    interpreter.run(
        R"(
            let int num = 1;
            while (num != 6)
                println num;
                num = num + 1;
            end
            while (num != 11)
                print num ' ';
                num = num + 1;
            end;
            println;
        )"
    );
}


TEST(InterpreterTest, Final){
    // this simple program serves as the first "real" test of nebula, it should calculate the 20th fibonacci number
    Interpreter interpreter;
    int res = interpreter.run_file("../examples/fib.neb");
    if (res != 0)
        interpreter.display_err();
    Value val = interpreter.result();
    EXPECT_EQ(val.as<int>(), 6765);
}

int main(int argc, char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}