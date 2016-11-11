add_executable(minimal_example minimal_example.cpp)
target_link_libraries(minimal_example fuerte)

### old
add_executable(minimal_example_old minimal_example_old.cpp)
target_link_libraries(minimal_example_old fuerte-old)

add_executable(FuerteBench
    FuerteBench/main.cpp
    FuerteBench/BucketReadTest.cpp
    FuerteBench/BucketReadTest.cpp
    FuerteBench/BucketTest.cpp
    FuerteBench/BucketVersionTest.cpp
    FuerteBench/BucketWriteTest.cpp
    FuerteBench/FuerteBench.cpp
)
target_link_libraries(FuerteBench fuerte-old gtest)
