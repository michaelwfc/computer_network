
/**
 * An std::deque<T> (pronounced "deck") represents a double-ended queue of elements that supports efficient insertion/removal at both the front and back of the container.
 * 
 * std::deque has the exact same functions as std::vector but also has 
 *  push_front and pop_front.
 * 
 */
#include <deque>
#include "printUtils.h"

int int_queue_demo(){
    std::deque<int> deq {5,6};
    printDeque(deq);

    deq.push_front(3);
    printDeque(deq);

    deq.pop_back();
    printDeque(deq);

    deq[1] = -2;
    printDeque(deq);
    
    return 0;
}

int char_queue_demo(){ 
    std::deque<char> deq {'a','b'};
    printDeque(deq);

    deq.push_front('c');
    deq.push_back('d');
    printDeque(deq);

    // print the size of deque
    std::cout << "deq size " << deq.size() << std::endl;

    // get a string from the deque
    std::string s1 = std::string(deq.begin(), deq.begin() + 2);
    std::cout << "string get from deque " << s1 << std::endl;
    printDeque(deq);

    // pop out a string from the deque
    char s2= deq.front();
    deq.pop_front();
    std::cout << "char pop out from front of queue " << s2 << std::endl;
    printDeque(deq);

    


}

int main(int argc, char const *argv[])
{
    char_queue_demo();
    return 0;
}

