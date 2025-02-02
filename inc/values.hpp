#ifndef VALUE_H
#define VALUE_H

#include <vector>
#include <cstring>
#include <cstddef>
#include <stdexcept>

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
        Value(ValueType type, const std::vector<std::byte>& val);
        Value(ValueType type) {this->type = type;};
        template <typename T>
        static Value create(ValueType type, const T& val);
        template <typename T>
        static Value* create_dyn(ValueType type, const T& val);
        static Value* create_dyn(ValueType type);
        template <typename T>
        T as() const;
        template <typename T>
        void update(const T& new_val);
        bool operator==(const Value& rhs) const;
        ValueType get_type() {return this->type;};
        bool is_null() {return this->type == NULL_TYPE;}
    private:
        std::vector<std::byte> val;
        ValueType type;
};

template <typename T>
Value Value::create(ValueType type, const T& val){
    std::vector<std::byte> bytes;
    bytes.resize(sizeof(T));
    std::memcpy(&bytes[0], &val, sizeof(T));
    return std::move(Value(type, bytes));
}

// this creates a new dynamically allocated Value of a given type USE WITH CAUTION
template <typename T>
Value* Value::create_dyn(ValueType type, const T& val){
    std::vector<std::byte> bytes;
    bytes.resize(sizeof(T));
    std::memcpy(&bytes[0], &val, sizeof(T));
    return new Value(type, bytes);
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


#endif
