
/**
 * An std::deque<T> (pronounced "deck") represents a double-ended queue of elements that supports efficient insertion/removal at both the front and back of the container.
 * 
 * std::deque has the exact same functions as std::vector but also has 
 *  push_front and pop_front.
 * 
 */
#include <deque>
#include "printUtils.h"

int main(){
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