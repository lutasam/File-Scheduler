This kit is a simplified version of bbfs by Joseph J. Pfeiffer, Jr., Ph.D.

The only difference is the automake build system has been replaced by a simple Makefile with no dependencies other
than pkg-config. The reliance on an autogenerated config.h has been removed. The idea is to make at easier base 
for students to start their own FUSE projects.

It was prepared by Tyler Bletsch at Duke University, 2020-07-30. It is based on the original bbfs dated 2018-02-04.

Before running this program, make sure you create the mountdir.

# Step 1 Install Amazon C++ SDK and other environment
compile the source code and install it. Follow this link: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/setup-linux.html

other environment, e.g., fuse, g++...

# Step 2 Authorization
put auth file into ~/.aws. Follow this link: https://docs.aws.amazon.com/sdkref/latest/guide/access-iam-users.html

# Step 3 Make
```
cd scheduler_exec
cmake
make
./scheduler ../rootdir ../mount 10
```

# Revise Suggestion

1. number of file -> total size of file

2. syn problem write rootdir locking

3. close or time limit x nerver commit changes
maybe just give a warning

4. specific circum

5. cloud request and response time /  o/c time

6. benchmarks simulate i/o pattern

7. lfu freq problem further access
non-cpu cache algorithm

# Test Requirement

## Performance Test

Four test subject: bbfs(FUSE), version1 scheduler, version2 scheduler, cloud(basic cloud file system based on FUSE & Amazon S3)

If the number of file is too high, the cost is **EXPENSIVE**! So just test 100 files.

In each case, you should do three tests: rtest(all read), wtest(all write), rwtest(x% read and y% write, you can decide it)

1. 100 files, 1KB file size, change the cache size: 4K, 8K, 16K, 32K

2. 100 files, 16K cache size, change the file size: 256B, 512B, 1K, 2K

## Overhead Test

Get the overhead of our program.

version2 scheduler: single thread overhead and multiple threads overhead

It should be very high, and this should be considered as a drawback of our program