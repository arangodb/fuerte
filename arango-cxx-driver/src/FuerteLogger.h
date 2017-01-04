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
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGO_CXX_DRIVER_FUERTE_LOGGER
#define ARANGO_CXX_DRIVER_FUERTE_LOGGER 1

#define ENABLE_FUERTE_LOG_ERROR 1
#define ENABLE_FUERTE_LOG_DEBUG 1
#define ENABLE_FUERTE_LOG_TRACE 1

#if defined(ENABLE_FUERTE_LOG_TRACE) || defined(ENABLE_FUERTE_LOG_DEBUG) || defined(ENABLE_FUERTE_LOG_ERROR) 
#include <iostream>
#endif

#ifdef ENABLE_FUERTE_LOG_ERROR
#define FUERTE_LOG_ERROR std::cout
#else
#define FUERTE_LOG_ERROR if (0) std::cout
#endif

#ifdef ENABLE_FUERTE_LOG_DEBUG
#define FUERTE_LOG_DEBUG std::cout
#else
#define FUERTE_LOG_DEBUG if (0) std::cout
#endif

#ifdef ENABLE_FUERTE_LOG_TRACE
#define FUERTE_LOG_TRACE std::cout
#else
#define FUERTE_LOG_TRACE if (0) std::cout
#endif

#endif

