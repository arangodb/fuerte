////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2017 ArangoDB GmbH, Cologne, Germany
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
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include <stdlib.h>

#include <boost/algorithm/string.hpp>

#include <fuerte/fuerte.h>

namespace f = ::arangodb::fuerte;

// setupAuthenticationFromEnv configures the given connection builder 
// with authentication settings specified in the environment.
inline void setupAuthenticationFromEnv(f::ConnectionBuilder& cbuilder) {
  auto auth = getenv("TEST_AUTHENTICATION");
  if (!auth) {
    // No authentication settings in environment, we're done.
    return;
  }
  std::vector<std::string> parts;
  boost::split(parts, auth, boost::is_any_of(":"));
  if (parts[0] == "basic") {
		if (parts.size() != 3) {
      throw std::invalid_argument("Expected username & password for basic authentication");
		}
    cbuilder.authenticationType(f::AuthenticationType::Basic);
    cbuilder.user(parts[1]);
    cbuilder.password(parts[2]);
  } else if (parts[0] == "jwt") {
		if (parts.size() != 3) {
			throw std::invalid_argument("Expected username & password for jwt authentication");
		}
    cbuilder.authenticationType(f::AuthenticationType::Jwt);
    cbuilder.user(parts[1]);
    cbuilder.password(parts[2]);
  } else {
    throw std::invalid_argument("Unknown authentication: " + parts[0]);
  }
}
