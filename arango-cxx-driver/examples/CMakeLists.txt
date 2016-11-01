add_executable(minimal_example minimal_example.cpp)
target_link_libraries(minimal_example fuerte)

add_executable(minimal_example_next minimal_example_next.cpp)
target_link_libraries(minimal_example_next fuerte-next)

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
