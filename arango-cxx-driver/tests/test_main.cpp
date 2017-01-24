#include "test_main.h"

using namespace std;

vector<string> arguments; // as it is not const this var has external linkage
                          // it is assigned a value in main

vector<string> parse_args(int const& argc, char const * const * argv){
  vector<string> rv;
  for(int i = 0; i < argc; i++){
    rv.emplace_back(argv[i]);
  }
  return rv;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); //removes google test parameters
    arguments = parse_args(argc,argv); // init global Schmutz
    return RUN_ALL_TESTS();
}
