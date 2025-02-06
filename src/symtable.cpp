#include <memory>
#include <unordered_map>

#include "../inc/values.hpp"
#include "../inc/symtable.h"

SymbolTable::SymbolTable(SymbolTable* parent){
    this->parent = parent;
}

// creates a new value on the symbol table and associates it with a pointer. 
// This function assumes that the symbol has already been determined not to exist
void SymbolTable::create(const std::string& symbol, ValueType type){
    this->table[symbol] = std::shared_ptr<Value> (Value::create_dyn(type));
}

// clears all values on the symtable
void SymbolTable::clear(){
    this->table.clear();
}

// returns a pointer to a symbol on the table, or a null pointer if the symbol does not exist
std::shared_ptr<Value> SymbolTable::get(const std::string& symbol){
    auto val_itt = this->table.find(symbol);
    if (val_itt != table.end())
        return val_itt->second;
    // search through all parent tables for a matching symbol
    SymbolTable* curr = this->parent;
    while (curr){
        val_itt = curr->table.find(symbol);
        if (val_itt != curr->table.end())
            return val_itt->second;
        else
            curr = curr->parent;
    }
    // no match was found, return nullptr
    return std::shared_ptr<Value>(nullptr);
}

// returns whether or not a given value exists in the +
bool SymbolTable::exists(const std::string& symbol){
    std::shared_ptr<Value> ptr = this->get(symbol);
    if (ptr.get() == nullptr)
        return false;
    return true;
}