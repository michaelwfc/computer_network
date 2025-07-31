/* Notice: there's no #include "Vector.h" here,
 * so your IDE (e.g. VSCode) might not know about
 * what a Vector<T> is. See the comment at the
 * bottom of "Vector.h" for more information about
 * this.
 */

#include <stdexcept>

/**
 * When we create a constructor, we need to initialize all of our member variables.
 */
// template<typename T>
// Vector<T>::Vector()
// {
//     _size = 0;
//     _capacity = 1;
//     _data = new T[_capacity];
// }

/**
 * Member initialization Lists
 * Member initialization lists are a C++ feature used in constructors to initialize member variables directly during object construction.
 * This is more efficient than assigning values in the constructor body, especially for complex objects.
 *
 * We can use initializer lists to declare and initialize them with desired values at once!
 * It’s quicker and more efficient to directly construct member variables with intended values
 *
 * Default constructor using member initialization list
 */
template <typename T>
Vector<T>::Vector() : _size(0), _capacity(4), _data(new T[_capacity]) {}


/** Copy constructor: T(const T&)
 * If your variable is a pointer, a memberwise copy will point to the same allocated data, not a fresh copy!
 * These pointers will point at the same underlying array!
 * This is problematic because anything done to one pointer affects the other
 * 
● Many times, you will want to create a copy that does more than just copies the member variables.
● Deep copy: an object that is a complete, independent copy of the original 
● In these cases, you’d want to override the default special member functions with your own implementation!
● Declare them in the header and write their implementation in the .cpp, like any function!
 */
// template <typename T>
// Vector<T>::Vector<T>(const Vector::Vector<T> &other) : _size(other._size), _capacity(other._capacity),  _data(other._data){}

template <typename T>
Vector<T>::Vector(const Vector &other) : _size(other._size), _capacity(other._capacity),  _data(new T[_capacity])
{
    // Now we have a “deep” copy of the data in our Vector.
    for (size_t i = 0; i < _size; i++)
    {
        _data[i] = other._data[i];
    }
}





template <typename T>
Vector<T>::~Vector()
{
    delete[] _data;
}

template <typename T>
void Vector<T>::resize()
{
    auto newData = new T[_capacity * 2];
    for (int i = 0; i < _size; i++)
    {
        newData[i] = _data[i];
    }
    delete[] _data;
    _capacity *= 2;
    _data = newData;
}

/**
 * const T& value — Pass by const reference
 *
 */
template <typename T>
void Vector<T>::push_back(const T &value)
{
    if (_size == _capacity)
    {
        resize();
    }
    _data[_size] = value;
    _size++;
}

/**
 * T&: The function returns a reference to an object of type T
 *
 */
template <typename T>
T& Vector<T>::at(size_t index)
{
    // Dereferencing a pointer without verifying it points to real memory is undefined behavior!
    if (index >= _size)
    {
        throw std::out_of_range("Index out of range");
    }
    return _data[index];
}

template <typename T>
T &Vector<T>::operator[](size_t index)
{
    return _data[index];
}

/**
 * assignment operator overload
 * Deep copy: Both copy constructor and assignment operator must create a deep copy of the data
 * 
 */

template <typename T>
Vector<T> &Vector<T>::operator=(const Vector<T> &other)
{
    if(this != &other) {
        delete[] _data; // Free existing memory

        _size = other._size;
        _capacity = other._capacity;
        _data = new T[_capacity];
        for(size_t i = 0; i < _size; i++) {
            _data[i] = other._data[i];
        }
    }
}


template <typename T>
size_t Vector<T>::size()
{
    return _size;
}

template <typename T>
bool Vector<T>::empty()
{
    return _size == 0;
}

/**
 * 在 C++ 模板中，使用嵌套类型 Vector<T>::iterator 是依赖于模板参数 T 的“依赖名称（dependent name）”，编译器不能确定它是一个类型还是变量或其他东西。
 * 所以必须使用 typename 关键字告诉它这是一个类型名。
 *
 */
template <typename T>
typename Vector<T>::iterator Vector<T>::begin()
{
    return _data;
}

/**
 * The end() function returns an iterator pointing one past the last valid element in the container. This is a standard convention in C++ used for iteration.
 * _data is a pointer to a dynamically allocated array (T*)
 * _size is the current number of elements in the container.
 * iterator is defined as using iterator = T*;
 *
 *
 */
template <typename T>
typename Vector<T>::iterator Vector<T>::end()
{
    return _data + _size; // is pointer arithmetic:
}