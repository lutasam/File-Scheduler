#pragma once
#include <iostream>
#include <cstring>
#include <fstream>
#include <memory>
#include <map>
#include <chrono>
#include <unordered_map>
#include <deque>
#include <vector>

using namespace std;

#ifndef COMMON_H
#define COMMON_H

#define SUCCESS 0
#define FAILURE -1

// global variables
const string MOUNT_DIR = "/home/vcm/ECE566/project/File-Scheduler/mount";
const string BUCKET_NAME = "ece566cloud2";
// const size_t NUM_THREAD = 2;
const long long BLOCK_SIZE = 2;

enum FileType
{
    NORMAL_FILE,
    DIRECTORY
};

class FileMeta
{
public:
    uint fileAccess;
    int fileType; // 0: normal file, 1: directory
    string owner;
    string group;
    uint size;
    uint atime;
    string name;
    string relativePath;
    string path; // it should be the absolute path, but I didn't use it currently.

    FileMeta() : fileAccess(0), fileType(FileType::NORMAL_FILE), owner(""), group(""), size(0), atime(0), name(""), relativePath(""), path("")
    {
    }
    FileMeta(uint _fileAccess, FileType _fileType, string _owner, string _group, uint _size,
             uint _atime, string _name, string _relativePath, string _path) : fileAccess(_fileAccess), fileType(_fileType), owner(_owner), group(_group), size(_size), atime(_atime),
                                                                              name(_name), relativePath(_relativePath), path(_path) {}
    bool IsEmpty()
    {
        return relativePath.empty();
    }
};

#endif // COMMON_H