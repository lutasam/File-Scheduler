#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <list>
#include "common.h"
#include "log.h"

using namespace std;

class DListNode
{
public:
    string filepath;
    FileMeta file;
    DListNode *pre, *next;
    DListNode(string _filepath = "", FileMeta _file = FileMeta()) : filepath(_filepath), file(_file), pre(nullptr), next(nullptr) {}
};

// this is the cache based on file size.
// please note that in this cache we don't require the file meta should be complete,
// but it must have these metas: name, relativePath, size, path

// class LRUCache
// {
// private:
//     size_t capacity;
//     size_t size;
//     unordered_map<string, DListNode *> cache;
//     DListNode *head, *tail;

//     void insert(DListNode * node)
//     {
//         cache[node->filepath] = node;
//         size += node->file.size;
//         tail->pre->next = node;
//         node->pre = tail->pre;
//         node->next = tail;
//         tail->pre = node;
//     }

//     FileMeta remove(DListNode * node)
//     {
//         FileMeta val = node->file;
//         size -= val.size;
//         cache.erase(node->filepath);
//         node->pre->next = node->next;
//         node->next->pre = node->pre;
//         node->pre = nullptr;
//         node->next = nullptr;
//         delete node;
//         return val;
//     }

//     void update(string key, FileMeta newVal)
//     {
//         remove(cache[key]);
//         DListNode *node = new DListNode(key, newVal);
//         insert(node);
//     }

//     FileMeta get(string key)
//     {
//         if (cache.find(key) == cache.end())
//         {
//             return FileMeta();
//         }
//         update(key, cache[key]->file);
//         return cache[key]->file;
//     }

//     vector<FileMeta> put(string key, FileMeta value)
//     {
//         vector<FileMeta> files;

//         if (value.size > capacity)
//         {
//             log_msg("File is too big to add in the file system, %d/%d\n", value.size, capacity);
//             return files;
//         }

//         if (cache.find(key) != cache.end())
//         {
//             update(key, value);
//             for (; size > capacity;)
//             {
//                 FileMeta file = remove(head->next);
//                 files.push_back(file);
//             }
//             return files;
//         }

//         DListNode *node = new DListNode(key, value);

//         for (; size + node->file.size > capacity;)
//         {
//             FileMeta file = remove(head->next);
//             files.push_back(file);
//         }

//         insert(node);
//         return files;
//     }

// public:
//     LRUCache(size_t _capacity = 1024) : capacity(_capacity), size(0), cache(), head(new DListNode()), tail(new DListNode())
//     {
//         if (_capacity <= 0)
//         {
//             cerr << "Cache Capacity should greater than 0." << endl;
//             exit(EXIT_FAILURE);
//         }
//         head->next = tail;
//         tail->pre = head;
//     }

//     ~LRUCache()
//     {
//         DListNode *curr = head;
//         for (; curr != nullptr;)
//         {
//             DListNode *temp = curr;
//             curr = curr->next;
//             delete temp;
//         }
//     }

//     size_t GetCapacity()
//     {
//         return capacity;
//     }

//     size_t GetSize()
//     {
//         return size;
//     }

//     bool IsFull()
//     {
//         return size == capacity;
//     }
//     // add a file to cache, if the cache is full, it will give you files poped from cache, otherwise it will give you an empty file vector
//     vector<FileMeta> AddFile(FileMeta newFile)
//     {
//         auto files = put(newFile.relativePath, newFile);
//         return files;
//     }

//     vector<FileMeta> SelectFilesForUpload()
//     {
//         vector<FileMeta> filesToUpload;
//         while (size > capacity)
//         {
//             FileMeta meta = remove(head->next);
//             filesToUpload.push_back(meta);
//         }
//         return filesToUpload;
//     }

//     void RemoveFile(string key)
//     {
//         if (cache.find(key) != cache.end())
//         {
//             FileMeta meta = remove(cache[key]);
//         }
//     }
// };

class FIFO
{
private:
    size_t capacity, size;
    DListNode *head, *tail;
    unordered_map<string, DListNode *> cache;

    // add to tail
    void insert(DListNode *node)
    {
        cache[node->filepath] = node;
        size += node->file.size;
        tail->pre->next = node;
        node->pre = tail->pre;
        node->next = tail;
        tail->pre = node;
    }

    // it will only remove the head node
    FileMeta removeHead()
    {
        auto curr = head->next;
        FileMeta val = curr->file;
        size -= val.size;
        cache.erase(curr->filepath);
        head->next = curr->next;
        curr->next->pre = head;
        delete curr;
        return val;
    }

    void update(string key, FileMeta newVal)
    {
        // removeHead();
        // DListNode *node = new DListNode(key, newVal);
        // insert(node);
        if (newVal.relativePath != key)
        {
            RemoveFile(key);
            DListNode *node = new DListNode(key, newVal);
            insert(node);
        }
        else
        {
            size += newVal.size - cache[key]->file.size;
            cache[key]->file = newVal;
        }
    }

    FileMeta get(string key)
    {
        if (cache.find(key) == cache.end())
        {
            return FileMeta();
        }
        // update(key, cache[key]->file);
        return cache[key]->file;
    }

    vector<FileMeta> put(string key, FileMeta value)
    {
        vector<FileMeta> files;
        // log_msg("[put] %s 1111", key.c_str());
        if (value.size > capacity)
        {
            // log_msg("File is too big to add in the file system, %d/%d\n", value.size, capacity);
            return files;
        }

        if (cache.find(key) != cache.end())
        {
            update(key, value);
            for (; size > capacity;)
            {
                FileMeta file = removeHead();
                files.push_back(file);
            }
            return files;
        }
        // log_msg("[put] 2222");
        DListNode *node = new DListNode(key, value);
        // log_msg("[put] 3333");
        for (; size + node->file.size > capacity;)
        {
            FileMeta file = removeHead();
            files.push_back(file);
        }
        // log_msg("[put] 4444");
        insert(node);
        // log_msg("[put] 5555");
        return files;
    }

public:
    FIFO(int _capacity) : capacity(_capacity), size(0), head(new DListNode()), tail(new DListNode())
    {
        head->next = tail;
        tail->pre = head;
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

    bool Contains(string key)
    {
        return cache.find(key) != cache.end();
    }

    // do not use this api directly if you change the size,
    // please use AddFile to update the file meta
    void Update(string key, FileMeta newVal)
    {
        update(key, newVal);
    }

    void RemoveFile(string key)
    {
        if (!Contains(key))
        {
            log_msg("file doesn't exist\n");
            return;
        }
        auto node = cache[key];
        cache.erase(node->filepath);
        size -= node->file.size;
        node->pre->next = node->next;
        node->next->pre = node->pre;
        delete node;
    }
};

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
            // log_msg("File is too big to add in the file system, %d/%d\n", value.size, capacity);
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

    bool Contains(string key)
    {
        return cache.find(key) != cache.end();
    }

    // do not use this api
    void Update(string key, FileMeta newVal)
    {
        update(key, newVal);
    }

    void RemoveFile(string key)
    {
        if (!Contains(key))
        {
            log_msg("file doesn't exist\n");
            return;
        }
        remove(cache[key]);
    }
};

class TwoQCache
{
private:
    unordered_map<string, FileMeta> cache;
    FIFO *fifo;
    LRUCache *lru;

    FileMeta get(string key)
    {
        if (fifo->Contains(key) || lru->Contains(key))
        {
            if (fifo->Contains(key))
            {
                // fifo->Update(key, cache[key]);
            }
            else
            {
                if (lru->Contains(key))
                {
                    lru->Update(key, cache[key]);
                }
            }
            return cache[key];
        }
        return FileMeta();
    }

    vector<FileMeta> put(string key, FileMeta value)
    {
        // log_msg("[CACHE LOG]: key=%s, size=%u\n", key.c_str(), value.size);
        vector<FileMeta> files;

        if (cache.find(key) != cache.end())
        {
            if (fifo->Contains(key))
            {
                fifo->RemoveFile(key);
            }
            files = lru->AddFile(value);
            for (auto file : files)
            {
                // log_msg("[CACHE LOG]: Upload file: %s\n", file.relativePath.c_str());
                cache.erase(file.relativePath);
            }
            return files;
        }
        // log_msg(" 1111 \n");
        cache[key] = value;
        files = fifo->AddFile(value);
        // log_msg(" 2222 \n");
        for (auto file : files)
        {
            files.push_back(file);
            cache.erase(file.relativePath);
            // log_msg("[CACHE LOG]: Upload file: %s", file.relativePath.c_str());
        }
        // log_msg(" 3333 \n");
        return files;
    }

public:
    TwoQCache(size_t cap) : fifo(new FIFO(cap)), lru(new LRUCache(cap)) {}

    ~TwoQCache()
    {
        delete fifo;
        delete lru;
    }

    size_t GetCapacity()
    {
        return fifo->GetCapacity() + lru->GetCapacity();
    }

    size_t GetSize()
    {
        return fifo->GetSize() + lru->GetSize();
    }

    bool IsFull()
    {
        return GetCapacity() == GetSize();
    }

    // use AddFile to add or update the filemeta to the cache
    // add a file to cache, if the cache is full, it will give you files poped from cache, otherwise it will give you an empty file vector
    vector<FileMeta> AddFile(FileMeta newFile)
    {
        auto files = put(newFile.relativePath, newFile);
        return files;
    }

    bool Contains(string key)
    {
        return fifo->Contains(key) || lru->Contains(key);
    }

    void RemoveFile(string key)
    {   
        this->cache.erase(key);
        if (fifo->Contains(key))
        {
            fifo->RemoveFile(key);
        }
        else if (lru->Contains(key))
        {
            lru->RemoveFile(key);
        }
        else
        {
            log_msg("No such file\n");
        }
    }
};