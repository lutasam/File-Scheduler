#pragma once
#include <iostream>
#include <cstring>
#include <fstream>
#include <memory>
#include <map>
#include <chrono>
#include <unordered_map>
#include <deque>

using namespace std;

#ifndef COMMON_H
#define COMMON_H

#define SUCCESS 0
#define FAILURE -1

// global variables
const string MOUNT_DIR = "/home/vcm/bbfs/mount";
const string BUCKET_NAME = "ece566cloud";

class FileMeta
{
public:
    uint fileAccess;
    string owner;
    string group;
    uint size;
    uint atime;
    string name;
    string relativePath;
    string path; // it should be the absolute path, but I didn't use it currently.

    FileMeta() : fileAccess(0), owner(""), group(""), size(0), atime(0), name(""), relativePath(""), path("")
    {
    }
    FileMeta(uint _fileAccess, string _owner, string _group, uint _size,
             uint _atime, string _name, string _relativePath, string _path) : fileAccess(_fileAccess), owner(_owner), group(_group), size(_size), atime(_atime),
                                                                              name(_name), relativePath(_relativePath), path(_path) {}
    bool IsEmpty()
    {
        return name.empty();
    }
};

#endif // COMMON_H