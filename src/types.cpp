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
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////
#include <fuerte/types.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

RestVerb to_RestVerb(std::string const& value) {
  std::string lowercase;
  lowercase.reserve(value.size());
  std::transform(value.begin(), value.end()
                ,std::back_inserter(lowercase), ::tolower
                );

  auto p = lowercase.c_str();

  if (strcasecmp(p, "delete") == 0) {
    return RestVerb::Delete;
  }

  if (strcasecmp(p, "get") == 0) {
    return RestVerb::Get;
  }

  if (strcasecmp(p, "post") == 0) {
    return RestVerb::Post;
  }

  if (strcasecmp(p, "put") == 0) {
    return RestVerb::Put;
  }

  if (strcasecmp(p, "head") == 0) {
    return RestVerb::Head;
  }

  if (strcasecmp(p, "patch") == 0) {
    return RestVerb::Patch;
  }

  if (strcasecmp(p, "options") == 0) {
    return RestVerb::Options;
  }

  return RestVerb::Illegal;
}

std::string to_string(RestVerb type) {
  switch (type) {
    case RestVerb::Illegal:
      return "illegal";

    case RestVerb::Delete:
      return "delete";

    case RestVerb::Get:
      return "get";

    case RestVerb::Post:
      return "post";

    case RestVerb::Put:
      return "put";

    case RestVerb::Head:
      return "head";

    case RestVerb::Patch:
      return "patch";

    case RestVerb::Options:
      return "options";
  }

  return "undefined";
}

std::string to_string(MessageType type) {
  switch (type) {
    case MessageType::Undefined:
      return "undefined";

    case MessageType::Request:
      return "request";

    case MessageType::Response:
      return "response";

    case MessageType::ResponseUnfinished: //needed for vst
      return "unfinised response";

    case MessageType::Authentication:
      return "authentication";
  }

  return "undefined";
}

std::string to_string(TransportType type) {
  switch (type) {
    case TransportType::Undefined:
      return "undefined";

    case TransportType::Http:
      return "http";

    case TransportType::Vst:
      return "vst";
  }

  return "undefined";
}


const std::string fu_content_type_unset("unset");
const std::string fu_content_type_vpack("application/x-velocypack");
const std::string fu_content_type_json("application/json");
const std::string fu_content_type_html("text/html");
const std::string fu_content_type_text("text/plain");
const std::string fu_content_type_dump("application/x-arango-dump");

ContentType to_ContentType(std::string const& val) {
  auto p = val.c_str();

  if (strcasecmp(p, "") == 0) {
    return ContentType::Unset;
  }
  if (val.find(fu_content_type_unset) != std::string::npos) {
    return ContentType::Unset;
  }

  if (val.find(fu_content_type_vpack) != std::string::npos) {
    return ContentType::VPack;
  }

  if (val.find(fu_content_type_json) != std::string::npos) {
    return ContentType::Json;
  }

  if (val.find(fu_content_type_html) != std::string::npos) {
    return ContentType::Html;
  }

  if (val.find(fu_content_type_text) != std::string::npos) {
    return ContentType::Text;
  }

  if (val.find(fu_content_type_dump) != std::string::npos) {
    return ContentType::Dump;
  }

  return ContentType::Custom;
}


std::string to_string(ContentType type) {
  switch (type) {
    case ContentType::Unset:
      return fu_content_type_unset;

    case ContentType::VPack:
      return fu_content_type_vpack;

    case ContentType::Json:
      return fu_content_type_json;

    case ContentType::Html:
      return fu_content_type_html;

    case ContentType::Text:
      return fu_content_type_text;

    case ContentType::Dump:
      return fu_content_type_dump;

    case ContentType::Custom:
      throw std::logic_error("custom content type could take different "
                             "values and is therefor not convertible to string");
  }

  throw std::logic_error("unknown content type");
}

std::string to_string(AuthenticationType type) {
  switch (type) {
    case AuthenticationType::None:
      return "none";
    case AuthenticationType::Basic:
      return "basic";
    case AuthenticationType::Jwt:
      return "jwt";
  }
  return "unknown";
}


ErrorCondition intToError(Error integral){
  static const std::vector<Error> valid = {
      0,    // NoError
      //1,  // ErrorCastError
      1000, // ConnectionError
      1001, // CouldNotConnect
      1002, // TimeOut
      1102, // VstReadError
      1103, // VstWriteError
      1104, // CancelledDuringReset
      1105, // MalformedURL
      3000, // CurlError
  };
  auto pos = std::find(valid.begin(), valid.end(), integral);
  if(pos != valid.end()){
    return static_cast<ErrorCondition>(integral);
  }
#ifdef FUERTE_DEVBUILD
  throw std::logic_error(
    std::string("Error: casting int to ErrorCondition: ")
               + std::to_string(integral)
  );
#endif
  return ErrorCondition::ErrorCastError;
}

std::string to_string(ErrorCondition error){
  switch(error){
    case ErrorCondition::NoError:
      return "No Error";
    case ErrorCondition::ErrorCastError:
      return "Error: casting int to ErrorCondition";

    case ErrorCondition::ConnectionError:
      return "Error: in connection";
    case ErrorCondition::CouldNotConnect:
      return "Error: unable to connect";
    case ErrorCondition::Timeout:
      return "Error: timeout";
    case ErrorCondition::VstReadError:
      return "Error: reading vst";
    case ErrorCondition::VstWriteError:
      return "Error: writing vst";
    case ErrorCondition::CanceledDuringReset:
      return "Error: cancel as result of other error";
    case ErrorCondition::MalformedURL:
      return "Error: malformed URL";

    case ErrorCondition::CurlError:
      return "Error: in curl";

  }
}


}}}
