#include <iostream>
#include <cstdlib>
#include "common.h"
#include "log.h"

using namespace std;

// this is the cache based on file size.
class LRUCache
{
private:
    class DListNode
    {
    public:
        string filepath;
        FileMeta file;
        DListNode *pre, *next;
        DListNode(string _filepath = "", FileMeta _file = FileMeta()) : filepath(_filepath), file(_file), pre(nullptr), next(nullptr) {}
    };

    size_t capacity;
    size_t size;
    unordered_map<string, DListNode *> cache;
    DListNode *head, *tail;

    void insert(DListNode *node)
    {
        cache[node->filepath] = node;
        size += node->file.size;
        tail->pre->next = node;
        node->pre = tail->pre;
        node->next = tail;
        tail->pre = node;
    }

    FileMeta remove(DListNode *node)
    {
        FileMeta val = node->file;
        size -= val.size;
        cache.erase(node->filepath);
        node->pre->next = node->next;
        node->next->pre = node->pre;
        node->pre = nullptr;
        node->next = nullptr;
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

    vector<FileMeta> put(string key, FileMeta value)
    {
        vector<FileMeta> files;

        if (value.size > capacity)
        {
            log_msg("File is too big to add in the file system, %d/%d\n", value.size, capacity);
            return files;
        }

        if (cache.find(key) != cache.end())
        {
            update(key, value);
            for (; size > capacity;)
            {
                FileMeta file = remove(head->next);
                files.push_back(file);
            }
            return files;
        }

        DListNode *node = new DListNode(key, value);

        for (; size + node->file.size > capacity;)
        {
            FileMeta file = remove(head->next);
            files.push_back(file);
        }

        insert(node);
        return files;
    }

public:
    LRUCache(size_t _capacity = 1024) : capacity(_capacity), size(0), cache(), head(new DListNode()), tail(new DListNode())
    {
        if (_capacity <= 0)
        {
            cerr << "Cache Capacity should greater than 0." << endl;
            exit(EXIT_FAILURE);
        }
        head->next = tail;
        tail->pre = head;
    }

    ~LRUCache()
    {
        DListNode *curr = head;
        for (; curr != nullptr;)
        {
            DListNode *temp = curr;
            curr = curr->next;
            delete temp;
        }
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
    // add a file to cache, if the cache is full, it will give you files poped from cache, otherwise it will give you an empty file vector
    vector<FileMeta> AddFile(FileMeta newFile)
    {
        auto files = put(newFile.relativePath, newFile);
        return files;
    }

    vector<FileMeta> SelectFilesForUpload() {
        vector<FileMeta> filesToUpload;
        while(size > capacity) {
            FileMeta meta = remove(head->next);
            filesToUpload.push_back(meta);
        }
        return filesToUpload;
    }

    void RemoveFile(string key) {
        if (cache.find(key) != cache.end()) {
            FileMeta meta = remove(cache[key]);
        }
    }
};