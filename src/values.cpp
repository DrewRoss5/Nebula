#include "../inc/values.hpp"

Value::Value(ValueType type, const std::vector<std::byte>& val, bool is_arr){
    this->type = type;
    this->val = val;
    if (is_arr){
        this->is_arr = true;
        this->arr = new NebulaArray(type);
    }
}

// creates a dynamically allocated pointer to an unitialized value
Value* Value::create_dyn(ValueType type, bool is_arr){
    return new Value(type, is_arr);
}

// deletes the memory associated with the value if it's an array
Value::~Value(){
    if (this->is_arr)
        delete arr;

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

// displays the value's unwrapped form
std::ostream& operator<<(std::ostream& out, const Value& val){
    switch (val.type){
        case INT:
            out << val.as<int>();
            break;
        case FLOAT:
            out << val.as<int>();
            break;
        case CHAR:
            out << val.as<char>();
            break;
        case BOOL:
            out << (val.as<bool>()) ? "true" : "false";
            break;
        case NULL_TYPE:
            out << "null";
            break;
    }
    return out;
}

// Nebula Array functions
// returns the refference to the array from the value, raises an error if the value is not an array
NebulaArray& Value::as_arr(){
    if (this->arr)
        throw std::runtime_error("cannot access array methods for a non-array value");
    return *this->arr;
}

// constructs a new array for 32 values
NebulaArray::NebulaArray(ValueType val_type){
    this->arr_ptr = new Value[32];
    this->val_type = val_type;
}

NebulaArray::~NebulaArray(){
    delete[] this->arr_ptr;
}

//this doubles the capacity of the array
void NebulaArray::realloc(){
    // create the new array
    Value* new_arr = new Value[this->capacity * 2];
    for (size_t i = 0; i < this->size; i++)
        new_arr[i] = arr_ptr[i];
    // clean up and update member variables
    delete[] this->arr_ptr;
    this->arr_ptr = new_arr;
    this->capacity *= 2;
}

// returns a reference to the value at the given index, or throws a std::runtime_error if the index is out of range
Value& NebulaArray::get(int index){
    if (index > this->size)
        throw std::runtime_error("cannot access element out range");
    // check if we are accessing the end o the array, and resize if needed
    else if (index == this->size){
        if (this->size == this->capacity)
            this->realloc();
        this->size++;
    }
    return this->arr_ptr[index];
}
