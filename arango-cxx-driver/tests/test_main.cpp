#include "gtest/gtest.h"
#include "BasicTests.h"
#include <vector>
#include <string>

using namespace std;

extern vector<string> arguments;
vector<string> arguments{};

int main(int argc, char **argv) {
    //take args
    ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
}
