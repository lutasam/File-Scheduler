#include <iostream>
#include <cstdlib>
// #include "common.h"

class LRUCache
{
private:
    size_t capacity;
    size_t size;

public:
    LRUCache(int _capacity = 10) : capacity(_capacity), size(0) {}
    size_t GetCapacity()
    {
        return capacity;
    }
    size_t GetSize()
    {
        return size;
    }
};