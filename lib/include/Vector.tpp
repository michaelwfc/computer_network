/* Notice: there's no #include "Vector.h" here,
 * so your IDE (e.g. VSCode) might not know about
 * what a Vector<T> is. See the comment at the 
 * bottom of "Vector.h" for more information about
 * this. 
 */

#include <stdexcept>

template<typename T>
Vector<T>::Vector()
{
    _size = 0;
    _capacity = 1;
    _data = new T[_capacity];
}

template<typename T>
Vector<T>::~Vector()
{
    delete[] _data;
}

template<typename T>
void Vector<T>::resize()
{ 
    auto newData = new T[_capacity * 2];
    for (int i = 0; i < _size; i++)
    {
        newData[i] = _data[i];
    }
    delete[] _data;
    _capacity *=2;
    _data = newData;
}

/**
 * const T& value — Pass by const reference
 * 
 */
template<typename T>
void Vector<T>::push_back(const T& value)
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
template<typename T>
T& Vector<T>::at(size_t index)
{
    if (index >= _size)
    {
        throw std::out_of_range("Index out of range");
    }
    return _data[index];
}

template<typename T>
T& Vector<T>::operator[](size_t index)
{
    return _data[index];
}

template<typename T>
size_t Vector<T>::size() 
{
    return _size;
}

template<typename T>
bool Vector<T>::empty()
{
    return _size == 0;
}


/**
 * 在 C++ 模板中，使用嵌套类型 Vector<T>::iterator 是依赖于模板参数 T 的“依赖名称（dependent name）”，编译器不能确定它是一个类型还是变量或其他东西。
 * 所以必须使用 typename 关键字告诉它这是一个类型名。
 * 
 */
template<typename T>
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
template<typename T>
typename Vector<T>::iterator Vector<T>::end()
{
    return _data + _size; //is pointer arithmetic:
}