#include "test_main.h"
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

using namespace std;

vector<string> arguments;  // as it is not const this var has external linkage
                           // it is assigned a value in main

vector<string> parse_args(int const& argc, char const* const* argv) {
  vector<string> rv;
  for (int i = 0; i < argc; i++) {
    rv.emplace_back(argv[i]);
  }
  return rv;
}

void handler(int sig) {
  void* array[20];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 20);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char** argv) {
  while (true) {
    //signal(SIGSEGV, handler);                // install our handler
    ::testing::InitGoogleTest(&argc, argv);  // removes google test parameters
    arguments = parse_args(argc, argv);      // init global Schmutz
    auto rc = RUN_ALL_TESTS();
    if (rc) {
      // Test failed
      return rc;
    }
    if (getenv("CONTINUOUS_TEST") == nullptr) {
      // No need for continuous testing, return.
      return 0;
    }
  }
}
