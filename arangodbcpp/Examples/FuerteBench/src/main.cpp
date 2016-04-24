////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#include "FuerteBench.h"

/*
  I'll supply a list of ID as file "ids.csv" on ID per line, example

  P1
  P2
  P12437

  The program should be started as

  single-read -f ids.csv -p n -l m -c collection -d database

  where ids.csv is a file like above, "n" is a number "the concurrency", "m" is
  a
  number "the loop", "collection" is the name of the collection and database is
  the name of the database.

  The program should

  - read in the ID
  - split the IDs into "n" equally sized buckets (almost the last one might be
  smaller)
  - start n threads
  - each thread should connection to ArangoDB and read the documents from its
  bucket from the collection named "collection" (it is an error if the
  collection
  does not exist and the program should exit)
  - each thread should repeat the read process "m" times
  - if all threads have finished, the program should report
  -- the number of successful reads
  -- the number of unsuccessful reads
  -- the total wallclock time spend
  -- the number of requests per seconds in total
  -- the number of requests per seconds for each thread
  -- the average time per request
  -- the minimum time per request
  -- the maximum time per request

  FUTURE

  FuerteBench should take a connection string, something like
  http+tcp://127.0.0.1:8529
  or similar
*/

int main(int argc, const char* argv[]) {
  FuerteBench app{argc, argv};
  if (app.start()) {
    app.outputReport();
  }
  return 0;
}
