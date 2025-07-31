/* This line makes sure that if another file #includes Vector.h
 * more than once, it will be as if it included it only once.
 *
 * Multiple includes can happen if, for example, we #include Vector.h
 * and also #include another file that itself #includes Vector.h
 *
 * #pragma once is technically not part of the C++ language, but is a
 * compiler extension that virtually all compilers support
 */
#pragma once

#include <cstddef>

template <typename T>
class Vector
{
public:
    /**
     * This is a type alias (same as typedef T* iterator;, but more modern).
     * It defines iterator as a shorthand for T* — a pointer to type T
     */
    using iterator = T *;

    Vector();

    // Add Copy constructor declaration
    Vector(const Vector& other);  

    ~Vector();

    void push_back(const T &value);

    T &at(size_t index);
    T &operator[](size_t index);

    // operator overload Assignment operator
    Vector &operator=(const Vector &other);

    size_t size();
    bool empty();

    iterator begin();
    iterator end();

private:
    size_t _size;
    size_t _capacity;
    T *_data;

    void resize();
};

/**
 * Use .tpp file for template definitions: 
 * rename Vector.cpp → Vector.tpp, and at the end of Vector.h add:
 * This is commonly used in the STL and Boost to separate template definitions into a dedicated header-like implementation file.
 * 
 */
#include "Vector.tpp"


/* Notice that the .h file for template #includes the .cpp file!
 * This is because, when instantiating templates, the compiler
 * must have full knowledge of both the declaration (this file)
 * and the definitions (.cpp file) of the template.
 *
 * Another way around this is to implement the template entirely
 * inside on file. Typically, these files are given the extension
 * .hpp, but this is arbitrary.
 *
 * If you want to learn more about why this is, please check out
 * this C++ blog (also check out other pages in this, as it's a
 * great reference for learning about C++):
 *
 * https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl
 */
//Include .cpp file inside .h	Technically works but bad form	❌ Not recommended
// #include "Vector.cpp"


