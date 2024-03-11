#pragma once
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <iostream>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/ListObjectsResult.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/PutObjectResult.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/GetObjectResult.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectResult.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <aws/s3/model/ListObjectsV2Result.h>
#include <vector>

#include "common.h"

using namespace Aws;
using namespace Aws::Auth;
using namespace std;

/*
 *  A "Hello S3" starter application which initializes an Amazon Simple Storage Service (Amazon S3) client
 *  and lists the Amazon S3 buckets in the selected region.
 *
 *  main function
 *
 *  Usage: 'hello_s3'
 *
 */

class AmazonS3Client
{
private:
    Aws::SDKOptions *options;
    Aws::S3::S3Client *s3Client;

public:
    AmazonS3Client();
    ~AmazonS3Client();

    /**
     * file path should be the path from the mount folder to the current file
     * i.e., the program are mounted on /home/vcm/x, and the file you want to upload is /home/vcm/x/a/file1.txt, the filepath should be /a/file1.txt
     * p.s. the filepath should be compressed
    */

    // currently no dir support, it will just list all files in the bucket
    vector<FileMeta> GetAllFileMeta();
    // upload the file to the cloud, 0 for success
    int UploadFile(string filepath);
    // download the file and store it in local, 0 for success
    int DownloadFile(string filepath);
    // delete the file on cloud, 0 for success
    int DeleteFile(string filepath);
    // get one file info from cloud
    FileMeta GetOneFile(string filepath);
};