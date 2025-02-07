cmake_minimum_required(VERSION 3.28)
project(Nebula VERSION 0.1.0)

find_package(GTest)
include(GoogleTest)
add_executable(unittests
               src/lexer.cpp 
               src/nodes.cpp 
               src/values.cpp 
               src/symtable.cpp 
               src/block.cpp 
               src/parser.cpp 
               src/interpreter.cpp
               test/tests.cpp )
               target_link_libraries(unittests PRIVATE GTest::gtest)
add_executable(nebula
               src/lexer.cpp 
               src/nodes.cpp 
               src/values.cpp 
               src/symtable.cpp 
               src/block.cpp 
               src/parser.cpp 
               src/interpreter.cpp
               src/main.cpp )
    
               target_link_libraries(unittests PRIVATE GTest::gtest)
