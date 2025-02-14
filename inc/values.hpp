#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <vector>
#include <cstring>
#include <cstddef>
#include <stdexcept>

class NebulaArray;

enum ValueType{
    INT,
    FLOAT,
    CHAR,
    BOOL,
    NULL_TYPE
};

class Value{
    public:
        Value() {}
        ~Value();
        Value(ValueType type, const std::vector<std::byte>& val, bool is_arr = false);
        Value(ValueType type, bool is_arr = false) {this->type = type;};
        template <typename T>
        static Value create(ValueType type, const T& val, bool is_arr = false);
        template <typename T>
        static Value* create_dyn(ValueType type, const T& val, bool is_arr = false);
        static Value* create_dyn(ValueType type, bool is_arr = false);
        template <typename T>
        T as() const;
        template <typename T>
        void update(const T& new_val);
        bool operator==(const Value& rhs) const;
        ValueType get_type() const {return this->type;};
        bool is_null() {return this->type == NULL_TYPE;}
        friend std::ostream& operator<<(std::ostream& out, const Value& val); 
        bool is_array() {return this->is_arr;}
        NebulaArray& as_arr();
    private:
        std::vector<std::byte> val;
        ValueType type;
        bool is_arr;
        NebulaArray* arr {nullptr};
};

template <typename T>
Value Value::create(ValueType type, const T& val, bool is_arr){
    std::vector<std::byte> bytes;
    bytes.resize(sizeof(T));
    std::memcpy(&bytes[0], &val, sizeof(T));
    return std::move(Value(type, bytes, is_arr));
}

// this creates a new dynamically allocated Value of a given type USE WITH CAUTION
template <typename T>
Value* Value::create_dyn(ValueType type, const T& val, bool is_arr){
    std::vector<std::byte> bytes;
    bytes.resize(sizeof(T));
    std::memcpy(&bytes[0], &val, sizeof(T));
    return new Value(type, bytes, is_arr);
}


template <typename T>
T Value::as() const{
    if (this->type ==  NULL_TYPE)
        throw std::runtime_error("cannot evaluate void value");
    T retval;
    std::memcpy(&retval, &this->val[0], sizeof(T));
    return retval;
}

template <typename T>
void Value::update(const T& new_val){
    std::memcpy(&this->val[0], &new_val, sizeof(T));
}

// this is the internal representation of arrays for nebula, simmilar to a minimized version of std::vector
class NebulaArray{
    public:
        NebulaArray() {this->arr_ptr = nullptr; this->val_type = NULL_TYPE;}
        NebulaArray(ValueType type);
        ~NebulaArray();
        Value& get(int index);
    private:
        int size {0};
        int capacity {32};
        Value* arr_ptr;
        ValueType val_type;
        void realloc();
};

#endif
