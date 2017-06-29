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
/// @author Frank Celler
/// @author Jan Christoph Uhde
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#define ENABLE_FUERTE_LOG_ERROR 1
#define ENABLE_FUERTE_LOG_DEBUG 1
#define ENABLE_FUERTE_LOG_TRACE 1

#include <fuerte/connection.h>
#include <iostream>
#include <fuerte/message.h>
#include <fuerte/loop.h>
#include <fuerte/requests.h>
#include <fuerte/helper.h>

using ConnectionBuilder = arangodb::fuerte::ConnectionBuilder;
using EventLoopService = arangodb::fuerte::EventLoopService;
using Request = arangodb::fuerte::Request;
using MessageHeader = arangodb::fuerte::MessageHeader;
using Response = arangodb::fuerte::Response;
using RestVerb = arangodb::fuerte::RestVerb;

namespace fu = ::arangodb::fuerte;

static void usage(char const* name) {
  // clang-format off
  std::cout << "Usage: " << name << " [OPTIONS] server-url" << "\n\n"
            << "OPTIONS:\n"
            << "  --host tcp://127.0.0.1:8529\n"
            << "  --path /_api/version\n"
            << "  --method GET|PUT|...\n"
            << "  --parameter force true\n"
            << "  --meta x-arango-async store\n"
            << " example: fuerte-get --host vst://127.0.0.1:8530 --path /_api/document/profiles/P45476 --method GET"
            << std::endl;
  // clang-format on
}

static bool isOption(char const* arg, char const* expected) {
  return (strcmp(arg, expected) == 0);
}

static std::string parseString(int argc, char* argv[], int& i) {
  ++i;

  if (i >= argc) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  return argv[i];
}

static RestVerb parseMethod(int argc, char* argv[], int& i) {
  return arangodb::fuerte::to_RestVerb(parseString(argc, argv, i));
}

int main(int argc, char* argv[]) {
  RestVerb method = RestVerb::Get;
  std::string host = "http://127.0.0.1:8529";
  std::string path = "/_api/version";
  std::string user = "";
  std::string password = "";
  arangodb::fuerte::StringMap meta;
  arangodb::fuerte::StringMap parameter;

//#warning TODO: add database flag

  bool allowFlags = true;
  int i = 1;

  try {
    while (i < argc) {
      char const* p = argv[i];

      if (allowFlags && isOption(p, "--help")) {
        usage(argv[0]);
        return EXIT_SUCCESS;
      } else if (allowFlags && (isOption(p, "-X") || isOption(p, "--method"))) {
        method = parseMethod(argc, argv, i);

      } else if (allowFlags && (isOption(p, "-H") || isOption(p, "--host"))) {
        host = parseString(argc, argv, i);
      } else if (allowFlags && (isOption(p, "-p") || isOption(p, "--path"))) {
        path = parseString(argc, argv, i);
      } else if (allowFlags &&
                 (isOption(p, "-P") || isOption(p, "--parameter"))) {
        std::string key = parseString(argc, argv, i);
        std::string value = parseString(argc, argv, i);

        parameter[key] = value;
      } else if (allowFlags && (isOption(p, "-M") || isOption(p, "--meta"))) {
        std::string key = parseString(argc, argv, i);
        std::string value = parseString(argc, argv, i);

        meta[key] = value;
      } else if (allowFlags && isOption(p, "--")) {
        allowFlags = false;
      } else {
        usage(argv[0]);
        return EXIT_FAILURE;
      }
      ++i;
    }
  } catch (std::exception const& ex) {
    std::cerr << "cannot parse arguments: " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  ConnectionBuilder builder;

  try {
    builder.host(host);
  } catch (std::exception const& ex) {
    std::cerr << "cannot understand server-url: " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  builder.user(user);
  builder.password(password);

  EventLoopService eventLoopService;
  auto connection = builder.connect(eventLoopService);

  auto cb = [](uint32_t err, std::unique_ptr<Request> request, std::unique_ptr<Response> response) {
    if (err) {
      std::cout << "--------------------------------------------------------------------------" << std::endl;
      std::cout << "received error: " << err << std::endl
                << to_string(request->header)
                << "request payload:"
                << (response ? fu::to_string(*response) : "no response")
                << std::endl;
    } else {
      std::cout << "--------------------------------------------------------------------------" << std::endl;
      std::cout << "received result:\n"
                << (response ? fu::to_string(*response) : "no response")
                << std::endl;
    }
  };

  arangodb::fuerte::VBuilder vbuilder;
  vbuilder.openObject();
  vbuilder.add("name",arangodb::velocypack::Value("superdb"));
  vbuilder.close();

  for (size_t i = 0; i < 1; ++i) {
    try {
      auto request = arangodb::fuerte::createRequest(method, path, parameter, vbuilder.slice());
      std::cout << "Sending Request (messageid will be replaced)"
                << fu::to_string(*request) << std::endl;

      auto id = connection->sendRequest(std::move(request), cb);
      std::cout << "Request was assigned ID: " << id << std::endl;
    } catch (std::exception const& ex) {
      std::cerr << "exception: " << ex.what() << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  /*try {
    arangodb::fuerte::run();
  } catch (std::exception const& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
    exit(EXIT_FAILURE);
  }*/

  return EXIT_SUCCESS;
}
