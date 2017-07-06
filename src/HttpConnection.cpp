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
/// @author Jan Uhde
/// @author John Bufton
/// @author Ewout Prangsma
////////////////////////////////////////////////////////////////////////////////

#include "HttpConnection.h"
#include <fcntl.h>
#include <velocypack/Parser.h>
#include <cassert>
#include <sstream>
#include <atomic>
#include <cassert>

#include <fuerte/helper.h>
#include <fuerte/types.h>
#include <fuerte/loop.h>

namespace arangodb {
namespace fuerte {
inline namespace v1 {
namespace http {

using namespace arangodb::fuerte::detail;

HttpConnection::HttpConnection(EventLoopService& eventLoopService, ConnectionConfiguration const& configuration)
    : Connection(eventLoopService, configuration) {
  _curlm.reset(new CurlMultiAsio(
      *eventLoopService.io_service(), 
        boost::bind(&HttpConnection::handleResult, this, _1, _2)));
}

HttpConnection::~HttpConnection() { 
  FUERTE_LOG_HTTPTRACE << "Destroying HttpConnection" << std::endl;
  if (!_messageStore.empty()){
    FUERTE_LOG_HTTPTRACE << "DESTROYING CONNECTION WITH: "
                         << _messageStore.size()
                         << " outstanding requests!"
                         << std::endl;
  }
  _messageStore.cancelAll();
  _curlm.reset();
}

MessageID HttpConnection::sendRequest(std::unique_ptr<Request> request, RequestCallback callback) {
  std::string dbString = (request->header.database) ? std::string("/_db/") + request->header.database.get() : std::string("");
  Destination destination = (_configuration._ssl ? "https://" : "http://")
                          + _configuration._host
                          + ":"
                          + _configuration._port
                          + dbString
                          + request->header.path.get();

  auto const& parameters = request->header.parameters;

  if (parameters && !parameters.get().empty()) {
    std::string sep = "?";

    for (auto p : parameters.get()) {
      destination += sep + urlEncode(p.first) + "=" + urlEncode(p.second);
      sep = "&";
    }
  }
  return queueRequest(destination, std::move(request), callback);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

uint64_t HttpConnection::queueRequest(Destination destination,
                                    std::unique_ptr<Request> request,
                                    RequestCallback callback) {
  FUERTE_LOG_HTTPTRACE << "queueRequest - start - at address: " << request.get() << std::endl;
  static std::atomic<uint64_t> ticketId(0);

  // Prepare a new request
  auto id = ++ticketId;
  request->messageID = id;
  createRequestItem(destination, std::move(request), callback);

  return id;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

int HttpConnection::curlDebug(CURL* handle, curl_infotype type, char* data,
                                size_t size, void* userptr) {
  RequestItem* inprogress = nullptr;
  curl_easy_getinfo(handle, CURLINFO_PRIVATE, &inprogress);

  std::string dataStr(data, size);
  std::string prefix("Communicator(" +
                     std::to_string(inprogress->_request->messageID) + "): ");

  switch (type) {
    case CURLINFO_TEXT:
      FUERTE_LOG_HTTPTRACE << prefix << "Text: " << dataStr;
      break;

    case CURLINFO_HEADER_OUT:
      logHttpHeaders(prefix + "Header >>", dataStr);
      break;

    case CURLINFO_HEADER_IN:
      logHttpHeaders(prefix + "Header <<", dataStr);
      break;

    case CURLINFO_DATA_OUT:
    case CURLINFO_SSL_DATA_OUT:
      logHttpBody(prefix + "Body >>", dataStr);
      break;

    case CURLINFO_DATA_IN:
    case CURLINFO_SSL_DATA_IN:
      logHttpBody(prefix + "Body <<", dataStr);
      break;

    case CURLINFO_END:
      break;
  }

  return 0;
}

void HttpConnection::logHttpBody(std::string const& prefix,
                                   std::string const& data) {
  std::string::size_type n = 0;

  while (n < data.length()) {
    FUERTE_LOG_HTTPTRACE << prefix << " " << data.substr(n, 80) << "\n";
    n += 80;
  }
}

void HttpConnection::logHttpHeaders(std::string const& prefix,
                                      std::string const& headerData) {
  std::string::size_type last = 0;
  std::string::size_type n;

  while (true) {
    n = headerData.find("\r\n", last);

    if (n == std::string::npos) {
      break;
    }

    FUERTE_LOG_HTTPTRACE << prefix << " "
                         << headerData.substr(last, n - last)
                         << "\n";
    last = n + 2;
  }
}

// stores headers in _responseHeaders (lowercase)
size_t HttpConnection::readHeaders(char* buffer, size_t size, size_t nitems,
                                     void* userptr) {
  size_t realsize = size * nitems;
  RequestItem* rip = (struct RequestItem*)userptr;

  std::string const header(buffer, realsize);
  size_t pivot = header.find_first_of(':');

  if (pivot != std::string::npos) {
    std::string key(header.c_str(), pivot);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    rip->_responseHeaders.emplace(
        key, header.substr(pivot + 2, header.length() - pivot - 4));
  }
  return realsize;
}

size_t HttpConnection::readBody(void* data, size_t size, size_t nitems,
                                  void* userp) {
  size_t realsize = size * nitems;

  RequestItem* rip = (struct RequestItem*)userp;

  try {
    rip->_responseBody.append((char*)data, realsize);
    return realsize;
  } catch (std::bad_alloc&) {
    return 0;
  }
}

void HttpConnection::transformResult(CURL* handle, StringMap&& responseHeaders,
                                       std::string const& responseBody,
                                       Response* response) {
#if  ENABLE_FUERTE_LOG_HTTPTRACE > 0
  std::cout << "header START" << std::endl;
  for(auto& p : responseHeaders){
    std::cout << p.first << "  " <<p.second << std::endl;
  }
  std::cout << "header END" << std::endl;
#endif

  // no available - response->header.requestType
  auto const& ctype = responseHeaders[fu_content_type_key];
  response->header.contentType(ctype);

  if (responseBody.length()) {
      VBuffer buffer;
      buffer.append(responseBody);
      response->setPayload(std::move(buffer), 0);
  }
  response->header.meta = std::move(responseHeaders);

}

std::string HttpConnection::createSafeDottedCurlUrl(
    std::string const& originalUrl) {
  std::string url;
  url.reserve(originalUrl.length());

  size_t length = originalUrl.length();
  size_t currentFind = 0;
  std::size_t found;
  std::vector<char> urlDotSeparators{'/', '#', '?'};

  while ((found = originalUrl.find("/.", currentFind)) != std::string::npos) {
    if (found + 2 == length) {
      url += originalUrl.substr(currentFind, found - currentFind) + "/%2E";
    } else if (std::find(urlDotSeparators.begin(), urlDotSeparators.end(),
                         originalUrl.at(found + 2)) != urlDotSeparators.end()) {
      url += originalUrl.substr(currentFind, found - currentFind) + "/%2E";
    } else {
      url += originalUrl.substr(currentFind, found - currentFind) + "/.";
    }
    currentFind = found + 2;
  }
  url += originalUrl.substr(currentFind);
  return url;
}

void HttpConnection::createRequestItem(const Destination& destination, std::unique_ptr<Request> request, RequestCallback callback) {
  // mop: the curl handle will be managed safely via unique_ptr and hold
  // ownership for rip
  auto requestItem = std::make_shared<RequestItem>(destination, std::move(request), callback);
  auto handle = requestItem->handle();
  struct curl_slist* requestHeaders = nullptr;
  auto fuRequest = requestItem->_request.get();
  if (fuRequest->header.meta) {
    for (auto const& header : fuRequest->header.meta.get()) {
      std::string thisHeader(header.first + ": " + header.second);
      requestHeaders = curl_slist_append(requestHeaders, thisHeader.c_str());
    }
  }

  std::string url = createSafeDottedCurlUrl(requestItem->_destination);
  requestItem->_requestHeaders = requestHeaders;
  curl_easy_setopt(handle, CURLOPT_HTTPHEADER, requestHeaders);
  curl_easy_setopt(handle, CURLOPT_HEADER, 0L);
  curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, HttpConnection::readBody);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, requestItem.get());
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, &HttpConnection::readHeaders);
  curl_easy_setopt(handle, CURLOPT_HEADERDATA, requestItem.get());
#if ENABLE_FUERTE_LOG_HTTPRACE > 0
  curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION, HttpConnection::curlDebug);
  curl_easy_setopt(handle, CURLOPT_DEBUGDATA, requestItem.get());
  curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
#endif
  curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, requestItem->_errorBuffer);

  // mop: XXX :S CURLE 51 and 60...
  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);

  auto connectTimeout = static_cast<long>(requestItem->_options.connectionTimeout);

  // mop: although curl is offering a MS scale connecttimeout this gets ignored
  // in at least 7.50.3; in doubt change the timeout to _MS below and hardcode
  // it to 999 and see if the requests immediately fail if not this hack can go
  // away

  if (connectTimeout <= 0) {
    connectTimeout = 1;
  }

  auto reqTimeout = std::chrono::duration_cast<std::chrono::milliseconds>(requestItem->_request->timeout());
  curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, reqTimeout.count());
  curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, connectTimeout);

  auto verb = fuRequest->header.restVerb.get();

  switch (verb) {
    case RestVerb::Post:
      curl_easy_setopt(handle, CURLOPT_POST, 1);
      break;

    case RestVerb::Put:
      // mop: apparently CURLOPT_PUT implies more stuff in curl
      // (for example it adds an expect 100 header)
      // this is not what we want so we make it a custom request
      curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "PUT");
      break;

    case RestVerb::Delete:
      curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
      break;

    case RestVerb::Head:
      curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "HEAD");
      break;

    case RestVerb::Patch:
      curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "PATCH");
      break;

    case RestVerb::Options:
      curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");
      break;

    case RestVerb::Get:
      break;

    case RestVerb::Illegal:
      throw std::runtime_error("Invalid request type " + to_string(verb));
      break;
  }

  // Setup authentication 
  switch (_configuration._authenticationType) {
    case AuthenticationType::None:
      // Do nothing
      break;
    case AuthenticationType::Basic:
      curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
      curl_easy_setopt(handle, CURLOPT_USERNAME, _configuration._user.c_str());
      curl_easy_setopt(handle, CURLOPT_PASSWORD, _configuration._password.c_str());
      break;
    case AuthenticationType::Jwt:
      throw std::invalid_argument("Jwt authentication is not yet support");
      break;
    default:
      throw std::runtime_error("Invalid authentication type " + to_string(_configuration._authenticationType));
      break;
  }

  std::string empty("");
  std::string& body = empty;

  auto pay = fuRequest->payload();
  auto paySize = boost::asio::buffer_size(pay);

  if (paySize > 0) {
    // https://curl.haxx.se/libcurl/c/CURLOPT_COPYPOSTFIELDS.html
    // DO NOT CHANGE BODY SIZE LATER!!
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, paySize);
    curl_easy_setopt(handle, CURLOPT_COPYPOSTFIELDS, boost::asio::buffer_cast<const char*>(pay));
  }

  requestItem->_startTime = std::chrono::steady_clock::now();
  _messageStore.add(std::move(requestItem));

  _curlm->addRequest(handle);
}

// CURL request is DONE, handle the results.
void HttpConnection::handleResult(CURL* handle, CURLcode rc) {
  RequestItem* requestItem = nullptr;
  curl_easy_getinfo(handle, CURLINFO_PRIVATE, &requestItem);
  if (requestItem == nullptr) {
    return;
  }

  std::string prefix("Communicator(" + std::to_string(requestItem->_request->messageID) +
                     "): ");

  FUERTE_LOG_DEBUG << prefix << "curl rc is : " << rc << " after "
                   << std::chrono::duration_cast<std::chrono::duration<double>>(
                          std::chrono::steady_clock::now() - requestItem->_startTime)
                          .count()
                   << " s\n";

  if (strlen(requestItem->_errorBuffer) != 0) {
    FUERTE_LOG_HTTPTRACE << prefix << "curl error details: " << requestItem->_errorBuffer
                     << "\n";
  }

  auto request_id = requestItem->_request->messageID;
  try {
    switch (rc) {
      case CURLE_OK: {
        long httpStatusCode = 200;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &httpStatusCode);

        std::unique_ptr<Response> fuResponse(new Response());
        fuResponse->header.responseCode = static_cast<unsigned>(httpStatusCode);
        fuResponse->messageID = requestItem->_request->messageID;
        transformResult(handle, std::move(requestItem->_responseHeaders),
                        std::move(requestItem->_responseBody),
                        dynamic_cast<Response*>(fuResponse.get()));

        auto response_id = fuResponse->messageID;
#if ENABLE_FUERTE_LOG_HTTPTRACE > 0
        std::cout << "CALLING ON SUCESS CALLBACK IN HTTP COMMUNICATOR" << std::endl;
        std::cout << "request id: " << request_id << std::endl;
        std::cout << "response id: " << response_id << std::endl;
        std::cout << "in progress: " << _messageStore.keys() << std::endl;
        //std::cout << std::endl << to_string(*rip->_request);
        //std::cout << to_string(*fuResponse);
#endif
        requestItem->_callback.invoke(0, std::move(requestItem->_request), std::move(fuResponse));
        requestItem->_request = nullptr;
        break;
      }

      case CURLE_COULDNT_CONNECT:
        onFailure(static_cast<Error>(ErrorCondition::CouldNotConnect), "connect failed");
        requestItem->_callback.invoke(static_cast<Error>(ErrorCondition::CouldNotConnect), std::move(requestItem->_request), {nullptr});
        break;
      case CURLE_SSL_CONNECT_ERROR:
        onFailure(static_cast<Error>(ErrorCondition::CouldNotConnect), "ssl connect failed");
        requestItem->_callback.invoke(static_cast<Error>(ErrorCondition::CouldNotConnect), std::move(requestItem->_request), {nullptr});
        break;
      case CURLE_COULDNT_RESOLVE_HOST:
        onFailure(static_cast<Error>(ErrorCondition::CouldNotConnect), "resolve host failed");
        requestItem->_callback.invoke(static_cast<Error>(ErrorCondition::CouldNotConnect), std::move(requestItem->_request), {nullptr});
        break;
      case CURLE_URL_MALFORMAT:
        requestItem->_callback.invoke(static_cast<Error>(ErrorCondition::MalformedURL), std::move(requestItem->_request), {nullptr});
        break;
      case CURLE_SEND_ERROR:
        requestItem->_callback.invoke(static_cast<Error>(ErrorCondition::CouldNotConnect), std::move(requestItem->_request), {nullptr});
        break;

      case CURLE_OPERATION_TIMEDOUT:
      case CURLE_RECV_ERROR:
      case CURLE_GOT_NOTHING:
        requestItem->_callback.invoke(static_cast<Error>(ErrorCondition::Timeout), std::move(requestItem->_request), {nullptr});
        break;

      default:
        FUERTE_LOG_ERROR << "Curl return " << rc << "\n";
        onFailure(static_cast<Error>(ErrorCondition::CurlError), "unknown curl error");
        requestItem->_callback.invoke(static_cast<Error>(ErrorCondition::CurlError), std::move(requestItem->_request), {nullptr});
        break;
    }
  } catch (std::exception const& e) {
    _messageStore.removeByID(request_id);
    throw e;
  }  catch (...) {
    _messageStore.removeByID(request_id);
    throw;
  }

  _messageStore.removeByID(request_id);
}

}
}
}
}
