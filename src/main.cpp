#include <iostream>
#include <string>

#include "../inc/interpreter.h"

int main(int argc, char** argv){
    if (argc != 2){
        std::cerr << "error: this program acepts exactly one argument" << std::endl;
        return 1;
    }
    std::string file_path = argv[1];
    Interpreter interpreter;
    int res = interpreter.run_file(file_path);
    if (res){
        interpreter.display_err();
        return 1;
    }
    return 0;
}