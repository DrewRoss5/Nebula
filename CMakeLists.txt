cmake_minimum_required(VERSION 3.28)
project(Nebula VERSION 0.1.0)

find_package(GTest)
include(GoogleTest)
add_executable(NebulaTests 
               src/lexer.cpp 
               src/nodes.cpp 
               src/values.cpp 
               src/symtable.cpp 
               src/block.cpp 
               src/parser.cpp 
               test/tests.cpp )
target_link_libraries(NebulaTests PRIVATE GTest::gtest)