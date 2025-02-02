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

/* TESTS FOR THE LEXER */
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


/* TESTS FOR THE VALUE CLASS */
TEST(ValueTests, Reading){
    Value int_val = Value::create(INT, 5);
    Value float_val = Value::create(FLOAT, 12.5);
    Value char_val = Value::create(CHAR, '%');
    Value bool_val = Value::create(BOOL, true);
    // test that values can be read correctly
    EXPECT_EQ(int_val.as<int>(), 5);
    EXPECT_EQ(float_val.as<double>(), 12.5);
    EXPECT_EQ(char_val.as<char>(), '%');
}
TEST(ValueTests, Writing){
    Value int_val = Value::create(INT, 5);
    int_val.update(666);
    EXPECT_EQ(int_val.as<int>(), 666);
}

/* TESTS FOR NON-BLOCK NODES */
TEST(NodeTests, Literals){
    // test values
    Value int_val = Value::create(INT, 5);
    Value float_val = Value::create(FLOAT, 12.5);
    Value char_val = Value::create(CHAR, '%');
    Value bool_val = Value::create(BOOL, true);
    // ensure each type of value can be evaluated as literal
    LiteralNode node = LiteralNode(int_val);
    EXPECT_EQ(node.eval(), int_val);
    node = LiteralNode(float_val);
    EXPECT_EQ(node.eval(), float_val);
    node = LiteralNode(char_val);
    EXPECT_EQ(node.eval(), char_val);
    node = LiteralNode(bool_val);
    EXPECT_EQ(node.eval(), bool_val);
}
TEST(NodeTests, Variables){
    std::shared_ptr<Value> int_ptr(Value::create_dyn(INT, 0));
    Value new_val = Value::create(INT, 42);
    // check that varables are updated correctly 
    VarNode int_var(int_ptr, true);
    EXPECT_EQ(int_var.eval().as<int>(), 0);
    int_var.assign(new_val);
    EXPECT_EQ(int_var.eval().as<int>(), 42);
}
TEST(NodeTests, Assignment){
    Value float_val = Value::create(FLOAT, 12.34);
    std::shared_ptr<Value> float_ptr(new Value(FLOAT));
    LiteralNode lit_val(float_val);
    VarNode var(FLOAT);
    var.set_ptr(float_ptr);
    AsgnNode assignment(&var, &lit_val);
    assignment.eval();
    EXPECT_EQ(var.eval().as<double>(), 12.34);
}
TEST(NodeTests, Comparison){
    std::shared_ptr<Value> val_ptr(Value::create_dyn(INT, 42));
    LiteralNode int_literal(*val_ptr);
    VarNode int_var(val_ptr, true);
    // ensure that eequality and inequality are determined correctly
    CompNode eq_node(&int_literal, &int_var, Equal);
    EXPECT_TRUE(eq_node.eval().as<bool>());
    int_var.assign(std::move(Value::create(INT, 10)));
    EXPECT_FALSE(eq_node.eval().as<bool>());
}
TEST(NodeTests, BoolLogic){
    LiteralNode false_lit(std::move(Value::create(BOOL, false)));
    LiteralNode true_lit(std::move(Value::create(BOOL, false)));
    std::shared_ptr<Value> val_ptr(std::move(Value::create_dyn(INT, 42)));
    LiteralNode int_literal(*val_ptr);
    VarNode int_var(val_ptr, true);
    CompNode eq_node(&int_literal, &int_var, Equal);
    // check that boolean expressions are properly evalutated
    BoolLogicNode and_node(&eq_node, &true_lit, LogicAnd);
    BoolLogicNode or_node(&eq_node, &false_lit, LogicAnd);
    EXPECT_TRUE(and_node.eval().as<bool>());
    int_var.assign(std::move(Value::create(INT, 5)));
    EXPECT_FALSE(or_node.eval().as<bool>());
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

/* BLOCK TESTS */
TEST(BlockTests, BaseBlocks){
    /* 
        This runs a test of a simple expression, assigning a value (2) to a variable and then
        multiplying that variable by a constant (5). Because the multiplication is the last node evaluated, tbis
        should evaluate to 10
    */
    // intialize child nodes (statements)
    std::shared_ptr<Value> int_ptr(Value::create_dyn(INT));
    LiteralNode zero(Value::create(INT, 0));
    LiteralNode two(Value::create(INT, 2));
    LiteralNode five(Value::create(INT, 5));
    VarNode int_var(int_ptr, false); // let int_var int = 0;
    int_var.assign(zero.eval()); // int_var = 0;
    AsgnNode asgn_node(&int_var, &two);
    ArithNode mul_node(&int_var, &five, ArithMul);
    // construct the block
    SymbolTable sym_table;
    BlockNode test_block(&sym_table);
    test_block.push_statement(&asgn_node);
    test_block.push_statement(&mul_node);
    EXPECT_EQ(test_block.eval().as<int>(), 10);
}
TEST(BlockTests, CondBlock){
    // logical literals
    LiteralNode lit_false(Value::create(BOOL, false));
    LiteralNode lit_true(Value::create(BOOL, true));
    // int literals
    LiteralNode one(Value::create(INT, 1));
    LiteralNode two(Value::create(INT, 2));
    // create the int variable
    std::shared_ptr<Value> int_ptr(Value::create_dyn(INT));
    VarNode int_var(int_ptr, false);
    // create the assignment nodes
    AsgnNode asgn_one (&int_var, &one);
    AsgnNode asgn_two (&int_var, &two);
    // create and evaluate the conditionals
    SymbolTable sym_table;
    CondBlockNode true_cond(&sym_table, &lit_true);
    true_cond.push_statement(&asgn_one);
    CondBlockNode false_cond(&sym_table, &lit_false);
    true_cond.push_statement(&asgn_two);
    true_cond.eval();
    EXPECT_TRUE(false_cond.eval().is_null());
    EXPECT_EQ(int_var.eval().as<int>(), 2);
}
TEST(BlockTests, LoopBlock){
    // initalize child nodes
    LiteralNode one(Value::create(INT, 1));
    LiteralNode ten(Value::create(INT, 10));
    std::shared_ptr<Value> int_ptr(Value::create_dyn(INT));
    VarNode int_var(int_ptr, false);
    int_var.assign(one.eval());
    ArithNode add_node(&int_var, &one,  ArithAdd);
    AsgnNode asgn_node(&int_var, &add_node);
    SymbolTable sym_table;
    CompNode comparison(&int_var, &ten, NEqual);
    LoopBlockNode loop(&sym_table, &comparison);
    loop.push_statement(&asgn_node);
    // this emulates a while loop that adds one to int_var until it is equal to one 
    Value result = loop.eval();
    EXPECT_EQ(result.as<int>(), 10);
    EXPECT_EQ(int_var.eval().as<int>(), 10);
}

int main(int argc, char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}