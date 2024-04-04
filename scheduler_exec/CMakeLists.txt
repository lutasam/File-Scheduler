cmake_minimum_required(VERSION 3.20)
project(scheduler)

set(SOURCES ../scheduler.cpp ../log.cpp ../amazon_s3_client.cpp)

# Set the AWS service components used by this project.
set(SERVICE_COMPONENTS s3)

# Set the C++ standard to use to build this target.
# At least C++ 11 is required for the AWS SDK for C++.
set(CMAKE_CXX_STANDARD 11)

find_path(FUSE_INCLUDE_DIRS fuse.h)
find_library(FUSE_LIBRARIES fuse)

# Find the AWS SDK for C++ package.
find_package(AWSSDK REQUIRED COMPONENTS ${SERVICE_COMPONENTS})

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${FUSE_LIBRARIES} ${AWSSDK_LINK_LIBRARIES})
# target_link_libraries(${PROJECT_NAME} ${AWSSDK_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${FUSE_INCLUDE_DIRS})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -D_FILE_OFFSET_BITS=64)
