#pragma once
#include <iostream>
#include <cstring>
#include <fstream>
#include <memory>
#include <map>
#include <chrono>

#define SUCCESS 0
#define FAILURE -1

using namespace std;

#ifndef COMMON_H
#define COMMON_H
// global variables
const string MOUNT_DIR = "/home/vcm/bbfs/mount";
const string BUCKET_NAME = "ece566cloud";

class FileMeta
{
public:
    string fileAccess;
    string owner;
    string group;
    uint size;
    uint atime;
    string name;

    FileMeta() : fileAccess("drwxrwxrwx"), owner(""), group(""), size(0), atime(0), name("") {}
    FileMeta(string _fileAccess, string _owner, string _group, uint _size, uint _atime, string _name) : fileAccess(_fileAccess), owner(_owner), group(_group), size(_size), atime(_atime), name(_name) {}
};

#endif // COMMON_H