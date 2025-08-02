#include <iostream>
#include <string>

#include <iostream>

class MyNumber
{
public:
    explicit MyNumber(int x)
    { // Constructor is now explicit
        value = x;
    }

    void show() const
    {
        std::cout << "Value: " << value << std::endl;
    }

private:
    int value;
};

int main()
{
    // MyNumber num = 5;      // ❌ Error: implicit conversion not allowed
    MyNumber num{5}; // ✅ OK: explicit construction
    num.show();      // Output: Value: 5
    return 0;
}
