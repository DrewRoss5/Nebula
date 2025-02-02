#ifndef SYM_TABLE_H
#define SYM_TABLE_H

#include <memory>
#include <string>
#include <unordered_map>

#include "../inc/values.hpp"

class SymbolTable{
    public:
        SymbolTable() {}
        SymbolTable(SymbolTable* parent);
        void create(const std::string& symbol, ValueType type);
        // getters
        std::shared_ptr<Value> get(const std::string& val);
        bool exists(const std::string& symbol);
    private:
        SymbolTable* parent {nullptr};
        std::unordered_map<std::string, std::shared_ptr<Value>> table;
};

#endif