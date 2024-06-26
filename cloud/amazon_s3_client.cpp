#include "amazon_s3_client.h"
#include <sys/stat.h>

string getFileName(string path)
{
    if (path[path.size() - 1] == '/')
    {
        path = path.substr(0, path.size() - 1);
    }
    size_t idx = path.rfind('/');
    return path.substr(idx + 1);
}

string getFilePath(string path)
{
    if (path[path.size() - 1] == '/')
    {
        return path;
    }
    size_t idx = path.rfind('/');
    return path.substr(0, idx + 1);
}

AmazonS3Client::AmazonS3Client()
{
    // Optionally change the log level for debugging.
    options = new Aws::SDKOptions();
    cerr << "[LOG]: Init Client, please wait..." << endl;
    options->loggingOptions.logLevel = Utils::Logging::LogLevel::Debug;
    Aws::InitAPI(*options); // Should only be called once.

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = "us-east-1";

    // You don't normally have to test that you are authenticated. But the S3 service permits anonymous requests, thus the s3Client will return "success" and 0 buckets even if you are unauthenticated, which can be confusing to a new user.
    auto provider = Aws::MakeShared<DefaultAWSCredentialsProviderChain>("alloc-tag");
    auto creds = provider->GetAWSCredentials();
    if (creds.IsEmpty())
    {
        cerr << "Failed authentication, check your secret key" << endl;
        exit(EXIT_FAILURE);
    }

    cerr << "[LOG]: Successfully connected to Cloud" << endl;
    s3Client = new Aws::S3::S3Client(clientConfig);
}

AmazonS3Client::~AmazonS3Client()
{
    Aws::ShutdownAPI(*options);
    delete options;
    delete s3Client;
}

vector<FileMeta> AmazonS3Client::GetAllFileMeta(string path)
{
    if (path[path.size() - 1] != '/')
    {
        path += '/';
    }
    Aws::S3::Model::ListObjectsRequest request;
    request.WithBucket(BUCKET_NAME);

    auto outcome = s3Client->ListObjects(request);
    if (!outcome.IsSuccess())
    {
        // cerr << "[LOG]: Error: GetAllFileMeta: " << outcome.GetError().GetMessage() << endl;
        log_msg(("[LOG]: Error: GetAllFileMeta: " + outcome.GetError().GetMessage() + "\n").c_str());
        return vector<FileMeta>();
    }
    vector<FileMeta> files;
    Aws::Vector<Aws::S3::Model::Object> objects = outcome.GetResult().GetContents();
    set<string> dirNames;
    for (Aws::S3::Model::Object &object : objects)
    {
        string currPath = "/" + object.GetKey();
        log_msg(("[LOG]: " + currPath + "\n").c_str());
        if (currPath.substr(0, path.size()) != path)
        {
            continue;
        }
        else
        {
            string afterPath = currPath.substr(path.size());
            // log_msg(("[LOG]: afterpath: " + afterPath + "\n").c_str());
            // file
            if (afterPath.find('/') == string::npos)
            {
                files.push_back(FileMeta((S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO),
                                         FileType::DIRECTORY,
                                         object.GetOwner().GetDisplayName(),
                                         object.GetOwner().GetDisplayName(),
                                         object.GetSize(),
                                         object.GetLastModified().SecondsWithMSPrecision(),
                                         getFileName(currPath),
                                         currPath,
                                         ""));
            }
            else // dir
            {
                string dirName = afterPath.substr(0, afterPath.find('/'));
                // log_msg(("[LOG]: dirname: " + dirName + "\n").c_str());
                if (dirNames.find(dirName) != dirNames.end())
                {
                    continue;
                }
                dirNames.insert(dirName);
                files.push_back(FileMeta((S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO),
                                         FileType::NORMAL_FILE,
                                         object.GetOwner().GetDisplayName(),
                                         object.GetOwner().GetDisplayName(),
                                         object.GetSize(),
                                         object.GetLastModified().SecondsWithMSPrecision(),
                                         dirName,
                                         path + dirName,
                                         ""));
            }
        }
    }
    return files;
}

int AmazonS3Client::UploadFile(string filename, string path, string fpath)
{
    if (path[path.size() - 1] != '/')
    {
        path += '/';
    }
    string filepath = path + filename;
    cout << filepath << endl;

    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(BUCKET_NAME);
    request.SetKey(filepath);

    std::shared_ptr<Aws::IOStream> inputData =
        Aws::MakeShared<Aws::FStream>(filepath.c_str(),
                                      fpath.c_str(),
                                      std::ios_base::in | std::ios_base::binary);

    if (!*inputData)
    {
        // cerr << "[LOG]: Error unable to read file at " << filepath << endl;
        log_msg(("[LOG]: Error unable to read file at " + filepath + "\n").c_str());
        return FAILURE;
    }

    request.SetBody(inputData);

    Aws::S3::Model::PutObjectOutcome outcome = s3Client->PutObject(request);

    if (!outcome.IsSuccess())
    {
        // cerr << "[LOG]: Error: UploadFile: " << outcome.GetError().GetMessage() << endl;
        log_msg(("[LOG]: Error: UploadFile: " + outcome.GetError().GetMessage() + "\n").c_str());
        return FAILURE;
    }
    // cerr << "[LOG]: Add file at '" << filepath << "' successfully." << endl;
    log_msg(("[LOG]: Add file at '" + filepath + "' successfully." + "\n").c_str());

    return SUCCESS;
}

// int AmazonS3Client::UploadAllFile(const vector<FileMeta> &files)
// {
//     vector<std::future<int>> futures;
//     for (auto file : files)
//     {
//         futures.push_back(async(launch::async, &AmazonS3Client::UploadFile, this, file.name, getFilePath(file.relativePath), file.path));
//     }

//     for (auto &future : futures)
//     {
//         int status = future.get();
//         if (status != SUCCESS)
//         {
//             return FAILURE;
//         }
//     }
//     return SUCCESS;
// }

int AmazonS3Client::UploadAllFile(const vector<FileMeta> &files)
{
    std::vector<std::thread> threads;
    for (auto file : files)
    {
        log_msg("[thread] %s, %s, %s\n", file.name.c_str(), getFilePath(file.relativePath).c_str(), file.path.c_str());
        threads.emplace_back(&AmazonS3Client::UploadFile, this, file.name, getFilePath(file.relativePath), file.path);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
    return SUCCESS;
}

int AmazonS3Client::DownloadFile(string filename, string path, string fpath)
{
    if (path[path.size() - 1] != '/')
    {
        path += '/';
    }
    string filepath = path + filename;

    Aws::S3::Model::GetObjectRequest getObjectRequest;
    getObjectRequest.SetBucket(BUCKET_NAME);
    getObjectRequest.SetKey(filepath);

    auto outcome = s3Client->GetObject(getObjectRequest);

    if (!outcome.IsSuccess())
    {
        // cerr << "[LOG]: Failed to download file: " << outcome.GetError().GetExceptionName() << " - " << outcome.GetError().GetMessage() << endl;
        log_msg(("[LOG]: Failed to download file: " + outcome.GetError().GetExceptionName() + " - " + outcome.GetError().GetMessage() + "\n").c_str());
        return FAILURE;
    }

    const Aws::S3::Model::GetObjectResult &result = outcome.GetResult();

    Aws::IOStream &fileStream = result.GetBody();

    ofstream outputFile(fpath, std::ios::binary);
    outputFile << fileStream.rdbuf();
    outputFile.close();

    // cerr << "[LOG]: File at '" << filepath << "' downloaded successfully" << endl;
    log_msg(("[LOG]: File at '" + filepath + "' downloaded successfully" + "\n").c_str());

    std::ifstream in(fpath, std::ifstream::ate | std::ifstream::binary);
    int size = in.tellg();
    in.close();

    return size;
}

int AmazonS3Client::DownloadFileByMultiThreads(string filename, string path, string fpath)
{
    if (path[path.size() - 1] != '/')
    {
        path += '/';
    }
    std::string filepath = path + filename;

    long long fileSize = GetFileSize(filename, path);

    // download file with threads
    // long long blockSize = fileSize / NUM_THREAD;
    int threadNum = fileSize == 0 ? 1 : (fileSize - 1) / BLOCK_SIZE + 1;

    outFile = std::ofstream(fpath, std::ofstream::binary | std::ofstream::ate);

    std::vector<std::thread> threads;
    std::vector<int> blockSizes(threadNum);
    for (int i = 0; i < threadNum; ++i)
    {
        long long startByte = i * BLOCK_SIZE;
        long long endByte = (i == threadNum - 1) ? fileSize - 1 : (i + 1) * BLOCK_SIZE - 1;
        log_msg("[LOG]: startByte: %ld, endByte: %ld\n", startByte, endByte);
        threads.emplace_back(&AmazonS3Client::DownloadBlock, this, filepath, fpath, startByte, endByte, std::ref(blockSizes[i]));
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
    

    int totalSize = 0;
    for (int size : blockSizes)
    {
        if (size == -1)
        {
            return FAILURE;
        }
        totalSize += size;
        log_msg("[LOG]: size: %d\n", size);
    }

    outFile.close();

    return totalSize;
}

int AmazonS3Client::DeleteFile(string filename, string path)
{
    if (path[path.size() - 1] != '/')
    {
        path += '/';
    }
    string filepath = path + filename;

    Aws::S3::Model::DeleteObjectRequest deleteObjectRequest;
    deleteObjectRequest.SetBucket(BUCKET_NAME);
    deleteObjectRequest.SetKey(filepath);

    auto outcome = s3Client->DeleteObject(deleteObjectRequest);

    if (!outcome.IsSuccess())
    {
        // cerr << "[LOG]: Failed to delete file: " << outcome.GetError().GetExceptionName() << " - " << outcome.GetError().GetMessage() << endl;
        log_msg(("[LOG]: Failed to delete file: " + outcome.GetError().GetExceptionName() + " - " + outcome.GetError().GetMessage() + "\n").c_str());
        return FAILURE;
    }
    // cerr << "[LOG]: File at '" << filepath << "' deleteed successfully" << endl;
    log_msg(("[LOG]: File at '" + filepath + "' deleteed successfully\n").c_str());
    return SUCCESS;
}

FileMeta AmazonS3Client::GetOneFile(string filename, string path)
{
    if (path[path.size() - 1] != '/')
    {
        path += '/';
    }
    string filepath = path + filename;

    Aws::S3::Model::ListObjectsV2Request listObjectsRequest;
    listObjectsRequest.WithBucket(BUCKET_NAME).WithPrefix(filepath).WithMaxKeys(1);

    auto outcome = s3Client->ListObjectsV2(listObjectsRequest);

    if (outcome.IsSuccess())
    {
        // cerr << "[LOG]: Error: " << outcome.GetError().GetMessage() << endl;
        log_msg(("[LOG]: Error: " + outcome.GetError().GetMessage() + "\n").c_str());
    }

    const Aws::Vector<Aws::S3::Model::Object> &objects = outcome.GetResult().GetContents();

    if (objects.empty())
    {
        // cerr << "[LOG]: Object not found." << endl;
        log_msg("[LOG]: Object not found.\n");
    }

    const Aws::S3::Model::Object &object = objects.front();

    return FileMeta((S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO),
                    FileType::NORMAL_FILE,
                    object.GetOwner().GetDisplayName(),
                    object.GetOwner().GetDisplayName(),
                    object.GetSize(),
                    object.GetLastModified().CurrentTimeMillis(),
                    filename,
                    filepath,
                    "");
}

long long AmazonS3Client::GetFileSize(string filename, string path)
{
    if (path[path.size() - 1] != '/')
    {
        path += '/';
    }
    string filepath = path + filename;

    Aws::S3::Model::HeadObjectRequest headObjectRequest;
    headObjectRequest.SetBucket(BUCKET_NAME);
    headObjectRequest.SetKey(filepath.c_str());

    auto headObjectOutcome = s3Client->HeadObject(headObjectRequest);
    if (!headObjectOutcome.IsSuccess())
    {
        log_msg(("Failed to get file size: " + headObjectOutcome.GetError().GetMessage() + "\n").c_str());
        exit(EXIT_FAILURE);
    }

    long long fileSize = headObjectOutcome.GetResult().GetContentLength();

    return fileSize;
}