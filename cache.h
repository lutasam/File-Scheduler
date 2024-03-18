#include <iostream>
#include <cstdlib>
#include "common.h"

using namespace std;

// this is the cache based on file number, we should change it to file size.
class LRUCache
{
private:
    class DListNode
    {
    public:
        string filename;
        FileMeta file;
        DListNode *pre, *next;
        DListNode(string _filename = "", FileMeta _file = FileMeta()) : filename(_filename), file(_file), pre(nullptr), next(nullptr) {}
    };

    size_t capacity;
    size_t size;
    unordered_map<string, DListNode *> cache;
    DListNode *head, *tail;

    void insert(DListNode *node)
    {
        cache[node->filename] = node;
        size++;
        tail->pre->next = node;
        node->pre = tail;
        node->next = tail;
        tail->pre = node;
    }

    FileMeta remove(DListNode *node)
    {
        FileMeta val = node->file;
        cache.erase(node->filename);
        size--;
        node->pre->next = node->next;
        node->next->pre = node->pre;
        delete node;
        return val;
    }

    void update(string key, FileMeta newVal)
    {
        remove(cache[key]);
        DListNode *node = new DListNode(key, newVal);
        insert(node);
    }

    FileMeta get(string key)
    {
        if (cache.find(key) == cache.end())
        {
            return FileMeta();
        }
        update(key, cache[key]->file);
        return cache[key]->file;
    }

    FileMeta put(string key, FileMeta value)
    {
        if (cache.find(key) != cache.end())
        {
            update(key, value);
            return FileMeta();
        }

        DListNode *node = new DListNode(key, value);
        if (size < capacity)
        {
            insert(node);
            return FileMeta();
        }
        else
        {
            FileMeta val = remove(head->next);
            insert(node);
            return val;
        }
    }

public:
    LRUCache(size_t _capacity = 10) : capacity(_capacity), size(0), cache(), head(new DListNode()), tail(new DListNode())
    {
        if (_capacity <= 0)
        {
            cerr << "Cache Capacity should greater than 0." << endl;
            exit(EXIT_FAILURE);
        }
    }

    ~LRUCache()
    {
        delete head;
        delete tail;
    }

    size_t GetCapacity()
    {
        return capacity;
    }

    size_t GetSize()
    {
        return size;
    }

    bool IsFull()
    {
        return size == capacity;
    }
    // add a file to cache, if the cache is full, it will give you a file poped from cache, otherwise it will give you an empty file
    // you can check the file by file.isEmpty()
    FileMeta AddFile(FileMeta newFile)
    {
        FileMeta val = put(newFile.name, newFile);
        return val;
    }
};