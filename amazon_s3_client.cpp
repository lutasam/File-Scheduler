#include "amazon_s3_client.h"

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

vector<FileMeta> AmazonS3Client::GetAllFileMeta()
{
    Aws::S3::Model::ListObjectsRequest request;
    request.WithBucket(BUCKET_NAME);

    auto outcome = s3Client->ListObjects(request);
    if (!outcome.IsSuccess())
    {
        cerr << "[LOG]: Error: GetAllFileMeta: " << outcome.GetError().GetMessage() << endl;
        return vector<FileMeta>();
    }
    vector<FileMeta> files;
    Aws::Vector<Aws::S3::Model::Object> objects = outcome.GetResult().GetContents();
    for (Aws::S3::Model::Object &object : objects)
    {
        files.push_back(FileMeta("--cloud---",
                                 object.GetOwner().GetDisplayName(),
                                 object.GetOwner().GetDisplayName(),
                                 object.GetSize(),
                                 object.GetLastModified().SecondsWithMSPrecision(),
                                 object.GetKey()));
    }
    return files;
}

int AmazonS3Client::UploadFile(string filepath)
{
    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(BUCKET_NAME);
    request.SetKey(filepath);

    std::shared_ptr<Aws::IOStream> inputData =
        Aws::MakeShared<Aws::FStream>(filepath.c_str(),
                                      (MOUNT_DIR + filepath).c_str(),
                                      std::ios_base::in | std::ios_base::binary);

    if (!*inputData)
    {
        cerr << "[LOG]: Error unable to read file at " << filepath << endl;
        return FAILURE;
    }

    request.SetBody(inputData);

    Aws::S3::Model::PutObjectOutcome outcome = s3Client->PutObject(request);

    if (!outcome.IsSuccess())
    {
        cerr << "[LOG]: Error: UploadFile: " << outcome.GetError().GetMessage() << endl;
    }
    cerr << "[LOG]: Add file at '" << filepath << "' successfully." << endl;

    return SUCCESS;
}

int AmazonS3Client::DownloadFile(string filepath)
{
    Aws::S3::Model::GetObjectRequest getObjectRequest;
    getObjectRequest.SetBucket(BUCKET_NAME);
    getObjectRequest.SetKey(filepath);

    auto outcome = s3Client->GetObject(getObjectRequest);

    if (!outcome.IsSuccess())
    {
        cerr << "[LOG]: Failed to download file: " << outcome.GetError().GetExceptionName() << " - " << outcome.GetError().GetMessage() << endl;
    }

    const Aws::S3::Model::GetObjectResult &result = outcome.GetResult();

    Aws::IOStream &fileStream = result.GetBody();

    ofstream outputFile(MOUNT_DIR + filepath, std::ios::binary);
    outputFile << fileStream.rdbuf();
    outputFile.close();

    cerr << "[LOG]: File at '" + filepath + "' downloaded successfully" << endl;

    return SUCCESS;
}

int AmazonS3Client::DeleteFile(string filepath)
{
    Aws::S3::Model::DeleteObjectRequest deleteObjectRequest;
    deleteObjectRequest.SetBucket(BUCKET_NAME);
    deleteObjectRequest.SetKey(filepath);

    auto outcome = s3Client->DeleteObject(deleteObjectRequest);

    if (!outcome.IsSuccess())
    {
        cerr << "[LOG]: Failed to delete file: " << outcome.GetError().GetExceptionName() << " - " << outcome.GetError().GetMessage() << endl;
        return FAILURE;
    }
    cerr << "[LOG]: File at '" + filepath + "' deleteed successfully" << endl;
    return SUCCESS;
}

FileMeta AmazonS3Client::GetOneFile(string filepath)
{
    Aws::S3::Model::ListObjectsV2Request listObjectsRequest;
    listObjectsRequest.WithBucket(BUCKET_NAME).WithPrefix(filepath).WithMaxKeys(1);

    auto outcome = s3Client->ListObjectsV2(listObjectsRequest);

    if (outcome.IsSuccess())
    {
        std::cout << "[LOG]: Error: " << outcome.GetError().GetMessage() << std::endl;
    }

    const Aws::Vector<Aws::S3::Model::Object> &objects = outcome.GetResult().GetContents();

    if (objects.empty())
    {
        std::cout << "[LOG]: Object not found." << std::endl;
    }

    const Aws::S3::Model::Object &object = objects.front();

    return FileMeta("--cloud---",
                    object.GetOwner().GetDisplayName(),
                    object.GetOwner().GetDisplayName(),
                    object.GetSize(),
                    object.GetLastModified().CurrentTimeMillis(),
                    object.GetKey());
}