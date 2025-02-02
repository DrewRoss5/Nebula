#include "../inc/values.hpp"

Value::Value(ValueType type, const std::vector<std::byte>& val){
    this->type = type;
    this->val = val;
}

// creates a dynamically allocated pointer to an unitialized value
Value* Value::create_dyn(ValueType type){
    return new Value(type);
}

// compares two Values, will only return true if they are of the same type and value
bool Value::operator==(const Value& rhs) const{
    if (this->type != rhs.type)
        return false;
    switch (this->type){
        case INT:
            return this->as<int>() == rhs.as<int>();
            break;
        case FLOAT:
            return this->as<double>() == rhs.as<double>();
            break;
        case CHAR:
            return this->val[0] == rhs.val[0];
            break;
        case BOOL:
            return this->as<bool>() == rhs.as<bool>();
            break;
     }
     return false;
}