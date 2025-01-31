#ifndef VALUES_H
#define VALUES_H

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
        Value(ValueType type, const std::vector<std::byte>& val);
        Value(ValueType type);
        template <typename T>
        static Value create(ValueType type, const T& val);
        template <typename T>
        T as();
        template <typename T>
        void update(const T& new_val);
    private:
        std::vector<std::byte> val;
        ValueType type;
};

Value::Value(ValueType type, const std::vector<std::byte>& val){
    this->type = type;
    this->val = val;
}

template <typename T>
Value Value::create(ValueType type, const T& val){
    std::vector<std::byte> bytes;
    bytes.resize(sizeof(T));
    std::memcpy(&bytes[0], &val, sizeof(T));
    return Value(type, bytes);
}

template <typename T>
T Value::as(){
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
