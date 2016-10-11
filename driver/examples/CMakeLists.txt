add_executable(FuerteBench
    FuerteBench/main.cpp
    FuerteBench/BucketReadTest.cpp
    FuerteBench/BucketReadTest.cpp
    FuerteBench/BucketTest.cpp
    FuerteBench/BucketVersionTest.cpp
    FuerteBench/BucketWriteTest.cpp
    FuerteBench/FuerteBench.cpp
)
target_link_libraries(FuerteBench fuerte gtest)
