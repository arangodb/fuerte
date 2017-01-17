#include <fuerte/types.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

VpackInit::VpackInit() : _translator(new arangodb::velocypack::AttributeTranslator){
    _translator->add("_key", uint8_t(1));
    _translator->add("_rev", uint8_t(2));
    _translator->add("_id", uint8_t(3));
    _translator->add("_from", uint8_t(4));
    _translator->add("_to", uint8_t(5));
    _translator->seal();
    arangodb::velocypack::Options::Defaults.attributeTranslator = _translator.get();
}

RestVerb to_RestVerb(std::string& val) {
  std::transform(val.begin(), val.end(), val.begin(), ::tolower );
  auto p = val.c_str();

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

ContentType to_ContentType(std::string const& val) {
  auto p = val.c_str();

  if (strcasecmp(p, "") == 0) {
    return ContentType::Unset;
  }

  if (val.find("application/json") != std::string::npos) {
    return ContentType::Json;
  }

  //TODO add missing!!!!


  return ContentType::Custom;
}


std::string to_string(ContentType type) {
  switch (type) {
    case ContentType::Unset:
      return "unset";

    case ContentType::Custom:
      return "custom";

    case ContentType::VPack:
      return "vpack";

    case ContentType::Dump:
      return "dump";

    case ContentType::Json:
      return "json";

    case ContentType::Html:
      return "html";

    case ContentType::Text:
      return "text";
  }

  return "undefined";
}


}}}
